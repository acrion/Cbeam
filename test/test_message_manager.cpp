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

#include <cbeam/concurrency/message_manager.hpp>
#include <cbeam/lifecycle/singleton.hpp>

#include <atomic>
#include <cmath> // for std::sqrt
#include <gtest/gtest.h>

// Helper function to check primality
static bool check_prime_number(uint64_t p)
{
    if (p < 2) return false;
    if (p % 2 == 0) return (p == 2);

    const uint64_t stop = static_cast<uint64_t>(std::sqrt((long double)p));
    for (uint64_t i = 3; i <= stop; i += 2)
    {
        if (p % i == 0) return false;
    }
    return true;
}

class MessageManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        cbeam::lifecycle::singleton_control::reset();
        cbeam::lifecycle::singleton_control::set_operational();
    }
};

// Just a small sanity test
TEST_F(MessageManagerTest, Construction)
{
    cbeam::concurrency::message_manager<uint64_t> mm;
}

// The prime test with the new 'wait_until_empty' functionality
TEST_F(MessageManagerTest, TestPrime)
{
    // We define two IDs for the message_manager:
    //   1) check_prime => checks primality
    //   2) count_prime => increments a counter if a prime was found
    constexpr size_t check_prime = 1;
    constexpr size_t count_prime = 2;

    cbeam::concurrency::message_manager<uint64_t> mm;

    // Shared counter to track how many primes are found
    std::atomic<size_t> prime_count{0};

    // Handler that counts primes
    mm.add_handler(count_prime, [&](uint64_t msg)
                   {
                       // Just increment the counter
                       ++prime_count; },
                   nullptr,
                   nullptr,
                   "count_prime");

    constexpr size_t n_threads = 4;
    for (size_t i = 0; i < n_threads; i++)
    {
        // Handler that checks primality, and if prime => sends a message to `count_prime`
        mm.add_handler(check_prime, [&](uint64_t number_to_check)
                       {
                           if (check_prime_number(number_to_check))
                           {
                               mm.send_message(count_prime, number_to_check);
                           } },
                       nullptr,
                       nullptr,
                       "check_prime_" + std::to_string(i));
    }

    // Send all numbers from 10000000001 to 10000100001 to `check_prime`
    for (uint64_t number_to_check = 10000000001ULL; number_to_check <= 10000100001ULL; ++number_to_check)
    {
        mm.send_message(check_prime, number_to_check);
    }

    // Now wait for the check_prime queue to be empty
    // (i.e. all primality checks have been popped and processed)
    mm.wait_until_empty(check_prime);

    // Also wait until the count_prime queue is empty.
    // This ensures that all prime-sending messages have been popped and processed.
    mm.wait_until_empty(count_prime);

    // Dispose the handlers
    mm.dispose(check_prime);
    mm.dispose(count_prime);

    // We expect 4306 primes in that numeric range
    EXPECT_EQ(prime_count.load(), static_cast<size_t>(4306));
}
