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

#include <gtest/gtest.h>

#include <stdint.h> // for uint8_t

#include <cstddef> // for std::size_t, size_t
#include <memory>  // for std::allocator

class StableReferenceBufferTest : public ::testing::Test
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

TEST_F(StableReferenceBufferTest, ConstructorTest)
{
    const std::size_t                         size         = 10;
    const unsigned long                       size_of_type = sizeof(int);
    cbeam::container::stable_reference_buffer buffer(size, size_of_type);

    EXPECT_NE(buffer.get(), nullptr);
    EXPECT_EQ(buffer.size(), size * size_of_type);
}

TEST_F(StableReferenceBufferTest, CopyConstructorTest)
{
    const std::size_t                         size         = 10;
    const unsigned long                       size_of_type = sizeof(int);
    cbeam::container::stable_reference_buffer buffer(size, size_of_type);

    cbeam::container::stable_reference_buffer copy(buffer);
    EXPECT_EQ(copy.get(), buffer.get());
}

TEST_F(StableReferenceBufferTest, AssignmentOperatorTest)
{
    const std::size_t                         size         = 10;
    const unsigned long                       size_of_type = sizeof(int);
    cbeam::container::stable_reference_buffer buffer(size, size_of_type);

    cbeam::container::stable_reference_buffer copy = buffer;
    EXPECT_EQ(copy.get(), buffer.get());
}

TEST_F(StableReferenceBufferTest, AppendTest)
{
    const std::size_t                         size         = 10;
    const unsigned long                       size_of_type = sizeof(int);
    cbeam::container::stable_reference_buffer buffer(size, size_of_type);

    int data_to_append[5] = {1, 2, 3, 4, 5};
    buffer.append(data_to_append, sizeof(data_to_append));

    EXPECT_EQ(buffer.size(), (size * size_of_type) + sizeof(data_to_append));
}

TEST_F(StableReferenceBufferTest, AppendRawTest)
{
    cbeam::container::stable_reference_buffer sharedBuffer;

    const int a = 3;
    const int b = 5;

    sharedBuffer.append(&a, sizeof(a));
    sharedBuffer.append(&b, sizeof(b));

    int* a2 = (int*)sharedBuffer.get();
    int* b2 = (int*)((uint8_t*)sharedBuffer.get() + sizeof(a));

    EXPECT_EQ(a, *a2);
    EXPECT_EQ(b, *b2);
}

TEST_F(StableReferenceBufferTest, IsKnownTest)
{
    const std::size_t                         size         = 10;
    const unsigned long                       size_of_type = sizeof(int);
    cbeam::container::stable_reference_buffer buffer(size, size_of_type);

    EXPECT_TRUE(buffer.is_known(buffer.get()));
    EXPECT_FALSE(buffer.is_known(nullptr));
}

TEST_F(StableReferenceBufferTest, UseCountTest)
{
    cbeam::container::stable_reference_buffer buffer1(10, sizeof(int));
    cbeam::container::stable_reference_buffer buffer2 = buffer1;

    EXPECT_EQ(buffer1.use_count(), 2);
    EXPECT_EQ(buffer2.use_count(), 2);

    buffer2.reset();
    EXPECT_EQ(buffer1.use_count(), 1);
}

TEST_F(StableReferenceBufferTest, ResetTest)
{
    cbeam::container::stable_reference_buffer buffer(10, sizeof(int));

    EXPECT_TRUE(buffer.is_known(buffer.get()));
    buffer.reset();
    EXPECT_FALSE(buffer.is_known(buffer.get()));
    EXPECT_EQ(buffer.use_count(), 0);
}

TEST_F(StableReferenceBufferTest, SwapTest)
{
    cbeam::container::stable_reference_buffer buffer1(10, sizeof(int));
    cbeam::container::stable_reference_buffer buffer2(20, sizeof(int));

    auto* original_buffer1_data = buffer1.get();
    auto* original_buffer2_data = buffer2.get();
    auto  original_buffer1_size = buffer1.size();
    auto  original_buffer2_size = buffer2.size();

    buffer1.swap(buffer2);

    EXPECT_EQ(buffer1.get(), original_buffer2_data);
    EXPECT_EQ(buffer2.get(), original_buffer1_data);

    EXPECT_EQ(buffer1.size(), original_buffer2_size);
    EXPECT_EQ(buffer2.size(), original_buffer1_size);

    EXPECT_TRUE(buffer1.is_known(buffer1.get()));
    EXPECT_TRUE(buffer2.is_known(buffer2.get()));
}

TEST_F(StableReferenceBufferTest, DelayDeallocationTest)
{
    cbeam::container::stable_reference_buffer buffer1(10, sizeof(int));
    EXPECT_EQ(buffer1.use_count(), 1);

    uint8_t* raw_pointer;
    {
        cbeam::container::stable_reference_buffer::delay_deallocation delay_deallocation;

        cbeam::container::stable_reference_buffer buffer2(10, sizeof(int));
        EXPECT_EQ(buffer2.use_count(), 2);

        raw_pointer  = (uint8_t*)buffer2.get();
        *raw_pointer = 42;
        buffer2.reset();
        EXPECT_TRUE(cbeam::container::stable_reference_buffer::is_known(raw_pointer));

        // The check using EXPECT_EQ(*raw_pointer, 42) verifies that the memory at the location pointed to by raw_pointer can be accessed and contains
        // the expected value. This implies that the memory has not been deallocated due to the presence of the delay_deallocation mechanism, which
        // is intended to prevent deallocation while it is in scope. While this does not provide an absolute guarantee of memory validity or accessibility,
        // it does serve as a practical confirmation under the assumption that the memory management mechanisms are functioning as expected.
        EXPECT_EQ(*raw_pointer, 42);
    }

    EXPECT_FALSE(cbeam::container::stable_reference_buffer::is_known(raw_pointer));

    cbeam::container::stable_reference_buffer buffer3(10, sizeof(int));
    EXPECT_EQ(buffer3.use_count(), 1);
}

TEST_F(StableReferenceBufferTest, DelayDeallocationPerformance)
{
    size_t count = 500000;

    while (count-- > 0)
    {
        cbeam::container::stable_reference_buffer::delay_deallocation delay_deallocation;
    }
}
