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

#include <cbeam/container/stable_reference_buffer.hpp> // for cbeam::cbeam::container::stable_reference_buffer
#include <cbeam/container/xpod.hpp>                    // for cbeam::cbeam::container::xpod::type
#include <cbeam/memory/pointer.hpp>                    // for cbeam::memory::pointer
#include <cbeam/serialization/string.hpp>              // for cbeam::serialization::cbeam::serialization::traits
#include <cbeam/serialization/traits.hpp>              // for cbeam::serialization::cbeam::serialization::traits, cbeam::serialization::serialized_object
#include <cbeam/serialization/xpod.hpp>                // for cbeam::serialization::cbeam::serialization::traits

#include <gtest/gtest.h>

#include <string> // for std::allocator, std::string, std::string_literals::operator""s, std::string_literals

using namespace std::string_literals;

class Serialization : public ::testing::Test
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

TEST_F(Serialization, serializeValue)
{
    cbeam::container::xpod::type test1 = 1;
    cbeam::container::xpod::type test2 = 2.0;
    cbeam::container::xpod::type test3 = "3";
    cbeam::container::xpod::type test4 = true;
    cbeam::container::xpod::type test5 = cbeam::memory::pointer("0xffffffff"s);

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<cbeam::container::xpod::type>::serialize(test1, stream);
    cbeam::serialization::traits<cbeam::container::xpod::type>::serialize(test2, stream);
    cbeam::serialization::traits<cbeam::container::xpod::type>::serialize(test3, stream);
    cbeam::serialization::traits<cbeam::container::xpod::type>::serialize(test4, stream);
    cbeam::serialization::traits<cbeam::container::xpod::type>::serialize(test5, stream);

    cbeam::container::xpod::type test1b, test2b, test3b, test4b, test5b;

    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<cbeam::container::xpod::type>::deserialize(it, test1b);
    cbeam::serialization::traits<cbeam::container::xpod::type>::deserialize(it, test2b);
    cbeam::serialization::traits<cbeam::container::xpod::type>::deserialize(it, test3b);
    cbeam::serialization::traits<cbeam::container::xpod::type>::deserialize(it, test4b);
    cbeam::serialization::traits<cbeam::container::xpod::type>::deserialize(it, test5b);

    EXPECT_EQ(test1, test1b);
    EXPECT_EQ(test2, test2b);
    EXPECT_EQ(test3, test3b);
    EXPECT_EQ(test4, test4b);
    EXPECT_EQ(test5, test5b);
}

TEST_F(Serialization, serializePod)
{
    long long   test1 = 1;
    double      test2 = 2.0;
    const bool  test3 = true;
    void*       test4 = (void*)0x42;
    std::string test5 = "test";

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<long long>::serialize(test1, stream);
    cbeam::serialization::traits<double>::serialize(test2, stream);
    cbeam::serialization::traits<bool>::serialize(test3, stream);
    cbeam::serialization::traits<void*>::serialize(test4, stream);

    long long test1b;
    double    test2b;
    bool      test3b;
    void*     test4b;

    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<long long>::deserialize(it, test1b);
    cbeam::serialization::traits<double>::deserialize(it, test2b);
    cbeam::serialization::traits<bool>::deserialize(it, test3b);
    cbeam::serialization::traits<void*>::deserialize(it, test4b);

    EXPECT_EQ(test1, test1b);
    EXPECT_EQ(test2, test2b);
    EXPECT_EQ(test3, test3b);
    EXPECT_EQ(test4, test4b);
}

TEST_F(Serialization, serializeString)
{
    const std::string test1 = "test1";
    std::string       test2 = "test2";

    cbeam::container::stable_reference_buffer stream;
    cbeam::serialization::traits<std::string>::serialize(test1, stream);
    cbeam::serialization::traits<std::string>::serialize(test2, stream);

    std::string test1b;
    std::string test2b;

    cbeam::serialization::serialized_object it = (cbeam::serialization::serialized_object)stream.get();
    cbeam::serialization::traits<std::string>::deserialize(it, test1b);
    cbeam::serialization::traits<std::string>::deserialize(it, test2b);

    EXPECT_EQ(test1, test1b);
    EXPECT_EQ(test2, test2b);
}
