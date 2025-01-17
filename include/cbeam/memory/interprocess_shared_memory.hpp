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

#include <cbeam/concurrency/named_recursive_mutex.hpp>
#include <cbeam/error/runtime_error.hpp> // for cbeam::error:runtime_error

#ifdef _WIN32
    #include <cbeam/platform/windows_config.hpp>

    #include <cbeam/logging/log_manager.hpp>
#else
    #include <fcntl.h>    // for O_CREAT, O_RDWR, S_IRUSR, S_IWUSR
    #include <sys/mman.h> // for size_t, mmap, munmap, shm_open, shm_unlink, MAP_FAILED, MAP_SHARED, PROT_READ, PROT_WRITE
    #include <unistd.h>   // for close, ftruncate
#endif

#include <cstddef> // for std::size_t

#include <exception> // for std::exception
#include <memory>    // for std::allocator, std::unique_ptr, std::make_unique
#include <mutex>     // for std::lock_guard
#include <string>    // for std::operator+, std::string, std::basic_string, std::char_traits, std::string_literals

namespace cbeam::memory
{
    /**
     * @class interprocess_shared_memory
     * @brief Provides a unified, platform-independent interface for managing shared memory segments.
     *
     * This class works similar to boost::interprocess::shared_memory_object with the difference that the shared memory is destroyed when
     * the last processes that uses it exits. The design aligns with common use cases where persistent shared memory in kernel space is
     * not a requirement. Furthermore, under Windows it uses native API, like boost::interprocess::windows_shared_memory does
     * (avoiding file system permission problems under C:\ProgramData\boost_interprocess). Under Windows, the shared memory is
     * only accessible from processes in the same user session.
     */
    class interprocess_shared_memory
    {
    public:
        class lock_guard
        {
            cbeam::concurrency::named_recursive_mutex& _mutex;

        public:
            explicit lock_guard(cbeam::concurrency::named_recursive_mutex& mutex)
                : _mutex(mutex)
            {
                _mutex.lock();
            }

            ~lock_guard() noexcept
            {
                _mutex.unlock();
            }

            lock_guard(const lock_guard&)            = default;
            lock_guard& operator=(const lock_guard&) = delete;
        };

        /**
         * @brief Constructor that initializes the shared memory segment.
         * @param unique_identifier A unique identifier for the shared memory segment.
         * @param size The initial size of the shared memory segment.
         */
        interprocess_shared_memory(const std::string& unique_identifier, std::size_t size)
#ifdef _WIN32
            : _shared_memory_name{"Local\\s_" + unique_identifier}
#else
            : _shared_memory_name{"/s_" + unique_identifier}
#endif
            , _mutex{(std::string{"m_"} + unique_identifier).c_str()}
        {
            using namespace std::string_literals;

            std::lock_guard<cbeam::concurrency::named_recursive_mutex> lock(_mutex);

#ifdef _WIN32
            _shared_memory = CreateFileMappingA(
                INVALID_HANDLE_VALUE,                          // use paging file
                NULL,                                          // default security
                PAGE_READWRITE,                                // read/write access
                static_cast<DWORD>(0xFFFFFFFF & (size >> 32)), // maximum object size (high-order DWORD)
                static_cast<DWORD>(0xFFFFFFFF & size),         // maximum object size (low-order DWORD)
                _shared_memory_name.c_str());                  // name of mapping object

            if (_shared_memory == NULL)
            {
                if (GetLastError() == ERROR_ALREADY_EXISTS)
                {
                    _shared_memory = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, _shared_memory_name.c_str());
                    if (_shared_memory == NULL)
                    {
                        throw cbeam::error::runtime_error("cbeam::memory::interprocess_shared_memory: Failed to open existing shared memory for '" + unique_identifier + "': " + platform::get_last_windows_error_message());
                    }
                }
                else
                {
                    throw cbeam::error::runtime_error("cbeam::memory::interprocess_shared_memory: Failed to create file mapping for '" + unique_identifier + "': " + platform::get_last_windows_error_message());
                }
            }

