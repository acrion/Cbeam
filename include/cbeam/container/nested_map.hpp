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

// no internal headers except runtime_error.hpp should be included here
#include <cbeam/error/runtime_error.hpp> // for cbeam::error:runtime_error

#include <cstddef> // for std::size_t

#include <map> // for std::map
#include <memory>
#include <string>      // for std::operator""s, std::string, std::string_literals
#include <type_traits> // for std::enable_if, std::is_same
#include <variant>     // for std::get_if

namespace cbeam::container
{
    /// \brief A map structure that can store nested maps of keys and values. By including serialization/nested_map.hpp, it gains serialization capability.
    template <typename Key, typename Value>
    struct nested_map
    {
        using table_of_values = std::map<Key, Value>;                  ///< A table mapping keys to values, capable of storing the actual data elements for serialization.
        using nested_tables   = std::map<Key, nested_map<Key, Value>>; ///< A map of keys to nested `nested_map` instances, allowing hierarchical data organization.
        using key_type        = Key;
        using mapped_type     = Value;

        nested_map() = default; ///< construct empty table

        nested_map(std::initializer_list<std::pair<const Key, Value>> init)
        {
            for (const auto& [key, value] : init)
            {
                data[key] = value;
            }
        }

        table_of_values data;       ///< key value pairs of Value instances that store the actual data
        nested_tables   sub_tables; ///< list of \ref nested_map "sub tables", each having a unique name

        /// \brief overwrite this \ref nested_map with a copy of the other
        nested_map& operator=(const nested_map& other)
        {
            if (this == &other)
            {
                return *this;
            }

            clear();
            merge(other);

            return *this;
        }

        /// \brief merges the other \ref nested_map into this
        void merge(const nested_map& other)
        {
            for (const auto& it : other.data)
            {
                data[it.first] = it.second;
            }

            for (const auto& it : other.sub_tables)
            {
                sub_tables[it.first] = it.second;
            }
        }

        void clear()
        {
            data.clear();
            sub_tables.clear();
        }

        /**
         * @brief Retrieves a value of type T associated with a given key from the map's values.
         *
         * This function template fetches the value of type T associated with the specified key in the nested_map.
         * If the key is not found in the map, a default-constructed value of type T is returned.
         *
         * @tparam T The type to be retrieved from the map's value. If the map uses std::variant as the value type,
         *           T is interpreted as the type to retrieve from the std::variant.
         * @param key The key whose value is to be retrieved.
         * @return The value of type T associated with the key if it exists, or a default-constructed value of type T if the key is not found.
         */
        template <typename T>
        typename std::enable_if<std::is_same<T, Value>::value, T>::type
        get_mapped_value_or_default(const Key& key) const
        {
            auto it = data.find(key);

            if (it == data.end())
            {
                return {};
            }
            else
            {
                return it->second;
            }
        }

        /**
         * @brief Retrieves a value of type T associated with a given key from the map's values.
         *
         * This function template fetches the value of type T associated with the specified key in the nested_map. If the map's
         * value type is std::variant, T is interpreted as the specific type to be extracted from the std::variant.
         * If the key is not found in the map, a default-constructed value of type T is returned.
         *
         * @tparam T The type to be retrieved from the map's value. If T is not the value type of the nested_map, T is interpreted as the type to retrieve from the std::variant.
         * @param key The key whose value is to be retrieved.
         * @return The value of type T associated with the key if it exists, or a default-constructed value of type T if the key is not found.
         */
        template <typename T>
        typename std::enable_if<!std::is_same<T, Value>::value, T>::type
        get_mapped_value_or_default(const Key& value) const
        {
            auto it = data.find(value);

            if (it == data.end())
            {
                return {};
            }
            else
            {
                return get_value_or_default<T>(it->second);
            }
        }

        /**
         * @brief Retrieves a value from a std::variant associated with a given key, based on a type index.
         *
         * This function template is specialized for maps where the value type is std::variant. It fetches the value
         * associated with the specified key and extracts the value at the specified type index within the std::variant.
         * If the key is not found in the map, a default-constructed value of the type at the given index in the std::variant is returned.
         *
         * @tparam T The type index within the std::variant from which to retrieve the value.
         * @param key The key whose value, a std::variant, is to be retrieved and inspected.
         * @return The value at the specified type index within the std::variant if the key exists; otherwise, a default-constructed value of the corresponding type.
         */
        template <std::size_t T>
        auto get_mapped_value_or_default(const Key& value) const
        {
            auto it = data.find(value);

            if (it == data.end())
            {
                return decltype(get_value_or_default<T>(it->second)){};
            }
            else
            {
                return get_value_or_default<T>(it->second);
            }
        }

