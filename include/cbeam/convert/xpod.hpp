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

#include <cbeam/container/xpod.hpp>
#include <cbeam/convert/string.hpp> // for cbeam::convert::to_string
#include <cbeam/memory/pointer.hpp> // for cbeam::memory::pointer

namespace cbeam::convert
{
    /**
     * @brief Converts an `xpod::type` variant to a `std::string`.
     *
     * Depending on the variant's active index, this function:
     * - Converts integers, doubles, and booleans via `convert::to_string(...)`
     * - Converts pointers by calling `static_cast<std::string>` on the memory::pointer
     * - Returns the string directly if the variant is holding a std::string
     *
     * @param val The xpod::type variant to convert.
     * @return A string representation of the variant's value.
     */
    inline std::string to_string(const container::xpod::type& val)
    {
        switch (val.index())
        {
        case container::xpod::type_index::integer:
            return convert::to_string(std::get<container::xpod::type_index::integer>(val));
        case container::xpod::type_index::number:
            return convert::to_string(std::get<container::xpod::type_index::number>(val));
        case container::xpod::type_index::boolean:
            return convert::to_string(std::get<container::xpod::type_index::boolean>(val));
        case container::xpod::type_index::pointer:
            return static_cast<std::string>(std::get<container::xpod::type_index::pointer>(val));
        case container::xpod::type_index::string:
            return std::get<container::xpod::type_index::string>(val);
        default:
            return {};
        }
    }
}
