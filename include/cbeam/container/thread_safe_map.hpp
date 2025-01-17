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
#include <cbeam/error/out_of_range.hpp>   // for cbeam::error::out_of_range
#include <cbeam/error/runtime_error.hpp>  // for cbeam::error::runtime_error
#include <cbeam/lifecycle/scoped_set.hpp> // for cbeam::lifecycle::scoped_set

#include <cassert>   // for assert
#include <stdexcept> // for std::out_of_range
#include <cstddef>   // for std::size_t

#include <atomic>    // for std::atomic
#include <map>       // for std::map
#include <mutex>     // for std::recursive_mutex, std::lock_guard
#include <string>    // for std::string, std::allocator
#include <utility>   // for std::pair

namespace cbeam::container
{
    /// \brief Thread-safe wrapper for std::map
    ///
    /// \tparam Key Type of the keys
    /// \tparam Value Type of the values
    template <typename Key, typename Value>
    class thread_safe_map : public thread_safe_container<std::map<Key, Value>>
    {
    public:
        /// \brief Access specified element with bounds checking
        ///
        /// \param key Key of the element to find
        /// \param what_arg Optional argument for the exception message
        /// \return Reference to the mapped value of the element with key equivalent to key
        /// \throw cbeam::error::out_of_range If the container does not have an element with the specified key
        Value& at(const Key& key, const std::string& what_arg = "Key not found")
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::at: not allowed during destruction of the map.");
            }

            try
            {
                return this->_container.at(key);
            }
            catch (const std::out_of_range&)
            {
                throw cbeam::error::out_of_range(what_arg);
            }
        }

        /// \brief Finds an element with a specific key
        ///
        /// \param key Key of the element to find
        /// \param what_arg Optional argument for the exception message
        /// \return Iterator to an element with key equivalent to key. If no such element is found, past-the-end (see end()) iterator is returned.
        /// \throw cbeam::error::out_of_range If what_arg is not empty and the key is not found
        auto find(const Key& key, const std::string& what_arg = {})
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::find: not allowed during modification of the map.");
            }
            auto it = this->_container.find(key);
            if (!what_arg.empty() && it == this->_container.end())
            {
                throw cbeam::error::out_of_range(what_arg);
            }
            return it;
        }

        /// \brief Access or insert specified element
        ///
        /// \param key Key of the element to find or insert
        /// \return Reference to the mapped value of the element with key equivalent to key
        Value& operator[](const Key& key)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::operator[]: not allowed during modification of the map.");
            }
            return this->_container[key];
        }

        /// \brief Removes the element with the specified key
        ///
        /// \param key Key of the element to remove
        /// \return The number of elements removed (0 or 1)
        std::size_t erase(const Key& key)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::erase: not allowed during modification of the map.");
            }
            lifecycle::scoped_set(this->_is_being_modified, true);
            return this->_container.erase(key);
        }

        /**
         * @brief Removes the element at the specified position.
         *
         * @param position The iterator pointing to the element to remove.
         * @return An iterator following the last removed element. If the iterator
         *         refers to the last element, the end() iterator is returned.
         *
         * @note This method is thread-safe and will block if another thread is currently
         *       writing to the map.
         */
        typename std::map<Key, Value>::iterator erase(typename std::map<Key, Value>::iterator position)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::erase: not allowed during modification of the map.");
            }
            lifecycle::scoped_set modifying(this->_is_being_modified, true);
            return this->_container.erase(position);
        }
        /// \brief Returns the number of elements matching specific key
        ///
        /// \param key Key of the elements to count
        /// \return Number of elements with specified key
        std::size_t count(const Key& key) const
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            return this->_container.count(key);
        }

        std::pair<typename std::map<Key, Value>::iterator, bool> insert(const Key& key, const Value& value)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::insert: not allowed during modification of the map.");
            }
            lifecycle::scoped_set modifying(this->_is_being_modified, true);
            return this->_container.insert({key, value});
        }

        /**
         * @brief Inserts a new element or assigns to the current element if the key already exists.
         *
         * @param pair A pair consisting of the key and value to insert into the map.
         * @return A pair consisting of an iterator to the inserted or assigned element and a bool
         *         denoting whether the insertion took place.
         *
         * @note This method is thread-safe and will block if another thread is currently
         *       writing to the map.
         */
        std::pair<typename std::map<Key, Value>::iterator, bool> insert(const std::pair<Key, Value>& pair)
        {
            std::lock_guard<std::recursive_mutex> lock(this->_mutex);
            if (this->_is_being_modified)
            {
                throw cbeam::error::runtime_error("cbeam::container::thread_safe_map::insert: not allowed during modification of the map.");
            }
            lifecycle::scoped_set modifying(this->_is_being_modified, true);
            return this->_container.insert(pair);
        }
    };
}
