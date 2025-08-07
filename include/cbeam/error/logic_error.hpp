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

#include <stdexcept>     // for std::logic_error
#include <string>        // for std::string

namespace cbeam::error
{
    /**
     * @class logic_error
     * @brief A Cbeam-specific logic error that also behaves like std::logic_error.
     *
     * This class inherits from cbeam::error::base_error and std::logic_error.
     * Catching cbeam::error::base_error will also catch this type,
     * and catching std::logic_error will catch it as well.
     */
    class logic_error
        : public base_error
        , public std::logic_error
    {
    public:
        /**
         * @brief Constructs a logic_error with the specified message.
         *
         * @param what_arg The error message describing the condition.
         */
        explicit logic_error(const std::string& what_arg)
            : std::logic_error(what_arg)
        {
        }

        /**
         * @brief Virtual destructor.
         */
        ~logic_error() override = default;

        /**
         * @brief Returns the descriptive string of this error.
         *
         * Internally uses std::logic_error::what().
         */
        const char* what() const noexcept override
        {
            return std::logic_error::what();
        }
    };
}
