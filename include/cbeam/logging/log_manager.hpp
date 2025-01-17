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

#include <cassert> // for assert

#include <atomic>     // for std::atomic
#include <filesystem> // for std::filesystem::operator/, std::filesystem::temp_directory_path, std::filesystem::path
#include <iostream>   // for std::operator<<, std::endl, std::basic_ostream, std::cerr
#include <memory>     // for std::shared_ptr, std::make_shared, std::__shared_ptr_access
#include <mutex>      // for std::recursive_mutex, std::lock_guard
#include <string>     // for std::char_traits, std::wstring, std::string

namespace cbeam::logging
{
    class log;

    /**
     * @brief The `log_manager` class provides a global logging facility that can be used throughout the application.
     *
     * It maintains a singleton `log_manager` instance with a shared `log` object.
     * Users can create a logfile via `create_logfile(...)` and then call `log_append(...)` to write logs.
     * The log path defaults to "Cbeam.log" in the system's temporary directory if not explicitly set.
     */
    class log_manager
    {
    public:
        virtual ~log_manager() noexcept;

        /**
         * @brief Creates or re-initializes the global log file at the specified path.
         *
         * If the log_manager singleton doesn't yet have a log, it is created with the given path.
         * Otherwise, the existing log is used. This method is safe to call multiple times.
         *
         * @param path The desired log file path.
         * @param instance An optional pointer to an existing log_manager instance (usually null).
         */
        static inline void create_logfile(const std::filesystem::path& path, log_manager* instance = nullptr);

        /**
         * @brief Appends a wide-string message to the current global log.
         *
         * If no log file was previously created, a default log file is created first.
         *
         * @param str The wide-string message to be appended.
         */
        static inline void log_append(const std::wstring& str);

        /**
         * @brief Appends a narrow-string message to the current global log.
         *
         * Internally converted to wide-string and forwarded to `log_append(const std::wstring&)`.
         *
         * @param str The narrow-string message to be appended.
         */
        static inline void log_append(const std::string& str);

    private:
        log_manager() = default;
        static inline log_manager* Instance();

        /**
         * @brief Determines if logging is still valid or if the manager is shutting down.
         *
         * @return true if logging can proceed, false otherwise.
         */
        static inline bool is_operational_for_logging();

        std::shared_ptr<log>               _log;
        std::recursive_mutex               _log_mutex;
        static inline std::atomic<bool>    _shutting_down{false};
        static inline std::recursive_mutex _shutting_down_mutex;
    };
}

/**
 * @def CBEAM_DEBUG_LOGGING
 * @brief Controls debug logging within the Cbeam library.
 *
 * When this macro is defined and set to 1, debug log messages are enabled.
 * Otherwise, if it is set to 0 or not defined, debug log messages are suppressed.
 *
 * @note This macro can be defined in project settings or directly in code before including cbeam headers.
 *       The default behavior is to enable debug logging if the _DEBUG macro is defined.
 */
#ifndef CBEAM_DEBUG_LOGGING
    #ifdef _DEBUG
        #define CBEAM_DEBUG_LOGGING 1
    #else
        #define CBEAM_DEBUG_LOGGING 0
    #endif
#endif

/// @def CBEAM_LOG(s)
/// @brief Logs a message using cbeam::logging::log_manager.
///
/// This macro logs a message, regardless of the CBEAM_DEBUG_LOGGING setting. It uses
/// the cbeam::logging::log_manager to append the log message.
/// @param s The message string to log.
#define CBEAM_LOG(s) ::cbeam::logging::log_manager::log_append(s);

/// @def CBEAM_LOG_DEBUG(s)
/// @brief Logs a debug message if CBEAM_DEBUG_LOGGING is enabled.
///
/// This macro logs a debug message using cbeam::logging::log_manager,
/// but only if CBEAM_DEBUG_LOGGING is defined and set to 1. If CBEAM_DEBUG_LOGGING is not defined
/// or set to 0, this macro does nothing.
///
/// @note CBEAM_DEBUG_LOGGING can be set by the project that includes cbeam headers.
/// @param s The message string to log.
#if CBEAM_DEBUG_LOGGING
    #define CBEAM_LOG_DEBUG(s) ::cbeam::logging::log_manager::log_append(s);
#else
    #define CBEAM_LOG_DEBUG(s)
#endif

#include "detail/logging_impl.hpp"  // for cbeam::logging::log
#include <cbeam/convert/string.hpp> // for cbeam::convert::from_string, cbeam::convert

namespace cbeam::logging
{
    namespace convert = ::cbeam::convert;

    inline log_manager::~log_manager() noexcept
    {
        try
        {
            std::lock_guard<std::recursive_mutex> lock(_shutting_down_mutex);
            _shutting_down = true;
        }
        catch (const std::system_error& ex)
        {
            std::cerr << "cbeam::logging::log_manager::~log_manager: Could not lock the mutex. This indicates a serious (unexpected) bug that must be fixed during development phase." << std::endl;
            assert(false);
        }
    }

    inline void log_manager::create_logfile(const std::filesystem::path& path, log_manager* instance)
    {
        if (!_shutting_down_mutex.try_lock() && instance)
        {
            // An instance of LogManager has been created without locking _shutting_down_mutex, which should be impossible
            assert(false);
        }

        if (!instance)
        {
            instance = Instance();
        }

        if (is_operational_for_logging())
        {
            std::lock_guard<std::recursive_mutex> lock(instance->_log_mutex);
            if (!instance->_log)
            {
                instance->_log = std::make_shared<log>(path);
            }
        }

        _shutting_down_mutex.unlock();
    }

    inline void log_manager::log_append(const std::wstring& str)
    {
        std::lock_guard<std::recursive_mutex> lockShuttingDown(_shutting_down_mutex);
        auto                                  instance = Instance();

        if (is_operational_for_logging())
        {
            std::lock_guard<std::recursive_mutex> lockLog(instance->_log_mutex);
            if (!instance->_log)
            {
                create_logfile(std::filesystem::temp_directory_path() / "Cbeam.log", instance);
            }
            instance->_log->append(str);
        }
    }

    inline void log_manager::log_append(const std::string& str)
    {
        std::lock_guard<std::recursive_mutex> lockShuttingDown(_shutting_down_mutex);
        if (is_operational_for_logging())
        {
            log_append(convert::from_string<std::wstring>(str));
        }
    }

    inline log_manager* log_manager::Instance()
    {
        static log_manager instance;
        return &instance;
    }

    inline bool log_manager::is_operational_for_logging()
    {
        if (_shutting_down)
        {
            std::cerr << "Error: The main function returned or the shared library is being unloaded, but a logging attempt has been made." << std::endl
                      << std::endl
                      << "This condition indicates a serious issue that must be resolved during the development phase." << std::endl
                      << "The static cbeam::logging::log_manager instance has been correctly destroyed by the C++ runtime" << std::endl
                      << "due to the termination of the main application or the unloading of a shared library. However," << std::endl
                      << "another thread is still attempting to perform logging operations. It is crucial to ensure that" << std::endl
                      << "all threads are joined before the main application exits or before the shared library is unloaded." << std::endl;
            assert(false);
        }

        return !_shutting_down;
    }
}
