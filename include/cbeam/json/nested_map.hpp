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

#include <cbeam/container/nested_map.hpp>
#include <cbeam/json/traits.hpp> // for cbeam::json::traits, cbeam::json::serialized_object

namespace cbeam::container
{
    class buffer;
}

namespace cbeam::json
{
    /**
     * @brief Generic serializer for `nested_map<Key, Value>` types.
     *
     * The serializer writes each entry from `map.data` and recursively
     * serializes any `map.sub_tables`. The result is appended to a buffer
     * in a JSON-like style (braces, commas, etc.).
     *
     * @tparam Map The type of the nested map (e.g., `cbeam::container::nested_map<Key, Value>`).
     */
    template <typename Map>
    struct nested_map_serializer
    {
        /**
         * @brief Serializes a nested map to JSON-like text, appending it to a buffer.
         *
         * @param map The nested map to serialize.
         * @param stream The buffer where the serialized output is written.
         */
        static void serialize(const Map& map, container::buffer& stream)
        {
            // We assume Map = cbeam::container::nested_map<Key, Value>.
            using Key   = typename Map::key_type;
            using Value = typename Map::mapped_type;

            // Start JSON object
            stream.append("{", 1);

            bool first = true;

            // 1) Serialize all entries in map.data
            for (const auto& [k, v] : map.data)
            {
                if (!first)
                {
                    stream.append(",", 1);
                }
                else
                {
                    first = false;
                }

                // Key
                traits<Key>::serialize(k, stream);
                stream.append(":", 1);

                // Value
                traits<Value>::serialize(v, stream);
            }

            // 2) Serialize all entries in map.sub_tables
            for (const auto& [subkey, submap] : map.sub_tables)
            {
                if (!first)
                {
                    stream.append(",", 1);
                }
                else
                {
                    first = false;
                }

                // Key
                traits<Key>::serialize(subkey, stream);
                stream.append(":", 1);

                // Recursively serialize
                nested_map_serializer<decltype(submap)>::serialize(submap, stream);
            }

            // End JSON object
            stream.append("}", 1);
        }
    };

    /**
     * @brief Specialization of `traits` for nested_map<Key, Value>, enabling JSON-like serialization.
     *
     * @tparam Key The key type of the nested map.
     * @tparam Value The value type of the nested map.
     */
    template <typename Key, typename Value>
    struct traits<cbeam::container::nested_map<Key, Value>>
    {
        static inline void serialize(const cbeam::container::nested_map<Key, Value>& map,
                                     container::buffer&                              stream)
        {
            nested_map_serializer<cbeam::container::nested_map<Key, Value>>::serialize(map, stream);
        }
    };

    template <typename Key, typename Value>
    struct traits<const cbeam::container::nested_map<Key, Value>>
    {
        static inline void serialize(const cbeam::container::nested_map<Key, Value>& map,
                                     container::buffer&                              stream)
        {
            nested_map_serializer<const cbeam::container::nested_map<Key, Value>>::serialize(map, stream);
        }
    };
}