            _region = MapViewOfFile(
                _shared_memory,      // handle to map object
                FILE_MAP_ALL_ACCESS, // read/write permission
                0,
                0,
                size);

            if (_region == NULL)
            {
                CloseHandle(_shared_memory);
                throw cbeam::error::runtime_error("cbeam::memory::interprocess_shared_memory: Failed to map view for '" + unique_identifier + "': " + platform::get_last_windows_error_message());
            }

            CBEAM_LOG_DEBUG("cbeam::memory::interprocess_shared_memory: Created or opened shared memory " + unique_identifier);
#else
            int shared_memory = shm_open(_shared_memory_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if (shared_memory == -1)
            {
                throw cbeam::error::runtime_error("cbeam::memory::interprocess_shared_memory: Failed to create/open shared memory: " + unique_identifier);
            }

            if (ftruncate(shared_memory, size) == -1)
            {
                close(shared_memory);
                throw cbeam::error::runtime_error("cbeam::memory::interprocess_shared_memory: Failed to set size of shared memory: " + unique_identifier);
            }

            void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory, 0);
            if (addr == MAP_FAILED)
            {
                close(shared_memory);
                throw cbeam::error::runtime_error("cbeam::memory::interprocess_shared_memory: Failed to map shared memory: " + unique_identifier);
            }

            close(shared_memory);
            _region = std::make_unique<posix_mapped_region>(addr, size);
#endif
        }

        virtual ~interprocess_shared_memory() noexcept
        {
            try
            {
                std::lock_guard<cbeam::concurrency::named_recursive_mutex> lock(_mutex);

#ifdef _WIN32
                UnmapViewOfFile(_region);
                CloseHandle(_shared_memory);
#else
                _region.reset();
                shm_unlink(_shared_memory_name.c_str());
#endif
            }
            catch (const std::system_error& ex)
            {
                CBEAM_LOG(std::string{"cbeam::memory::interprocess_shared_memory::~interprocess_shared_memory: Could not lock the mutex. This indicates a serious (unexpected) bug that must be fixed during development phase: "} + ex.what());
                assert(false);
            }
        }

        /**W
         * @brief Retrieves the starting address of the shared memory region.
         * @return A pointer to the beginning of the shared memory region.
         */
        void* data() const
        {
#ifdef _WIN32
            return _region;
#else
            return _region->get_address();
#endif
        }

        /**
         * @brief Returns the size of the shared memory region.
         * @return The size of the shared memory region in bytes.
         */
        size_t capacity() const noexcept
        {
#ifdef _WIN32
            MEMORY_BASIC_INFORMATION info;
            VirtualQuery(_region, &info, sizeof(info));
            return info.RegionSize;
#else
            return _region->get_size();
#endif
        }

        /**
         * @brief Acquires a lock_guard for mutex synchronization.
         * @return A lock_guard object that locks the mutex until destruction (while it is in scope).
         */
        lock_guard get_lock_guard() const
        {
            return lock_guard(_mutex);
        }

        interprocess_shared_memory(const interprocess_shared_memory&)            = delete;
        interprocess_shared_memory& operator=(const interprocess_shared_memory&) = delete;
        interprocess_shared_memory(interprocess_shared_memory&&)                 = delete;
        interprocess_shared_memory& operator=(interprocess_shared_memory&&)      = delete;

    private:
#ifdef _WIN32
        HANDLE _shared_memory;
        LPVOID _region;
#else
        class posix_mapped_region
        {
        public:
            posix_mapped_region(void* addr, size_t size)
                : _addr(addr)
                , _size(size) {}

            ~posix_mapped_region() noexcept
            {
                munmap(_addr, _size);
            }

            void*  get_address() const noexcept { return _addr; }
            size_t get_size() const noexcept { return _size; }

        private:
            void*  _addr;
            size_t _size;
        };

        std::unique_ptr<posix_mapped_region> _region;
#endif
        const std::string                          _shared_memory_name;
        mutable concurrency::named_recursive_mutex _mutex;
    };
}
