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
#include <cbeam/logging/log_manager.hpp> // for CBEAM_LOG

#include <stdlib.h> // for realloc, malloc, free

#include <cstdint> // for uint8_t
#include <cstring> // for std::memcpy, std::size_t
#include <new>     // for std::bad_alloc
#include <string>  // for std::operator+, std::allocator, std::to_string, std::char_traits
#include <utility> // for std::swap

#ifdef DBG
    #include <string>
#endif

namespace cbeam::container
{
    /// \brief Manages memory a byte buffer, offering dynamic appending. This class is
    /// designed for scenarios where performance and control are crucial, such as in systems with dynamic memory
    /// requirements or real-time constraints. It uses malloc and free for efficient resizing of memory blocks
    /// without initialization overhead, as it internally uses the plain data type uint8_t.
    class buffer
    {
    public:
        buffer() = default; ///< Will not create any memory block. Use `append` to create one or append bytes to an existing one.

        /// \brief Create a managed memory block with optional element size.
        /// \param size Number of elements to allocate. Defaults to allocating single bytes if size_of_type is not specified.
        /// \param size_of_type (Optional) Byte size of each element to allocate. Defaults to 1 if not specified, allowing for byte-sized handling similar to std::memset.
        buffer(const std::size_t size, const std::size_t size_of_type = 1)
            : _size{size * size_of_type}
            , _buffer{(uint8_t*)malloc(_size)}
        {
            if (!_buffer)
            {
                CBEAM_LOG("cbeam::container::buffer: Out of RAM (" + std::to_string(_size) + ")");
                throw std::bad_alloc();
            }
        }

        /// \brief Deallocate the memory block.
        virtual ~buffer() noexcept
        {
            buffer::reset();
        }

        /// \brief Create an instance from a given memory block from address.
        explicit buffer(const void* address, const std::size_t length_of_buffer)
        {
            uint8_t* new_buffer = (uint8_t*)realloc(_buffer, length_of_buffer);
            if (!new_buffer)
            {
                CBEAM_LOG("cbeam::container::buffer::append: Out of RAM (" + std::to_string(_size) + "+" + std::to_string(length_of_buffer) + ")");
                throw std::bad_alloc();
            }

            _buffer = new_buffer;
            _size   = length_of_buffer;
            std::memcpy(_buffer + _size, address, length_of_buffer);
        }

        /// \brief Copy construction means that the other buffer is deep copied to construct this instance.
        buffer(const buffer& other)
        {
            *this = other;
        }

        /// \brief append the given buffer to the end of the current buffer. If there is no current buffer yet, it will be allocated. Memory will be copied if required, otherwise expanded.
        virtual void append(const void* buffer_to_append, const std::size_t length_of_buffer)
        {
            uint8_t* new_buffer = (uint8_t*)realloc(_buffer, _size + length_of_buffer);
            if (!new_buffer)
            {
                CBEAM_LOG("cbeam::container::buffer::append: Out of RAM (" + std::to_string(_size) + "+" + std::to_string(length_of_buffer) + ")");
                throw std::bad_alloc();
            }

            _buffer = new_buffer;
            std::memcpy(_buffer + _size, buffer_to_append, length_of_buffer);

            _size += length_of_buffer;
        }

        /// \brief returns the size of the buffer in bytes
        virtual std::size_t size() const noexcept
        {
            return _size;
        }

        /// \brief make a deep copy of the other buffer, overwriting the content of this buffer
        virtual buffer& operator=(const buffer& other)
        {
            if (this != &other)
            {
                if (other._buffer == nullptr)
                {
                    throw cbeam::error::runtime_error("cbeam::container::buffer copy assignment operator has been passed a default constructed (therefore invalid) instance");
                }

                uint8_t* new_buffer = (uint8_t*)realloc(_buffer, other._size);
                if (!new_buffer)
                {
                    CBEAM_LOG("cbeam::container::buffer::append: Out of RAM (" + std::to_string(other._size) + ")");
                    throw std::bad_alloc();
                }
                _buffer = new_buffer;
                std::memcpy(_buffer, other._buffer, other._size);
                _size = other._size;
            }
            return *this;
        }

        /// \brief return a pointer to the managed memory block
        virtual void* get() const noexcept
        {
            return _buffer;
        }

        /// \brief Resets the shared_buffer instance, deallocating the managed memory block
        virtual void reset() noexcept
        {
            free(_buffer);
            _buffer = nullptr;
            _size   = 0;
        }

        /// \brief Swaps the contents of this shared_buffer with another shared_buffer
        /// \param other Another shared_buffer to swap with
        virtual void swap(buffer& other) noexcept
        {
            std::swap(_buffer, other._buffer);
            std::swap(_size, other._size);
        }

    protected:
        std::size_t _size{0};
        uint8_t*    _buffer{nullptr};
    };
}
