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

#include <cbeam/encoding/utf8.hpp> // for cbeam::encoding::has_utf8_specific_encoding, cbeam::encoding::is_valid_utf8

#include <gtest/gtest.h>

#include <string> // for std::allocator, std::string
#include <vector> // for std::vector

namespace cbeam::encoding
{
    TEST(Encoding, IsValidUtf8OrHasUtf8SpecificEncoding)
    {
        struct TestCase
        {
            std::string input;
            bool        isValidUtf8Expected;
            bool        hasUtf8SpecificEncodingExpected;
        };

        std::vector<TestCase> testCases = {
            {"Hello", true, false},           // ASCII text
            {"\xC2\xA9", true, true},         // UTF-8 encoded sign (©)
            {"\xE2\x82\xAC", true, true},     // UTF-8 encoded sign (€)
            {"\xF0\x9F\x98\x80", true, true}, // UTF-8 encoded sign (😀)
            {"\xC2", false, false},           // Invalid UTF-8 encoding (only first byte of © sign)
            {"\x80", false, false},           // Invalid UTF-8 encoding (a continuation byte without a leading byte)
        };

        int i = 0;
        for (const auto& testCase : testCases)
        {
            bool isValidUtf8Result             = is_valid_utf8(testCase.input);
            bool hasUtf8SpecificEncodingResult = has_utf8_specific_encoding(testCase.input);

            EXPECT_EQ(isValidUtf8Result, testCase.isValidUtf8Expected) << "Fail in is_valid_utf8 with input " << i << " (" << testCase.input << ")";
            EXPECT_EQ(hasUtf8SpecificEncodingResult, testCase.hasUtf8SpecificEncodingExpected) << "Fail in has_utf8_specific_encoding with input " << i << " (" << testCase.input << ")";

            ++i;
        }
    }
}
