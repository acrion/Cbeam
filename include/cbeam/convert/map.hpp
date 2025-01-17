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

#include <cbeam/convert/string.hpp> // for cbeam::convert::indent

#include <ostream>     // for std::basic_ostream, std::endl, std::ostringstream
#include <string>      // for std::char_traits, std::operator<<, std::string
#include <type_traits> // for std::enable_if, std::false_type, std::true_type, std::void_t

namespace cbeam::convert
{
    /**
     * @brief Type trait to check if a given type is a nested map.
     *
     * This type trait checks if a given type T has members 'nested_tables' and 'table_of_values',
     * which are characteristic of cbeam::container::nested_map. It inherits from std::true_type
     * if T is a nested map, and std::false_type otherwise.
     *
     * @tparam T The type to check.
     */
    template <typename T, typename = void>
    struct is_nested_map : std::false_type
    {
    };

    template <typename T>
    struct is_nested_map<T, std::void_t<typename T::nested_tables, typename T::table_of_values>> : std::true_type
    {
    };

    /**
     * @brief Type trait to check if a given type supports key_type and mapped_type, similar to std::map.
     *
     * This trait checks if a type T defines 'key_type' and 'mapped_type', which are typical for
     * associative containers like std::map. It inherits from std::true_type if T supports these types,
     * and std::false_type otherwise.
     *
     * @tparam T The type to check.
     */
    template <typename T, typename = void>
    struct has_key_and_mapped_type : std::false_type
    {
    };

    template <typename T>
    struct has_key_and_mapped_type<T, std::void_t<typename T::key_type, typename T::mapped_type>> : std::true_type
    {
    };

    /**
     * @brief Converts a container supporting key_type and mapped_type to a formatted std::string.
     *
     * This function template converts a container, which supports key_type and mapped_type (like std::map),
     * to a formatted std::string. It excludes containers of type cbeam::container::nested_map.
     * The conversion relies on the existence of to_string methods for key_type and mapped_type.
     * Each key-value pair is converted to a string and concatenated, with indentation provided to align with surrounding context.
     *
     * @tparam T The container type.
     * @param table The container to be converted.
     * @param indentation The indentation level for formatting the output.
     * @return A formatted string representation of the container.
     */
    template <typename T>
    inline typename std::enable_if<has_key_and_mapped_type<T>::value && !is_nested_map<T>::value, std::string>::type to_string(const T& table, const int indentation)
    {
        std::ostringstream os;
        for (const auto& it : table)
        {
            os << indent(indentation) << to_string(it.first)
               << indent(1) << to_string(it.second) << std::endl;
        }
        return os.str();
    }

    /**
     * @brief Converts a container supporting key_type and mapped_type to a formatted std::string.
     *
     * This function template converts a container, which supports key_type and mapped_type (like std::map),
     * to a formatted std::string. It excludes containers of type cbeam::container::nested_map.
     * The conversion relies on the existence of to_string methods for key_type and mapped_type.
     * Each key-value pair is converted to a string and concatenated.
     *
     * @tparam T The container type.
     * @param table The container to be converted.
     * @return A formatted string representation of the container.
     */
    template <typename T>
    inline typename std::enable_if<has_key_and_mapped_type<T>::value && !is_nested_map<T>::value, std::string>::type to_string(const T& table)
    {
        return to_string<T>(table, 0);
    }
}
