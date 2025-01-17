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

#include <cbeam/container/stable_interprocess_map.hpp> // for cbeam::container::stable_interprocess_map
#include <cbeam/error/out_of_range.hpp>                // for cbeam::error::out_of_range
#include <cbeam/random/generators.hpp>                 // for cbeam::random::random_string

#include <gtest/gtest.h>

#include <algorithm> // for std::max
#include <atomic>    // for std::atomic
#include <chrono>    // for std::chrono::milliseconds
#include <cstddef>   // for std::size_t
#include <thread>    // for std::thread, std::this_thread::sleep_for
#include <vector>    // for std::vector, std::allocator

using cbeam::container::stable_interprocess_map;

class StableInterprocessMapTest : public ::testing::Test
{
protected:
    stable_interprocess_map<int, int>  map;
    static constexpr const std::size_t initial_byte_size{1024};

    StableInterprocessMapTest()
        : map(cbeam::random::random_string(16), initial_byte_size) {}

    void SetUp() override
    {
    }

    void TearDown() override
    {
        map.clear();
    }
};

TEST_F(StableInterprocessMapTest, InsertAndRetrieveValue)
{
    int test_key   = 1;
    int test_value = 42;

    map.insert(test_key, test_value);
    int retrieved_value = map.at(test_key);

    EXPECT_EQ(retrieved_value, test_value);
}

TEST_F(StableInterprocessMapTest, HandleNonExistingKey)
{
    int non_existing_key = 2;

    EXPECT_THROW({ map.at(non_existing_key); }, cbeam::error::out_of_range);
}

TEST_F(StableInterprocessMapTest, Clear)
{
    int test_key   = 2;
    int test_value = 42;

    map.insert(test_key, test_value);
    map.clear();

    EXPECT_THROW({ map.at(test_key); }, cbeam::error::out_of_range);
}

TEST_F(StableInterprocessMapTest, Empty)
{
    int test_key   = 2;
    int test_value = 42;

    map.insert(test_key, test_value);
    map.clear();

    EXPECT_TRUE(map.empty());
}

TEST_F(StableInterprocessMapTest, Size)
{
    EXPECT_EQ(map.size(), 0);

    map.insert(1, 42);

    EXPECT_EQ(map.size(), 1);

    map.insert(2, 42);

    EXPECT_EQ(map.size(), 2);

    map.insert(2, 43);

    EXPECT_EQ(map.size(), 2);

    map.clear();

    EXPECT_EQ(map.size(), 0);
}

TEST_F(StableInterprocessMapTest, OverwriteExistingValue)
{
    int test_key      = 3;
    int initial_value = 42;
    int new_value     = 43;

    map.insert(test_key, initial_value);
    map.insert(test_key, new_value);

    int retrieved_value = map.at(test_key);
    EXPECT_EQ(retrieved_value, new_value);
}

TEST_F(StableInterprocessMapTest, BadAllocTest)
{
    EXPECT_THROW(
        {
            for (std::size_t i = 0; i < initial_byte_size; ++i)
            {
                map.insert(i, i);
            }
        },
        cbeam::error::runtime_error);
}

TEST_F(StableInterprocessMapTest, ConcurrentInsertionsStressTest)
{
    const int                thread_count{50};
    std::vector<std::thread> threads;
    std::atomic<bool>        running{true};

    for (int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(
            [this, i, &running]
            {
                do
                {
                    map.insert(i, i * 2);
                    EXPECT_EQ(map.at(i), i * 2);
                } while (running); });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(333));
    running = false;

    for (auto& t : threads)
    {
        t.join();
    }

    for (int i = 0; i < thread_count; ++i)
    {
        EXPECT_EQ(map.at(i), i * 2);
    }
}
