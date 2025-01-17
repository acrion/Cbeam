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

#include <cbeam/serialization/traits.hpp> // for cbeam::serialization::serialized_object, cbeam::serialization::traits

#include <cstddef> // for std::size_t

#include <iosfwd> // for std::stringstream
#include <string> // for std::string

namespace cbeam::container
{
    class buffer;
}

namespace cbeam::serialization
{
    /**
     * @brief Specialization of `traits` for `std::string`, providing both serialization and deserialization.
     *
     * The serialization format stores the string length (std::size_t) followed by the character data.
     */
    template <>
    struct traits<std::string>
    {
        /**
         * @brief Serializes a `std::string` into a binary buffer.
         *
         * First appends the length of the string (as `std::size_t`),
         * then each character in the string.
         *
         * @param str The string to serialize.
         * @param stream The buffer to which serialized data is appended.
         */
        static inline void serialize(const std::string& str, container::buffer& stream)
        {
            std::size_t size = str.size();
            stream.append(reinterpret_cast<const char*>(&size), sizeof(size));

            for (const auto& c : str)
            {
                traits<char>::serialize(c, stream);
            }
        }

        /**
         * @brief Deserializes a `std::string` from a binary buffer.
         *
         * Reads the string length first, then reads that many characters.
         * The iterator `it` is advanced accordingly.
         *
         * @param it A reference to the current position in the serialized data.
         * @param str The string to populate with deserialized content.
         */
        static inline void deserialize(serialized_object& it, std::string& str)
        {
            std::stringstream s;
            str.clear();

            char*        localIt = reinterpret_cast<char*>(it);
            std::size_t* size    = reinterpret_cast<std::size_t*>(it);
            localIt += sizeof(*size);

            str = std::string(localIt, *size);
            it  = localIt + *size;
        }
    };
}
