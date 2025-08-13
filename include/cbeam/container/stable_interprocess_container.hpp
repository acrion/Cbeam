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

#include <cbeam/container/buffer.hpp>                  // for cbeam::container::buffer
#include <cbeam/error/runtime_error.hpp>               // for cbeam::error:runtime_error
#include <cbeam/logging/log_manager.hpp>               // for CBEAM_LOG
#include <cbeam/memory/interprocess_shared_memory.hpp> // for cbeam::memory::interprocess_shared_memory
#include <cbeam/serialization/traits.hpp>              // for cbeam::serialization::traits, cbeam::serialization::serialized_object

#include <cstring> // for memcpy, size_t, std::memcpy, std::size_t
#include <string>  // for std::allocator, std::operator+, std::string_literals::operator""s, std::char_traits, std::to_string, std::string, std::string_literals

namespace cbeam::container
{
    /**
     * @class stable_interprocess_container
     * @brief Manages type-safe, interprocess container operations with stable serialization.
     * @details This template class, inheriting from cbeam::memory::interprocess_shared_memory, is specifically designed
     * to provide a stable and consistent framework for managing shared data structures across various shared libraries,
     * even when compiled with different compilers or versions. The 'stable' aspect of this container relates to its capability
     * for automatic serialization and deserialization, ensuring that the data remains coherent and reliable, regardless of
     * the environment it operates in. This feature is particularly crucial in multi-library environments where binary
     * format inconsistencies are common.
     *
     * The container is intended as a base class for developing specialized container types, such as `cbeam::container::stable_interprocess_map`.
     * It facilitates seamless and efficient shared data management in interprocess and multi-library contexts. The class ensures
     * that the size of the shared memory segment remains constant once initialized, preventing any runtime size modifications.
     *
     * Serialization traits for basic data types are provided by default. For specialized data types, custom serialization traits
     * must be implemented based on `cbeam::serialization::traits` to handle specific serialization behaviors.
     *
     * - **Stable Serialization/Deserialization**: Offers robust serialization mechanisms to maintain data integrity across different
     *   compiler binaries, ensuring stable data exchange in interprocess environments.
     * - **Base Class for Specialized Containers**: Serves as a foundational class for developing specialized container types
     *   that require interprocess communication capabilities.
     * - **Type Safety and Flexibility**: As a template class, it allows for type-safe operations on various data types, making it
     *   versatile for a range of applications.
     *
     * ### Usage
     * This class is essential for applications involving complex, multi-library interactions where shared data structures need to
     * be managed efficiently and consistently, especially in environments with diverse compiler tools and versions.
     *
     * @tparam T The type of elements to be stored in the container. Requires implementation of serialization traits if T is not
     *           a basic data type.
     */
    template <typename T>
    class stable_interprocess_container : public memory::interprocess_shared_memory
    {
    public:
        /**
         * @brief Constructs a `stable_interprocess_container` with a specific size and unique identifier.
         * @details This constructor initializes a container designed for interprocess communication, providing
         * automatic serialization and deserialization to ensure stable operations across different shared library contexts.
         * The term 'stable' refers to the container's ability to maintain data integrity and consistency through these
         * serialization processes. Basic serialization traits for fundamental data types are inherently supported. For
         * specialized data types, additional serialization traits need to be defined, adhering to the structure
         * provided by `cbeam::serialization::traits`.
         *
         * The class is intended as a base class for specialized container types, such as `cbeam::container::stable_interprocess_map`,
         * enabling them to efficiently manage shared data in a multi-library environment. The size of the shared memory
         * segment, once set during construction, remains constant throughout the lifecycle of the container.
         *
         * @param unique_identifier A string representing a unique identifier for the shared memory segment. This identifier
         *                          is essential for distinguishing between different shared memory segments, especially in
         *                          environments with multiple libraries or processes.
         * @param size The fixed size of the shared memory segment, specified in bytes. This size determines the capacity
         *             of the container and cannot be altered after initialization.
         */
        stable_interprocess_container(const std::string& unique_identifier, std::size_t size)
            : interprocess_shared_memory(unique_identifier, size)
        {
        }

        stable_interprocess_container(const stable_interprocess_container&)            = delete;
        stable_interprocess_container& operator=(const stable_interprocess_container&) = delete;
        stable_interprocess_container(stable_interprocess_container&&)                 = delete;
        stable_interprocess_container& operator=(stable_interprocess_container&&)      = delete;

