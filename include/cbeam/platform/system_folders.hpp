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

#include <cbeam/error/runtime_error.hpp> // for cbeam::error:runtime_error
#include <vector>

#ifdef _WIN32
    #include "windows_config.hpp"
    #include <Knownfolders.h>
    #include <shlobj.h>
    #pragma comment(lib, "comsuppw.lib")
    #pragma comment(lib, "kernel32.lib")
#else
    #include <cbeam/config.hpp>
    #include <limits.h> // for PATH_MAX
    #include <stdlib.h> // for realpath
    #if defined(__APPLE__)
        #include <mach-o/dyld.h> // for _NSGetExecutablePath
    #endif

    #if HAVE_PWD_H
        #include <pwd.h> // for getpwuid, passwd
    #endif
    #if HAVE_UNISTD_H
        #include <unistd.h> // for getuid
    #endif
#endif

#include <filesystem> // for std::filesystem::path, std::filesystem::operator/, std::filesystem::exists
#include <mutex>      // for std::mutex, std::lock_guard
#include <string>     // for std::allocator, std::operator+, std::char_traits, std::string_literals::operator""s, std::string_literals

namespace cbeam::filesystem
{
    /**
     * @brief Retrieves the path to the user's home directory, based on the operating system.
     *
     * On Windows, it uses `SHGetKnownFolderPath(FOLDERID_Profile)`.
     * On Linux or Darwin, it uses `getpwuid(getuid())`.
     *
     * @return A `std::filesystem::path` corresponding to the user's home folder.
     * @throws cbeam::error::runtime_error if the function fails to determine the home directory.
     */
    inline std::filesystem::path get_home_dir()
    {
        static std::filesystem::path home_folder;
        static std::mutex            mtx;
        std::lock_guard<std::mutex>  lock(mtx);

        if (home_folder.empty())
        {
#ifdef _WIN32
            PWSTR   pszPath = nullptr;
            HRESULT hr      = SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &pszPath);

            if (hr == S_OK && pszPath)
            {
                home_folder = pszPath;
            }
            else
            {
                throw cbeam::error::runtime_error("Failed to determine path to user's home directory (FOLDERID_Profile)");
            }
#elif HAVE_PWD_H && HAVE_UNISTD_H

            struct passwd* p = getpwuid(getuid());
            if (p == nullptr)
            {
                throw cbeam::error::runtime_error("Failed to determine path to user's home directory (passwd::pw_dir)");
            }
            home_folder = p->pw_dir;
#else
    #error Unsupported platform
#endif
        }

        return home_folder;
    }

    /**
     * @brief Retrieves the path for storing user-specific application data.
     *
     * On Windows, this is typically `%AppData%`.
     * On Linux, it returns `~/.local/share`.
     * On macOS, it returns `~/Library/Application Support`.
     *
     * @return A `std::filesystem::path` corresponding to the user-specific application data folder.
     * @throws cbeam::error::runtime_error if the path does not exist or cannot be determined.
     */
    inline std::filesystem::path get_user_data_dir()
    {
        using namespace std::string_literals;

        static std::filesystem::path app_data_folder;
        static std::mutex            mtx;
        std::lock_guard<std::mutex>  lock(mtx);

#ifdef _WIN32
        PWSTR   pszPath = nullptr;
        HRESULT hr      = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pszPath);

        if (hr == S_OK && pszPath)
        {
            app_data_folder = pszPath;
        }
        else
        {
            throw cbeam::error::runtime_error("Failed to determine Cbeam %APPDATA% path");
        }
#else
    #if defined(__linux__)
        app_data_folder = get_home_dir() / ".local"s / "share"s;
    #elif defined(__APPLE__)
        app_data_folder = get_home_dir() / "Library" / "Application Support";
    #else
        #error Unknown platform (please extended the code to check an appropriate macro)
    #endif
#endif

        if (!std::filesystem::exists(app_data_folder))
        {
            throw cbeam::error::runtime_error("Path '" + app_data_folder.string() + "' is expected to exist on this system.");
        }

        return app_data_folder;
    }

    /**
     * @brief Retrieves the path for storing user-specific cache data.
     *
     * On Windows, this is typically `%LocalAppData%`.
     * On Linux, it returns `~/.cache`.
     * On macOS, it returns `~/Library/Caches`.
     *
     * @return A `std::filesystem::path` corresponding to the user-specific cache folder.
     * @throws cbeam::error::runtime_error if the path does not exist or cannot be determined.
     */
    inline std::filesystem::path get_user_cache_dir()
    {
        using namespace std::string_literals;

        static std::filesystem::path cache_dir;
        static std::mutex            mtx;
        std::lock_guard<std::mutex>  lock(mtx);

#ifdef _WIN32
        PWSTR   pszPath = nullptr;
        HRESULT hr      = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &pszPath);

        if (hr == S_OK && pszPath)
        {
            cache_dir = pszPath;
        }
        else
        {
            throw cbeam::error::runtime_error("Failed to determine %LOCALAPPDATA% path");
        }
