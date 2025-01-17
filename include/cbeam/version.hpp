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

#ifdef USING_CMAKE
    #include "version_generated.hpp"
#endif

#include <string>

namespace cbeam
{
    /**
     * @brief Returns the library version string, for example "1.2.3".
     *
     * - If `USING_CMAKE` is defined and the current git commit is tagged, this function uses the generated `CBEAM_VERSION` macro,
     *   which corresponds to the commit tag (with the leading 'v' removed).
     * - Otherwise - either because the current commit does not have a tag or `USING_CMAKE` is not defined - it returns "0.0.0".
     *
     * Note: Official Cbeam releases are always tagged.
     *
     * @return A string containing the version.
     */
    inline std::string get_version()
    {
#ifdef USING_CMAKE
        return CBEAM_VERSION;
#else
        return "0.0.0";
#endif
    }
}
