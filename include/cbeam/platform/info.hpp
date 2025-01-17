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

#include <cctype>  // for isdigit, std::isdigit
#include <cstddef> // for std::size_t

#include <string> // for std::allocator, std::operator+, std::string, std::char_traits, std::operator""s, std::to_string, std::string_literals

namespace cbeam::platform
{
    /**
     * @brief Returns the bitness (e.g. "64") of the current platform based on `sizeof(std::size_t)`.
     *
     * @return A string representing the number of bits, typically "32" or "64".
     */
    inline std::string get_bit_architecture()
    {
        return std::to_string(sizeof(std::size_t) * 8);
    }

    /**
     * @brief Returns a string identifying the platform architecture (e.g., "x86", "ARM", "MIPS", etc.).
     *
     * The detection is based on predefined compiler macros.
     *
     * @return A string describing the CPU architecture, or "Unknown" if not recognized.
     */
    inline std::string get_platform_architecture()
    {
        using namespace std::string_literals;

#if defined(__arm__) || defined(__aarch64__)
        return "ARM"s;
#elif defined(__powerpc64__) || defined(__powerpc__)
        return "PowerPC"s;
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
        return "x86"s;
#elif defined(__mips__) || defined(__mips64)
        return "MIPS"s;
#elif defined(__riscv)
        return "RISC-V"s;
#elif defined(__sparc__) || defined(__sparc)
        return "SPARC"s;
#elif defined(__AVR__)
        return "AVR"s;
#elif defined(__pic__) || defined(__PIC__)
        return "PIC"s;
#elif defined(ESP32) || defined(ESP8266)
        return "ESP"s;
#elif defined(__MSP430__)
        return "MSP430"s;
#elif defined(__SH1__) || defined(__SH2__) || defined(__SH3__) || defined(__SH4__)
        return "SuperH"s;
#else
        return "Unknown"s;
#endif
    }

    /**
     * @brief Returns a string describing both the architecture and bitness (e.g. "x86_64", "ARM32").
     *
     * @return A string combining the platform architecture and the number of bits.
     */
    inline std::string get_architecture()
    {
        const std::string platform = get_platform_architecture();
        const std::string bit      = get_bit_architecture();

        if (!platform.empty() && std::isdigit(platform.back()))
        {
            return platform + "_" + bit;
        }
        else
        {
            return platform + bit;
        }
    }

    /**
     * @brief Returns the kernel or operating system name (e.g., "Windows", "Linux", "Darwin").
     *
     * The detection is based on predefined macros.
     *
     * @return A string containing the kernel/OS name, or "Unknown" if not recognized.
     */
    inline std::string get_kernel_name()
    {
        using namespace std::string_literals;

#ifdef _WIN32
        return "Windows"s;
#elif defined(__linux__)
        return "Linux"s; // includes Android
#elif defined(__APPLE__)
        return "Darwin"s; // both macOS and iOS
#elif defined(__MACH__)
        return "Mach"s; // macOS before Darwin
#elif defined(__FreeBSD__)
        return "FreeBSD"s;
#elif defined(__sun) && defined(__SVR4)
        return "Solaris"s;
#elif defined(__NetBSD__)
        return "NetBSD"s;
#elif defined(__OpenBSD__)
        return "OpenBSD"s;
#elif defined(__vxworks)
        return "VxWorks"s;
#elif defined(__QNX__)
        return "QNX"s;
#else
        return "Unknown"s;
#endif
    }
}
