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

#include <cbeam/logging/log_manager.hpp> // for CBEAM_LOG

#ifdef _WIN32
    #include "windows_config.hpp"
#else
    #include <time.h> // for clock_getres, timespec, CLOCK_MONOTONIC
#endif

#include <algorithm> // for std::min
#include <chrono>    // for std::chrono::operator-, std::chrono::duration, std::chrono::high_resolution_clock
#include <mutex>     // for std::mutex, std::lock_guard
#include <string>    // for std::string_literals::operator""s, std::string_literals

namespace cbeam::platform
{
    /**
     * @brief Retrieves the smallest measurable time unit (epsilon) of the high-resolution clock.
     *
     * This function calculates the precision of std::chrono::high_resolution_clock by determining
     * the minimum time interval that can be measured. It utilizes platform-specific methods to
     * obtain this value. On Windows, it uses QueryPerformanceFrequency; on Linux, clock_getres;
     * and on macOS, mach_timebase_info. If the precision cannot be determined through these methods,
     * a fallback algorithm is used to estimate it. The estimated value will never be smaller than
     * the actual epsilon.
     *
     * @return The smallest measurable time interval of the high-resolution clock in seconds.
     */
    static inline double get_clock_precision()
    {
        static double               clock_precision{-1.0};
        static std::mutex           mtx;
        std::lock_guard<std::mutex> lock(mtx);

        if (clock_precision == -1.0)
        {
            using namespace std::string_literals;

#ifdef _WIN32
            LARGE_INTEGER frequency;
            if (QueryPerformanceFrequency(&frequency))
            {
                clockPrecision = 1.0 / frequency.QuadPart;
            }
#elif defined(__linux__)
            struct timespec ts;
            if (clock_getres(CLOCK_MONOTONIC, &ts) == 0)
            {
                clock_precision = ts.tv_sec + ts.tv_nsec * 1e-9;
            }
#elif defined(__APPLE__)
            mach_timebase_info_data_t info;
            if (mach_timebase_info(&info) == 0)
            {
                clockPrecision = (double)info.numer / info.denom * 1e-9;
            }
#endif

            if (clock_precision == -1.0)
            {
                CBEAM_LOG("Unable to determine resolution of high resolution clock, using fallback algorithm"s);
                int iterations = 100;
                while (iterations--)
                {
                    auto   startTime = std::chrono::high_resolution_clock::now();
                    double result;
                    do
                    {
                        result = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startTime).count();
                    } while (result == 0.0);

                    clock_precision = std::min(clock_precision, result);
                }
            }
        }

        return clock_precision;
    }
}