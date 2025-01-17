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

#include <cbeam/container/stable_interprocess_container.hpp> // for cbeam::container::stable_interprocess_container
#include <cbeam/error/out_of_range.hpp>                      // for cbeam::error:out_of_range
#include <cbeam/error/runtime_error.hpp>                     // for cbeam::error:runtime_error
#include <cbeam/serialization/map.hpp>                       // IWYU pragma: keep (required for serialization of the template type of stable_interprocess_container, which is a std::map)

#include <cstddef> // for std::size_t

#include <functional>       // for std::function
#include <initializer_list> // for std::initializer_list
#include <map>              // for std::allocator, std::map
#include <string>           // for std::string
#include <utility>          // for std::pair

namespace cbeam::container
{
    /**
     * @class stable_interprocess_map
     * @brief Provides a type-safe, interprocess map with stable serialization.
     * @details This template class extends the `stable_interprocess_container` to provide a map-like container
     * that can be safely used across different processes and shared library boundaries. It inherits the stability
     * and consistency features of `stable_interprocess_container`, making it suitable for interprocess communication
     * and data sharing in a multi-library environment.
     *
     * Serialization and deserialization of the map are automatically handled to ensure data integrity and consistency
     * when shared across different compilers or compiler versions.
     *
     * @tparam Key The type of the keys in the map.
     * @tparam Value The type of the values in the map.
     */
    template <typename Key, typename Value>
    class stable_interprocess_map : public stable_interprocess_container<std::map<Key, Value>>
    {
    public:
        /**
         * @brief Constructs a `stable_interprocess_map` with a unique identifier and fixed size.
         * @param unique_identifier A unique identifier for the shared memory segment.
         * @param size The size of the shared memory segment, in bytes. This size determines the capacity
         *             of the map and cannot be altered after initialization.
         */
        stable_interprocess_map(const std::string& unique_identifier, std::size_t size)
            : stable_interprocess_container<std::map<Key, Value>>(unique_identifier, size)
        {
        }

        /**
         * @brief Assigns key-value pairs to the map from an initializer list.
         * @param list An initializer list of key-value pairs.
         * @return Reference to this `stable_interprocess_map` object.
         */
        stable_interprocess_map& operator=(const std::initializer_list<std::pair<const Key, Value>>& list)
        {
            std::map<Key, Value> local_instance;

            for (const auto& item : list)
            {
                local_instance[item.first] = item.second;
            }

            auto lock = this->get_lock_guard();
            this->serialize(local_instance);
            return *this;
        }

        /**
         * @brief Retrieves the value associated with a specific key.
         * @param key The key to look up.
         * @return The value associated with the key.
         * @throw std::out_of_range if the key is not found.
         */
        Value at(const Key& key) const
        {
            std::map<Key, Value> local_instance;
            {
                auto lock = this->get_lock_guard();

                local_instance = this->deserialize();
            }

            try
            {
                return local_instance.at(key);
            }
            catch (const std::out_of_range& ex)
            {
                throw cbeam::error::out_of_range(ex.what());
            }
        }

        /**
         * @brief Retrieves the value associated with a key, or a default value if the key is not found.
         * @param key The key to look up.
         * @param default_value The default value to return if the key is not found.
         * @return The value associated with the key, or the default value.
         */
        Value at_or_default(const Key& key, const Value& default_value) const
        {
            std::map<Key, Value> local_instance;
            {
                auto lock = this->get_lock_guard();

                local_instance = this->deserialize();
            }

            return local_instance.count(key) == 1 ? local_instance.at(key) : default_value;
        }

        /**
         * @brief Inserts a key-value pair into the map.
         * @param key The key of the element to insert.
         * @param value The value to be associated with the key.
         */
        void insert(const Key& key, const Value& value)
        {
            auto lock = this->get_lock_guard();

            auto local_instance = this->deserialize();
            local_instance[key] = value;
            this->serialize(local_instance);
        }

        /**
         * @brief Erases the element associated with a specific key.
         * @param key The key of the element to be erased.
         */
        void erase(const Key& key)
        {
            auto lock = this->get_lock_guard();

            auto local_instance = this->deserialize();
            local_instance.erase(key);
            this->serialize(local_instance);
        }

        /**
         * @brief Counts the number of elements with a specific key.
         * @param key The key to count in the map.
         * @return The number of elements with the specified key.
         */
        std::size_t count(const Key& key) const
        {
            auto lock = this->get_lock_guard();

            return this->deserialize().count(key);
        }

        /**
         * @brief Updates an existing entry in the map or inserts a new one if the key does not exist.
         *
         * This method abstracts the common pattern of checking for an existing key, updating its value if present,
         * or inserting a new key-value pair if the key is not found. The method handles locking, deserialization,
         * and serialization of the map internally.
         *
         * @param key The key to update or insert.
         * @param updater A function that updates the value associated with the key. It takes a reference to the value type.
         * @param default_value The default value to insert if the key is not found in the map.
         *
         * @example cbeam/container/stable_interprocess_map.hpp
         * @code{.cpp}
         * stable_interprocess_map<int, std::string> myMap("my_shared_memory", 1024);
         * myMap.update_or_insert(42, [](std::string& value) { value += " updated"; }, "default value");
         * @endcode
         */
        void update_or_insert(const Key& key, std::function<void(Value&)> updater, const Value& default_value)
        {
            auto lock           = this->get_lock_guard();
            auto local_instance = this->deserialize();

            if (local_instance.find(key) != local_instance.end())
            {
                updater(local_instance[key]);
            }
            else
            {
                local_instance[key] = default_value;
            }

            this->serialize(local_instance);
        }

        /**
         * @brief Updates the value of an existing entry in the map.
         *
         * This method updates the value of an existing key in the map. If the key does not exist, it throws an exception.
         * It handles locking, deserialization, and serialization of the map internally.
         *
         * @param key The key of the entry to update.
         * @param updater A function that updates the value associated with the key. It takes a reference to the value type.
         * @param error_string An optional string that will be used for the exception that is thrown if the key does not exist in the map.
         * @return a const reference to the updated value
         *
         * @exception cbeam::error::runtime_error Thrown if the key does not exist in the map.
         *
         * @example cbeam/container/stable_interprocess_map.hpp
         * stable_interprocess_map<int, std::string> myMap("my_shared_memory", 1024);
         * myMap.update(42, [](std::string& value) { value += " updated"; });
         */
        const Value update(const Key& key, std::function<void(Value&)> updater, const std::string& error_string = {"cbeam::stable_interprocess_map::update: key not found"})
        {
            auto lock           = this->get_lock_guard();
            auto local_instance = this->deserialize();

            if (local_instance.find(key) == local_instance.end())
            {
                throw cbeam::error::runtime_error(error_string);
            }

            updater(local_instance[key]);
            this->serialize(local_instance);
            return local_instance.at(key);
        }
    };
}
