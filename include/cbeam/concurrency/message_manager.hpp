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

#include <cbeam/concurrency/thread.hpp>
#include <cbeam/concurrency/threaded_object.hpp>
#include <cbeam/logging/log_manager.hpp>
#include <cbeam/platform/compiler_compatibility.hpp>
#include <cbeam/random/generators.hpp>

#include <atomic>             // for std::atomic
#include <condition_variable> // for std::condition_variable
#include <deque>
#include <functional>
#include <map>
#include <memory> // for std::unique_ptr, std::make_unique
#include <mutex>  // for std::mutex, std::lock_guard, std::unique_lock
#include <set>    // for std::set, std::operator!=, std::_Rb_tree_const_iterator
#include <thread> // for std::thread
#include <unordered_set>
#include <utility> // for std::forward

namespace cbeam::concurrency
{
    /**
     * @brief Manages message queues and handlers for asynchronous message processing.
     *
     * The `message_manager` class template is designed for efficient management of asynchronous message processing in a multithreaded environment.
     * It allows sending, receiving, and handling of messages of a specified type. The messages are processed by message handlers which are managed by this class.
     *
     * MessageDataType The type of message data that will be managed by this instance. For non-Plain Old Data (POD) types, unnecessary copying can be avoided
     * by encapsulating the message in a `std::shared_ptr`, thus enabling efficient handling of complex data structures.
     *
     * @example cbeam/concurrency/message_manager.hpp
     * @code{.cpp}
     * constexpr size_t check_prime = 1;
     * constexpr size_t count_prime = 2;
     *
     * cbeam::concurrency::message_manager<uint64_t> mm;
     *
     * // shared counter to track how many primes are found (our result)
     * std::atomic<size_t> prime_count{0};
     *
     * // handler that counts primes
     * mm.add_handler(count_prime, [&](uint64_t msg) { ++prime_count; });
     *
     * // Handler that checks primality, and if prime => sends a message to `count_prime`
     * // Note: You can add more than one handler for an ID (such as check_prime)
     * //       to distribute the CPU load across multiple cores.
     * mm.add_handler(check_prime, [&](uint64_t number_to_check)
     * {
     *     if (is_prime(number_to_check))
     *     {
     *         mm.send_message(count_prime, number_to_check);
     *     }
     * };
     *
     * // Send all numbers from 1 to 100001 to `check_prime`
     * for (uint64_t number_to_check = 1; number_to_check <= 100001; ++number_to_check)
     * {
     *     mm.send_message(check_prime, uint64_t{number_to_check});
     * }
     *
     * // Now wait for the check_prime queue to be empty
     * // (i.e. all primality checks have been popped and processed)
     * mm.wait_until_empty(check_prime);
     *
     * // Also wait until the count_prime queue is empty.
     * // This ensures that all prime-sending messages have been popped and processed.
     * mm.wait_until_empty(count_prime);
     *
     * // Dispose the handlers
     * mm.dispose(check_prime);
     * mm.dispose(count_prime);
     * @endcode
     */
    template <typename MessageDataType>
    class message_manager
    {
    public:
        using message_id_type = std::size_t;

        /**
         * @brief Typedef for a message logging function.
         *
         * Defines a function type used for logging messages. The function takes three parameters: the message ID, the message data of type `MessageDataType`,
         * and a boolean flag indicating whether the message is outgoing (true) or incoming (false). This allows for flexible integration of custom logging functionality within the message processing flow.
         */
        using message_logger_type = std::function<void(message_id_type, MessageDataType, bool)>;

        /**
         * @brief Enum class defining the order in which messages are processed.
         *
         * This enumeration specifies the order in which messages are handled by the message handlers. The available ordering types are:
         * - FIFO (First In First Out): Messages are processed in the order they are received. This is a traditional approach that ensures a predictable processing order.
         * - FILO (First In Last Out): The most recently received message is processed first. This approach can be useful for prioritizing newer messages.
         * - RANDOM: Messages are processed in a random order. In certain algorithmic contexts, such as fill algorithms for raster graphics, introducing randomness
         * can reduce the overall length of the message queue. This is due to the reduction of path dependencies and the increased entropy in message processing, which
         * avoids consistent biases towards older (FIFO) or newer (FILO) messages. However, for long message queues, the efficiency might decrease due to the overhead
         * of random access in a `std::deque`. Nevertheless, the RANDOM mode can be beneficial if it aligns with the algorithmic needs and the message queues remain short.
         */
        enum class order_type
        {
            FIFO,  ///< Process messages in the order they were received.
            FILO,  ///< Process the most recently received message first.
            RANDOM ///< Process messages in a random order, reducing path dependencies in certain algorithms but less efficient for long queues.
        };

