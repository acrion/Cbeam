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

#include <cbeam/memory/pointer.hpp> // for cbeam::memory::pointer, cbeam::memory::operator<<

#include <cstddef> // for std::size_t

#include <iosfwd>  // for std::ostream
#include <string>  // for std::string
#include <variant> // for std::visit, std::variant

namespace cbeam::container::xpod
{
    /**
     * @brief A variant designed for basic data types. memory::pointer is used in place of void* to provide additional features.
     *
     * @details By including the header 'serialization/xpod.hpp', this variant gains serialization capabilities.
     * This functionality is the primary reason for using memory::pointer instead of raw void*.
     * Thanks to its serializability, it can be used as key and value types in cbeam::container::nested_map
     * (which gains serialization capabilities by including 'serialization/nested_map.hpp').
     */
    using type = std::variant<long long, double, bool, memory::pointer, std::string>;

    namespace type_index
    {
        constexpr std::size_t integer{0}; // type index for long long
        constexpr std::size_t number{1};  // type index for double
        constexpr std::size_t boolean{2}; // type index for bool
        constexpr std::size_t pointer{3}; // type index for memory::pointer
        constexpr std::size_t string{4};  // type index for std::string
    };

    /**
     * @brief Overload of the insertion operator to output the contents of an xpod::type.
     *
     * This uses std::visit to print the currently active variant type in a human-readable way.
     *
     * @param os The output stream.
     * @param v The xpod::type variant to print.
     * @return std::ostream& A reference to the output stream for chaining.
     */
    inline std::ostream& operator<<(std::ostream& os, const xpod::type& v)
    {
        std::visit([&os](auto&& arg)
                   { os << arg; },
                   v);
        return os;
    }
} // namespace cbeam::container::xpod
