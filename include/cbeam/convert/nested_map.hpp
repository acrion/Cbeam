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
#include <cbeam/convert/map.hpp>    // for cbeam::convert::is_nested_map
#include <cbeam/convert/string.hpp> // for cbeam::convert::indent

#include <ostream>     // for std::basic_ostream, std::endl, std::ostringstream
#include <string>      // for std::char_traits, std::string, std::operator<<
#include <type_traits> // for std::decay_t, std::enable_if

namespace cbeam::convert
{
    template <typename Key, typename Value>
    inline std::string to_string(const container::nested_map<Key, Value>& map, const int indentation);

    template <typename Key, typename Value>
    inline std::string to_string(container::nested_map<Key, Value>& map, const int indentation);

    /**
     * @brief Converts a std::map of nested_maps to a formatted std::string with specified indentation.
     *
     * This function template converts a std::map where each value is a nested_map. It formats each key-value pair
     * of the outer map and recursively calls to_string on each nested_map, increasing the indentation level
     * with each recursive call for clarity and readability.
     *
     * @tparam Key The type of the keys in the outer std::map.
     * @tparam Value The type of the nested_maps in the outer std::map.
     * @param nested_maps The std::map of nested_maps to be converted.
     * @param indentation The initial indentation level for formatting the output.
     * @return A formatted string representation of the std::map of nested_maps.
     */
    template <typename Key, typename Value>
    inline std::string to_string(const std::map<Key, container::nested_map<Key, Value>>& nested_maps, const int indentation)
    {
        std::ostringstream os;
        for (const auto& it : nested_maps)
        {
            os << indent(indentation) << to_string(it.first) << std::endl
               << to_string<Key, Value>(it.second, indentation + 1);
        }
        return os.str();
    }

    /**
     * @brief Converts a std::map of nested_maps to a formatted std::string.
     *
     * This function template converts a std::map where each value is a nested_map. It formats each key-value pair
     * of the outer map and recursively calls to_string on each nested_map, increasing the indentation level
     * with each recursive call for clarity and readability.
     *
     * @tparam Key The type of the keys in the outer std::map.
     * @tparam Value The type of the nested_maps in the outer std::map.
     * @param nested_maps The std::map of nested_maps to be converted.
     * @return A formatted string representation of the std::map of nested_maps.
     */
    template <typename Key, typename Value>
    inline std::string to_string(const std::map<Key, container::nested_map<Key, Value>>& nested_maps)
    {
        return to_string<Key, Value>(nested_maps, 0);
    }

    /**
     * @brief Converts a nested_map to a formatted std::string with specified indentation.
     *
     * This overload takes a const reference to a nested_map and an indentation level.
     * It converts both the direct key-value pairs and the sub-tables of nested_maps into a formatted string.
     *
     * @tparam Key The type of the keys in the nested_map.
     * @tparam Value The type of the values in the nested_map.
     * @param map The nested_map to be converted.
     * @param indentation The indentation level for formatting the output.
     * @return A formatted string representation of the nested_map.
     */
    template <typename Key, typename Value>
    inline std::string to_string(const container::nested_map<Key, Value>& map, const int indentation)
    {
        return to_string(map.data, indentation + 1)
             + to_string(map.sub_tables, indentation + 1);
    }

    /**
     * @brief Converts a nested_map to a formatted std::string.
     *
     * This overload takes a const reference to a nested_map.
     * It converts both the direct key-value pairs and the sub-tables of nested_maps into a formatted string.
     *
     * @tparam Key The type of the keys in the nested_map.
     * @tparam Value The type of the values in the nested_map.
     * @param map The nested_map to be converted.
     * @return A formatted string representation of the nested_map.
     */
    template <typename Key, typename Value>
    inline std::string to_string(const container::nested_map<Key, Value>& map)
    {
        return to_string(map, -1);
    }

    /**
     * @brief Converts a table to a formatted std::string, provided the table is a nested_map.
     *
     * This template function converts a table to a formatted std::string, but only if the table type
     * is identified as a nested_map using the is_nested_map trait. It leverages the specific to_string
     * overload for nested_map types to perform the conversion.
     *
     * @tparam T The table type, expected to be a nested_map.
     * @param table The table to be converted.
     * @return A formatted string representation of the table if it is a nested_map, otherwise an empty string.
     */
    template <typename T>
    inline typename std::enable_if<is_nested_map<T>::value, std::string>::type to_string(const T& table)
    {
        using DataTableType = typename std::decay_t<decltype(table)>;
        using KeyType       = typename DataTableType::key_type;
        using ValueType     = typename DataTableType::mapped_type;

        return cbeam::convert::to_string<KeyType, ValueType>(table);
    }
}