        /**
         * @brief Retrieves the value associated with a given key, throwing an exception if the key is not found.
         *
         * This function template attempts to retrieve the value associated with the specified key in the nested_map.
         * This template specialization is used if T is the Value type. if the key is found, it returns the associated value, otherwise  an exception is thrown.
         *
         * @tparam T The type of the value to be retrieved.
         * @param key The key whose value is to be retrieved.
         * @param error_msg Optional custom error message for the exception. If empty, a default message is used.
         * @return The value associated with the key of type T.
         * @throws cbeam::error::runtime_error if the key is not found or if the type T does not match the value's type.
         */
        template <typename T>
        typename std::enable_if<std::is_same<T, Value>::value, T>::type
        get_mapped_value_or_throw(const Key& value, const std::string& error_msg = {}) const
        {
            using namespace std::string_literals;

            typename table_of_values::iterator it = data.find(value);

            if (it != data.end())
            {
                return it->second;
            }

            throw cbeam::error::runtime_error(error_msg.empty() ? "get_mapped_value_or_throw: missing value"s : error_msg);
        }

        /**
         * @brief Retrieves the value associated with a given key, throwing an exception if the key is not found or the type does not match.
         *
         * This function template is specialized for maps where the value type is std::variant. It  attempts to retrieve the value associated with the specified key in the nested_map.
         * If T is one of the types in the variant, it attempts to retrieve the value of type T from the variant.
         * If the key is not found, or if the type T is not compatible with the stored value, an exception is thrown.
         *
         * @tparam T The type of the value to be retrieved.
         * @param key The key whose value is to be retrieved.
         * @param error_msg Optional custom error message for the exception. If empty, a default message is used.
         * @return The value associated with the key of type T.
         * @throws cbeam::error::runtime_error if the key is not found or if the type T does not match the value's type.
         */
        template <typename T>
        typename std::enable_if<!std::is_same<T, Value>::value, T>::type
        get_mapped_value_or_throw(const Key& value, const std::string& error_msg = {}) const
        {
            using namespace std::string_literals;

            nested_map*                        instance = const_cast<nested_map*>(this);
            typename table_of_values::iterator it       = instance->data.find(value);

            if (it != instance->data.end())
            {
                T* ptr = std::get_if<T>(&it->second);

                if (ptr)
                {
                    return *ptr;
                }
                else
                {
                    throw cbeam::error::runtime_error(error_msg.empty() ? "get_mapped_value_or_throw: wrong type of value"s : error_msg);
                }
            }

            throw cbeam::error::runtime_error(error_msg.empty() ? "get_mapped_value_or_throw: missing value"s : error_msg);
        }

        /**
         * @brief Retrieves a value from a std::variant associated with a given key, based on a type index.
         *
         * This function template is specialized for maps where the value type is std::variant. It fetches the value
         * associated with the specified key and extracts the value at the specified type index within the std::variant.
         * If the key is not found in the map, an exception is thrown.
         *
         * @tparam T The type index within the std::variant from which to retrieve the value.
         * @param key The key whose value, a std::variant, is to be retrieved and inspected.
         * @param error_msg Optional custom error message for the exception. If empty, a default message is used.
         * @return The value at the specified type index within the std::variant if the key exists; otherwise, a default-constructed value of the corresponding type.
         */
        template <std::size_t T>
        auto get_mapped_value_or_throw(const Key& value, const std::string& error_msg = {}) const
        {
            using namespace std::string_literals;

            auto it = data.find(value);

            if (it != data.end())
            {
                auto* ptr = std::get_if<T>(&it->second);

                if (ptr)
                {
                    return *ptr;
                }
                else
                {
                    throw cbeam::error::runtime_error(error_msg.empty() ? "get_mapped_value_or_throw: wrong type of value"s : error_msg);
                }
            }

            throw cbeam::error::runtime_error(error_msg.empty() ? "get_mapped_value_or_throw: missing value"s : error_msg);
        }
    };

    template <typename Key, typename Value>
    inline bool operator==(const nested_map<Key, Value>& lhs, const nested_map<Key, Value>& rhs)
    {
        return lhs.data == rhs.data && lhs.sub_tables == rhs.sub_tables;
    }
}
