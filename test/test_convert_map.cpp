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

#include <cbeam/convert/map.hpp> // for cbeam::convert::to_string

#include <gtest/gtest.h>

#include <map>    // for std::map, std::operator!=
#include <string> // for std::allocator, std::basic_string, std::string, std::operator<

namespace cbeam::convert
{
    TEST(ConvertMap, Basic)
    {
        std::map<std::string, int> map{{"a", 3}, {"b", 4}, {"c", 5}};

        std::string result = to_string(map);

        std::string expected =
            R"(a	3
b	4
c	5
)";

        EXPECT_EQ(result, expected);
    }
}
