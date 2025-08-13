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

#include <atomic>
#include <barrier>
#include <cstring>
#include <stdint.h> // for uint8_t
#include <thread>
#include <vector>

static void fill_bytes(void* p, std::size_t n, uint8_t v)
{
    std::memset(p, v, n);
}

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

// Regression: buffer ctor must not copy past end (would have ASAN OOB).
TEST_F(StableReferenceBufferTest, BufferCtorCopiesWithinBounds)
{
    const std::size_t    n = 64;
    std::vector<uint8_t> src(n, 0xAB);

    cbeam::container::buffer b(src.data(), src.size()); // should memcpy at start, not past end
    ASSERT_EQ(b.size(), n);
    ASSERT_NE(b.get(), nullptr);

    auto* p = static_cast<uint8_t*>(b.get());
    for (std::size_t i = 0; i < n; ++i)
    {
        EXPECT_EQ(p[i], 0xAB) << "Mismatch at index " << i;
    }
}

// Regression: copy ctor/assignment must preserve _size.
TEST_F(StableReferenceBufferTest, CopyPreservesSizeAndUseCount)
{
    cbeam::container::stable_reference_buffer a(10, sizeof(int)); // 40 bytes
    EXPECT_EQ(a.size(), 10 * sizeof(int));

    cbeam::container::stable_reference_buffer b(a); // copy-ctor
    EXPECT_EQ(b.size(), a.size());                  // would be 0 previously
    EXPECT_EQ(a.use_count(), 2u);
    EXPECT_EQ(b.use_count(), 2u);

    cbeam::container::stable_reference_buffer c;
    c = a;                         // copy-assign
    EXPECT_EQ(c.size(), a.size()); // would be 0 previously
    EXPECT_EQ(a.use_count(), 3u);
    EXPECT_EQ(c.use_count(), 3u);
}

// Behaviour: append on shared buffer must do Copy-on-Write (no UAF).
TEST_F(StableReferenceBufferTest, AppendTriggersCopyOnWriteWhenShared)
{
    // Allocate and put a distinct pattern.
    cbeam::container::stable_reference_buffer a(16, 1);
    fill_bytes(a.get(), a.size(), 0x11);

    // Share pointer (use_count == 2).
    cbeam::container::stable_reference_buffer b = a;

    auto* a_ptr_before = static_cast<uint8_t*>(a.get());
    auto* b_ptr_before = static_cast<uint8_t*>(b.get());
    ASSERT_EQ(a_ptr_before, b_ptr_before);
    ASSERT_EQ(a.use_count(), 2u);

    // Append on 'a' must NOT reallocate in-place (since shared). It must COW.
    uint8_t tail[8];
    std::memset(tail, 0x22, sizeof(tail));
    a.append(tail, sizeof(tail));

    // After COW, 'a' and 'b' must diverge.
    auto* a_ptr_after = static_cast<uint8_t*>(a.get());
    auto* b_ptr_after = static_cast<uint8_t*>(b.get());
    EXPECT_NE(a_ptr_after, b_ptr_after) << "Append on shared instance must not move shared block in place";

    // Original content in 'b' must remain intact (0x11).
    for (std::size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_EQ(b_ptr_after[i], 0x11) << "Original buffer should be unchanged after COW";
    }

    // 'a' must have old content + appended tail.
    ASSERT_EQ(a.size(), 16 + sizeof(tail));
    for (std::size_t i = 0; i < 16; ++i)
    {
        EXPECT_EQ(a_ptr_after[i], 0x11);
    }
    for (std::size_t i = 16; i < a.size(); ++i)
    {
        EXPECT_EQ(a_ptr_after[i], 0x22);
    }

    // Refcounts: old block now only referenced by 'b'.
    EXPECT_EQ(b.use_count(), 1u);
    // New block for 'a' should start at initial count (usually 1).
    EXPECT_GE(a.use_count(), 1u);
}

// Behaviour: append with exclusive ownership may move pointer but must preserve refcount mapping.
TEST_F(StableReferenceBufferTest, AppendExclusiveOwnerKeepsMapConsistent)
{
    cbeam::container::stable_reference_buffer a(32, 1);
    EXPECT_EQ(a.use_count(), 1u);
    auto* old_ptr = a.get();

    uint8_t ext[16];
    std::memset(ext, 0x7A, sizeof(ext));
    a.append(ext, sizeof(ext));

    auto* new_ptr = a.get();
    EXPECT_TRUE(cbeam::container::stable_reference_buffer::is_known(new_ptr));
    EXPECT_EQ(a.use_count(), 1u);

    if (new_ptr != old_ptr)
    {
        // Old pointer must no longer be tracked.
        EXPECT_FALSE(cbeam::container::stable_reference_buffer::is_known(old_ptr));
    }
}

