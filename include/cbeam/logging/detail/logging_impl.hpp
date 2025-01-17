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

namespace cbeam::concurrency
{
    template <typename Derived, typename MessageType>
    class threaded_object;

    template <typename Message>
    class message_manager;
}

#include <cbeam/concurrency/message_manager.hpp>
#include <cbeam/concurrency/thread.hpp> // for cbeam::concurrency::get_thread_name, cbeam::concurrency::thread_id_type, cbeam::concurrency::to_string, CBEAM:CONCURRENCY::GET_CURRENT_THREAD_ID, CBEAM_DEFAULT_THREAD_ID
#include <cbeam/convert/string.hpp>     // for cbeam::convert::from_string, cbeam::convert::to_string

#include <cassert> // for assert

#include <chrono>     // for std::chrono::system_clock
#include <exception>  // for std::exception
#include <filesystem> // for std::filesystem::path, std::filesystem::remove
#include <fstream>    // for std::operator<<, std::basic_ostream, std::endl, std::basic_ofstream, std::ofstream, std::wofstream, std::wstringstream
#include <iostream>   // for std::wcerr
#include <memory>     // for std::unique_ptr, std::make_unique
#include <mutex>      // for std::mutex, std::lock_guard
#include <string>     // for std::char_traits, std::operator<<, std::wstring, std::string_literals::operator""s, std::operator+, std::string_literals, std::string

namespace cbeam::logging
{
    /**
     * @brief The `log` class provides basic file-based logging functionality.
     *
     * It creates or overwrites a log file at a specified path, writes log entries
     * with timestamps and thread information, and ensures thread safety via a mutex.
     */
    class log
    {
    public:
        /**
         * @brief Constructs a log object that manages logging to a file.
         *
         * The constructor attempts to create parent directories, remove any existing log file,
         * and then writes an initial header line.
         *
         * @param log_path The filesystem path to the log file.
         */
        explicit log(const std::filesystem::path& log_path)
        {
            using namespace std::string_literals;
            _log_path = log_path;

            std::filesystem::create_directories(log_path.parent_path());

            try
            {
                std::filesystem::remove(_log_path);
            }
            catch (const std::exception& ex)
            {
                do_append(convert::from_string<std::wstring>("Could not delete old log file: "s + ex.what()));
            }

            do_append(L"-------------------------------- start of log --------------------------------"s);
        }

        /**
         * @brief Destructor writes a final footer line before the log object is destroyed.
         *
         * Also locks the mutex to ensure no other threads are writing at the same time.
         */
        ~log() noexcept
        {
            try
            {
                using namespace std::string_literals;
                std::lock_guard<std::mutex> lock(_mtx);
                do_append(L"--------------------------------  end of log  --------------------------------"s);
            }
            catch (const std::system_error& ex)
            {
                std::wcerr << "cbeam::logging::~log: Could not lock the mutex. This indicates a serious (unexpected) bug that must be fixed during development phase: "
                           << ex.what() << std::endl;
                assert(false);
            }
        }

        /**
         * @brief Appends a wide-string message to the log, along with thread and timestamp information.
         *
         * This method attempts a non-blocking lock first for performance, falling back to a blocking lock if needed.
         *
         * @param str The wide string message to be logged.
         */
        void append(const std::wstring& str) noexcept
        {
            try
            {
                std::wstring                header;
                concurrency::thread_id_type thread_id = cbeam::concurrency::get_current_thread_id();
                if (!_mtx.try_lock())
                {
                    // We just try to lock to optimize performance. If locking fails, we postpone the creation of the header
                    header = create_header(thread_id);
                    _mtx.lock();
                }

                do_append(str, thread_id, header);
            }
            catch (const std::exception& ex)
            {
                std::wcerr << "cbeam::logging::append: " << ex.what() << std::endl;
                assert(false);
            }

            _mtx.unlock();
        }

        /**
         * @brief Appends a narrow-string message to the log (converted internally to wide-string).
         *
         * @param str The narrow string message to be logged.
         */
        void append(const std::string& str) noexcept
        {
            append(std::wstring(str.begin(), str.end()));
        }

    private:
        struct message
        {
            std::wstring                text;
            concurrency::thread_id_type threadId{};
        };

        /**
         * @brief Internal function that writes the message to the file, with an optional pre-constructed header.
         *
         * @param str The message to append.
         * @param thread_id The ID of the thread writing the log.
         * @param time A cached header string (to avoid repeated generation if the lock had to be retried).
         */
        void do_append(const std::wstring& str, concurrency::thread_id_type thread_id = {}, std::wstring time = {}) const noexcept
        {
            try
            {
                if (thread_id == cbeam::concurrency::thread_id_type{})
                    thread_id = cbeam::concurrency::get_current_thread_id();
                if (time.empty())
                    time = create_header(thread_id);

                std::wofstream(_log_path, std::ofstream::app) << time << str << std::endl;
            }
            catch (const std::exception& ex)
            {
                std::wcerr << str << " " << ex.what() << std::endl;
            }
        }

        /**
         * @brief Creates a header string containing the current time, thread ID, and thread name.
         *
         * @param thread_id The ID of the thread to be named in the header.
         * @return A formatted wide string with time, ID, and thread name.
         */
        static std::wstring create_header(concurrency::thread_id_type thread_id)
        {
            std::wstringstream stream;
            stream << current_time_to_wstring() << L" ("
                   << concurrency::to_string(thread_id, 0xFFFF) << L" "
                   << concurrency::get_thread_name(thread_id)
                   << L"): ";
            return stream.str();
        }

        /**
         * @brief Gets the current time as a wide string in "YYYY-MM-DD HH:MM:SS.mmm" format.
         *
         * @return A wide string representing the current local time (with milliseconds).
         */
        static std::wstring current_time_to_wstring()
        {
            return convert::from_string<std::wstring>(convert::to_string(std::chrono::system_clock::now()));
        }

        std::unique_ptr<concurrency::message_manager<message>> _message_manager;
        std::filesystem::path                                  _log_path;
        std::mutex                                             _mtx;
    };
}
