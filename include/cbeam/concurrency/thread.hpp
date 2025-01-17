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

#include <cbeam/convert/string.hpp> // for cbeam::convert::from_string

#include <cstddef> // for std::size_t

#include <iomanip> // for std::operator<<, std::setfill, std::setw
#include <sstream> // for std::basic_ostream, std::hex, std::uppercase, std::wstringstream
#include <string>  // for allocator, wstring
#include <thread>

#ifdef _WIN32
    #include <cbeam/platform/windows_config.hpp>
#else
    #include <pthread.h> // for pthread_getname_np, pthread_t

    #ifdef __linux__
        #include <sys/prctl.h>
    #endif
#endif

namespace cbeam::concurrency
{
#ifdef _WIN32
    using thread_id_type = HANDLE;
#else
    using thread_id_type = pthread_t;
#endif

    /**
     * @brief Retrieves the current thread's native identifier.
     *
     * On Windows, the returned identifier is a `HANDLE` to the current thread.
     * On other platforms, this is a `pthread_t`.
     *
     * @return thread_id_type The native identifier of the current thread.
     */
    inline thread_id_type get_current_thread_id()
    {
#ifdef _WIN32
        return GetCurrentThread();
#else
        return pthread_self();
#endif
    }

#ifdef _WIN32
    #pragma pack(push, 8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD  dwType;     // Must be 0x1000.
        LPCSTR szName;     // Pointer to name (in user addr space).
        DWORD  dwThreadID; // Thread ID (-1=caller thread).
        DWORD  dwFlags;    // Reserved for future use, must be zero.
    } THREADNAME_INFO;
    #pragma pack(pop)

    /**
     * @brief Sets the name for a thread with a specified Thread ID.
     *
     * This Windows-specific function uses a debugger exception mechanism
     * to associate a name with a thread in debugging tools like Visual Studio.
     *
     * @param dwThreadID The numeric thread ID.
     * @param thread_name The name to set for the thread.
     */
    inline void set_thread_name(uint32_t dwThreadID, const char* thread_name)
    {
        const DWORD     MS_VC_EXCEPTION = 0x406D1388;
        THREADNAME_INFO info;
        info.dwType     = 0x1000;
        info.szName     = thread_name;
        info.dwThreadID = dwThreadID;
        info.dwFlags    = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    /**
     * @brief Sets the name of the current thread.
     *
     * @param thread_name The name to set for the current thread.
     */
    inline void set_thread_name(const char* thread_name)
    {
        set_thread_name(GetCurrentThreadId(), thread_name);
    }

    /**
     * @brief Sets the name of a `std::thread` on Windows.
     *
     * @param thread The `std::thread` object whose name is to be set.
     * @param thread_name The name to assign to that thread.
     */
    inline void set_thread_name(std::thread& thread, const char* thread_name)
    {
        DWORD thread_id = ::GetThreadId(static_cast<HANDLE>(thread.native_handle()));
        set_thread_name(thread_id, thread_name);
    }
#elif defined(__APPLE__)
    /**
     * @brief Sets the name of a `std::thread` on Apple platforms.
     *
     * Note that on Apple platforms, setting a name for a `std::thread` object
     * other than the current thread is not supported.
     *
     * @param thread The `std::thread` object to name (no-op here).
     * @param thread_name The name to assign (ignored on Apple).
     */
    inline void set_thread_name(std::thread& thread, const char* thread_name)
    {
        // not possible under darwin
    }

    /**
     * @brief Sets the name of the current thread on Apple platforms.
     *
     * @param thread_name The name to assign to the current thread.
     */
    inline void set_thread_name(const char* thread_name)
    {
        pthread_setname_np(thread_name);
    }
#else
    /**
     * @brief Sets the name of a `std::thread` on non-Windows, non-Apple platforms (e.g. Linux).
     *
     * @param thread The `std::thread` object whose name is to be set.
     * @param thread_name The name to assign to that thread.
     */
    inline void set_thread_name(std::thread& thread, const char* thread_name)
    {
        pthread_setname_np(thread.native_handle(), thread_name);
    }

    /**
     * @brief Sets the name of the current thread on non-Windows, non-Apple platforms (e.g. Linux).
     *
     * @param thread_name The name to assign to the current thread.
     */
    inline void set_thread_name(const char* thread_name)
    {
        prctl(PR_SET_NAME, thread_name, 0, 0, 0);
    }
#endif

    /**
     * @brief Retrieves the name of the specified thread.
     *
     * @param id The thread identifier.
     * @return std::wstring The name of the thread, or an empty string if not available or on failure.
     */
    inline std::wstring get_thread_name(thread_id_type id)
    {
#ifdef _WIN32
        PWSTR   data;
        HRESULT hr = GetThreadDescription(id, &data);
        if (SUCCEEDED(hr))
        {
            std::wstring str(data);
            LocalFree(data);
            return str;
        }
#else
        char thread_name[64];
        if (pthread_getname_np(id, thread_name, sizeof(thread_name)) == 0)
        {
            return convert::from_string<std::wstring>(thread_name);
        }
#endif
        return L"";
    }

    /**
     * @brief Returns a hexadecimal string representation of the given thread ID.
     *
     * @param id The thread identifier.
     * @param mask A bitmask to apply to the thread ID before conversion.
     *             May be used to achieve a shortened string representation.
     *             Defaults to all bits set.
     * @return std::wstring The hexadecimal string representation of the thread ID.
     */
    inline std::wstring to_string(concurrency::thread_id_type id, std::size_t mask = (std::size_t)-1)
    {
        std::wstringstream ss;

#if defined(__GNUC__) && (__GNUC__ >= 11) && __linux__
        /// directly converting a std::thread::id to a std::string with Linux GCC 11 and GCC 13 causes
        /// undefined reference to `std::basic_ostream<char, std::char_traits<char> >& std::operator<< <char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, std::thread::id)'
        std::hash<decltype(id)> hasher;
        std::size_t             hash_value = hasher(id) & mask;
        ss << std::setfill(L'0') << std::setw(4) << std::hex << std::uppercase << hash_value;
#else
        ss << std::setfill(L'0') << std::setw(4) << std::hex << std::uppercase
           << (reinterpret_cast<std::size_t>(id) & mask);
#endif

        return ss.str();
    }
}
