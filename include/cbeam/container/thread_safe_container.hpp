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

#include <cbeam/error/runtime_error.hpp>     // for cbeam::error:runtime_error
#include <cbeam/lifecycle/scoped_set.hpp> // for cbeam::lifecycle::scoped_set

#include <cassert> // for assert
#include <cstddef> // for std::size_t

#include <atomic>    // for std::atomic
#include <mutex>     // for std::recursive_mutex, std::lock_guard
#include <string>    // for std::string, std::allocator

namespace cbeam::container
{
    /// \brief Base class for thread safe containers
    ///
    /// \tparam T Type of the container
    template <typename T>
    class thread_safe_container
    {
    public:
        /// \brief Inner class providing RAII-style locking mechanism.
        ///
        /// This class implements a lock guard for thread-safe operations,
        /// utilizing a recursive mutex. It locks the mutex upon construction
        /// and unlocks it upon destruction, ensuring exception safety and
        /// preventing deadlocks in a scoped block.
        class lock_guard
        {
            std::recursive_mutex& _mutex;

        public:
            /// \brief Constructs the lock guard and locks the provided mutex.
            ///
            /// \param mutex The mutex to be locked.
            explicit lock_guard(std::recursive_mutex& mutex)
                : _mutex(mutex)
            {
                _mutex.lock();
            }

            /// \brief Destructor that unlocks the mutex.
            ~lock_guard() noexcept
            {
                _mutex.unlock();
            }

            // Deleted copy constructor and assignment operator to prevent copying.
            lock_guard(const lock_guard&)            = delete;
            lock_guard& operator=(const lock_guard&) = delete;
        };

        /// \brief Acquires a lock guard for the set, ensuring thread safety.
        ///
        /// This method provides a mechanism to acquire a lock on the set's mutex,
        /// ensuring that no other thread can modify the set while the lock is held.
        /// The returned lock guard object will release the lock when it goes out of scope.
        ///
        /// \return A lock_guard object that manages the lock on the set's mutex.
        ///
        /// \throw cbeam::error::runtime_error if the set is being modified during the call.
        lock_guard get_lock_guard() const
        {
            if (_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_container::get_lock_guard: not allowed during destruction of the container.");
            }
            return lock_guard(_mutex);
        }

        /// \brief Default constructor
        thread_safe_container() = default;

        virtual ~thread_safe_container() noexcept
        {
            assert(!_is_being_modified && "cbeam::container::thread_safe_container: destruction during modification or duplicate destruction");
            _is_being_modified = true;
        }

        /// \brief Checks if the container is empty
        ///
        /// \return true if the container is empty, false otherwise
        bool empty() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            return _container.empty();
        }

        /// \brief Clears the contents
        void clear()
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_container::clear: not allowed during modification of the container.");
            }
            lifecycle::scoped_set modifying(_is_being_modified, true);
            _container.clear();
        }

        /// \brief Returns the number of elements
        ///
        /// \return The number of elements in the container
        std::size_t size() const
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            return _container.size();
        }

        /// \brief Returns an iterator to the beginning
        auto begin()
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            return _container.begin();
        }

        /// \brief Returns an iterator to the end
        auto end()
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            return _container.end();
        }

        bool is_being_modified() const
        {
            return _is_being_modified;
        }

    protected:
        mutable std::recursive_mutex _mutex;     ///< Mutex to protect access to _container
        T                            _container; ///< Internal non-thread safe container
        std::atomic<bool>            _is_being_modified{false};
    };
}
