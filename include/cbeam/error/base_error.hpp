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

#include <exception>

namespace cbeam::error
{
    /**
     * @class base_error
     * @brief The root class for all Cbeam-specific exceptions, akin to std::exception.
     *
     * This class inherits virtually from std::exception to allow multiple inheritance
     * with standard library exceptions (e.g., std::runtime_error or std::system_error),
     * without generating multiple subobjects of std::exception.
     */
    class base_error : public virtual std::exception
    {
    public:
        /**
         * @brief Default constructor.
         */
        base_error() = default;

        /**
         * @brief Virtual destructor.
         */
        ~base_error() override = default;

        /**
         * @brief Returns a generic explanatory string for all Cbeam base errors.
         *
         * @return A pointer to a null-terminated string with an error message.
         */
        const char* what() const noexcept override
        {
            return "cbeam::error::base_error";
        }
    };
}
