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

#include <cbeam/container/thread_safe_container.hpp>
#include <cbeam/error/runtime_error.hpp>  // for cbeam::error:runtime_error
#include <cbeam/lifecycle/scoped_set.hpp> // for cbeam::lifecycle::scoped_set

#include <cassert> // for assert
#include <cstddef> // for std::size_t

#include <atomic>  // for std::atomic
#include <mutex>   // for std::recursive_mutex, std::lock_guard
#include <set>     // for std::set
#include <utility> // for std::pair

namespace cbeam::container
{
    /// \brief Thread-safe wrapper for std::set
    ///
    /// \tparam T Type of the elements
    template <typename T>
    class thread_safe_set : public thread_safe_container<std::set<T>>
    {
    public:
        /**
         * @brief Constructs an element in-place within the underlying `std::set`.
         *
         * This function uses perfect forwarding to create an element of type `T`
         * directly within the set. It throws a `runtime_error` if the set is currently
         * being modified.
         *
         * @tparam Args The parameter pack for constructing the `T` object.
         * @param args Arguments to forward to the constructor of `T`.
         * @return A `std::pair` containing an iterator to the inserted element and a boolean indicating success/failure.
         */
        template <typename... Args>
        std::pair<typename std::set<T>::iterator, bool> emplace(Args&&... args)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_set::emplace: not allowed during modification of the set.");
            }
            lifecycle::scoped_set modifying(this->_is_being_modified, true);
            return this->_container.emplace(std::forward<Args>(args)...);
        }

        /**
         * @brief Inserts a copy of `value` into the underlying `std::set`.
         *
         * If the set is currently being modified, a `runtime_error` is thrown.
         *
         * @param value The value to insert into the set.
         * @return `true` if the insertion was successful, `false` if the value already exists.
         */
        bool insert(const T& value)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_set::insert: not allowed during modification of the set.");
            }
            lifecycle::scoped_set(this->_is_being_modified, true);
            auto result = this->_container.insert(value);
            return result.second;
        }

        /**
         * @brief Erases a value from the set if it exists.
         *
         * If the set is currently being modified, a `runtime_error` is thrown.
         *
         * @param value The value to erase.
         * @return `true` if the value was present and erased, `false` otherwise.
         */
        bool erase(const T& value)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_set::erase: not allowed during modification of the set.");
            }
            lifecycle::scoped_set(this->_is_being_modified, true);
            return this->_container.erase(value) > 0;
        }

        /**
         * @brief Checks whether the specified value is contained in the set.
         *
         * This function acquires a lock to ensure thread safety while searching.
         *
         * @param value The value to look for.
         * @return `true` if the set contains the value, `false` otherwise.
         */
        bool contains(const T& value) const
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            return this->_container.find(value) != this->_container.end();
        }
    };
}
