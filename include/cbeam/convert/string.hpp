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

#include <cbeam/platform/compiler_compatibility.hpp>

#include <time.h> // for localtime, strftime, time_t

#include <cctype> // std::tolower

#include <algorithm> // for std::transform
#include <chrono>    // for std::chrono::duration_cast, std::chrono::milliseconds, std::chrono::seconds, std::chrono::time_point
#include <codecvt>   // for std::codecvt_utf8_utf16, note https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0618r0.html
#include <cstdint>   // for std::uintptr_t
#include <iomanip>   // for std::operator<<, std::setfill, std::setw
#include <locale>    // for std::wstring_convert, std::locale

#include <sstream>     // for std::basic_ostream, std::operator<<, std::stringstream, std::basic_ios::imbue, std::hex, std::showbase, std::istringstream, std::ostream
#include <stdexcept>   // for std::range_error
#include <string>      // for std::basic_string, std::char_traits, std::string, std::wstring, std::allocator
#include <type_traits> // for std::declval, std::enable_if, std::false_type, std::true_type, std::void_t

namespace cbeam::convert
{
    /**
     * @brief Returns a string consisting of `indentation` tab characters.
     *
     * @param indentation The number of tabs to generate.
     * @return A `std::string` composed of `indentation` tab characters.
     */
    inline std::string indent(int indentation)
    {
        return std::string(indentation, '\t');
    }

    /**
     * @brief Converts characters A-Z in the given string to lower case and returns the modified string.
     *
     * @details This function is compatible with UTF-8-encoded strings (because the byte representations
     * of A-Z only occur in ASCII range, which is unaffected by multi-byte sequences).
     *
     * @param s The string to be transformed to lower case.
     * @return A new string with all A-Z characters converted to lower case.
     */
    inline std::string to_lower(std::string s)
    {
        std::transform(s.begin(),
                       s.end(),
                       s.begin(),
                       [](unsigned char c)
                       { return (char)std::tolower(c); });
        return s;
    }

    /**
     * @brief Escapes specified characters in a given string.
     *
     * This function takes an input string and escapes characters that are found in the
     * `characters_to_escape` string. Each occurrence of any character from
     * `characters_to_escape` is prefixed with the specified `escape_character`.
     *
     * @param input The string to be processed.
     * @param escape_character The character used for escaping.
     * @param characters_to_escape A string containing all characters that should be escaped.
     * @return A new string with the specified characters escaped.
     *
     * @example cbeam/convert/string.hpp
     * @code{.cpp}
     * std::string original = "Hello, world!";
     * std::string escaped = escape_string(original, '\\', ",!");
     * // escaped would be "Hello\, world\!"
     * @endcode
     */
    inline std::string escape_string(const std::string& input, const char escape_character, const std::string& characters_to_escape)
    {
        std::ostringstream escaped;
        for (char ch : input)
        {
            if (characters_to_escape.find(ch) != std::string::npos)
            {
                escaped << escape_character;
            }
            escaped << ch;
        }
        return escaped.str();
    }

    /**
     * @brief Unescapes specified characters in a given string.
     *
     * This function takes an input string and removes the escape characters that precede any of
     * the characters specified in `characters_to_unescape`. Only escape characters that
     * directly precede a character from `characters_to_unescape` are removed.
     *
     * @param input The string to be processed.
     * @param escape_character The character used for escaping.
     * @param characters_to_unescape A string containing all characters that should be unescaped.
     * @return std::string A new string with the specified characters unescaped.
     *
     * @exception std::bad_alloc Thrown if memory allocation fails during string processing.
     *
     * @example cbeam/convert/string.hpp
     * @code{.cpp}
     * std::string escaped = "Hello\\, world\\!";
     * std::string original = unescape_string(escaped, '\\', ",!");
     * // original would be "Hello, world!"
     * @endcode
     */
    inline std::string unescape_string(const std::string& input, char escape_character, const std::string& characters_to_unescape)
    {
        std::string result;
        result.reserve(input.size());

        auto it = input.begin();
        while (it != input.end())
        {
            char c = *it;
            if (c == escape_character && it + 1 != input.end() && characters_to_unescape.find(*(it + 1)) != std::string::npos)
            {
                result.push_back(*(++it));
            }
            else
            {
                result.push_back(c);
            }
            ++it;
        }

        return result;
    }

