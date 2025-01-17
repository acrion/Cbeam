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

#include <cbeam/container/buffer.hpp>     // for buffer (ptr only), cbeam::container::buffer
#include <cbeam/serialization/traits.hpp> // for cbeam::serialization::traits, cbeam::serialization::serialized_object

#include <cstdint> // for uint8_t
#include <cstring> // for memcpy, size_t, std::memcpy

#include <map> // for std::map

namespace cbeam::serialization
{
    /**
     * @brief Provides serialization and deserialization logic for standard map-like containers.
     *
     * @tparam Map A map-like container (e.g., `std::map<Key,Value>`) that supports
     * iteration via `begin()` and `end()`, and insertion via `insert(...)`.
     */
    template <typename Map>
    struct map_serializer
    {
        /**
         * @brief Serializes all key-value pairs of the map into a binary format appended to `stream`.
         *
         * The first thing written is the map size (number of elements), followed by
         * each key and value serialized using `traits`.
         *
         * @param map The map to serialize.
         * @param stream A buffer to which the map data is appended.
         */
        static void serialize(const Map& map, container::buffer& stream)
        {
            size_t size = map.size();
            stream.append(reinterpret_cast<const char*>(&size), sizeof(size));

            for (const auto& pair : map)
            {
                traits<typename Map::key_type>::serialize(pair.first, stream);
                traits<typename Map::mapped_type>::serialize(pair.second, stream);
            }
        }

        /**
         * @brief Deserializes map data from the buffer into a `Map` instance, replacing its current contents.
         *
         * The number of elements is read first, then each key-value pair is deserialized
         * and inserted into the map. The iterator `it` is advanced accordingly.
         *
         * @param it A reference to a serialized_object pointer indicating where to read from.
         * @param map The map object to populate with deserialized data.
         */
        static void deserialize(serialized_object& it, Map& map)
        {
            map.clear();

            size_t   size;
            uint8_t* localIt = reinterpret_cast<uint8_t*>(it);
            std::memcpy(&size, localIt, sizeof(size));
            localIt += sizeof(size);
            it = localIt;

            for (size_t i = 0; i < size; ++i)
            {
                typename Map::key_type    key;
                typename Map::mapped_type value;
                traits<typename Map::key_type>::deserialize(it, key);
                traits<typename Map::mapped_type>::deserialize(it, value);
                map.insert({key, value});
            }
        }
    };

    /**
     * @brief Specialization of `traits` for `std::map<Key, Value>` providing serialization/deserialization.
     *
     * Uses `map_serializer<std::map<Key,Value>>` under the hood.
     *
     * @tparam Key   The key type of the std::map.
     * @tparam Value The mapped type of the std::map.
     */
    template <typename Key, typename Value>
    struct traits<std::map<Key, Value>>
    {
        static inline void serialize(const std::map<Key, Value>& map, container::buffer& stream)
        {
            map_serializer<std::map<Key, Value>>::serialize(map, stream);
        }

        static inline void deserialize(serialized_object& it, std::map<Key, Value>& map)
        {
            map_serializer<std::map<Key, Value>>::deserialize(it, map);
        }
    };

    /**
     * @brief Specialization of `traits` for `const std::map<Key, Value>` supporting serialization only.
     */
    template <typename Key, typename Value>
    struct traits<const std::map<Key, Value>>
    {
        static inline void serialize(const std::map<Key, Value>& map, container::buffer& stream)
        {
            map_serializer<const std::map<Key, Value>>::serialize(map, stream);
        }
    };
}