    private:
        struct message_queue
        {
            std::deque<MessageDataType>              queue;
            std::shared_ptr<std::mutex>              queue_mutex{std::make_shared<std::mutex>()};
            std::shared_ptr<std::condition_variable> queue_cv{std::make_shared<std::condition_variable>()};
            std::condition_variable                  queue_cv_empty;
            std::atomic<std::size_t>                 busy_count{0};
            message_logger_type                      message_logger;
            std::mutex                               message_logger_mutex;
        };

        class message_handler : public threaded_object<message_handler, MessageDataType>
        {
        public:
            message_handler(typename threaded_object<message_handler, MessageDataType>::construction_token,
                            const std::shared_ptr<message_queue>&                       queue,
                            const message_id_type                                       message_id,
                            std::function<void(MessageDataType message_type)>           on_message,
                            const std::function<void(const std::exception& exception)>& on_exception,
                            const std::function<void()>&                                on_exit,
                            const std::string&                                          thread_name,
                            const order_type                                            order)
                : threaded_object<message_handler, MessageDataType>{}
                , _message_queue{queue}
                , _message_id{message_id}
                , _on_message{on_message}
                , _on_exception{on_exception}
                , _on_exit{on_exit}
                , _thread_name{thread_name}
                , _order{order} { CBEAM_LOG_DEBUG("cbeam::concurrency::message_manager: constructor for thread + '" + thread_name + "' received message queue " + cbeam::convert::to_string(&queue->queue) + " to message handler"); }

            ~message_handler() override = default;

            void on_start() override
            {
                set_thread_name(_thread_name.c_str());
                CBEAM_LOG_DEBUG("cbeam::concurrency::message_manager: Thread '" + _thread_name + "' now waits for messages in queue " + cbeam::convert::to_string(&_message_queue->queue));
            }

            bool is_message_available() override { return !_message_queue->queue.empty(); }

            MessageDataType get_message() override
            {
                MessageDataType message;

                switch (_order)
                {
                case order_type::FIFO:
                    message = _message_queue->queue.front();
                    _message_queue->queue.pop_front();
                    break;
                case order_type::FILO:
                    message = _message_queue->queue.back();
                    _message_queue->queue.pop_back();
                    break;
                case order_type::RANDOM:
                {
                    const size_t rnd = cbeam::random::random_number(_message_queue->queue.size());
                    message          = _message_queue->queue.at(rnd);
                    _message_queue->queue.erase(_message_queue->queue.begin() + rnd);
                    break;
                }
                }

                _message_queue->busy_count.fetch_add(1, std::memory_order_acq_rel);

                CBEAM_SUPPRESS_WARNINGS_PUSH() // suppress false positive "may be used uninitialized"
                return message;
                CBEAM_SUPPRESS_WARNINGS_POP()
            }

            void on_message(const MessageDataType& message_data) noexcept override
            {
                try
                {
                    {
                        std::lock_guard<std::mutex> lock_logger(_message_queue->message_logger_mutex);
                        if (_message_queue->message_logger) { _message_queue->message_logger(_message_id, message_data, false); }
                    }

                    _on_message(message_data);
                }
                catch (const std::exception& ex)
                {
                    _on_exception(ex);
                }

                // Decrement the busy_count now that we're done
                auto still_busy = _message_queue->busy_count.fetch_sub(1, std::memory_order_acq_rel) - 1;

                // We also need to ensure we hold the queue mutex to safely check queue->queue.empty().
                // Because multiple threads might pop from the queue concurrently.
                std::lock_guard<std::mutex> lock_queue_mutex(*_message_queue->queue_mutex);

                // If the queue is empty and no thread is busy => notify waiting threads
                if (_message_queue->queue.empty() && still_busy == 0)
                {
                    _message_queue->queue_cv_empty.notify_all();
                }
            }

            void on_exit() override { _on_exit(); }

        private:
            std::shared_ptr<message_queue>                       _message_queue;
            const message_id_type                                _message_id;
            std::function<void(MessageDataType message_data)>    _on_message;
            std::function<void(const std::exception& exception)> _on_exception;
            std::function<void()>                                _on_exit;
            std::string                                          _thread_name;
            order_type                                           _order;
        }; // class message_handler

        std::mutex                                                                      _threads_mutex;
        std::map<message_id_type, std::unordered_set<std::unique_ptr<message_handler>>> _threads;
        std::mutex                                                                      _message_queues_mutex;
        std::map<message_id_type, std::shared_ptr<message_queue>>                       _message_queues;

