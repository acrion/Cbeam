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

#include <cbeam/lifecycle/singleton.hpp> // for cbeam::lifecycle::singleton, cbeam::lifecycle::singleton_control

#include <gtest/gtest.h>

#include <algorithm> // for std::equal, std::max
#include <cstddef>   // for std::size_t
#include <memory>    // for std::allocator, std::shared_ptr
#include <string>    // for std::basic_string
#include <thread>    // for std::thread
#include <vector>    // for std::vector

template <std::size_t T>
class TestClass
{
public:
    static inline int destruction_count = 0;
    static inline int instance_count    = 0;
    int               _test{};

    TestClass()
    {
        ++instance_count;
    }

    ~TestClass() noexcept
    {
        ++destruction_count;
    }
};

class SingletonTest : public ::testing::Test
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

TEST_F(SingletonTest, ResourceRelease)
{
    {
        std::shared_ptr<TestClass<1>> singleton1 = cbeam::lifecycle::singleton<TestClass<1>>::get("singleton1");
    }

    EXPECT_EQ(TestClass<1>::destruction_count, 0);
    cbeam::lifecycle::singleton<TestClass<1>>::reset();
    EXPECT_EQ(TestClass<1>::destruction_count, 1);
}

TEST_F(SingletonTest, InstanceCreation)
{
    std::shared_ptr<TestClass<2>> instance1 = cbeam::lifecycle::singleton<TestClass<2>>::get("singleton2");
    std::shared_ptr<TestClass<2>> instance2 = cbeam::lifecycle::singleton<TestClass<2>>::get("singleton2");

    instance1.get()->_test = 5;

    EXPECT_EQ(instance1.get()->_test, instance2.get()->_test);
    EXPECT_EQ(TestClass<2>::instance_count, 1);
}

TEST_F(SingletonTest, ThreadSafety)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([]()
                             { auto instance = cbeam::lifecycle::singleton<TestClass<3>>::get("singleton3"); });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(TestClass<3>::instance_count, 1);
}

TEST_F(SingletonTest, ResetFunctionality1)
{
    {
        auto instance = cbeam::lifecycle::singleton<TestClass<4>>::get("singleton4");
        EXPECT_EQ(TestClass<4>::destruction_count, 0);
    }
    EXPECT_EQ(TestClass<4>::destruction_count, 0);

    cbeam::lifecycle::singleton_control::reset();
    EXPECT_EQ(TestClass<4>::destruction_count, 1);
}

TEST_F(SingletonTest, ResetFunctionality2)
{
    {
        auto instance = cbeam::lifecycle::singleton<TestClass<5>>::get("singleton5");
        EXPECT_EQ(TestClass<5>::destruction_count, 0);
    }
    EXPECT_EQ(TestClass<5>::destruction_count, 0);

    cbeam::lifecycle::singleton<TestClass<5>>::reset();
    EXPECT_EQ(TestClass<5>::destruction_count, 1);
}
