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

#ifdef _WIN32
    #include <cbeam/platform/windows_config.hpp>
#else
    #include <cerrno>
#endif

#include <cbeam/error/base_error.hpp>

#include <string>       // for std::string
#include <system_error> // for std::system_error

namespace cbeam::error
{
    /**
     * @class system_error
     * @brief Custom exception class for handling system-level errors in a cross-platform manner.
     *
     * The `cbeam::error::system_error` class extends the standard `std::system_error` by providing
     * a constructor that automatically sets the appropriate error category and code based on the platform.
     * This class simplifies the creation of system errors with custom messages while ensuring that the
     * error codes and categories are consistent with the underlying operating system's conventions.
     *
     * Inherits from cbeam::error::base_error and std::system_error via
     * virtual inheritance. This ensures a single std::exception subobject
     * when combined with other cbeam::error classes.
     *
     * Usage:
     * This class can be used to throw exceptions with detailed error messages, especially in scenarios
     * where system calls fail. It automatically uses the correct error code and category without requiring
     * them to be specified explicitly.
     *
     * @example cbeam/error/system_error.hpp
     * @code{.cpp}
     * throw cbeam::error::system_error("Custom error message");
     * @endcode
     *
     * Under the hood:
     * - On Windows platforms, this class uses `GetLastError()` to retrieve the last error code set by the Windows API,
     *   and `std::system_category()` to categorize the error as a system-specific error. This is suitable for errors
     *   that are specific to the Windows API and its various system calls.
     *
     * - On Unix-based platforms (like Linux and macOS), the class uses `errno` for the error code, which is the standard
     *   way of reporting errors in POSIX-compliant systems. It utilizes `std::generic_category()` for the error category,
     *   reflecting the more generic nature of `errno`. This is appropriate since `errno` covers a wide range of error
     *   conditions that are not limited to specific system calls.
     *
     * Rationale:
     * The differentiation between `std::system_category()` on Windows and `std::generic_category()` on Unix-based systems
     * allows for more accurate representation of errors in a platform-specific manner. `std::system_category()` is used
     * on Windows to align with the Windows-specific error codes provided by `GetLastError()`, while `std::generic_category()`
     * is used on Unix-based systems for its broader applicability across various types of errors indicated by `errno`.
     */
    class system_error
        : public virtual base_error
        , public virtual std::system_error
    {
    public:
#ifdef _WIN32
        /**
         * @brief Constructs a system_error using GetLastError() on Windows.
         *
         * @param message The message describing this system error.
         */
        explicit system_error(const std::string& message = "")
            : std::system_error(::GetLastError(), std::system_category(), message)
        {
        }
#else
        /**
         * @brief Constructs a system_error using errno on Unix-based systems.
         *
         * @param message The message describing this system error.
         */
        explicit system_error(const std::string& message = "")
            : std::system_error(errno, std::generic_category(), message)
        {
        }
#endif

        /**
         * @brief Virtual destructor.
         */
        ~system_error() override = default;

        /**
         * @brief Returns the descriptive string of this error, using std::system_error::what().
         */
        const char* what() const noexcept override
        {
            return std::system_error::what();
        }
    };
}
