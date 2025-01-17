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

#include <cbeam/container/nested_map.hpp> // for cbeam::container::nested_map
#include <cbeam/convert/nested_map.hpp>   // for cbeam::convert::to_string

#include <gtest/gtest.h>

#include <map>    // for std::operator!=
#include <string> // for std::allocator, std::string

namespace cbeam::convert
{
    TEST(ConvertNestedMap, Basic)
    {
        cbeam::container::nested_map<std::string, int> map;
        map.data = {{"a", 3}, {"b", 4}, {"c", 5}};
        decltype(map) subTable1, subTable2;
        subTable1.data      = {{"d", 6}, {"e", 7}, {"f", 8}};
        subTable2.data      = {{"g", 9}, {"h", 10}};
        map.sub_tables["i"] = subTable1;
        map.sub_tables["j"] = subTable2;

        std::string result = to_string(map);

        std::string expected =
            R"(a	3
b	4
c	5
i
		d	6
		e	7
		f	8
j
		g	9
		h	10
)";

        EXPECT_EQ(result, expected);
    }
}