// safe_get(): returns nullptr if single owner, non-null if additional guard/owner exists.
TEST_F(StableReferenceBufferTest, SafeGetRespectsUseCount)
{
    cbeam::container::stable_reference_buffer a(8, 1);
    // Single owner -> nullptr (by design)
    EXPECT_EQ(a.safe_get(), nullptr);

    // Add a second reference -> now allowed.
    cbeam::container::stable_reference_buffer b = a;
    EXPECT_NE(a.safe_get(), nullptr);
    EXPECT_NE(b.safe_get(), nullptr);
}

// delay_deallocation should actually decrement and free after scope ends.
TEST_F(StableReferenceBufferTest, DelayDeallocationActuallyDecrementsAndFrees)
{
    uint8_t* raw = nullptr;
    {
        cbeam::container::stable_reference_buffer::delay_deallocation guard;

        cbeam::container::stable_reference_buffer tmp(4, 1);
        raw  = static_cast<uint8_t*>(tmp.get());
        *raw = 0x42;
        EXPECT_TRUE(cbeam::container::stable_reference_buffer::is_known(raw));

        // Drop last SRB reference inside scope.
        tmp.reset();
        // Still protected by guard.
        EXPECT_TRUE(cbeam::container::stable_reference_buffer::is_known(raw));
        EXPECT_EQ(*raw, 0x42);
    }
    // After guard is gone, raw should be deallocated (untracked).
    EXPECT_FALSE(cbeam::container::stable_reference_buffer::is_known(raw));
}

// Creating from raw pointer forbids append (must throw).
TEST_F(StableReferenceBufferTest, ConstructFromRawPointerProhibitsAppend)
{
    cbeam::container::stable_reference_buffer owner(8, 1);
    void*                                     p = owner.get();

    // reference from raw
    cbeam::container::stable_reference_buffer ref(p);
    EXPECT_EQ(ref.size(), 0u);

    uint8_t x = 0;
    EXPECT_THROW(ref.append(&x, 1), cbeam::error::logic_error);
}

// Multithreaded read + mutator append must not crash nor corrupt memory (COW).
TEST_F(StableReferenceBufferTest, MultiThreadedReadersAndOneAppender)
{
    // Heavier test; enable in TSAN/ASAN builds.
    cbeam::container::stable_reference_buffer base(1024, 1);
    fill_bytes(base.get(), base.size(), 0xEE);

    cbeam::container::stable_reference_buffer shared = base; // readers will hold this

    constexpr int     readers = 8;
    constexpr int     iters   = 1000;
    std::atomic<bool> stop{false};
    std::barrier      sync_start(readers + 1);

    auto reader = [&]()
    {
        sync_start.arrive_and_wait();
        for (int i = 0; i < iters && !stop.load(); ++i)
        {
            auto* p = static_cast<uint8_t*>(shared.safe_get());
            if (p)
            {
                for (std::size_t j = 0; j < shared.size(); ++j)
                {
                    // Either original 0xEE, or some reader sees a COW'ed block if it ever changes.
                    // But it must never access freed memory (TSAN/ASAN would catch).
                    volatile uint8_t v = p[j];
                    (void)v;
                }
            }
        }
    };

    std::vector<std::thread> ts;
    for (int i = 0; i < readers; ++i)
        ts.emplace_back(reader);

    sync_start.arrive_and_wait();

    // Mutator performs appends; must COW and never invalidate 'shared'.
    for (int k = 0; k < 100; ++k)
    {
        uint8_t blob[256];
        std::memset(blob, k & 0xFF, sizeof(blob));
        base.append(blob, sizeof(blob));
    }

    stop.store(true);
    for (auto& t : ts)
        t.join();

    // Shared must still be the original block.
    EXPECT_EQ(shared.size(), 1024u);
    auto* sp = static_cast<uint8_t*>(shared.get());
    for (std::size_t j = 0; j < shared.size(); ++j)
    {
        EXPECT_EQ(sp[j], 0xEE);
    }
}

// force many unique entries to probe shared-map capacity behaviour.
TEST_F(StableReferenceBufferTest, DISABLED_MapCapacityStress)
{
    // This will allocate many distinct buffers and thus many map entries.
    // Adjust count to exceed current shared-memory capacity to observe expected failure/exception.
    std::vector<cbeam::container::stable_reference_buffer> vec;
    try
    {
        for (int i = 0; i < 5000; ++i)
        {
            vec.emplace_back(8, 1);
            *static_cast<uint8_t*>(vec.back().get()) = static_cast<uint8_t>(i);
        }
        SUCCEED() << "No capacity issue with current configuration.";
    }
    catch (const std::exception& ex)
    {
        // Acceptable outcome: serialize() complained about capacity (not memory corruption).
        SUCCEED() << "Hit expected capacity constraint: " << ex.what();
    }
}