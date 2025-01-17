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

#include <cbeam/filesystem/path.hpp> // for cbeam::filesystem::path

#include <gtest/gtest.h>

#include <filesystem> // for std::filesystem::path
#include <utility>    // for std::pair
#include <vector>     // for std::vector, std::allocator

TEST(PathTest, NormalizePath)
{
    std::vector<std::pair<std::filesystem::path, std::filesystem::path>> testPaths = {
#ifdef _WIN32
        {"\\foo\\bar\\..", "\\foo\\"},
        {"\\foo\\bar\\bar\\..\\..\\", "\\foo\\"},
        {"\\foo\\bar\\bar\\..\\..", "\\foo\\"},
#else
        {"/foo/bar/..", "/foo/"},
        {"/foo/bar/bar/../../", "/foo/"},
        {"/foo/bar/bar/../..", "/foo/"},
#endif
    };

    for (const auto& [input, expected] : testPaths)
    {
        cbeam::filesystem::path acrionPath(input);
        ASSERT_EQ((std::filesystem::path)acrionPath, expected);
    }
}
