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

#include <cbeam/config.hpp> // for HAVE_DLFCN_H

#ifdef _WIN32
    #include "windows_config.hpp"
#elif HAVE_DLFCN_H
    #include <cstdlib>  // for realpath
    #include <dlfcn.h>  // for dladdr, Dl_info
    #include <limits.h> // for PATH_MAX
#endif

#include <filesystem> // for std::filesystem::path
#include <stdexcept>  // for std::runtime_error
#include <string>     // for std::allocator, std::operator+, std::char_traits, std::string

namespace cbeam::platform
{
    /**
     * @brief Retrieves the absolute path to the runtime binary (either an executable or a shared library) that contains a specified symbol.
     *
     * This function attempts to locate the runtime binary containing the specified symbol.
     * By default, if no symbol is provided (@p symbol_inside_runtime_binary is nullptr),
     * the function employs a local static symbol defined within itself. This approach is useful
     * for identifying the path of the binary that includes this function.
     *
     * @note Under Linux and macOS, this function may not yield the intended results if the executable
     * was invoked through a symbolic link not present in the application's current directory but rather in
     * the executing shell's PATH. This is due to the fact that the function relies on resolving
     * the runtime context of the symbol, which may not correctly reflect symbolic links used during invocation.
     *
     * @warning When relying on the default parameter, linker optimizations might cause the local static symbol to
     * reside in a binary different from the intended target, especially when multiple shared libraries including this header
     * are linked to the same executable. This can result in unexpected path resolutions.
     * To ensure accurate results, it is recommended to explicitly pass a pointer to a static symbol
     * known to reside within the desired binary.
     *
     * @param symbol_inside_runtime_binary A pointer to a symbol within the runtime binary. If nullptr, a local static symbol is used.
     * @return std::filesystem::path The absolute path to the runtime binary containing the specified symbol.
     * @throws std::runtime_error if the path cannot be determined or if the symbol is not associated with any binary.
     */
    inline std::filesystem::path get_path_to_runtime_binary(const void* symbol_inside_runtime_binary = nullptr)
    {
        if (symbol_inside_runtime_binary == nullptr)
        {
            static const char symbol_inside_this_runtime_binary{};
            symbol_inside_runtime_binary = &symbol_inside_this_runtime_binary;
        }
#ifdef _WIN32
        char    path[MAX_PATH];
        HMODULE hm = NULL;

        if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)symbol_inside_runtime_binary,
                               &hm)
            || GetModuleFileName(hm, path, sizeof(path)) <= 0)
        {
            throw std::runtime_error("cbeam::platform::get_path_to_runtime_binary: Could not get path to current runtime binary: " + get_last_windows_error_message());
        }
        else
        {
            return std::filesystem::absolute(std::filesystem::path(path));
        }
#elif HAVE_DLFCN_H
        Dl_info dl_info;
        if (!dladdr(symbol_inside_runtime_binary, &dl_info))
        {
            throw std::runtime_error("cbeam::platform::get_path_to_runtime_binary: Could not resolve symbol");
        }

        if (dl_info.dli_fname)
        {
            char resolved_path[PATH_MAX];
            if (realpath(dl_info.dli_fname, resolved_path) != nullptr)
            {
                return std::filesystem::path(resolved_path);
            }
            else
            {
                throw std::runtime_error("cbeam::platform::get_path_to_runtime_binary: Could not resolve symbolic link '" + std::string{dl_info.dli_fname} + "'");
            }
        }
        else
        {
            throw std::runtime_error("cbeam::platform::get_path_to_runtime_binary: Symbol is not associated with a shared library or executable");
        }
#else
    #error Unsupported platform
#endif
        throw std::runtime_error("cbeam::platform::get_path_to_runtime_binary: Could not get path to current runtime binary");
    }
}