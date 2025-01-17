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

#include <cbeam/error/runtime_error.hpp> // for cbeam::error:runtime_error
#include <cbeam/random/generators.hpp>   // for cbeam::random::random_string

#include <cstddef> // for std::size_t

#include <algorithm>    // for std::equal
#include <exception>    // for std::exception
#include <filesystem>   // for std::filesystem::path, std::filesystem::exists, std::filesystem::operator!=, std::filesystem::is_directory, std::filesystem::operator/, std::filesystem::operator==, std::filesystem::directory_iterator
#include <list>         // for std::list
#include <regex>        // for std::regex_search, std::regex
#include <string>       // for std::allocator, std::operator+, std::operator!=, std::string_literals::operator""s, std::char_traits, std::string, std::operator==, std::basic_string, std::string_literals
#include <system_error> // for std::error_code

namespace cbeam::filesystem
{
    class path
    {
    public:
        /**
         * @brief Explicit constructor for cbeam::filesystem::path.
         *
         * Constructs a new path object by normalizing the provided std::filesystem::path.
         * The normalization ensures that the path is in a canonical and consistent format,
         * which is useful for reliable path comparisons and operations.
         *
         * @param path The std::filesystem::path object to be normalized and used as the base path.
         */
        explicit path(const std::filesystem::path& path)
            : _base_path(normalize(path)) {}

        /**
         * @brief Cast operator to std::filesystem::path
         *
         * This operator allows the cbeam::filesystem::path object to be implicitly
         * converted to std::filesystem::path. It returns the underlying _base_path member,
         * facilitating seamless integration and use with standard filesystem functions.
         *
         * @return A std::filesystem::path object representing the _base_path member.
         */
        operator std::filesystem::path() // NOLINT(google-explicit-constructor)
        {
            return _base_path;
        }

        /**
         * @brief Retrieves a list of subdirectories matching a given regular expression filter.
         *
         * @details This function scans the directory specified by the _base_path member and collects
         * all subdirectories that match the provided regular expression filter. By default,
         * it matches all subdirectories. Under Windows, if a subdirectory is represented by a
         * shortcut (*.LNK file), it will be resolved and included in its resolved form in the
         * resulting list. This ensures that the list reflects the actual directories rather than
         * the shortcut files.
         *
         * @param filter A std::regex object representing the filter to apply. The default value ".*"
         * matches all subdirectories. The filter is applied to the resolved path names.
         * @return A std::list of std::filesystem::path objects representing the filtered subdirectories.
         */
        std::list<std::filesystem::path> get_subdirs(const std::regex& filter = std::regex(".*"))
        {
            std::list<std::filesystem::path> directories_list;

            if (std::filesystem::exists(_base_path) && std::filesystem::is_directory(_base_path))
            {
                for (auto it = std::filesystem::directory_iterator(_base_path); it != std::filesystem::directory_iterator(); ++it)
                {
                    bool        is_directory = std::filesystem::is_directory(*it);
                    std::string path_str     = it->path().string();

                    if (is_directory && std::regex_search(path_str, filter))
                    {
                        directories_list.emplace_back(path_str);
                    }
                }
            }

            return directories_list;
        }

        /**
         * @brief Creates the directory specified by _base_path.
         *
         * @details This function attempts to create a new directory based on the _base_path member.
         * If 'delete_prior_creating' is set to true, any existing directory at this path will
         * be deleted before creating the new one. Exception handling is implemented to catch
         * and throw errors during the creation process.
         *
         * @param delete_prior_creating If true, deletes the existing directory at _base_path before creating a new one. Default is false.
         * @throws cbeam::error::runtime_error Throws an exception if the directory cannot be created. This corresponds to the
         * return code of std::filesystem::create_directories with one exception: Under Windows, it is regarded as an error if
         * the path ends with a '\' separator, even if the directory has been created successfully. In contrast, this method only
         * regards it is an error if the directory has not been created (being consist with Linux and macOS, where directory paths
         * with trailing '/' are common).
         */
        void create_directory(bool delete_prior_creating = false) const
        {
            using namespace std::string_literals;

            bool        success = false;
            std::string msg     = "unknown error"s;
            try
            {
                if (delete_prior_creating)
                {
                    std::filesystem::remove_all(_base_path);
                }

                if (!std::filesystem::exists(_base_path))
                {
                    // According to https://en.cppreference.com/w/cpp/filesystem/create_directory this should return true
                    // if the directory was created. But in Windows SDK 10.0.19041.0, it returns false if the path ends with
                    // a '\' and its last component has been created successfully.
                    std::filesystem::create_directories(_base_path);
                }
                success = std::filesystem::exists(_base_path);
            }
            catch (std::exception& ex)
            {
                msg = ex.what();
            }

            if (!success)
            {
                std::string err_msg("cbeam::filesystem::path::create(\"" + _base_path.string() + "\"): " + msg);
                throw cbeam::error::runtime_error(err_msg);
            }
        }

