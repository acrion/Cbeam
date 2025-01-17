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

#include <cbeam/json/map.hpp>
#include <cbeam/json/nested_map.hpp>
#include <cbeam/json/string.hpp> // for cbeam::json::traits
#include <cbeam/json/traits.hpp> // for cbeam::json::traits, cbeam::json::serialized_object

#include <cbeam/container/stable_reference_buffer.hpp> // for cbeam::container::stable_reference_buffer
#include <cbeam/convert/buffer.hpp>
#include <cbeam/lifecycle/singleton.hpp>
#include <cbeam/memory/pointer.hpp> // for cbeam::memory::pointer

#include <gtest/gtest.h>

#include <string> // for std::allocator, std::string, std::string_literals::operator""s, std::string_literals

using namespace std::string_literals;

class JSONTest : public ::testing::Test
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

TEST_F(JSONTest, serializeValue)
{
    int         test1 = 1;
    double      test2 = 2.0;
    std::string test3 = "3";
    bool        test4 = true;

    cbeam::container::stable_reference_buffer stream;
    cbeam::json::traits<int>::serialize(test1, stream);
    stream.append(",", 1);
    cbeam::json::traits<double>::serialize(test2, stream);
    stream.append(",", 1);
    cbeam::json::traits<std::string>::serialize(test3, stream);
    stream.append(",", 1);
    cbeam::json::traits<bool>::serialize(test4, stream);

    EXPECT_EQ(cbeam::convert::to_string(stream), "\"1\",\"2\",\"3\",\"1\"");
}

TEST_F(JSONTest, stringIntMap)
{
    std::map<std::string, int> test_map{{"42", 1}, {"2", 314}, {"-5", 64}};

    cbeam::container::stable_reference_buffer stream;
    cbeam::json::traits<decltype(test_map)>::serialize(test_map, stream);

    EXPECT_EQ(cbeam::convert::to_string(stream), R"({"-5":"64","2":"314","42":"1"})");
}

TEST_F(JSONTest, boolStringMap)
{
    std::map<std::string, bool> test_map{{"42", true}, {"2", false}, {"-5", false}};

    cbeam::container::stable_reference_buffer stream;
    cbeam::json::traits<decltype(test_map)>::serialize(test_map, stream);

    EXPECT_EQ(cbeam::convert::to_string(stream), R"({"-5":"0","2":"0","42":"1"})");
}

TEST_F(JSONTest, serializeNestedTables1)
{
    cbeam::container::nested_map<std::string, bool> test_map{{"42", true}, {"2", false}, {"-5", false}};

    cbeam::container::stable_reference_buffer stream;
    cbeam::json::traits<decltype(test_map)>::serialize(test_map, stream);

    EXPECT_EQ(cbeam::convert::to_string(stream), R"({"-5":"0","2":"0","42":"1"})");
}

TEST_F(JSONTest, serializeNestedTables2)
{
    using NestedMapTestType = cbeam::container::nested_map<std::string, bool>;
    NestedMapTestType test_map{{"42", true}, {"2", false}, {"abc", false}};
    test_map.sub_tables["-5"].data["-6"] = true;

    cbeam::container::stable_reference_buffer stream;
    cbeam::json::traits<decltype(test_map)>::serialize(test_map, stream);

    EXPECT_EQ(cbeam::convert::to_string(stream), R"({"2":"0","42":"1","abc":"0","-5":{"-6":"1"}})");
}

// TEST_F(JSONTest, serializePod)
// {
//     long long   test1 = 1;
//     double      test2 = 2.0;
//     const bool  test3 = true;
//     void*       test4 = (void*)0x42;
//     std::string test5 = "test";
//
//     cbeam::container::stable_reference_buffer stream;
//     cbeam::json::traits<long long>::serialize(test1, stream);
//     cbeam::json::traits<double>::serialize(test2, stream);
//     cbeam::json::traits<bool>::serialize(test3, stream);
//     cbeam::json::traits<void*>::serialize(test4, stream);
//
//     long long test1b;
//     double    test2b;
//     bool      test3b;
//     void*     test4b;
//
//     cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
//     cbeam::json::traits<long long>::deserialize(it, test1b);
//     cbeam::json::traits<double>::deserialize(it, test2b);
//     cbeam::json::traits<bool>::deserialize(it, test3b);
//     cbeam::json::traits<void*>::deserialize(it, test4b);
//
//     EXPECT_EQ(test1, test1b);
//     EXPECT_EQ(test2, test2b);
//     EXPECT_EQ(test3, test3b);
//     EXPECT_EQ(test4, test4b);
// }

//
//    TEST_F(JSONTest, serializeString)
//    {
//        const std::string test1 = "test1";
//        std::string       test2 = "test2";
//
//        container::stable_reference_buffer stream;
//        cbeam::json::traits<std::string>::serialize(test1, stream);
//        cbeam::json::traits<std::string>::serialize(test2, stream);
//
//        std::string test1b;
//        std::string test2b;
//
//        serialized_object it = (serialized_object)stream.get();
//        cbeam::json::traits<std::string>::deserialize(it, test1b);
//        cbeam::json::traits<std::string>::deserialize(it, test2b);
//
//        EXPECT_EQ(test1, test1b);
//        EXPECT_EQ(test2, test2b);
//    }

// TEST_F(JSONTest, serializeNestedTables3)
// {
//     using NestedMapTestType = cbeam::container::nested_map<int, double>;
//     NestedMapTestType subSubTable;
//     subSubTable.data[2] = 42.42;
//
//     NestedMapTestType subTable;
//     subTable.data[1]        = 3.14159;
//     subTable.sub_tables[-2] = subSubTable;
//
//     NestedMapTestType test;
//     test.data[0]        = 2.718281828;
//     test.sub_tables[-1] = subTable;
//
//     cbeam::container::stable_reference_buffer stream;
//     cbeam::serialization::traits<decltype(test)>::serialize(test, stream);
//
//     EXPECT_EQ(cbeam::convert::to_string(stream), "");
// }

// TEST_F(JSONTest, serializeNestedTables4)
// {
//     using NestedMapTestType = cbeam::container::nested_map<std::string, double>;
//     NestedMapTestType subSubTable;
//     subSubTable.data["innermost"] = 42.42;
//
//     NestedMapTestType subTable;
//     subTable.data["middle"]              = 3.14159;
//     subTable.sub_tables["sub-sub-table"] = subSubTable;
//
//     NestedMapTestType test;
//     test.data["outer"]           = 2.718281828;
//     test.sub_tables["sub-table"] = subTable;
//
//     cbeam::container::stable_reference_buffer stream;
//     cbeam::serialization::traits<decltype(test)>::serialize(test, stream);
//
//     EXPECT_EQ(cbeam::convert::to_string(stream), "");
// }
