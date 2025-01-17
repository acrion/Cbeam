/*
Copyright (c) 2025 acrion innovations GmbH
Authors: Stefan Zipproth, s.zipproth@acrion.ch

This file is part of Cbeam, see https://github.com/acrion/cbeam and https://cbeam.org

Cbeam is offered under a commercial and under the AGPL license.
For commercial licensing, contact us at https://acrion.ch/sales. For AGPL licensing, see below.

AGPL licensing:

Cbeam is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Cbeam is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with Cbeam. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <cbeam/logging/log_manager.hpp>

#include <atomic>             // for std::atomic
#include <condition_variable> // for std::condition_variable
#include <functional>
#include <memory> // for std::unique_ptr, std::make_unique
#include <mutex>  // for std::mutex, std::lock_guard, std::unique_lock
#include <set>    // for std::set, std::operator!=, std::_Rb_tree_const_iterator
#include <string>
#include <thread>  // for std::thread
#include <utility> // for std::forward

namespace cbeam::concurrency
{
    /**
     *  @brief Base class for creating and managing a thread.
     *  This class follows the Curiously Recurring Template Pattern (CRTP)
     *  to allow derived classes to implement their specific worker logic.
     *  @details The first argument of the inherited class’ constructor needs to be the private threaded_object::construction_token
     *  to prevent construction without threaded_object::create.
     *  @example cbeam/concurrency/threaded_object.hpp
     *  @code{.cpp}
     *     class MyThreadedObject : threaded_object<MyThreadedObject>
     *     {
     *     public:
     *         MyThreadedObject(typename cbeam::concurrency::threaded_object<MyThreadedObject>::construction_token,
     *                          const int demo_parameter)
     *             : threaded_object<MyThreadedObject>{_mtx, _cv}
     *         {
     *         }
     *         void worker()
     *         {
     *             while (this->is_running())
     *             {
     *                 {
     *                     std::unique_lock<std::mutex> lock(_mtx);
     *
     *                     // We may have missed a notification during previous work
     *                     // outside this lock, so check the `ready' shared variable
     *                     // and the running state before waiting for a notification.
     *                     if (!ready && this->is_running())
     *                     {
     *                         // Wait for other thread to notify us
     *                         // via _cv->notify_one() or _cv->notify_all().
     *                         _cv->wait(lock, [&]
     *                         {
     *                             return ready || !this->is_running();
     *                         });
     *                     }
     *                 }
     *                 // Either
     *                 // (1) ready has been set to true by another thread
     *                 //     (==> do some work here), or
     *                 // (2) _running has been set to false by the base class,
     *                 //     which notified us.
     *            }
     *        }
     *     private:
     *         std::mutex _mtx;
     *         std::condition_variable _cv;
     *         bool ready{false}; // sample shared variable
     *     }
     *  @endcode
     *  See https://en.cppreference.com/w/cpp/thread/condition_variable
     *  The method `worker` needs to be public to enable creation via static method threaded_object::create().
     *  @tparam Derived The derived class which implements the worker method.
     */
    template <typename Derived, typename MessageDataType>
    class threaded_object
    {
    public:
        threaded_object() = default;
        /**
         * @brief Destructor.
         * Safely shuts down the managed thread by setting _running to false,
         * notifying the condition variable,
         * and then joining the thread.Catches and logs any std::system_error thrown by thread::join.
         * Asserts in debug mode if such an exception is caught,
         * as it indicates a serious bug that must be addressed.
         */
        virtual ~threaded_object() noexcept
        {
            {
                std::unique_lock<std::mutex> lock(*_mtx);
                _running = false;
            }
            _cv->notify_all();

            if (_t.joinable())
            {
                try
                {
                    _t.join();
                }
                catch (const std::system_error& ex)
                {
                    CBEAM_LOG(std::string{"cbeam::concurrency::threaded_object::~threaded_object: "} + ex.what());
                    assert(false);
                }
            }
        }

        /**
         * @brief Factory method to create and start a thread for the derived class.
         *
         * This method creates an instance of the derived class and starts its thread.
         * The worker method of the derived class is used as the thread function.
         *
         * @details The constructor of the Derived class must ensure that the object is fully initialized
         * and ready for the worker to be executed in a separate thread.
         *
         * @example cbeam/concurrency/threaded_object.hpp
         * @code{.cpp}
         * auto myObject = cbeam::concurrency::threaded_object<MyThreadedObject>::create(42);
         * @endcode
         *
         * @tparam Args Types of arguments to pass to the constructor of Derived.
         * @param args Arguments to pass to the constructor of Derived.
         * @return A unique_ptr to the newly created instance of Derived.
         */
        template <typename... Args>
        static std::unique_ptr<Derived> create(std::shared_ptr<std::mutex> mtx, std::shared_ptr<std::condition_variable> cv, Args&&... args)
        {
            std::unique_ptr<Derived> obj = std::make_unique<Derived>(construction_token{}, std::forward<Args>(args)...);
            obj->_mtx                    = mtx;
            obj->_cv                     = cv;
            obj->_t                      = std::thread(&threaded_object::worker, obj.get());
            return obj;
        }

    protected:
        /* @brief Construction token for controlled instantiation of derived classes.
         */
        class construction_token
        {
            friend class threaded_object<Derived, MessageDataType>;
            construction_token() {}
        };

        virtual void on_start()
        {
        }

        virtual bool is_message_available()
        {
            return false;
        }

        virtual MessageDataType get_message()
        {
            return {};
        }

        virtual void on_message(const MessageDataType& /*message_data*/) noexcept
        {
        }

        virtual void on_exit()
        {
        }

        std::shared_ptr<std::mutex>              _mtx;
        std::shared_ptr<std::condition_variable> _cv;

    private:
        void worker()
        {
            on_start();

            while (_running)
            {
                MessageDataType message_data;

                {
                    std::unique_lock<std::mutex> lock(*_mtx);

                    // We might have missed a notification during previous work outside this lock,
                    // so check if a message is available prior waiting for a notification.
                    if (!is_message_available() && _running)
                    {
                        // wait for other thread to notify us via _cv->notify_one() or _cv->notify_all()
                        _cv->wait(lock, [&]
                                  { return is_message_available() || !_running; });
                    }

                    if (_running)
                    {
                        message_data = get_message();
                    }
                }

                if (_running)
                {
                    on_message(message_data);
                }
            }

            on_exit();
        }

        std::thread       _t;             ///< Thread handler.
        std::atomic<bool> _running{true}; ///< Atomic flag that the worker() must check to detect if it should proceed or exit

        threaded_object(const threaded_object&) = delete;

        threaded_object& operator=(const threaded_object&) = delete;
    };
}
