# Copyright (c) 2025 acrion innovations GmbH
# Authors: Stefan Zipproth, s.zipproth@acrion.ch
#
# This file is part of Cbeam, see https://github.com/acrion/cbeam and https://cbeam.org
#
# Cbeam is offered under a commercial and under the AGPL license.
# For commercial licensing, contact us at https://acrion.ch/sales. For AGPL licensing, see below.
#
# AGPL licensing:
#
# Cbeam is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Cbeam is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Cbeam. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.25.2)
cmake_policy(VERSION 3.25.2)

include(FetchContent)
FetchContent_Declare(
    acrion_cmake
    GIT_REPOSITORY git@github.com:acrion/cmake.git
    GIT_TAG main
)
FetchContent_MakeAvailable(acrion_cmake)

include(${acrion_cmake_SOURCE_DIR}/set-cpp-version.cmake)

include(${acrion_cmake_SOURCE_DIR}/get-project-version-from-git-tag.cmake)
get_project_version_from_git_tag("origin/main")

project(cbeam VERSION ${version_major}.${version_minor}.${version_patch} DESCRIPTION "Cbeam library")

include(CheckIncludeFiles)
check_include_files("dlfcn.h" HAVE_DLFCN_H)
check_include_files("fcntl.h" HAVE_FCNTL_H)
check_include_files("pthread.h" HAVE_PTHREAD_H)
check_include_files("pwd.h" HAVE_PWD_H)
check_include_files("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_files("sys/posix_shm.h" HAVE_POSIX_SHM_H)
check_include_files("unistd.h" HAVE_UNISTD_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version_generated.hpp.cmake ${CMAKE_CURRENT_BINARY_DIR}/version_generated.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config_generated.hpp.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_generated.hpp)

source_group(concurrency REGULAR_EXPRESSION concurrency/)
source_group(container REGULAR_EXPRESSION container/)
source_group(convert REGULAR_EXPRESSION convert/)
source_group(encoding REGULAR_EXPRESSION encoding/)
source_group(error REGULAR_EXPRESSION error/)
source_group(filesystem REGULAR_EXPRESSION filesystem/)
source_group(json REGULAR_EXPRESSION json/)
source_group(lifecycle REGULAR_EXPRESSION lifecycle/)
source_group(logging REGULAR_EXPRESSION logging/)
source_group(memory REGULAR_EXPRESSION memory/)
source_group(platform REGULAR_EXPRESSION platform/)
source_group(random REGULAR_EXPRESSION random/)
source_group(serialization REGULAR_EXPRESSION serialization/)

SET(SRCS
    include/cbeam/concurrency/message_manager.hpp
    include/cbeam/concurrency/named_recursive_mutex.hpp
    include/cbeam/concurrency/process.hpp
    include/cbeam/concurrency/thread.hpp
    include/cbeam/concurrency/threaded_object.hpp
    include/cbeam/container/buffer.hpp
    include/cbeam/container/find.hpp
    include/cbeam/container/nested_map.hpp
    include/cbeam/container/shared_array.hpp
    include/cbeam/container/stable_interprocess_container.hpp
    include/cbeam/container/stable_interprocess_map.hpp
    include/cbeam/container/stable_reference_buffer.hpp
    include/cbeam/container/thread_safe_container.hpp
    include/cbeam/container/thread_safe_map.hpp
    include/cbeam/container/thread_safe_set.hpp
    include/cbeam/container/xpod.hpp
    include/cbeam/convert/buffer.hpp
    include/cbeam/convert/map.hpp
    include/cbeam/convert/nested_map.hpp
    include/cbeam/convert/string.hpp
    include/cbeam/convert/xpod.hpp
    include/cbeam/error/base_error.hpp
    include/cbeam/error/logic_error.hpp
    include/cbeam/error/out_of_range.hpp
    include/cbeam/error/overflow_error.hpp
    include/cbeam/error/runtime_error.hpp
    include/cbeam/error/system_error.hpp
    include/cbeam/encoding/utf8.hpp
    include/cbeam/filesystem/io.hpp
    include/cbeam/filesystem/path.hpp
    include/cbeam/filesystem/stdout_redirector.hpp
    include/cbeam/json/map.hpp
    include/cbeam/json/nested_map.hpp
    include/cbeam/json/string.hpp
    include/cbeam/json/traits.hpp
    include/cbeam/lifecycle/item_registry.hpp
    include/cbeam/lifecycle/scoped_set.hpp
    include/cbeam/lifecycle/singleton.hpp
    include/cbeam/logging/detail/logging_impl.hpp
    include/cbeam/logging/log_manager.hpp
    include/cbeam/logging/logger.hpp
    include/cbeam/memory/interprocess_shared_memory.hpp
    include/cbeam/memory/pointer.hpp
    include/cbeam/platform/clock_precision.hpp
    include/cbeam/platform/com_initializer.hpp
    include/cbeam/platform/compiler_compatibility.hpp
    include/cbeam/platform/info.hpp
    include/cbeam/platform/runtime.hpp
    include/cbeam/platform/system_folders.hpp
    include/cbeam/platform/windows_config.hpp
    include/cbeam/random/generators.hpp
    include/cbeam/serialization/direct.hpp
    include/cbeam/serialization/map.hpp
    include/cbeam/serialization/nested_map.hpp
    include/cbeam/serialization/string.hpp
    include/cbeam/serialization/traits.hpp
    include/cbeam/serialization/xpod.hpp
    include/cbeam/version.hpp
)

add_library(${PROJECT_NAME} INTERFACE ${SRCS})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "cbeam")

target_include_directories(${PROJECT_NAME}
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

if (MINGW)
    target_link_libraries(${PROJECT_NAME} INTERFACE runtimeobject) # required for com_initializer.hpp - under MSVC, the header itself cares about the linker dependency
else ()
    target_link_libraries(${PROJECT_NAME})
endif ()

include(${acrion_cmake_SOURCE_DIR}/do-logging.cmake)

project(cbeam_test VERSION ${version_major}.${version_minor}.${version_patch} DESCRIPTION "unit test executable for cbeam")

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
)

if (WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()

FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(
    ${PROJECT_NAME}
    test/test_buffer.cpp
    test/test_circular_buffer.cpp
    test/test_com_initializer.cpp
    test/test_convert.cpp
    test/test_convert_map.cpp
    test/test_convert_nested_map.cpp
    test/test_encoding.cpp
    test/test_item_registry.cpp
    test/test_json.cpp
    test/test_map_serialization.cpp
    test/test_message_manager.cpp
    test/test_path.cpp
    test/test_serialize.cpp
    test/test_singleton.cpp
    test/test_nested_map_serialization.cpp
    test/test_serialize_nested_map_xpod.cpp
    test/test_stable_reference_buffer.cpp
    test/test_stable_interprocess_map.cpp
    test/test_thread_safe_map.cpp
    test/test_threaded_object.cpp
)

include(${acrion_cmake_SOURCE_DIR}/set-compiler-flags.cmake)

target_link_libraries(
    ${PROJECT_NAME}
    cbeam
    Threads::Threads
    GTest::gtest_main
)
include(GoogleTest)
#gtest_discover_tests(${PROJECT_NAME})

include(${acrion_cmake_SOURCE_DIR}/run-tests.cmake)
