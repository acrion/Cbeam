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

#include <cbeam/container/nested_map.hpp>

#include <cbeam/serialization/xpod.hpp> // IWYU pragma: keep (required to serialize cbeam::container::xpod::type)

#include <cbeam/container/stable_reference_buffer.hpp> // for cbeam::container::stable_reference_buffer
#include <cbeam/container/xpod.hpp>                    // for cbeam::container::xpod::type
#include <cbeam/memory/pointer.hpp>                    // for cbeam::memory::pointer, cbeam::memory::operator<, cbeam::memory::operator==
#include <cbeam/serialization/nested_map.hpp>
#include <cbeam/serialization/traits.hpp> // for cbeam::serialization::serialized_object

#include <gtest/gtest.h>

#include <string>  // for std::operator==, std::allocator, std::string_literals::operator""s, std::string_literals
#include <variant> // for std::operator==, std::operator<, std::variant

using namespace std::string_literals;

using NestedMapTestType = cbeam::container::nested_map<cbeam::container::xpod::type, cbeam::container::xpod::type>;

class SerializableNestedMapTest : public ::testing::Test
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

TEST_F(SerializableNestedMapTest, serializeTable)
{
    NestedMapTestType subSubTable;
    subSubTable.data[true] = "test0";

    NestedMapTestType subTable;
    subTable.data["0xffffff"]                                  = "test1";
    subTable.sub_tables[cbeam::memory::pointer("0xeeeeeeee"s)] = subSubTable;

    NestedMapTestType test;
    test.data["test2"]     = 1;
    test.data[1.0]         = true;
    test.sub_tables[false] = subTable;

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<decltype(test)>::serialize(test, stream);
    NestedMapTestType                       test2;
    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<decltype(test2)>::deserialize(it, test2);

    EXPECT_EQ(test2, test);
}

TEST_F(SerializableNestedMapTest, serializeEmptyTable)
{
    NestedMapTestType test;

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<decltype(test)>::serialize(test, stream);

    NestedMapTestType                       test2;
    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<decltype(test2)>::deserialize(it, test2);

    EXPECT_EQ(test2, test);
}

TEST_F(SerializableNestedMapTest, serializeSingleDataEntry)
{
    NestedMapTestType test;
    test.data["key"] = "value";

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<decltype(test)>::serialize(test, stream);

    NestedMapTestType                       test2;
    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<decltype(test2)>::deserialize(it, test2);

    EXPECT_EQ(test2, test);
}

TEST_F(SerializableNestedMapTest, serializeNestedTables)
{
    NestedMapTestType subSubTable;
    subSubTable.data[true] = "innermost";

    NestedMapTestType subTable;
    subTable.data["middle"]    = 1;
    subTable.sub_tables[false] = subSubTable;

    NestedMapTestType test;
    test.data["outer"] = 1.0;
    test.sub_tables[1] = subTable;

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<decltype(test)>::serialize(test, stream);

    NestedMapTestType                       test2;
    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<decltype(test2)>::deserialize(it, test2);

    EXPECT_EQ(test2, test);
}
