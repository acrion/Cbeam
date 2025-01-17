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

#include <cstddef> // for std::size_t

#include <map>     // for std::map
#include <variant> // for std::variant, std::get_if, std::variant_alternative_t, std::visit

namespace cbeam::container
{
    /**
     * @brief Checks if a std::map using std::variant as a key contains a specific key.
     *
     * This function searches a std::map whose keys are comprised of a std::variant of different types,
     * determining whether a specific key is present. It is generic and can be used for any combination
     * of variants and keys.
     *
     * @tparam Key The type of the key being searched. Must be one of the VariantTypes.
     * @tparam Value The value type of the std::map, which is irrelevant for this function.
     * @tparam VariantTypes The types of the std::variant used as the key type for the std::map.
     * @param t The std::map to be searched.
     * @param key The key to be searched for in t.
     * @return true if t contains a key-value pair with the given key, otherwise false.
     */
    template <typename Key, typename Value, typename... VariantTypes>
    bool key_exists(const std::map<std::variant<VariantTypes...>, Value>& t, const Key& key)
    {
        for (const auto& pair : t)
        {
            bool exists = std::visit([&key](auto&& lhs) -> bool
                                     {
                                         if constexpr (std::is_same_v<decltype(lhs), Key>)
                                         {
                                             return lhs == key;
                                         }
                                         return false; },
                                     pair.first);
            if (exists)
            {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns the value of type T in the std::variant, if it is available, otherwise the default value of T
     * @tparam T the type that shall be retrieved from the given std::variant instance
     * @tparam Types the types of the std::variant value
     * @param value the std::variant to read from
     * @return If the value at type index T in the std::variant instance does exist, returns it, otherwise (if its does not exist or the type is not available in the std::variant) the default value of the corresponding type.
     */
    template <typename T, typename... Types>
    T get_value_or_default(const std::variant<Types...>& value) noexcept
    {
        T* ptr = std::get_if<T>(const_cast<std::variant<Types...>*>(&value));
        if (ptr)
        {
            return *ptr;
        }
        else
        {
            return {};
        }
    }

    /**
     * Returns the value at type index T of the given std::variant instance. If the value is not set in the std::variant or T is a type index that is not available in the std::variant instance, the default value of the corresponding type will be returned.
     * @tparam T the type index to be accessed in the value
     * @tparam Types the types of the std::variant value
     * @param value the std::variant to read from
     * @return if the value at type index T in the std::variant instance does exist, returns it, otherwise (if its does not exist or the type index is not available in the std::variant) the default value of the corresponding type.
     */
    template <std::size_t T, typename... Types>
    auto get_value_or_default(const std::variant<Types...>& value) noexcept
    {
        using variant_type = std::variant<Types...>;
        using target_type  = std::variant_alternative_t<T, variant_type>;

        const target_type* ptr = std::get_if<T>(&value);
        if (ptr)
        {
            return *ptr;
        }
        else
        {
            return target_type{};
        }
    }
}
