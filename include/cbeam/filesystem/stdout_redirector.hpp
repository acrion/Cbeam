
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
#include <cbeam/logging/log_manager.hpp> // for CBEAM_LOG_DEBUG

#include <cassert> // for assert
#include <cstdio>  // for freopen, fclose, stdout

#include <filesystem> // for std::filesystem::path
#include <mutex>      // for std::mutex, std::lock_guard
#include <string>     // for std::operator+

#ifdef _WIN32
    #include <cbeam/platform/windows_config.hpp>
#endif

namespace cbeam::filesystem
{
    /// \brief Class for redirecting stdout to a file and subsequently resetting it to the system's default state.
    /// \details This class redirects the standard output (stdout) to a specified file upon its construction.
    /// Upon destruction, it resets stdout back to the system's default standard output. This reset is to the
    /// system's default stdout regardless of whether stdout was originally redirected to another destination
    /// prior to the instantiation of this object.
    class stdout_redirector
    {
    public:
        /// \brief Constructor that redirects stdout to the specified file.
        /// \param file_path The path of the file to redirect stdout to.
        explicit stdout_redirector(const std::filesystem::path& file_path)
        {
            std::lock_guard<std::mutex> lock(_mtx);
            if (freopen(file_path.string().c_str(), "w", stdout) == nullptr)
            {
                throw cbeam::error::runtime_error("cbeam::filesystem::stdout_redirector: Could not redirect stdout to file " + file_path.string());
            }
        }

        /// \brief Destructor that resets stdout to the system's default state.
        /// \details On destruction, stdout is reset to the system's default output. If this operation encounters
        /// an error, it will be logged rather than throwing an exception, due to the nature of destructors in C++.
        /// Moreover, the absence of the system's default stdout is a valid scenario, e.g. when the executable is
        /// launched through an IDE that imposes such restrictions.
        virtual ~stdout_redirector() noexcept
        {
            try
            {
                std::lock_guard<std::mutex> lock(_mtx);
                auto                        redirected_stdout = stdout;
                fflush(stdout);
                bool done = false;
#ifdef _WIN32
                // "CONOUT$" is the special device name for the console on Windows, analogous to "/dev/tty" on Linux.
                done = freopen("CONOUT$", "w", stdout) != nullptr;
#else
                done = freopen("/dev/tty", "w", stdout) != nullptr;
#endif
                if (!done)
                {
                    CBEAM_LOG("cbeam::filesystem::stdout_redirector: Could not set stdout to default");
                }

                if (redirected_stdout != stdout) // This check prevents closing stdout if it wasn't successfully redirected back.
                {
                    assert(fclose(redirected_stdout) == 0);
                }
            }
            catch (const std::system_error& ex)
            {
                CBEAM_LOG("cbeam::filesystem::stdout_redirector: Could not lock the mutex. This indicates a serious (unexpected) bug that must be fixed during development phase.");
                assert(false);
            }
        }

    private:
        std::mutex _mtx;
    };
}
