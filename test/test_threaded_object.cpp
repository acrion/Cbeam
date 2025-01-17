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

#include <cbeam/concurrency/threaded_object.hpp> // for cbeam::concurrency::threaded_object, cbeam::concurrency::tag_tracker

#include <gtest/gtest.h>

#include <atomic> // for std::atomic
#include <chrono> // for std::chrono::milliseconds
#include <memory> // for std::unique_ptr, std::allocator
#include <thread> // for std::this_thread::sleep_for

class TestThreadedObject : public cbeam::concurrency::threaded_object<TestThreadedObject, int>
{
public:
    TestThreadedObject(typename cbeam::concurrency::threaded_object<TestThreadedObject, int>::construction_token)
        : cbeam::concurrency::threaded_object<TestThreadedObject, int>()
    {
    }

    void on_start() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        _has_run = true;
    }

    void on_exit() override
    {
        _finished = true;
    }

    std::atomic<bool> _has_run{false};
    std::atomic<bool> _finished{false};
};

TEST(ThreadedObjectTest, CreateTest)
{
    std::shared_ptr<std::mutex>              mtx{std::make_shared<std::mutex>()};
    std::shared_ptr<std::condition_variable> cv{std::make_shared<std::condition_variable>()};
    auto                                     obj = cbeam::concurrency::threaded_object<TestThreadedObject, int>::create(mtx, cv);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_NE(obj, nullptr);
}

TEST(ThreadedObjectTest, WorkerThreadRunsTest)
{
    std::shared_ptr<std::mutex>              mtx{std::make_shared<std::mutex>()};
    std::shared_ptr<std::condition_variable> cv{std::make_shared<std::condition_variable>()};
    auto                                     obj = cbeam::concurrency::threaded_object<TestThreadedObject, int>::create(mtx, cv);
    ASSERT_FALSE(obj->_has_run);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(obj->_has_run);
}

TEST(ThreadedObjectTest, DestructorTest)
{
    {
        std::shared_ptr<std::mutex>              mtx{std::make_shared<std::mutex>()};
        std::shared_ptr<std::condition_variable> cv{std::make_shared<std::condition_variable>()};
        auto                                     obj = cbeam::concurrency::threaded_object<TestThreadedObject, int>::create(mtx, cv);
    }
    SUCCEED();
}
