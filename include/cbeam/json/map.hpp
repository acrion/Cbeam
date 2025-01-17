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

#include <cbeam/container/buffer.hpp> // for buffer (ptr only), cbeam::container::buffer
#include <cbeam/json/traits.hpp>      // for cbeam::serialization::traits, cbeam::serialization::serialized_object

#include <cstdint> // for uint8_t
#include <cstring> // for memcpy, size_t, std::memcpy

#include <map> // for std::map

namespace cbeam::json
{
    /**
     * @brief Provides JSON-style serialization logic for any map type `Map`.
     *
     * This serializer inserts comma separators between elements and wraps
     * the serialized key-value pairs in braces `{}` when used via `traits`.
     *
     * @tparam Map A map-like type providing `begin()`, `end()`, and `key_type`, `mapped_type`.
     */
    template <typename Map>
    struct map_serializer
    {
        /**
         * @brief Serializes a map to a JSON-like representation, appending the result to a buffer.
         *
         * Each key-value pair is serialized (via `traits`) and separated by commas.
         *
         * @param map The map to serialize.
         * @param stream A buffer to which the JSON-like output is appended.
         */
        static void serialize(const Map& map, container::buffer& stream)
        {
            bool first = true;

            for (const auto& pair : map)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    stream.append(",", 1);
                }

                traits<typename Map::key_type>::serialize(pair.first, stream);
                stream.append(":", 1);
                traits<typename Map::mapped_type>::serialize(pair.second, stream);
            }
        }
    };

    /**
     * @brief Specialization of `traits` for `std::map<Key, Value>`, providing JSON serialization.
     *
     * @tparam Key The key type of the std::map.
     * @tparam Value The mapped type of the std::map.
     */
    template <typename Key, typename Value>
    struct traits<std::map<Key, Value>>
    {
        static inline void serialize(const std::map<Key, Value>& map, container::buffer& stream)
        {
            stream.append("{", 1);
            map_serializer<std::map<Key, Value>>::serialize(map, stream);
            stream.append("}", 1);
        }
    };

    template <typename Key, typename Value>
    struct traits<const std::map<Key, Value>>
    {
        static inline void serialize(const std::map<Key, Value>& map, container::buffer& stream)
        {
            stream.append("{", 1);
            map_serializer<const std::map<Key, Value>>::serialize(map, stream);
            stream.append("}", 1);
        }
    };
}