        /**
         * @brief Copies the directory specified by _base_path to a target location.
         *
         * This function copies the entire directory pointed to by _base_path to the specified
         * target path. If the target directory already exists, it will be removed before
         * copying. The copy operation is recursive, including all subdirectories and files.
         *
         * @param target The std::filesystem::path object representing the target directory to copy to.
         * @throws cbeam::error::runtime_error Throws an exception if the source directory does not exist, is not a directory, or an existing target directory cannot be removed.
         */
        void copy_to(const std::filesystem::path& target) const
        {
            if (!std::filesystem::exists(_base_path) || !std::filesystem::is_directory(_base_path))
            {
                throw cbeam::error::runtime_error("cbeam::filesystem::path::copy_to: source directory " + _base_path.string() + " does not exist or is not a directory");
            }
            if (std::filesystem::exists(target))
            {
                path(target).remove();
            }

            std::filesystem::copy(_base_path, target, std::filesystem::copy_options::recursive);
        }

        /**
         * @brief Removes the directory specified by _base_path.
         *
         * @details This function attempts to securely remove the directory pointed to by _base_path.
         * It first renames the directory to avoid partial deletion issues, especially on Windows.
         * The renaming involves appending a random string to ensure uniqueness. If the rename
         * operation fails, an exception is thrown. After renaming, the directory is deleted.
         * If the removal fails, the directory is renamed back to its original name and an
         * exception is thrown. This approach aims to mitigate risks associated with partial
         * deletions and improve the reliability of the removal process.
         *
         * @throws cbeam::error::runtime_error Throws an exception if renaming or removal fails.
         */
        void remove() const
        {
            std::error_code             error;
            const std::filesystem::path temp_path = std::filesystem::path(remove_trailing_directory_separators(_base_path).string() + cbeam::random::random_string(16));

            // We first try to rename, because under Windows, remove_all can result in a modified directory with partly deleted files in case an error occurs.
            // We deliberately do not move to a unique subdir of std::filesystem::temp_directory_path(), because this might require to actually copy the complete
            // directory tree. In contrast, we rename only the last path component, which is an atomic operation. The usage of this method implies that we
            // should have write access to rename the directory to a path next to it (if not, removing the directory would also fail).
            // Note that under Windows, a DLL inside the directory that is in use does not prevent the directory from being renamed by this method.
            std::filesystem::rename(_base_path, temp_path, error);
            if (error)
            {
                throw cbeam::error::runtime_error(error.message());
            }
            std::filesystem::remove_all(temp_path, error);
            if (error)
            {
                std::filesystem::rename(temp_path, _base_path, error); // rename back to original
                throw cbeam::error::runtime_error(error.message());
            }
        }

        /**
         * @brief Equality comparison operator for cbeam::filesystem::path.
         *
         * Compares this normalized path object with another for equality. Two path objects
         * are considered equal if their normalized _base_path members are the same. This
         * ensures that paths in different formats representing the same directory or file
         * are correctly identified as equal.
         *
         * @param other Another cbeam::filesystem::path object to compare with.
         * @return true if both path objects represent the same normalized path, false otherwise.
         */
        bool operator==(const path& other) const
        {
            return _base_path == other._base_path;
        }

