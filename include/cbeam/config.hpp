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

#ifdef USING_CMAKE
    #include "config_generated.hpp"
#else
    // Without CMake, we guess if certain headers are available on the system
    #if defined(__GLIBC__) || defined(__APPLE__)
        #define HAVE_DLFCN_H    1
        #define HAVE_FCNTL_H    1
        #define HAVE_PTHREAD_H  1
        #define HAVE_PWD_H      1
        #define HAVE_SYS_MMAN_H 1
        #define HAVE_UNISTD_H   1
    #endif

    #if defined(__APPLE__)
        #define HAVE_POSIX_SHM_H 1
    #endif
#endif
