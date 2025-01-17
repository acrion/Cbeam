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

#include <cbeam/container/stable_reference_buffer.hpp> // for cbeam::container::stable_reference_buffer
#include <cbeam/logging/log_manager.hpp>               // for CBEAM_LOG
#include <cbeam/serialization/traits.hpp>              // for cbeam::serialization::serialized_object, cbeam::serialization::traits

#include <cassert> // for assert

#include <exception> // for std::exception
#include <string>    // for std::string_literals::operator""s, std::operator+, std::string_literals

namespace cbeam::serialization
{
    /**
     * @brief Serializes an object of type T, returning a pointer to the serialized data.
     * @details This function serializes the given instance of type T into a `stable_reference_buffer`.
     * It returns a container::stable_reference_buffer to the serialized data. In case of an error
     * it will be logged and an empty stream will be returned.
     *
     * @tparam T The type of the object to serialize.
     * @param instance The instance of type T to serialize.
     * @return container::stable_reference_buffer The serialized data.
     */
    template <typename T>
    static inline container::stable_reference_buffer serialize(const T& instance) noexcept
    {
        using namespace std::string_literals;
        container::stable_reference_buffer byte_stream;

        try
        {
            traits<T>::serialize(instance, byte_stream);
        }
        catch (const std::exception& ex)
        {
            CBEAM_LOG("cbeam::serialization: Exception while serializing: "s + ex.what());
        }
        catch (...)
        {
            CBEAM_LOG("cbeam::serialization: Unknown exception while serializing"s);
        }

        return byte_stream;
    }

    /**
     * @brief Deserializes data into an object of type T.
     * @details This function takes a reference to a serialized object and deserializes it into an instance of type T.
     * It uses the `traits<T>::deserialize` method to convert the serialized data back into the object.
     * Afterwards, the serialized_object it will be incremented and point to the next object in the buffer.
     *
     * @tparam T The type of object to deserialize.
     * @param it Reference to the serialized data.
     * @return T An instance of type T constructed from the serialized data.
     */
    template <typename T>
    static inline T deserialize(serialized_object& it)
    {
        T result;
        traits<T>::deserialize(it, result);
        return result;
    }

    /**
     * @brief Deserializes data into an object of type T.
     * @details Similar to the other `deserialize` function, but takes a const reference to the serialized object.
     * This overload allows deserialization from a const serialized object, creating a temporary copy of the iterator
     * for the deserialization process. Therefore, after deserialization, `it` will not point to the next object in the buffer.
     *
     * @tparam T The type of object to deserialize.
     * @param it Constant reference to the serialized data.
     * @return T An instance of type T constructed from the serialized data.
     */
    template <typename T>
    static inline T deserialize(const serialized_object& it)
    {
        T                 result;
        serialized_object it2 = it;
        traits<T>::deserialize(it2, result);
        return result;
    }
}
