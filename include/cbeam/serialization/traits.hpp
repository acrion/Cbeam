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

#include <cbeam/container/buffer.hpp> // for cbeam::container::buffer

#include <cstdint> // for uint8_t
#include <cstring> // for memcpy, std::memcpy

#include <type_traits> // for std::is_standard_layout, std::is_trivial

namespace cbeam::serialization
{
    /// \brief Represents a serialized value in memory.
    ///
    /// \details This type alias defines a generic pointer to represent serialized data in memory.
    ///          It is used as a handle to manage and manipulate serialized objects
    ///          within the cbeam framework.
    using serialized_object = void*;

    /// \brief Defines the traits required for serializing and deserializing objects of type T.
    ///
    /// \details This struct declares static methods as templates for serializing and deserializing objects.
    ///          The default implementation is applicable for types that are both standard-layout and trivial.
    ///          For types that do not meet these criteria, a specialized implementation of these methods must be provided
    ///          to accommodate type-specific serialization behavior.
    template <typename T>
    struct traits
    {
        static_assert(std::is_standard_layout<T>::value && std::is_trivial<T>::value,
                      "A specialization of cbeam::serialization::traits must be implemented for types that are not both standard-layout and trivial.");

        /// \brief Required to serialize an object of type T into a shared_buffer stream.
        ///
        /// \param val The object to serialize.
        /// \param stream The shared_buffer stream into which the object is serialized.
        ///
        /// \details This method should take an object of type T and serialize it into a `container::buffer` stream.
        ///          by using its `append` method.
        static inline void serialize(const T& val, container::buffer& stream)
        {
            // implementation for types that are both standard-layout and trivial
            stream.append(reinterpret_cast<const char*>(&val), sizeof(val));
        }

        /// \brief Required to deserialize an object of type T from a serialized memory block, incrementing the iterator `it`.
        ///
        /// \param it A reference to a pointer to the serialized memory block.
        /// \param val A reference to the object of type T, where the deserialized data will be stored.
        ///
        /// \details This method should deserialize an object of type T from a serialized memory block,
        ///          incrementing the iterator `it` to point to the next part of the serialized data.
        ///          It should handle different types of data, managing memory and type conversion as needed.
        static inline void deserialize(serialized_object& it, T& val)
        {
            // implementation for types that are both standard-layout and trivial
            uint8_t* local_it = reinterpret_cast<uint8_t*>(it);
            std::memcpy(&val, local_it, sizeof(val));
            local_it += sizeof(val);
            it = local_it;
        }
    };
}
