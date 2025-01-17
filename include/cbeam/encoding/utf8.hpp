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

#include <cstddef> // for std::size_t

#include <string> // for std::basic_string, std::string

namespace cbeam::encoding
{
    /// \brief Returns if the given string conforms with UTF8 encoding
    inline bool is_valid_utf8(const std::string& s)
    {
        auto len = s.size();
        for (std::size_t i = 0; i < len; ++i)
        {
            unsigned char c = s[i];
            if (c < 0x80)
            {
                continue;
            }
            else if ((c >= 0xC2 && c <= 0xDF) && (i + 1 < len))
            {
                if (!(s[i + 1] & 0x80) || (s[i + 1] & 0x40)) return false;
                ++i;
            }
            else if ((c >= 0xE0 && c <= 0xEF) && (i + 2 < len))
            {
                if (!(s[i + 1] & 0x80) || (s[i + 1] & 0x40) || !(s[i + 2] & 0x80) || (s[i + 2] & 0x40)) return false;
                i += 2;
            }
            else if ((c >= 0xF0 && c <= 0xF4) && (i + 3 < len))
            {
                if (!(s[i + 1] & 0x80) || (s[i + 1] & 0x40) || !(s[i + 2] & 0x80) || (s[i + 2] & 0x40) || !(s[i + 3] & 0x80) || (s[i + 3] & 0x40)) return false;
                i += 3;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    /// \brief Checks if the given string uses encoding that is specific to UTF-8.
    /// \details This function returns true if the string contains characters encoded in a manner that
    /// is specific to UTF-8 and not part of the ASCII range (0-127). It helps to identify
    /// strings that are likely not extended ASCII (0-255), but rather UTF-8 encoded, based on
    /// the presence of multibyte characters that are conformant to the UTF-8 encoding rules.
    /// \param s the string to check
    /// \return true if the string uses UTF-8 specific encoding; false otherwise.
    inline bool has_utf8_specific_encoding(const std::string& s)
    {
        bool all_ascii = true;
        for (unsigned char c : s)
        {
            if (c >= 128)
            {
                all_ascii = false;
                break;
            }
        }
        if (all_ascii) return false;

        return is_valid_utf8(s);
    }
}