    public:
        /**
         * @brief Sends a message of specified ID and data.
         *
         * This method enqueues a message of a specified ID. The message is added to the queue corresponding to this ID and will be processed according to the queue's order type (FIFO, FILO, or RANDOM).
         * If a maximum queue size is specified and the queue is full, this method will block until there is space available in the queue. The `add_handler` method does not need to be called prior to this;
         * `message_id` can be any arbitrary number, assuming a corresponding handler is or will be registered. As it uses an internal `std::map`, IDs do not need to be sequential numbers.
         *
         * @param message_id The message ID to be sent. This can be any arbitrary number and does not require sequential order.
         * @param message_data The message data to be sent. This must be of the type specified by the `MessageDataType` template parameter.
         * @param max_queued_messages The maximum number of messages allowed in the queue of this ID. If the queue is full, this method will block until space is available. A value of 0 means no limit on the queue size.
         *
         * @note This method is thread-safe and ensures that messages are enqueued in a synchronized manner.
         */
        void send_message(const message_id_type message_id, const MessageDataType& message_data, const std::size_t max_queued_messages = 0)
        {
            std::shared_ptr<message_queue> queue;
            {
                std::lock_guard<std::mutex> lock(_message_queues_mutex);
                queue = _message_queues[message_id];
                if (!queue) { queue = _message_queues[message_id] = std::make_shared<message_queue>(); }
            }

            {
                std::unique_lock<std::mutex> lock(*queue->queue_mutex);
                queue->queue_cv->wait(lock, [&]
                                      { return max_queued_messages == 0 || queue->queue.size() < max_queued_messages; });
                queue->queue.emplace_back(message_data);
            }

            CBEAM_LOG_DEBUG("cbeam::concurrency::message_manager(" + cbeam::convert::to_string(this) + ")::send_message: adding message to receiver " + std::to_string(message_id) + " to queue " + cbeam::convert::to_string(&queue->queue));
            queue->queue_cv->notify_all();

            std::lock_guard<std::mutex> lock_logger(queue->message_logger_mutex);
            if (queue->message_logger) { queue->message_logger(message_id, message_data, true); }
        }

        /**
         * @brief Adds a message handler for a specified message ID.
         *
         * This method registers a new message handler for a specified ID. The handler will process messages sent to the given message ID. Handlers can be customized
         * with functions to execute on message reception, exception occurrence, idle state, and handler exit. The method supports specifying a thread name and the order in which messages are processed.
         *
         * @param message_id The message ID for which the handler is being added. It can be any arbitrary number and does not require sequential order.
         * @param on_message A function to be called when a message is received. The function should accept a message of the type specified by the `MessageDataType` template parameter.
         * @param on_exception Optional. A function to be called if an exception occurs during message processing. It must accept a `std::exception` object.
         * @param on_exit Optional. A function to be called when the message handler is about to exit.
         * @param thread_name Optional. A name for the thread running the handler. Useful for debugging purposes.
         * @param order The order in which messages will be processed by this handler. Can be FIFO (First In First Out), FILO (First In Last Out), or RANDOM.
         *
         * @note This method is thread-safe and synchronizes the addition of handlers.
         */
        void add_handler(const message_id_type                                message_id,
                         std::function<void(MessageDataType message_data)>    on_message,
                         std::function<void(const std::exception& exception)> on_exception = nullptr,
                         std::function<void()>                                on_exit      = nullptr,
                         const std::string&                                   thread_name  = {},
                         const order_type                                     order        = order_type::FIFO)
        {
            std::lock_guard<std::mutex> lock(_threads_mutex);
            std::lock_guard<std::mutex> lock2(_message_queues_mutex);

            auto& queue = _message_queues[message_id];
            if (!queue) { queue = std::make_shared<message_queue>(); }
            CBEAM_LOG_DEBUG("cbeam::concurrency::message_manager(" + cbeam::convert::to_string(this) + ")::add_handler(" + std::to_string(message_id) + ", ...): passing queue " + cbeam::convert::to_string(&queue->queue) + " to message handler");
            _threads[message_id].emplace(message_handler::create(queue->queue_mutex,
                                                                 queue->queue_cv,
                                                                 queue,
                                                                 message_id,
                                                                 on_message,
                                                                 on_exception,
                                                                 on_exit,
                                                                 thread_name + "_" + std::to_string(message_id),
                                                                 order));
        }

        void wait_until_empty(const message_id_type message_id)
        {
            std::shared_ptr<message_queue> queue;
            {
                std::lock_guard<std::mutex> lock(_message_queues_mutex);
                auto                        it = _message_queues.find(message_id);
                if (it == _message_queues.end() || !(it->second))
                {
                    // no known queue => nothing to wait for
                    return;
                }
                queue = it->second;
            }

            std::unique_lock<std::mutex> lock_q(*queue->queue_mutex);

            queue->queue_cv_empty.wait(lock_q, [&]
                                       { return queue->queue.empty() && (queue->busy_count.load(std::memory_order_acquire) == 0); });
        }

        void dispose(const message_id_type message_id)
        {
            std::lock_guard<std::mutex> lock(_threads_mutex);
            std::lock_guard<std::mutex> lock2(_message_queues_mutex);
            _threads[message_id].clear();
        }

        void set_logger(const message_id_type message_id,
                        message_logger_type   on_message)
        {
            std::lock_guard<std::mutex> lock_logger(_message_queues[message_id]->message_logger_mutex);
            _message_queues[message_id]->message_logger = on_message;
        }
    };
}
