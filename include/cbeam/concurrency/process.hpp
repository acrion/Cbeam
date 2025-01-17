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
    #include <unistd.h> // for getpid, pid_t
#endif

namespace cbeam::concurrency
{
    /**
     * @typedef process_id_type
     * @brief Defines a platform-independent type for process identifiers.
     *
     * On Windows, this is defined as DWORD, representing the native process ID type.
     * On Unix-based systems (Linux, macOS), this is defined as pid_t, the standard type for process IDs.
     */
#ifdef _WIN32
    using process_id_type = DWORD;
#else
    using process_id_type = pid_t;
#endif

    /**
     * @brief Retrieves the current process's identifier in a platform-independent manner.
     *
     * On Windows, this function wraps the GetCurrentProcessId() call.
     * On Unix-based systems, it calls getpid().
     *
     * @return process_id_type The unique identifier of the current process.
     */
    inline process_id_type get_current_process_id()
    {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return ::getpid();
#endif
    }
}
