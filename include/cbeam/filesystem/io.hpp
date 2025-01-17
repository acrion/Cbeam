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
#include <cbeam/random/generators.hpp> // for cbeam::random::random_string

#include <filesystem> // for std::filesystem::path, std::filesystem::exists, std::filesystem::operator/, std::filesystem::temp_directory_path, std::filesystem::create_directories
#include <fstream>    // for std::ios, std::operator|, std::basic_ostream, std::ofstream, std::basic_istream::read, std::basic_istream::seekg, std::basic_istream::tellg, std::ifstream
#include <string>     // for std::operator+, std::allocator, std::char_traits, std::string, std::operator<<

namespace cbeam::filesystem
{
    /// \brief Reads the given file as std::string. Throws cbeam::error::runtime_error in case of errors
    inline std::string read_file(std::filesystem::path file_path)
    {
        std::ifstream ifs(file_path.string(), std::ios::binary | std::ios::ate);
        if (ifs.fail())
        {
            throw cbeam::error::runtime_error("Could not open file '" + file_path.string() + "' for reading.");
        }
        auto fileSize = ifs.tellg();
        if (fileSize == -1)
        {
            throw cbeam::error::runtime_error("Could not determine the size of file '" + file_path.string() + "'.");
        }
        ifs.seekg(0, std::ios::beg);
        if (ifs.fail())
        {
            throw cbeam::error::runtime_error("Could not seek to the beginning of file '" + file_path.string() + "'.");
        }
        std::string content(fileSize, '\0');
        ifs.read(&content[0], fileSize);
        if (ifs.fail() && !ifs.eof())
        {
            throw cbeam::error::runtime_error("Could not read from file '" + file_path.string() + "'.");
        }
        return content;
    }

    /// \brief Creates or overwrites the given file with the given content. Throws cbeam::error::runtime_error in case of errors
    inline void write_file(const std::filesystem::path& file_path, const std::string& content)
    {
        std::ofstream ofs(file_path.string());
        if (ofs.fail())
        {
            throw cbeam::error::runtime_error("Could not open file '" + file_path.string() + "' for writing.");
        }
        ofs << content;
        if (ofs.fail())
        {
            throw cbeam::error::runtime_error("Could not write to file '" + file_path.string() + "'.");
        }
        ofs.close();
    }

    /// \brief create a file under the given path, if it does not exist yet; otherwise, the file content is left unchanges, but its modification time is updated, analogous to bash `touch`
    inline void touch(const std::filesystem::path& p)
    {
        std::ofstream file(p, std::ios::app);
    }

    /**
     * get path to non-existing unique temp file
     * @param extension optional file extension
     * @return the full path to a non-existing file
     */
    inline std::filesystem::path unique_temp_file(const std::string& extension = {})
    {
        std::filesystem::path unique_path;

        do
        {
            unique_path = (std::filesystem::temp_directory_path() / random::random_string(16)).replace_extension(extension);
        } while (std::filesystem::exists(unique_path));

        return unique_path;
    }

    /// \brief get unique and non-existing temp directory
    inline std::filesystem::path unique_temp_dir()
    {
        std::filesystem::path unique_path;

        do
        {
            unique_path = std::filesystem::temp_directory_path() / random::random_string(16);
        } while (std::filesystem::exists(unique_path));

        return unique_path;
    }

    /**
     * create unique temp file, analogous to bash `mktemp`
     * @param extension optional file extension
     * @return the full path to a new empty file
     */
    inline std::filesystem::path create_unique_temp_file(const std::string& extension = {})
    {
        std::filesystem::path unique_path = unique_temp_file(extension);
        touch(unique_path);
        return unique_path;
    }

    /// \brief create unique temp directory, analogous to bash `mktemp -d`
    inline std::filesystem::path create_unique_temp_dir()
    {
        std::filesystem::path unique_path = unique_temp_dir();
        std::filesystem::create_directories(unique_path);
        return unique_path;
    }
}
