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

#include <cbeam/container/buffer.hpp> // for cbeam::convert::indent

#include <ostream>     // for std::basic_ostream, std::endl, std::ostringstream
#include <string>      // for std::char_traits, std::operator<<, std::string
#include <type_traits> // for std::enable_if, std::false_type, std::true_type, std::void_t

namespace cbeam::convert
{
    /**
     * @brief Creates a `std::string` from the contents of a `container::buffer`.
     *
     * @param b A reference to a `container::buffer`.
     * @return A `std::string` with the same size and contents as the provided buffer.
     */
    inline std::string to_string(const container::buffer& b)
    {
        return std::string((char*)b.get(), b.size());
    }
}
