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

#include <cbeam/error/system_error.hpp>

#ifdef _WIN32
    #include <cbeam/platform/windows_config.hpp>
#else
    #include <cbeam/config.hpp>

    #if HAVE_PTHREAD_H
        #include <pthread.h> // for pthread_mutex_t, pthread_mutex_destroy, pthread_mutex_init, pthread_mutex_lock, pthread_mutex_unlock, pthread_mutexattr_destroy, pthread_mutexattr_init, pthread_mutexattr_setpshared, pthread_mutexattr_settype, PTHREA...
    #endif

    #if HAVE_SYS_MMAN_H
        #include <sys/mman.h> // for munmap, mmap, shm_open, MAP_FAILED, MAP_SHARED, PROT_READ, PROT_WRITE
    #endif

    #if HAVE_UNISTD_H
        #include <unistd.h> // for close, ftruncate, NULL
    #endif

    #if HAVE_FCNTL_H
        #include <fcntl.h> // for O_CREAT, O_RDWR, S_IRUSR, S_IWUSR
    #endif

    #if HAVE_POSIX_SHM_H
        #include <sys/posix_shm.h> // for PSHMNAMLEN
    #endif
#endif

#include <string>    // for std::operator+, std::string

namespace cbeam::concurrency
{
    inline std::size_t get_max_shm_name_length()
    {
#if _WIN32
        return MAX_PATH;
#elif defined(__APPLE__)
        return PSHMNAMLEN; // under __APPLE__ the maximum shm name length is not NAME_MAX, but PSHMNAMLEN, which is shorter
#elif defined(NAME_MAX)
    return NAME_MAX;
#else
    #error Unsupported platform
#endif
    }

    /**
     * @class named_recursive_mutex
     * @brief Provides a cross-platform interface for recursive named mutexes, enabling interprocess synchronization.
     *
     * On Linux and Mac OS X, this class mimics the functionality of boost::interprocess::named_recursive_mutex.
     * On Windows, it utilizes the native Windows API for named mutexes, which differs from Linux and Mac OS X in:
     *  - Non-persistent mutexes: they get destroyed when the last referencing process terminates.
     *  - Limited to the same user session.
     * This implementation opts for the native Windows approach over the Boost library to leverage platform-specific optimizations
     * and circumvent file system permission issues commonly encountered with C:\ProgramData\boost_interprocess.
     */
    class named_recursive_mutex
    {
    public:
        /**
         * @brief Constructs a named_recursive_mutex with a specified name.
         *
         * Initializes the mutex based on the operating system. On Windows, it creates a native mutex using CreateMutexA.
         * On Unix-based systems, it sets up a shared memory and initializes a pthread mutex.
         *
         * @exception cbeam::error::system_error Thrown if mutex creation or initialization fails.
         *
         * @param name The unique name of the mutex for interprocess identification.
         */
        named_recursive_mutex(const std::string& name)
        {
            if (name.length() > get_max_shm_name_length())
            {
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex: '" + name + "' exceeds maximum lengths for shm names of " + std::to_string(get_max_shm_name_length()));
            }

#ifdef _WIN32
            _mutex = CreateMutexA(NULL, FALSE, name.c_str());
            if (_mutex == NULL)
            {
                throw cbeam::error::system_error("Failed to create mutex: " + name);
            }
#else
            int fd = shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
            bool is_new = true;

            if (fd == -1)
            {
                if (errno == EEXIST)
                {
                    fd = shm_open(name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
                    if (fd == -1)
                    {
                        throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex: Failed to open existing shared memory: " + name);
                    }
                    is_new = false;
                }
                else
                {
                    throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex: Failed to open shared memory: " + name);
                }
            }

            if (is_new) // On macOS, ftruncate fails if the shared memory object, previously created with shm_open using the same name, has already been truncated to the same size by a prior ftruncate call.
            {
                if (ftruncate(fd, sizeof(pthread_mutex_t)) == -1)
                {
                    close(fd);
                    throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex: Failed to truncate shared memory: " + name);
                }
            }

            void* addr = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (addr == MAP_FAILED)
            {
                close(fd);
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex: Failed to map shared memory: " + name);
            }

            close(fd);

            _mutex = reinterpret_cast<pthread_mutex_t*>(addr);

            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

            if (pthread_mutex_init(_mutex, &attr) != 0)
            {
                munmap(addr, sizeof(pthread_mutex_t));
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex: Failed to initialize mutex: " + name);
            }

            pthread_mutexattr_destroy(&attr);
#endif
        }

        /**
         * @brief Destructor for named_recursive_mutex.
         *
         * Cleans up resources allocated for the mutex. On Windows, it closes the handle to the mutex.
         * On Unix-based systems, it destroys the pthread mutex and unmaps the shared memory.
         */
        virtual ~named_recursive_mutex() noexcept
        {
#ifdef _WIN32
            if (_mutex != NULL)
            {
                CloseHandle(_mutex);
            }
#else
            pthread_mutex_destroy(_mutex);
            munmap(_mutex, sizeof(pthread_mutex_t));
#endif
        }

        /**
         * @brief Acquires the mutex lock.
         *
         * On Windows, it uses WaitForSingleObject to wait for the mutex.
         * On Unix-based systems, it locks the pthread mutex.
         *
         * @exception cbeam::error::system_error Thrown if locking the mutex fails.
         */
        void lock() const
        {
#ifdef _WIN32
            DWORD waitResult = WaitForSingleObject(_mutex, INFINITE);
            switch (waitResult)
            {
            case WAIT_OBJECT_0:
                break;
            case WAIT_ABANDONED:
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex::lock()");
            }
#else
            if (pthread_mutex_lock(_mutex) != 0)
            {
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex::lock()");
            }
#endif
        }

        /**
         * @brief Releases the mutex lock.
         *
         * On Windows, it releases the mutex using ReleaseMutex.
         * On Unix-based systems, it unlocks the pthread mutex.
         *
         * @exception cbeam::error::system_error Thrown if unlocking the mutex fails.
         */
        void unlock() const
        {
#ifdef _WIN32
            if (!ReleaseMutex(_mutex))
            {
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex::unlock()");
            }
#else
            if (pthread_mutex_unlock(_mutex) != 0)
            {
                throw cbeam::error::system_error("cbeam::concurrency::named_recursive_mutex::unlock()");
            }
#endif
        }

        // Copy and move constructors and assignment operators are deleted to prevent copying and moving of the mutex.
        named_recursive_mutex(const named_recursive_mutex&)            = delete;
        named_recursive_mutex& operator=(const named_recursive_mutex&) = delete;
        named_recursive_mutex(named_recursive_mutex&&)                 = delete;
        named_recursive_mutex& operator=(named_recursive_mutex&&)      = delete;

    private:
#ifdef _WIN32
        HANDLE _mutex; ///< Handle to the named mutex on Windows.
#elif __linux__ || __APPLE__
        pthread_mutex_t* _mutex; ///< Pointer to the pthread_mutex_t for Unix-based systems.
#endif
    };
}