    /// \brief Converts a given std::string to a specified type.
    ///
    /// This function template facilitates the conversion of a std::string to a variety of
    /// types, including fundamental types (like int, float, double) and pointer types (e.g., void*).
    /// \details For floating point types such as float and double, a period ('.') is used
    /// as the decimal separator, independent of the current locale setting. For void* type,
    /// the string is interpreted as a hexadecimal number with an optional leading "0x". This
    /// function leverages std::istringstream for conversion, imbuing it with the "C" locale
    /// to ensure consistent parsing behavior across different locales.
    /// \note Refer to https://en.cppreference.com/w/cpp/io/basic_istream/operator_gtgt for
    /// detailed behavior of the extraction operator used in this function.
    /// \tparam T The target type for the conversion. The type must support extraction via
    /// std::istringstream.
    /// \param str The string to be converted.
    /// \return The converted value of type T.
    template <typename T>
    T from_string(const std::string& str)
    {
        T                  result;
        std::istringstream istr(str);
        istr.imbue(std::locale("C"));
        istr >> result;
        return result;
    }

    /**
     * @brief Converts the given std::string to std::wstring using UTF-8 to UTF-16 encoding.
     *
     * This function utilizes std::wstring_convert with std::codecvt_utf8_utf16 to perform
     * the conversion from string to wide string. It attempts the conversion and returns the
     * resultant wide string if successful. In case of a range error (due to invalid UTF-8 input),
     * it falls back to an element-wise conversion.
     *
     * @note The std::wstring_convert is deprecated in C++17. See the proposal to remove it at
     * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2872r2.pdf for more details.
     *
     * @param str The string to be converted.
     * @return The converted wide string in UTF-16 encoding, or the element-wise conversion result in case of error.
     * @warning The fallback conversion retains byte sequences that do not represent valid Unicode characters unaltered.
     */
    template <>
    inline std::wstring from_string<std::wstring>(const std::string& str)
    {
        try
        {
            CBEAM_SUPPRESS_WARNINGS_PUSH() // note https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2872r2.pdf
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            CBEAM_SUPPRESS_WARNINGS_POP()
            return converter.from_bytes(str);
        }
        catch (const std::range_error&)
        {
            // Fallback to element-wise conversion in case of invalid UTF-8 input.
            // CBEAM_LOG_DEBUG is not used because it relies on these conversion methods.
            std::wstring converted;
            for (char c : str)
            {
                converted.push_back(static_cast<unsigned char>(c));
            }
            return converted;
        }
    }

    /**
     * @brief The `has_insertion_operator` trait provides static meta-information about whether a type T has overloaded the `operator<<` for insertion into an output stream.
     *
     * This trait is useful for template metaprogramming and generic programming, allowing you to write code that works differently depending on whether a type can be streamed or not.
     *
     * ## Template Parameters:
     *
     * - `T`: The type to be checked for the existence of an overload for `operator<<`.
     * - `U`: (optional) An unused template parameter used to enable SFINAE (Substitution Failure Is Not An Error).
     *
     * ## Member Types:
     *
     * - `value`: A static constant `bool` member that is `true` if and only if `T` has an overload for `operator<<`, and `false` otherwise.
     *
     * ## Specializations:
     *
     * - The primary template is `std::false_type`, indicating that by default, types do not have an overload for `operator<<`.
     * - A template specialization exists for types `T` for which a valid overload for `operator<<` is found. This specialization uses `std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>` to ensure that the overload is actually callable.
     *
     * ## Example Usage:
     *
     * ```cpp
     * template <typename T>
     * void print_value(const T& value) {
     *   if constexpr (has_insertion_operator<T>::value) {
     *     std::cout << value;
     *   } else {
     *     // Handle types without an insertion operator here...
     *   }
     * }
     * ```
     *
     * ## See also:
     *
     * - `std::ostream`
     * - `operator<<`
     * - `std::enable_if`
     * - `std::declval`
     * - `std::void_t`
     */
    template <typename T, typename = void>
    struct has_insertion_operator : std::false_type
    {
    };

    template <typename T>
    struct has_insertion_operator<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type
    {
    };

