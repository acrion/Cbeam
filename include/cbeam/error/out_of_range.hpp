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

#include <stdexcept> // for std::out_of_range
#include <string>    // for std::string

namespace cbeam::error
{
    /**
     * @class out_of_range
     * @brief A Cbeam-specific out_of_range error that also behaves like std::out_of_range.
     *
     * This class inherits from cbeam::error::base_error and std::out_of_range via
     * virtual inheritance. As a result, throwing this exception can be caught by:
     * - catch(const cbeam::error::base_error&)
     * - catch(const cbeam::error::out_of_range&)
     * - catch(const std::out_of_range&)
     * - catch(const std::logic_error&)
     * - catch(const std::exception&)
     */
    class out_of_range
        : public virtual cbeam::error::base_error
        , public virtual std::out_of_range
    {
    public:
        /**
         * @brief Constructs an out_of_range error with the specified message.
         *
         * @param what_arg The error message describing the out-of-range condition.
         */
        explicit out_of_range(const std::string& what_arg)
            : std::out_of_range(what_arg)
        {
        }

        /**
         * @brief Virtual destructor.
         */
        ~out_of_range() override = default;

        /**
         * @brief Returns the descriptive string of this error.
         */
        const char* what() const noexcept override
        {
            return std::out_of_range::what();
        }
    };
}
