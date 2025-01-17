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

#include <cbeam/container/stable_reference_buffer.hpp> // for cbeam::container::stable_reference_buffer
#include <cbeam/serialization/map.hpp>                 // for cbeam::serialization::traits
#include <cbeam/serialization/string.hpp>              // IWYU pragma: keep (required to serialize std::string)
#include <cbeam/serialization/traits.hpp>              // for cbeam::serialization::serialized_object

#include <gtest/gtest.h>

#include <algorithm> // for std::equal
#include <map>       // for std::map
#include <string>    // for std::basic_string, std::string_literals::operator""s, std::allocator, std::operator==, std::string, std::string_literals

class SerializableMapTest : public ::testing::Test
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

TEST_F(SerializableMapTest, intMap)
{
    std::map<int, int> test_map{{42, 1}, {2, 314}, {-5, 64}};

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<decltype(test_map)>::serialize(test_map, stream);

    std::map<int, int> test_map_result;

    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<decltype(test_map_result)>::deserialize(it, test_map_result);

    EXPECT_EQ(test_map, test_map_result);
}

TEST_F(SerializableMapTest, intStringMap)
{
    using namespace std::string_literals;

    const std::map<int, std::string> test_map{{42, "test1"s}, {2, "test2"s}};

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<decltype(test_map)>::serialize(test_map, stream);

    std::map<int, std::string> test_map_result;

    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<decltype(test_map_result)>::deserialize(it, test_map_result);

    EXPECT_EQ(test_map, test_map_result);
}
