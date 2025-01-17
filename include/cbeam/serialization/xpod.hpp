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
#include <cbeam/error/runtime_error.hpp>  // for cbeam::error:runtime_error
#include <cbeam/memory/pointer.hpp>       // for cbeam::memory::pointer
#include <cbeam/serialization/traits.hpp> // for cbeam::serialization::serialized_object

#include <cassert> // for assert
#include <cstdint> // for uint8_t
#include <cstring> // for memcpy, std::memcpy

#include <string>  // for std::string
#include <variant> // for std::get

namespace cbeam::container
{
    class buffer;
}

namespace cbeam::serialization
{
    /**
     * @brief Specialization of `traits` for `cbeam::container::xpod::type`.
     *
     * The variant can hold: `long long`, `double`, `bool`, `memory::pointer`, or `std::string`.
     * During serialization, we first store the variant index (1 byte),
     * then the corresponding data in a type-dependent format.
     */
    template <>
    struct traits<container::xpod::type>
    {
        /**
         * @brief Serialize the given xpod::type variant into the buffer `stream`.
         *
         * The serialization stores:
         *   1) A single byte (valIndex) representing which type is active.
         *   2) The associated data for that type.
         *
         * @param val The variant to serialize.
         * @param stream The buffer to which data is appended.
         */
        static inline void serialize(const container::xpod::type& val, container::buffer& stream)
        {
            assert(val.index() >= 0 && val.index() <= 0xff);

            char valIndex = (char)val.index();

            stream.append(&valIndex, 1);

            switch (val.index())
            {
            case container::xpod::type_index::integer:
            {
                const auto intValue = std::get<container::xpod::type_index::integer>(val);
                stream.append(reinterpret_cast<const char*>(&intValue), sizeof(intValue));
                break;
            }
            case container::xpod::type_index::number:
            {
                const auto doubleValue = std::get<container::xpod::type_index::number>(val);
                stream.append(reinterpret_cast<const char*>(&doubleValue), sizeof(doubleValue));
                break;
            }
            case container::xpod::type_index::boolean:
            {
                const auto boolValue = std::get<container::xpod::type_index::boolean>(val);
                stream.append(reinterpret_cast<const char*>(&boolValue), sizeof(boolValue));
                break;
            }
            case container::xpod::type_index::pointer:
            {
                // For pointers, we serialize them as a string in hex representation.
                const auto strValue = (std::string)std::get<container::xpod::type_index::pointer>(val);
                const auto strLen   = strValue.size();
                stream.append(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
                stream.append(strValue.data(), strLen);
                break;
            }
            case container::xpod::type_index::string:
            {
                const auto strValue = std::get<container::xpod::type_index::string>(val);
                const auto strLen   = strValue.size();
                stream.append(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
                stream.append(strValue.data(), strLen);
                break;
            }
            default:
                break;
            }
        }

        /**
         * @brief Deserialize an xpod::type variant from the buffer pointed to by `it`.
         *
         * Reads a single byte to determine the active index, then reads the corresponding
         * data into `val`. Advances `it` to point past the consumed data.
         *
         * @param it The iterator (raw pointer) position in the serialized data.
         * @param val The xpod::type variant to populate.
         * @throws cbeam::error::runtime_error if the variant index is invalid.
         */
        static inline void deserialize(serialized_object& it, container::xpod::type& val)
        {
            uint8_t* localIt = (uint8_t*)it;

            const unsigned long valIndex = *localIt++;
            switch (valIndex)
            {
            case container::xpod::type_index::integer:
            {
                long long intValue;
                std::memcpy(&intValue, localIt, sizeof(intValue));
                val = intValue;
                localIt += sizeof(intValue);
                break;
            }
            case container::xpod::type_index::number:
            {
                double doubleValue;
                std::memcpy(&doubleValue, localIt, sizeof(doubleValue));
                val = doubleValue;
                localIt += sizeof(doubleValue);
                break;
            }
            case container::xpod::type_index::boolean:
            {
                bool boolValue;
                std::memcpy(&boolValue, localIt, sizeof(boolValue));
                val = boolValue;
                localIt += sizeof(boolValue);
                break;
            }
            case container::xpod::type_index::pointer:
            {
                size_t strLen;
                std::memcpy(&strLen, localIt, sizeof(strLen));
                localIt += sizeof(strLen);
                val = cbeam::memory::pointer(std::string(localIt, localIt + strLen));
                localIt += strLen;
                break;
            }
            case container::xpod::type_index::string:
            {
                size_t strLen;
                std::memcpy(&strLen, localIt, sizeof(strLen));
                localIt += sizeof(strLen);
                val = std::string(localIt, localIt + strLen);
                localIt += strLen;
                break;
            }
            default:
                throw cbeam::error::runtime_error("cbeam::serialization::deserialize: invalid ByteStream");
                break;
            }

            it = localIt;
        }
    };
}
