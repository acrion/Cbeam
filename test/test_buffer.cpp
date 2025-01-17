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

#include <cbeam/container/buffer.hpp> // for cbeam::container::buffer

#include <gtest/gtest.h>

#include <cstddef> // for std::size_t
#include <memory>  // for std::allocator

TEST(BufferTest, ConstructorAllocatesMemory)
{
    const std::size_t        size = 10;
    cbeam::container::buffer localBuffer(size);
    EXPECT_EQ(localBuffer.size(), size);
}

TEST(BufferTest, CopyConstructorCreatesDeepCopy)
{
    const std::size_t        size = 10;
    cbeam::container::buffer originalBuffer(size);
    cbeam::container::buffer copyBuffer(originalBuffer);

    originalBuffer.append("test", 4);
    EXPECT_NE(originalBuffer.size(), copyBuffer.size());
}

TEST(BufferTest, CopyAssignmentOperatorCreatesDeepCopy)
{
    const std::size_t        size = 10;
    cbeam::container::buffer originalBuffer(size);
    cbeam::container::buffer copyBuffer = originalBuffer;

    originalBuffer.append("test", 4);
    EXPECT_NE(originalBuffer.size(), copyBuffer.size());
}

TEST(BufferTest, AppendIncreasesSizeCorrectly)
{
    const char*              data         = "test";
    const std::size_t        lengthOfData = 4;
    cbeam::container::buffer testBuffer;
    testBuffer.append(data, lengthOfData);

    EXPECT_EQ(testBuffer.size(), lengthOfData);
}

TEST(BufferTest, ResetDeallocatesMemory)
{
    cbeam::container::buffer testBuffer;
    testBuffer.append("test", 4);
    testBuffer.reset();
    EXPECT_EQ(testBuffer.size(), 0);
    EXPECT_EQ(testBuffer.get(), nullptr);
}

TEST(BufferTest, SwapSwapsContents)
{
    cbeam::container::buffer buffer1(10);
    cbeam::container::buffer buffer2(20);

    std::size_t size1BeforeSwap = buffer1.size();
    std::size_t size2BeforeSwap = buffer2.size();

    buffer1.swap(buffer2);

    EXPECT_EQ(buffer1.size(), size2BeforeSwap);
    EXPECT_EQ(buffer2.size(), size1BeforeSwap);
}
