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

#include <cbeam/error/out_of_range.hpp>

#include <cstddef> // for std::size_t

#include <array>     // for std::array
#include <utility>   // for std::forward

namespace cbeam::container
{
    /// \brief A template class representing a circular buffer.
    ///
    /// This class implements a circular buffer of fixed size. It provides
    /// basic iterator access and allows elements to be added to the back,
    /// automatically overwriting the oldest elements once the buffer is full.
    ///
    /// \tparam T The type of elements in the buffer.
    /// \tparam S The fixed size of the buffer.
    template <typename T, std::size_t S>
    class circular_buffer
    {
    public:
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using iterator        = typename std::array<T, S>::iterator;
        using const_iterator  = typename std::array<T, S>::const_iterator;

        /// \brief Default constructor.
        circular_buffer() = default;

        /// \brief Gets an iterator to the beginning of the buffer.
        /// \return An iterator to the first element.
        auto begin() noexcept
        {
            return _buffer.begin();
        }

        /// \brief Gets an iterator to the end of the buffer.
        /// \return An iterator to the element following the last element.
        auto end() noexcept
        {
            return _full ? _buffer.end() : _buffer.begin() + _i;
        }

        /// \brief Gets the number of elements in the buffer.
        /// \return The number of elements in the buffer.
        std::size_t size() const
        {
            return _full ? S : _i;
        }

        /// \brief Gets the maximum number of elements the buffer can hold.
        /// \return The maximum size of the buffer.
        std::size_t max_size() const
        {
            return S;
        }

        /// \brief Adds an element to the back of the buffer.
        ///
        /// This method adds a new element to the back of the buffer,
        /// potentially overwriting the oldest element if the buffer is full.
        ///
        /// \param t The element to add.
        void push_back(const T& t)
        {
            _buffer[_i] = t;
            _i          = (_i + 1) % S;
            if (_i == 0)
            {
                _full = true;
            }
        }

        bool empty() const noexcept { return !_full && _i == 0; }

        void clear() noexcept
        {
            _i    = 0;
            _full = false;
        }

        /// \brief Adds an element to the back of the buffer.
        ///
        /// This method adds a new element to the back of the buffer by moving it,
        /// potentially overwriting the oldest element if the buffer is full.
        ///
        /// \param args The element to add.
        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            _buffer[_i] = T(std::forward<Args>(args)...);
            _i          = (_i + 1) % S;
            if (_i == 0)
            {
                _full = true;
            }
        }

        /// \brief Accesses the element at specified location with bounds checking.
        /// \param pos The position of the element to return.
        /// \return A reference to the element at specified location.
        /// \throw cbeam::error::out_of_range if pos is out of range.
        reference at(size_type pos)
        {
            if (pos >= size())
            {
                throw cbeam::error::out_of_range("Position out of range");
            }

            return _buffer[(pos + (_full ? _i : 0)) % S];
        }

        /// \brief Accesses the element at specified location with bounds checking (const version).
        /// \param pos The position of the element to return.
        /// \return A const reference to the element at specified location.
        /// \throw cbeam::error::out_of_range if pos is out of range.
        const_reference at(size_type pos) const
        {
            if (pos >= size())
            {
                throw cbeam::error::out_of_range("Position out of range");
            }

            return _buffer[(pos + (_full ? _i : 0)) % S];
        }

        /// \brief Accesses the element at specified location without bounds checking.
        /// \param pos The position of the element to return.
        /// \return A reference to the element at specified location.
        reference operator[](size_type pos)
        {
            return _buffer[(pos + (_full ? _i : 0)) % S];
        }

        /// \brief Accesses the element at specified location without bounds checking (const version).
        /// \param pos The position of the element to return.
        /// \return A const reference to the element at specified location.
        const_reference operator[](size_type pos) const
        {
            return _buffer[(pos + (_full ? _i : 0)) % S];
        }

        /// \brief Accesses the first element in the buffer.
        /// \return A reference to the first element.
        reference front()
        {
            return _buffer[_full ? _i : 0];
        }

        /// \brief Accesses the first element in the buffer (const version).
        /// \return A const reference to the first element.
        const_reference front() const
        {
            return _buffer[_full ? _i : 0];
        }

        /// \brief Accesses the last element in the buffer.
        /// \return A reference to the last element.
        reference back()
        {
            return _buffer[(_i + S - 1) % S];
        }

        /// \brief Accesses the last element in the buffer (const version).
        /// \return A const reference to the last element.
        const_reference back() const
        {
            return _buffer[(_i + S - 1) % S];
        }

    private:
        std::array<T, S> _buffer;      ///< The internal buffer storage.
        std::size_t      _i{0};        ///< The index of the next element to be overwritten.
        bool             _full{false}; ///< Flag indicating whether the buffer is full.
    };
}
