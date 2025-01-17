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

/**
 * @file windows_config.hpp
 * @brief Header file to manage inclusion of windows.h with specific settings.
 * @details The settings disable the definition of certain macros like min, max, and MSG, which are known to cause
 * conflicts with widespread libraries. The WINVER and _WIN32_WINNT macros are set to ensure that the application
 * can use certain features and APIs that were introduced in Windows 10. The WIN32_LEAN_AND_MEAN definition
 * excludes some less commonly used APIs from windows.h to reduce the overall footprint.
 */

#pragma once

#ifdef _WIN32
    #ifndef WINVER
        #define WINVER 0x0A00
    #endif
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0A00
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>

    #include <string>

namespace cbeam::platform
{
    /**
     * @brief Retrieves a descriptive error message for the last Windows API error.
     *
     * This function attempts to retrieve a human-readable error message corresponding to the last error code set by the Windows API.
     * The function assumes that an error has occurred and does not perform error detection; it should be called
     * only when an error is known to have occurred. It returns a generic message if it fails to retrieve a specific error message.
     *
     * @return A std::string containing the error message. If unable to retrieve a specific message, it returns "unknown error".
     */
    inline std::string get_last_windows_error_message()
    {
        std::string message;

        for (int attempts = 0; attempts < 2; ++attempts)
        {
            DWORD error_code     = GetLastError(); // Get the last error code.
            LPSTR message_buffer = nullptr;

            const size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                               NULL,
                                               error_code,
                                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                               (LPSTR)&message_buffer,
                                               0,
                                               NULL);

            if (size > 0)
            {
                message = {message_buffer, size};
            }
            else
            {
                // This compiles to the documentation of `FormatMessageA`: "To get extended error information, call GetLastError".
                // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagea
                // But we only make 1 further attempt to avoid an infinite loop.
                error_code = GetLastError();
            }

            // The documentation does not define if there are error cases with allocated buffer ("If the function fails, the return value is zero")
            // So we assume that a changed value of message_buffer means that it points to allocated memory.
            // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagea
            if (message_buffer)
            {
                LocalFree(message_buffer);
            }
        }

        if (message.empty())
        {
            // get_last_windows_error_message is not meant to detect if there was an error, but is based on the assumption that there was an error.
            // Detecting generically if there was an error is impossible, because
            // (1) "some functions set the last-error code to 0 on success and others do not"
            // (2) some functions do not set the error code if they fail ("Most functions that set the thread's last-error code set it when they fail.")
            // https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
            message = "unknown error";
        }

        return message;
    }

    inline std::string get_last_windows_error_message()
    {
        DWORD error_code = GetLastError(); // we do not check if error_code == 0, because according to documentation "some functions set the last-error code to 0 on success and others do not" https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror

        LPSTR  message_buffer = nullptr;
        size_t size           = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL,
                                     error_code,
                                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     (LPSTR)&message_buffer,
                                     0,
                                     NULL);

        if (size > 0)
        {
            std::string message(message_buffer, size);
            LocalFree(message_buffer);
            return message;
        }
        else
        {
            // This compiles to the documentation of `FormatMessageA`: "To get extended error information, call GetLastError".
            // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagea
            return get_last_windows_error_message();
        }
    }
}
#endif
