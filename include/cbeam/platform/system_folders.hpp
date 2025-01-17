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

#ifdef _WIN32
    #include "windows_config.hpp"
    #include <Knownfolders.h>
    #include <shlobj_core.h>
    #pragma comment(lib, "comsuppw.lib")
    #pragma comment(lib, "kernel32.lib")
#else
    #include <cbeam/config.hpp>

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
}
