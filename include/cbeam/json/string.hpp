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

#include <cbeam/convert/string.hpp>
#include <cbeam/json/traits.hpp> // for cbeam::serialization::serialized_object, cbeam::serialization::traits

#include <cstddef> // for std::size_t

#include <iosfwd> // for std::stringstream
#include <string> // for std::string

namespace cbeam::container
{
    class buffer;
}

namespace cbeam::json
{
    /**
     * @brief Specialization of `traits` for `std::string` providing JSON serialization.
     *
     * Strings are enclosed in quotes and special characters are escaped
     * using `cbeam::convert::escape_string`.
     */
    template <>
    struct traits<std::string>
    {
        static inline void serialize(const std::string& str, container::buffer& stream)
        {
            stream.append("\"", 1);
            const std::string escaped_string = convert::escape_string(str, escape_character, characters_to_escape);
            stream.append(escaped_string.c_str(), escaped_string.length());
            stream.append("\"", 1);
        }
    };
}
