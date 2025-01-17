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
#include <cbeam/serialization/traits.hpp> // for cbeam::serialization::traits, cbeam::serialization::serialized_object

namespace cbeam::container
{
    class buffer;
}

namespace cbeam::serialization
{
    /**
     * @brief Provides serialization and deserialization logic for `nested_map<Key, Value>`.
     *
     * A nested map has `data` (a standard map of Key to Value) plus `sub_tables`
     * (a map of Key to further nested_map). Both must be recursively serialized.
     */
    template <typename Map>
    struct nested_map_serializer
    {
        /**
         * @brief Serializes a nested map by serializing `map.data` and `map.sub_tables`, appending to `stream`.
         *
         * @param map The nested_map to serialize.
         * @param stream The buffer to which the serialized data is appended.
         */
        static void serialize(const Map& map, container::buffer& stream)
        {
            traits<decltype(map.data)>::serialize(map.data, stream);
            traits<decltype(map.sub_tables)>::serialize(map.sub_tables, stream);
        }

        /**
         * @brief Deserializes the content for `map.data` and `map.sub_tables` from the buffer, replacing `map`’s contents.
         *
         * @param it The position in the buffer from which to read.
         * @param map The nested_map object to populate.
         */
        static void deserialize(serialized_object& it, Map& map)
        {
            map.clear();

            traits<decltype(map.data)>::deserialize(it, map.data);
            traits<decltype(map.sub_tables)>::deserialize(it, map.sub_tables);
        }
    };

    /**
     * @brief Specialization of `traits` for `cbeam::container::nested_map<Key, Value>`.
     *
     * Uses `nested_map_serializer` for serialization and deserialization.
     *
     * @tparam Key   The key type of the nested map.
     * @tparam Value The value type of the nested map.
     */
    template <typename Key, typename Value>
    struct traits<cbeam::container::nested_map<Key, Value>>
    {
        static inline void serialize(const cbeam::container::nested_map<Key, Value>& map, container::buffer& stream)
        {
            nested_map_serializer<cbeam::container::nested_map<Key, Value>>::serialize(map, stream);
        }

        static inline void deserialize(serialized_object& it, cbeam::container::nested_map<Key, Value>& map)
        {
            nested_map_serializer<cbeam::container::nested_map<Key, Value>>::deserialize(it, map);
        }
    };

    /**
     * @brief Specialization of `traits` for `const cbeam::container::nested_map<Key, Value>` supporting serialization only.
     */
    template <typename Key, typename Value>
    struct traits<const cbeam::container::nested_map<Key, Value>>
    {
        static inline void serialize(const cbeam::container::nested_map<Key, Value>& map, container::buffer& stream)
        {
            nested_map_serializer<const cbeam::container::nested_map<Key, Value>>::serialize(map, stream);
        }
    };
}
