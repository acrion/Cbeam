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

#include <cbeam/container/buffer.hpp>         // for cbeam::container::buffer
#include <cbeam/container/nested_map.hpp>     // for cbeam::container::nested_map
#include <cbeam/container/xpod.hpp>           // for cbeam::container::xpod::type
#include <cbeam/memory/pointer.hpp>           // for cbeam::memory::pointer, cbeam::memory::operator<, cbeam::memory::operator==
#include <cbeam/serialization/direct.hpp>     // for cbeam::serialization::deserialize
#include <cbeam/serialization/nested_map.hpp> // for cbeam::serialization::traits
#include <cbeam/serialization/xpod.hpp>       // IWYU pragma: keep (required to serialize cbeam::container::xpod::type)

#include <gtest/gtest.h>

#include <algorithm> // for std::equal
#include <string>    // for std::basic_string, std::allocator, std::operator==
#include <variant>   // for std::operator<, std::operator==

class ConvertNestedMapXpodTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        cbeam::lifecycle::singleton_control::reset();           // delete all singletons (and setting shutdown state to true to avoid further accesses)
        cbeam::lifecycle::singleton_control::set_operational(); // end shutting down state for singletons to enable further tests
    }
};

TEST_F(ConvertNestedMapXpodTest, BasicTest)
{
    cbeam::container::nested_map<cbeam::container::xpod::type, cbeam::container::xpod::type> map;
    map.data = {{0.3, "Value"}, {"Key", 42}, {true, false}};
    decltype(map) subTable1, subTable2;
    subTable1.data         = {{false, 0.5}, {-1.5, 43}, {"Key 2", true}};
    subTable2.data         = {{42, "Value2"}, {"Key 2", true}};
    map.sub_tables[false]  = subTable1;
    map.sub_tables["test"] = subTable2;

    cbeam::container::buffer serializedTable;
    cbeam::serialization::traits<decltype(map)>::serialize(map, serializedTable);

    auto deserializedTable = cbeam::serialization::deserialize<decltype(map)>(serializedTable.get());

    EXPECT_EQ(map, deserializedTable);
}