    /**
     * @brief Converts the value to a string, using the C-locale (i.e., '.' as a decimal separator).
     *
     * @tparam T The type of the value, which must be valid for insertion into a std::ostream.
     * @param value The value to be converted.
     * @return A string representation of the value.
     */
    template <typename T>
    inline typename std::enable_if<has_insertion_operator<T>::value, std::string>::type to_string(const T& value)
    {
        std::stringstream stream;
        stream.imbue(std::locale("C"));
        stream << value;
        return stream.str();
    }

    /**
     * @brief Converts a pointer to a string in hex syntax with a leading "0x".
     *
     * @tparam T The pointed-to type.
     * @param val A pointer to an object of type T.
     * @return A string in hexadecimal representation, prefixed with "0x".
     */
    template <typename T>
    inline std::string to_string(T* const& val)
    {
        std::stringstream stream;
        stream << std::hex << std::showbase << reinterpret_cast<std::uintptr_t>(val);
        return stream.str();
    }

    /**
     * @brief Converts the given std::wstring to std::string using UTF-16 to UTF-8 encoding.
     *
     * This function utilizes std::wstring_convert with std::codecvt_utf8_utf16 to perform
     * the conversion from wide string to string. It attempts the conversion and returns the
     * resultant string if successful. In case of a range error (due to invalid UTF-16 input),
     * it falls back to an element-wise conversion.
     *
     * @note The std::wstring_convert is deprecated in C++17. See the proposal to remove it at
     * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2872r2.pdf for more details.
     *
     * @param str The wide string to be converted.
     * @return The converted string in UTF-8 encoding, or the element-wise conversion result in case of error.
     * @warning The fallback conversion retains byte sequences that do not represent valid Unicode characters unaltered.
     */
    inline std::string to_string(const std::wstring& str)
    {
        try
        {
            CBEAM_SUPPRESS_WARNINGS_PUSH() // note https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2872r2.pdf
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            CBEAM_SUPPRESS_WARNINGS_POP()
            return converter.to_bytes(str);
        }
        catch (const std::range_error&)
        {
            // Fallback to element-wise conversion in case of invalid UTF-16 input.
            // CBEAM_LOG_DEBUG is not used because it relies on these conversion methods.
            std::string converted;
            for (wchar_t c : str)
            {
                if ((c >> 8) != 0)
                {
                    converted.push_back(static_cast<char>(c >> 8));
                }
                if ((c & 0xff) != 0)
                {
                    converted.push_back(static_cast<char>(c & 0xff));
                }
            }
            return converted;
        }
    }

    /**
     * \brief Converts a std::chrono::time_point object to a formatted string.
     *
     * The format of the string is "YYYY-MM-DD HH:MM:SS.mmm", where "mmm" represents milliseconds.
     *
     * @tparam T The clock type used for the `std::chrono::time_point`.
     * @param time The `std::chrono::time_point` object to convert.
     * @return std::string The formatted string representing the time point.
     *
     * @example cbeam/convert/string.hpp
     * @code{.cpp}
     * // Usage example
     * auto now = std::chrono::system_clock::now();
     * std::string formatted_time = to_string(now);
     * // formatted_time might be, for example, "2023-10-28 12:34:56.789"
     * @endcode
     */
    template <typename T>
    inline std::string to_string(std::chrono::time_point<T> time)
    {
        char sRep[100];
        {
            time_t curr_time = T::to_time_t(time);
//            struct tm buffer;
//            localtime_s(&buffer, &curr_time);
#pragma warning(suppress : 4996) // The implementation of localtime_s in Microsoft CRT is incompatible with the C standard since it has reversed parameter order and returns errno_t.
            struct tm* buffer = localtime(&curr_time);
            strftime(sRep, sizeof(sRep), "%Y-%m-%d %H:%M:%S", buffer);
        }

        typename T::duration t  = time.time_since_epoch();
        auto                 ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - std::chrono::duration_cast<std::chrono::seconds>(t));

        std::stringstream result;
        result << sRep << "." << std::setfill('0') << std::setw(3) << ms.count(); // append milliseconds 000..999
        return result.str();
    }

    /**
     * @brief Converts any value that can be handled by `to_string(...)` into a std::wstring.
     *
     * Internally calls `convert::to_string(value)` and then `from_string<std::wstring>(...)`.
     *
     * @tparam T The type of the value to convert.
     * @param value The value to convert to std::wstring.
     * @return A std::wstring representation of the input value.
     */
    template <typename T>
    std::wstring to_wstring(T value)
    {
        return from_string<std::wstring>(to_string(value));
    }
}