        /**
         * @brief Clears the contents of the container.
         * @details This method clears all the elements in the container, effectively resetting its content to an empty state.
         * The operation is performed in a thread-safe manner using a lock guard to ensure exclusive access during the modification.
         * This is essential in an interprocess or multithreaded environment to maintain data consistency and prevent concurrent access issues.
         */
        virtual void clear()
        {
            auto lock = get_lock_guard(); // Locks the mutex to ensure exclusive access
            serialize(T{});               // Resets the container by serializing an empty instance of T
        }

        /**
         * @brief Checks if the container is empty.
         * @return A boolean value indicating whether the container is empty.
         * @details This method returns true if the container is empty, and false otherwise. The check is performed under a lock guard to ensure
         * thread-safe access to the container's state. This approach is crucial in environments with concurrent access to the container,
         * as it maintains data integrity and consistency.
         */
        virtual bool empty() const
        {
            auto lock = get_lock_guard(); // Locks the mutex for safe access
            return deserialize().empty(); // Deserializes the container and checks if it's empty
        }

        /**
         * @brief Retrieves the size of the container.
         * @return The number of elements in the container.
         * @details This method returns the number of elements currently held in the container. The operation is carried out under a lock guard
         * to ensure thread-safe access in an interprocess or multithreaded context. This secure approach is essential for maintaining the accuracy
         * and consistency of the size information in a shared environment.
         */
        virtual size_t size() const
        {
            auto lock = get_lock_guard();
            return deserialize().size();
        }

        /**
         * @brief Iterates over the elements of the container and performs an action.
         * @param func A function to be applied to each element in the container.
         * @details This method iterates over each element in the container and applies the provided function `func` to it.
         * The iteration is safely encapsulated within the method, mitigating the need for external synchronization like lock guards.
         * This design choice eliminates the risks associated with exposing begin() and end() iterators, which could lead to inconsistent
         * states if the container's end() changes due to external modifications in a concurrent environment.
         *
         * ### Example
         * \code{.cpp}
         * stable_interprocess_container<MyType> container("identifier", size);
         * container.foreach([](const MyType& item) {
         *     // Perform actions with item
         *     return true; // Continue iterating
         * });
         * \endcode
         */
        template <typename Func>
        void foreach (Func func)
        {
            T local_instance;
            {
                auto lock      = get_lock_guard();
                local_instance = deserialize();
            }

            for (const auto& item : local_instance)
            {
                if (!func(item))
                {
                    break;
                }
            }
        }

    protected:
        /**
         * @brief Deserializes the shared memory data into a container object.
         * @return A deserialized instance of the container.
         * @details This protected method deserializes the data stored in the shared memory into a container object of type T.
         * It is used internally by various public methods to safely access and manipulate the container's data.
         * The method ensures that the data integrity is maintained during the deserialization process, which is crucial in an interprocess context.
         */
        T deserialize() const
        {
            cbeam::serialization::serialized_object it = data();

            if (it)
            {
                T result;
                cbeam::serialization::traits<T>::deserialize(it, result);
                return result;
            }
            else
            {
                return {};
            }
        }

        /**
         * @brief Serializes a container object and stores it in shared memory.
         * @param container The container object to be serialized.
         * @details This protected method serializes the provided container object and stores the serialized data in the shared memory.
         * It ensures that the size of the serialized data does not exceed the capacity of the shared memory. If it does, an exception is thrown.
         * This method is crucial for maintaining the integrity and consistency of the container's data in an interprocess environment.
         *
         * @throw cbeam::error::runtime_error if the serialized size exceeds the shared memory capacity.
         */
        void serialize(const T& container)
        {
            using namespace std::string_literals;

            container::buffer buffer;
            cbeam::serialization::traits<const T>::serialize(container, buffer);

            if (buffer.size() > capacity())
            {
                std::string errorMessage = "cbeam::stable_interprocess_container::serialize: size of serialized container ("s + std::to_string(buffer.size())
                                         + " bytes) exceeds shared memory size ("s + std::to_string(capacity()) + " bytes). Set environment variable CBEAM_SRB_MAP_BYTES to configure a higher value."s;
                CBEAM_LOG(errorMessage);
                throw cbeam::error::runtime_error(errorMessage);
            }
            std::memcpy(data(), buffer.get(), buffer.size());
        }
    };
}
