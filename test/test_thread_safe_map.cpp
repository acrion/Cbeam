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

#include <cbeam/container/thread_safe_map.hpp> // for cbeam::container::thread_safe_map

#include <gtest/gtest.h>

#include <map>       // for std::_Rb_tree_iterator
#include <string>    // for std::allocator, std::basic_string, std::string
#include <utility>   // for std::pair

TEST(ThreadSafeMapTest, InsertAndRetrieveElement)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map.insert(1, "Test");
    EXPECT_EQ(map.at(1), "Test");
}

TEST(ThreadSafeMapTest, RetrieveNonExistentElementThrows)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    EXPECT_THROW(map.at(2), cbeam::error::out_of_range);
}

TEST(ThreadSafeMapTest, EraseElement)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map.insert(1, "Test");
    EXPECT_EQ(map.erase(1), 1u);
    EXPECT_THROW(map.at(1), cbeam::error::out_of_range);
}

TEST(ThreadSafeMapTest, ClearMap)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map.insert(1, "Test");
    map.clear();
    EXPECT_TRUE(map.empty());
}

TEST(ThreadSafeMapTest, CheckMapSize)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map.insert(1, "Test");
    EXPECT_EQ(map.size(), 1u);
}

TEST(ThreadSafeMapTest, CountExistingAndNonExistingElements)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map.insert(1, "Test");
    EXPECT_EQ(map.count(1), 1u);
    EXPECT_EQ(map.count(2), 0u);
}

TEST(ThreadSafeMapTest, SubscriptOperatorForInsertionAndAccess)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map[2] = "Hello";
    EXPECT_EQ(map.at(2), "Hello");
}

TEST(ThreadSafeMapTest, IteratorValidity)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    map.insert(1, "Test");
    auto it = map.begin();
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, "Test");
}

TEST(ThreadSafeMapTest, AccessNonExistentElementThrowsOutOfRange)
{
    cbeam::container::thread_safe_map<int, std::string> map;
    EXPECT_THROW(map.at(99), cbeam::error::out_of_range);
}
