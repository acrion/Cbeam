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

#include <cbeam/convert/string.hpp> // for cbeam::convert::from_string, cbeam::convert::to_string, cbeam::convert::to_wstring, cbeam::convert::to_lower

#include <gtest/gtest.h>

#include <cstdint> // for std::uintptr_t, uintptr_t

#include <chrono>        // for std::chrono::system_clock
#include <ostream>       // for std::basic_ostream, std::operator<<, std::hex, std::stringstream
#include <regex>         // for std::regex_match, std::regex
#include <string>        // for std::allocator, std::wstring, std::string, std::char_traits

namespace cbeam::convert
{
    TEST(ToLowerTest, HandlesUmlautsAndAccents)
    {
        EXPECT_EQ(to_lower("A"), "a");
        EXPECT_EQ(to_lower("Ä"), "Ä"); // to_lower only converts characters A-Z
    }

    TEST(FromStringTest, ConvertToInt)
    {
        EXPECT_EQ(cbeam::convert::from_string<int>("123"), 123);
        EXPECT_EQ(cbeam::convert::from_string<int>("-123"), -123);
    }

    TEST(FromStringTest, ConvertToDouble)
    {
        EXPECT_DOUBLE_EQ(cbeam::convert::from_string<double>("123.456"), 123.456);
        EXPECT_DOUBLE_EQ(cbeam::convert::from_string<double>("-123.456"), -123.456);
    }

    TEST(FromStringTest, ConvertToBool)
    {
        EXPECT_EQ(cbeam::convert::from_string<bool>("1"), true);
        EXPECT_EQ(cbeam::convert::from_string<bool>("0"), false);
    }

    TEST(FromStringTest, ConvertToVoidPointer)
    {
        std::uintptr_t    testAddress     = 0x12345678;
        void*             expectedPointer = reinterpret_cast<void*>(testAddress);
        std::stringstream ss;
        ss << "0x" << std::hex << testAddress;
        void* result = cbeam::convert::from_string<void*>(ss.str());
        EXPECT_EQ(result, expectedPointer);
    }

    TEST(FromStringTest, ConvertToWString_ValidUTF8)
    {
        std::string  utf8String = "Hallo Welt"; // a valid UTF-8 string.
        std::wstring wString    = cbeam::convert::from_string<std::wstring>(utf8String);
        std::wstring expected   = L"Hallo Welt";
        EXPECT_EQ(wString, expected);
    }

    TEST(FromStringTest, ConvertToWString_InvalidUTF8)
    {
        std::string  invalidUtf8String = "Hallo \xFF Welt"; // invalid UTF-8 sequence
        std::wstring wString           = cbeam::convert::from_string<std::wstring>(invalidUtf8String);
        // Erwarte eine elementweise Konvertierung.
        std::wstring expected = L"Hallo \xFF Welt";
        EXPECT_EQ(wString, expected);
    }

    TEST(FromStringTest, ConvertToWString_EmptyString)
    {
        std::string  emptyString = "";
        std::wstring wString     = cbeam::convert::from_string<std::wstring>(emptyString);
        std::wstring expected    = L"";
        EXPECT_EQ(wString, expected);
    }

    TEST(FromStringTest, ConvertToWString_SpecialCharacters)
    {
        std::string  specialCharsString = "Élève - Überprüfung";
        std::wstring wString            = cbeam::convert::from_string<std::wstring>(specialCharsString);
        std::wstring expected           = L"Élève - Überprüfung";
        EXPECT_EQ(wString, expected);
    }

    TEST(ToStringTest, ConvertIntToString)
    {
        int         value  = 123;
        std::string result = cbeam::convert::to_string(value);
        EXPECT_EQ(result, "123");
    }

    TEST(ToStringTest, ConvertNegativeIntToString)
    {
        int         value  = -123;
        std::string result = cbeam::convert::to_string(value);
        EXPECT_EQ(result, "-123");
    }

    TEST(ToStringTest, ConvertDoubleToString)
    {
        double      value  = 123.456;
        std::string result = cbeam::convert::to_string(value);
        EXPECT_EQ(result, "123.456");
    }

    TEST(ToStringTest, ConvertBooleanToString)
    {
        bool        value  = true;
        std::string result = cbeam::convert::to_string(value);
        EXPECT_EQ(result, "1");
    }

    TEST(ToStringTest, ConvertPointerToString)
    {
        int*        value    = (int*)0x12345678;
        std::string result   = cbeam::convert::to_string(value);
        std::string expected = "0x12345678";
        EXPECT_EQ(result, expected);
    }

    TEST(ToStringTest, ConvertCharToString)
    {
        char        value  = 'A';
        std::string result = cbeam::convert::to_string(value);
        EXPECT_EQ(result, "A");
    }

    TEST(ToStringWStringTest, ConvertValidUTF16ToString)
    {
        std::wstring wString = L"Hallo Welt";
        std::string  result  = cbeam::convert::to_string(wString);
        EXPECT_EQ(result, "Hallo Welt");
    }

    TEST(ToStringWStringTest, ConvertSpecialCharacters)
    {
        std::wstring wString = L"Élève - Überprüfung";
        std::string  result  = cbeam::convert::to_string(wString);
        EXPECT_EQ(result, "Élève - Überprüfung");
    }

    TEST(ToStringWStringTest, ConvertEmptyWString)
    {
        std::wstring wString = L"";
        std::string  result  = cbeam::convert::to_string(wString);
        EXPECT_EQ(result, "");
    }

    TEST(ToStringWStringTest, ConvertInvalidUTF16)
    {
        std::wstring wString  = L"Hallo \xD800 Welt"; // Invalid UTF-16 (surrogate half without a pair)
        std::string  result   = cbeam::convert::to_string(wString);
        std::string  expected = "Hallo \xD8 Welt"; // we expect that 0xD800 will be converted to a single byte 0xD8, leaving out \0x00 ("Hallo Ø Welt" in extended ASCII encoding)
        EXPECT_EQ(result, expected);
    }

    TEST(TimePointToStringTest, ConvertTimePointToString)
    {
        auto        now    = std::chrono::system_clock::now();
        std::string result = cbeam::convert::to_string(now);

        // Überprüfe das Format "YYYY-MM-DD HH:MM:SS.mmm"
        std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})");
        EXPECT_TRUE(std::regex_match(result, pattern));
    }

    TEST(ToWStringTest, ConvertIntToWString)
    {
        int          value  = 123;
        std::wstring result = cbeam::convert::to_wstring(value);
        EXPECT_EQ(result, L"123");
    }

    TEST(ToWStringTest, ConvertDoubleToWString)
    {
        double       value  = 123.456;
        std::wstring result = cbeam::convert::to_wstring(value);
        EXPECT_EQ(result, L"123.456");
    }

    TEST(ToWStringTest, ConvertBoolToWString)
    {
        bool         value  = true;
        std::wstring result = cbeam::convert::to_wstring(value);
        EXPECT_EQ(result, L"1");
    }
}