#else
    #if defined(__linux__)
        cache_dir = get_home_dir() / ".cache"s;
    #elif defined(__APPLE__)
        cache_dir = get_home_dir() / "Library" / "Caches";
    #else
        #error Unknown platform (please extended the code to check an appropriate macro)
    #endif
#endif

        if (!std::filesystem::exists(cache_dir))
        {
            throw cbeam::error::runtime_error("Path '" + cache_dir.string() + "' is expected to exist on this system.");
        }

        return cache_dir;
    }

    /**
     * @brief Retrieves the path to the binary that contains the current code.
     *
     * Pragmatic, cross-platform implementation:
     *  - Windows: uses GetModuleHandleExW (by address) + GetModuleFileNameW, falling back to the EXE.
     *  - Linux:   resolves "/proc/self/exe".
     *  - macOS:   uses _NSGetExecutablePath and canonicalises with realpath.
     *
     * @param include_filename If true (default), returns the full path including the filename.
     *                         If false, returns the directory containing the binary.
     * @return A std::filesystem::path pointing to the current binary (or its directory).
     * @throws cbeam::error::runtime_error on failure or if the resulting path does not exist.
     */
    inline std::filesystem::path get_current_binary_path(bool include_filename = true)
    {
        static std::filesystem::path cached_full_path;
        static std::mutex            mtx;
        std::lock_guard<std::mutex>  lock(mtx);

        if (cached_full_path.empty())
        {
#ifdef _WIN32
            // Try to get the module that contains this function (works for both EXE and DLL).
            HMODULE mod = nullptr;
            if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                    reinterpret_cast<LPCWSTR>(&get_current_binary_path),
                                    &mod))
            {
                // Pragmatic fallback: use the main EXE.
                mod = nullptr;
            }

            std::vector<wchar_t> buffer(1024);
            for (;;)
            {
                DWORD len = GetModuleFileNameW(mod, buffer.data(), static_cast<DWORD>(buffer.size()));
                if (len == 0)
                {
                    throw cbeam::error::runtime_error(
                        std::string("Failed to determine path to current binary: ")
                        + cbeam::platform::get_last_windows_error_message());
                }
                // If the buffer was too small, the result may be truncated; grow and retry.
                if (len < buffer.size() - 1)
                {
                    cached_full_path = std::filesystem::path(buffer.data(), buffer.data() + len);
                    break;
                }
                buffer.resize(buffer.size() * 2);
            }

#elif defined(__linux__)
            // Read the /proc/self/exe symlink.
            std::vector<char> buffer(1024);
            for (;;)
            {
                ssize_t len = ::readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
                if (len < 0)
                {
                    throw cbeam::error::runtime_error("Failed to determine path to current binary via /proc/self/exe");
                }
                if (static_cast<size_t>(len) >= buffer.size() - 1)
                {
                    buffer.resize(buffer.size() * 2);
                    continue;
                }
                buffer[len] = '\0';
                cached_full_path = std::filesystem::path(buffer.data());
                break;
            }

#elif defined(__APPLE__)
            // First call to get required buffer size.
            uint32_t size = 0;
            if (_NSGetExecutablePath(nullptr, &size) != -1)
            {
                // According to Apple docs, this should return -1 and set 'size'.
                throw cbeam::error::runtime_error("Failed to determine required buffer size for _NSGetExecutablePath");
            }

            std::vector<char> buffer(size);
            if (_NSGetExecutablePath(buffer.data(), &size) != 0)
            {
                throw cbeam::error::runtime_error("Failed to determine path to current binary via _NSGetExecutablePath");
            }

            // Canonicalise to resolve symlinks if possible.
            char resolved[PATH_MAX];
            if (::realpath(buffer.data(), resolved) != nullptr)
            {
                cached_full_path = std::filesystem::path(resolved);
            }
            else
            {
                // Fallback to the non-canonicalised path.
                cached_full_path = std::filesystem::path(buffer.data());
            }
#else
    #error Unknown platform
#endif
        }

        const auto result = include_filename ? cached_full_path : cached_full_path.parent_path();

        if (!std::filesystem::exists(result))
        {
            throw cbeam::error::runtime_error("Path '" + result.string() + "' is expected to exist on this system.");
        }

        return result;
    }
}
