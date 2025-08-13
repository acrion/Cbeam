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

#include <cbeam/error/base_error.hpp>

#include <stdexcept> // for std::bad_alloc
#include <string>    // for std::string

namespace cbeam::error
{
    /**
     * @class bad_alloc
     * @brief A Cbeam-specific logic error that also behaves like std::bad_alloc.
     *
     * This class inherits from cbeam::error::base_error and std::bad_alloc.
     * Catching cbeam::error::base_error will also catch this type,
     * and catching std::bad_alloc will catch it as well.
     */
    class bad_alloc
        : public base_error
        , public std::bad_alloc
    {
    public:
        /**
         * @brief Constructs a bad_alloc with the specified message.
         */
        explicit bad_alloc()
            : std::bad_alloc()
        {
        }

        /**
         * @brief Virtual destructor.
         */
        ~bad_alloc() override = default;
    };
}
