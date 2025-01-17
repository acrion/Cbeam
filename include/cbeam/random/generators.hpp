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

#pragma once

#include <random> // for std::mt19937, std::random_device, std::uniform_int_distribution
#include <string> // for std::string

namespace cbeam::random
{
    /// @brief Returns a reference to a thread-local std::mt19937 random number generator.
    ///
    /// This function creates a thread-local std::mt19937 random number generator if it doesn't already exist,
    /// and returns a reference to it. The generator is initialized with std::random_device upon first use,
    /// ensuring good initial randomness.
    ///
    /// @return Reference to the thread-local std::mt19937 generator.
    inline std::mt19937& default_generator()
    {
        /// Thread-local generator: Each thread gets its own instance of the generator.
        /// This ensures independence between threads and thread-safe random number generation.
        thread_local static std::mt19937 gen{std::random_device{}()};
        return gen;
    }

    /// \brief Returns a random number in the range [0, n-1].
    /// @param n The upper bound (exclusive) for the random number.
    /// @param gen Reference to a std::mt19937 random number generator, defaulting to the thread-local generator.
    /// @return A random number between 0 and n-1.
    inline std::size_t random_number(const std::size_t n, std::mt19937& gen = default_generator())
    {
        std::uniform_int_distribution<std::size_t> dist(0, n - 1);
        return dist(gen);
    }

    /// \brief Generates a random string of specified length.
    ///
    /// This function creates a random string consisting of alphanumeric characters (both lowercase and uppercase)
    /// and digits. It uses a thread-local `std::uniform_int_distribution` to pick characters uniformly from the
    /// character set. This ensures efficient and thread-safe generation of random strings.
    ///
    /// @param length The desired length of the random string.
    /// @param gen A reference to a `std::mt19937` random number generator, defaulting to the thread-local generator.
    /// @return A random string of the specified length.
    inline std::string random_string(std::string::size_type length, std::mt19937& gen = default_generator())
    {
        static auto& chrs = "0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

        std::string s;

        s.reserve(length);

        while (length--)
        {
            s += chrs[pick(gen)];
        }

        return s;
    }
}
