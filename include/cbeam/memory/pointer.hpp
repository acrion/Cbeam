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

#include <cbeam/container/stable_reference_buffer.hpp> // for cbeam::container::stable_reference_buffer
#include <cbeam/convert/string.hpp>                    // for cbeam::convert::from_string, cbeam::convert::to_string
#include <cbeam/error/runtime_error.hpp>               // for cbeam::error:runtime_error

#include <iosfwd> // for std::ostream
#include <memory> // for std::shared_ptr, std::allocator, std::make_shared, std::__shared_ptr_access
#include <string> // for std::operator+, std::char_traits, std::string, std::operator<<

namespace cbeam::memory
{
    /**
     * @brief The `pointer` class is one of the types supported by \ref container::xpod::type.
     *
     * It provides an optional reference-counted mechanism (via \ref stable_reference_buffer
     * or an internal `std::shared_ptr`) to manage the underlying memory. Otherwise, it simply
     * stores a raw pointer.
     */
    class pointer
    {
    public:
        pointer() = default;

        /**
         * @brief Construct from the given raw pointer. Checks if this ptr is managed by \ref stable_reference_buffer.
         *
         * @param ptr A raw pointer that may or may not be managed by stable_reference_buffer.
         * If it is managed, a reference to it is kept in `_mem`.
         */
        pointer(void* ptr)
            : _ptr(ptr)
        {
            if (container::stable_reference_buffer::is_known(ptr))
            {
                _mem = std::make_shared<container::stable_reference_buffer>(ptr);
            }
        }

        /**
         * @brief Construct from the given pointer in string representation (e.g. "0x...").
         *
         * Internally calls `convert::from_string<void*>(str_ptr)`.
         *
         * @param str_ptr The string representation of the pointer.
         */
        pointer(const std::string& str_ptr)
            : pointer(convert::from_string<void*>(str_ptr))
        {
        }

        /**
         * @brief Construct from an existing \ref stable_reference_buffer, incrementing its reference counter.
         *
         * @param mem A stable_reference_buffer object referencing some memory.
         */
        explicit pointer(const container::stable_reference_buffer& mem)
            : _ptr((void*)mem.get())
            , _mem(std::make_shared<container::stable_reference_buffer>(mem))
        {
        }

        /**
         * @brief Construct a new pointer object from a `std::shared_ptr<T>`.
         *
         * This constructor allows the creation of a pointer object from a `std::shared_ptr` with any template argument.
         * The lifetime of the referenced object is maintained by the shared pointer's reference count.
         *
         * @example cbeam/memory/pointer.hpp
         * @code{.cpp}
         * std::shared_ptr<MyType> my_shared_ptr = std::make_shared<MyType>();
         * cbeam::pointer ptr(my_shared_ptr);
         * @endcode
         *
         * @tparam T The type of the object that the std::shared_ptr is pointing to.
         * @param ptr A std::shared_ptr to an object of type T.
         */
        template <typename T>
        pointer(const std::shared_ptr<T>& ptr)
            : _ptr(static_cast<void*>(ptr.get()))
            , _holder(new std::shared_ptr<T>(ptr))
        {
        }

        /**
         * @brief Checks whether the pointer is managed by \ref stable_reference_buffer (or a `std::shared_ptr`) or not.
         *
         * @return true if the pointer is managed, false otherwise.
         */
        bool is_managed() const
        {
            return (bool)_mem;
        }

        /**
         * @brief Converts the pointer to a raw `void*`.
         *
         * @return A `void*` pointing to the underlying memory (or nullptr if empty).
         */
        operator void*() const
        {
            return _ptr;
        }

        /**
         * @brief Copy assignment operator. Increments the reference counter if the other pointer is managed.
         *
         * @param other The pointer object to copy from.
         * @return A reference to this object.
         */
        pointer& operator=(const pointer& other)
        {
            if (this == &other)
            {
                return *this;
            }

            _ptr    = other._ptr;
            _holder = other._holder;
            if (other._mem)
            {
                _mem = std::make_shared<container::stable_reference_buffer>(*other._mem);
            }
            else
            {
                _mem.reset();
            }

            return *this;
        }

        /**
         * @brief Converts the pointer to a `stable_reference_buffer` if it is managed, otherwise throws a runtime_error.
         *
         * @exception cbeam::error::runtime_error If the pointer is not managed by a stable_reference_buffer.
         * @return A copy of the underlying stable_reference_buffer.
         */
        explicit operator container::stable_reference_buffer() const
        {
            if (!_mem)
            {
                throw cbeam::error::runtime_error("cbeam::pointer(\"" + (std::string) * this + "\") is not an cbeam::container::stable_reference_buffer");
            }
            return *_mem;
        }

        /**
         * @brief Converts the pointer to a hex string with leading "0x".
         *
         * @return A string representation of the pointer value in hexadecimal format.
         */
        explicit operator std::string() const
        {
            return convert::to_string(_ptr);
        }

    private:
        void*                                               _ptr{nullptr};
        std::shared_ptr<container::stable_reference_buffer> _mem;
        std::shared_ptr<void>                               _holder; ///< In case this instance was constructed with a shared pointer, we increment its reference counter via _holder.
    };

    /**
     * @brief Outputs the `pointer` object as its hex string representation.
     *
     * @param os The output stream.
     * @param p The pointer object to output.
     * @return std::ostream& A reference to the output stream for chaining.
     */
    inline std::ostream& operator<<(std::ostream& os, const pointer& p)
    {
        os << static_cast<std::string>(p);
        return os;
    }

    /**
     * @brief Equality comparison operator for `pointer`.
     *
     * @param lhs Left-hand side pointer.
     * @param rhs Right-hand side pointer.
     * @return true if both pointers refer to the same memory address, false otherwise.
     */
    inline bool operator==(const pointer& lhs, const pointer& rhs)
    {
        return static_cast<void*>(lhs) == static_cast<void*>(rhs);
    }

    /**
     * @brief Inequality comparison operator for `pointer`.
     */
    inline bool operator!=(const pointer& lhs, const pointer& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief Less-than comparison operator for `pointer`.
     *
     * Compares the underlying memory addresses.
     */
    inline bool operator<(const pointer& lhs, const pointer& rhs)
    {
        return static_cast<void*>(lhs) < static_cast<void*>(rhs);
    }

    /**
     * @brief Less-or-equal comparison operator for `pointer`.
     */
    inline bool operator<=(const pointer& lhs, const pointer& rhs)
    {
        return (lhs == rhs) || (lhs < rhs);
    }

    /**
     * @brief Greater-than comparison operator for `pointer`.
     */
    inline bool operator>(const pointer& lhs, const pointer& rhs)
    {
        return !(lhs <= rhs);
    }

    /**
     * @brief Greater-or-equal comparison operator for `pointer`.
     */
    inline bool operator>=(const pointer& lhs, const pointer& rhs)
    {
        return !(lhs < rhs);
    }
}