        /**
         * @brief Inequality comparison operator for cbeam::filesystem::path.
         *
         * Compares this normalized path object with another for inequality. Two path objects
         * are considered unequal if their normalized _base_path members are different. This
         * ensures that paths in different formats representing different directories or files
         * are correctly identified as unequal.
         *
         * @param other Another cbeam::filesystem::path object to compare with.
         * @return true if the path objects represent different normalized paths, false otherwise.
         */
        bool operator!=(const path& other) const
        {
            return _base_path != other._base_path;
        }

        /**
         * @brief Equality comparison operator for cbeam::filesystem::path.
         *
         * Compares this normalized path object with a std::filesystem::path object for equality.
         * The comparison is based on the normalized _base_path member of this object and
         * the std::filesystem::path object.
         *
         * @param other A std::filesystem::path object to compare with this object.
         * @return true if this path object and the other std::filesystem::path object
         *         represent the same normalized path, false otherwise.
         */
        bool operator==(const std::filesystem::path& other) const
        {
            return _base_path == other;
        }

        /**
         * @brief Inequality comparison operator for cbeam::filesystem::path.
         *
         * Compares this normalized path object with a std::filesystem::path object for inequality.
         * The comparison is based on the normalized _base_path member of this object and
         * the std::filesystem::path object.
         *
         * @param other A std::filesystem::path object to compare with this object.
         * @return true if this path object and the other std::filesystem::path object
         *         represent different normalized paths, false otherwise.
         */
        bool operator!=(const std::filesystem::path& other) const
        {
            return _base_path != other;
        }

        /**
         * @brief Removes trailing directory separators from a given path.
         *
         * This static function cleans the input path by removing any trailing directory separators.
         * It accounts for different types of separators (forward slash, backslash, and the platform's
         * preferred separator) to ensure compatibility across various platforms. This function is useful
         * for normalizing paths before processing them further.
         *
         * @param p The std::filesystem::path object representing the path to be processed.
         * @return A new std::filesystem::path object with trailing directory separators removed.
         */
        static std::filesystem::path remove_trailing_directory_separators(const std::filesystem::path& p)
        {
            std::string s = p.string();

            // although '/' and '\' should be the only path separators, this is not defined by the C++ standard, so we check the preferred one in addition to care about potential exotic platforms
            while (!s.empty() && (s.back() == std::filesystem::path::preferred_separator || s.back() == '/' || s.back() == '\\'))
            {
                s.pop_back();
            }
            return std::filesystem::path(s);
        }

    private:
        /**
         * @brief Normalizes a given filesystem path.
         *
         * @details This static function processes the input path and returns its normalized
         * form. The normalization process involves:
         * - Resolving relative components (like "..").
         * - Ensuring that a directory path ends with the platform specific path separator character.
         * - On Windows, automatically resolving Windows specific shortcuts (not symlinks etc).
         *
         * The resulting path is a more canonical form that is suitable for further processing
         * and comparison.
         *
         * @param p The std::filesystem::path object representing the path to be normalized.
         * @return A normalized std::filesystem::path object.
         */
        static std::filesystem::path normalize(const std::filesystem::path& p)
        {
            using namespace std::string_literals;
            std::filesystem::path result;

            std::size_t numParent = 0;
            auto        it        = p.end();

            while (it != p.begin())
            {
                --it;
                if (it->string() != "/" && it->string() != "\\" && !it->empty())
                {
                    if (it->string() == "..")
                    {
                        ++numParent;
                    }
                    else if (numParent > 0)
                    {
                        --numParent;
                    }
                    else
                    {
                        std::string colon = ":"s;
                        if (std::equal(colon.rbegin(), colon.rend(), it->string().rbegin()))
                        {
#ifdef _WIN32
                            result = (it->wstring() + std::filesystem::path::preferred_separator) / result;
#else
                            result = (it->string() + std::filesystem::path::preferred_separator) / result;
#endif
                        }
                        else
                        {
                            result = *it / result;
                        }
                    }
                }
            }

            if (*p.begin() == "/"s)
            {
                result = *p.begin() / result;
            }

            return result;
        }

        std::filesystem::path _base_path;
    };
}
