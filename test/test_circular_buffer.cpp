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

#include <cbeam/container/circular_buffer.hpp> // for cbeam::container::circular_buffer

#include <gtest/gtest.h>

#include <stdexcept> // for std::out_of_range
#include <string>    // for std::allocator, std::string, std::string_literals

namespace cbeam::container
{
    using namespace std::string_literals;

    TEST(CircularBuffer, TestDefaultConstructor)
    {
        circular_buffer<int, 5> buffer;
        EXPECT_EQ(buffer.size(), 0);
        EXPECT_EQ(buffer.max_size(), 5);
        EXPECT_TRUE(buffer.empty());
    }

    TEST(CircularBuffer, TestPushBack)
    {
        circular_buffer<int, 5> buffer;
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);
        EXPECT_EQ(buffer.size(), 3);
        EXPECT_FALSE(buffer.empty());
        EXPECT_EQ(buffer.front(), 1);
        EXPECT_EQ(buffer.back(), 3);
    }

    TEST(CircularBuffer, TestEmplaceBack)
    {
        circular_buffer<std::string, 5> buffer;
        buffer.emplace_back(3, 'a'); // Creates a string with 3 copies of 'a'
        EXPECT_EQ(buffer.size(), 1);
        EXPECT_EQ(buffer.front(), "aaa");
    }

    TEST(CircularBuffer, TestElementAccess)
    {
        circular_buffer<int, 5> buffer;
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);
        EXPECT_EQ(buffer[0], 1);
        EXPECT_EQ(buffer[1], 2);
        EXPECT_EQ(buffer[2], 3);
        EXPECT_EQ(buffer.at(0), 1);
        EXPECT_EQ(buffer.at(1), 2);
        EXPECT_EQ(buffer.at(2), 3);
        EXPECT_THROW(buffer.at(3), cbeam::error::out_of_range);
    }

    TEST(CircularBuffer, TestBeginEnd)
    {
        circular_buffer<int, 5> buffer;
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);
        auto it = buffer.begin();
        EXPECT_EQ(*it, 1);
        ++it;
        EXPECT_EQ(*it, 2);
        ++it;
        EXPECT_EQ(*it, 3);
        ++it;
        EXPECT_EQ(it, buffer.end());
    }

    TEST(CircularBuffer, TestClear)
    {
        circular_buffer<int, 5> buffer;
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);
        buffer.clear();
        EXPECT_EQ(buffer.size(), 0);
        EXPECT_TRUE(buffer.empty());
    }

    TEST(CircularBuffer, TestOverflow)
    {
        circular_buffer<int, 3> buffer;
        buffer.push_back(1);
        buffer.push_back(2);
        buffer.push_back(3);
        buffer.push_back(4); // This should overwrite the first element
        EXPECT_EQ(buffer.size(), 3);
        EXPECT_EQ(buffer.front(), 2);
        EXPECT_EQ(buffer.back(), 4);
    }
}
