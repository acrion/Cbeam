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

/**
 * @file compiler_compatibility.hpp
 * @brief Header file containing macros for compiler compatibility and warning suppression.
 *
 * This file may contain various utilities and macros to handle compiler-specific behavior and
 * compatibility issues.
 */

// clang-format off
/**
 * @def CBEAM_SUPPRESS_WARNINGS_PUSH()
 * Temporarily suppresses compiler warnings for third-party library includes.
 *
 * This macro is intended to be used before including a third-party library header that generates
 * compiler warnings. It works by pushing the current warning state onto a stack and setting the
 * compiler to ignore specific warnings. This helps in keeping the build logs clean from warnings
 * in external code that are not of concern for the project.
 *
 * The suppression mechanism varies based on the compiler (MSVC, GCC, Clang, Mingw).
 */
#if defined(_MSC_VER)
    // MSVC-specific warning suppression
    #define CBEAM_SUPPRESS_WARNINGS_PUSH() \
        __pragma(warning(push, 0))
#elif defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__) || defined(__MINGW64__)
    // GCC, Clang, and Mingw-specific warning suppression
    #define CBEAM_SUPPRESS_WARNINGS_PUSH()                              \
        _Pragma("GCC diagnostic push")                                  \
        _Pragma("GCC diagnostic ignored \"-Wsign-compare\"")            \
        _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") \
        _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")     \
        _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")        \
        _Pragma("GCC diagnostic ignored \"-Wextra\"")                   \
        _Pragma("GCC diagnostic ignored \"-Wnonnull\"")                 \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"")
#else
    #define CBEAM_SUPPRESS_WARNINGS_PUSH()
#endif

/**
 * @def CBEAM_SUPPRESS_WARNINGS_POP()
 * Restores the compiler warning state to what it was before CBEAM_SUPPRESS_WARNINGS_PUSH.
 *
 * This macro should be used after including a third-party library header to revert the compiler's
 * warning state back to its previous configuration. It ensures that the warning suppression is
 * only effective for the specific includes where it's applied.
 *
 * The restoration mechanism varies based on the compiler (MSVC, GCC, Clang, Mingw).
 */
#if defined(_MSC_VER)
    // MSVC-specific warning restoration
    #define CBEAM_SUPPRESS_WARNINGS_POP() \
        __pragma(warning(pop))
#elif defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__) || defined(__MINGW64__)
    // GCC, Clang, and Mingw-specific warning restoration
    #define CBEAM_SUPPRESS_WARNINGS_POP() \
        _Pragma("GCC diagnostic pop")
#else
    #define CBEAM_SUPPRESS_WARNINGS_POP()
#endif
// clang-format on
