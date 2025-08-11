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
    #include <cbeam/logging/log_manager.hpp>
    #include <cbeam/error/runtime_error.hpp>

    #include <roapi.h>
    #pragma comment(lib, "runtimeobject.lib")
#endif

namespace cbeam::platform
{
    /// \brief Manages the initialization and deinitialization of COM.
    /// \details This class will initialize COM for the process when created
    /// and deinitialize it when destroyed. It does nothing on non-Windows platforms.
    /// It also does nothing if COM is already initialized.
    class com_initializer
    {
    public:
        /// \brief Constructor: If we are on Windows and COM is not initialized, initialize it in the specified mode
        /// \param multi_threaded If true, initializes COM in multithreaded apartment (MTA) mode.
        ///                      If false, initializes COM in single-threaded apartment (STA) mode.
        /// \param do_throw If true, throw a cbeam::error::runtime_error in case COM cannot be initialized. Default: true.
        ///                 Note that it is not error if COM was already initialized.
        com_initializer(const bool multi_threaded, const bool do_throw = true)
        {
#ifdef _WIN32
            HRESULT hr = Windows::Foundation::Initialize(multi_threaded ? RO_INIT_MULTITHREADED : RO_INIT_SINGLETHREADED);
            if (FAILED(hr))
            {
                const std::string errorMessage{"acrion::acrion::platform::com_initializer: Could not initialize COM"};
                CBEAM_LOG(errorMessage);
                if (do_throw)
                {
                    throw cbeam::error::runtime_error(errorMessage);
                }
            }

            _deinitialize_on_destruction = hr == S_OK; // S_FALSE means COM was already initialized
#endif
        }

        /// \brief Destructor: Deinitializes COM in case the constructor initialized it. Does nothing if COM was already initialized at construction time.
        ~com_initializer() noexcept
        {
#ifdef _WIN32
            if (_deinitialize_on_destruction)
            {
                Windows::Foundation::Uninitialize();
            }
#endif
        }

        /// \brief Checks if COM will be deinitialized upon destruction of this object.
        /// \return Returns true if this object will deinitialize COM on destruction, false otherwise.
        bool should_deinitialize_on_destruction() const
        {
            return _deinitialize_on_destruction;
        }

        // Prevent copy and assignment.
        com_initializer(const com_initializer&)            = delete;
        com_initializer& operator=(const com_initializer&) = delete;

    protected:
        bool _deinitialize_on_destruction;
    };
}
