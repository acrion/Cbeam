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

#include <cbeam/error/overflow_error.hpp> // for cbeam::error:overflow_error
#include <cbeam/error/runtime_error.hpp>  // for cbeam::error:runtime_error

#include <cstddef> // for std::size_t

#include <mutex>     // for std::mutex, std::lock_guard
#include <set>       // for std::allocator, std::set, std::operator!=, std::_Rb_tree_const_iterator
#include <string>    // for std::operator+, std::char_traits, std::to_string

namespace cbeam::lifecycle
{
    /**
     * @class item_registry
     * @brief Manages the registration and deregistration of items with unique identifiers.
     *
     * This class implements a system for managing item registration and deregistration, assigning
     * unique identifiers to each item. Identifiers of deregistered items are recycled and reassigned
     * to new items upon registration. In limited mode (specified maximum number of items), the range
     * of unique identifiers is [0..max_number_of_items-1]. In unlimited mode (maximum number set to zero),
     * identifiers are assigned dynamically with no upper limit, reusing identifiers from deregistered
     * items when available.
     */
    class item_registry
    {
    public:
        /**
         * @brief Constructor for item_registry.
         *
         * Initializes the item registry with a specified maximum number of items. If the maximum number
         * is set to zero, the registry operates in an unlimited mode, allowing an indefinite number
         * of items to be registered. In limited mode, the identifiers range from 0 to
         * max_number_of_items - 1.
         *
         * @param max_number_of_items The maximum number of items that can be registered. A value of zero
         *                            indicates no limit on the number of items.
         */
        explicit item_registry(std::size_t max_number_of_items = 0)
            : _max_number_of_items(max_number_of_items)
        {
            for (std::size_t i = 0; i < _max_number_of_items; ++i)
            {
                _available_numbers.insert(i);
            }
        }

        /**
         * @brief Registers an item and returns its unique identifier.
         *
         * Registers a new item and assigns a unique identifier to it. If there are identifiers available
         * from previously deregistered items, one of these will be reassigned. Identifiers are assigned
         * sequentially. If the registry is full in limited mode (max_number_of_items > 0), or if the
         * identifier count overflows in unlimited mode, an exception is thrown.
         *
         * @return The unique identifier for the newly registered item.
         * @throws cbeam::error::overflow_error if the registry is full in limited mode or if the identifier count overflows in unlimited mode.
         */
        std::size_t register_item()
        {
            std::lock_guard<std::mutex> lock(_registry_mutex);

            if (_max_number_of_items > 0)
            {
                if (!_available_numbers.empty())
                {
                    auto        it          = _available_numbers.begin();
                    std::size_t item_number = *it;
                    _available_numbers.erase(it);
                    return item_number;
                }
                throw cbeam::error::overflow_error("cbeam::lifecycle::item_registry::register_item: No more item slots available. increase the maximum number of items to register more.");
            }
            else
            {
                if (!_available_numbers.empty())
                {
                    auto        it          = _available_numbers.begin();
                    std::size_t item_number = *it;
                    _available_numbers.erase(it);
                    return item_number;
                }
                if (_next_item_number == std::numeric_limits<decltype(_next_item_number)>::max())
                {
                    throw cbeam::error::overflow_error("cbeam::lifecycle::item_registry::register_item: Maximum item count reached.");
                }
                return _next_item_number++;
            }
        }

        /**
         * \brief Deregisters an item given its unique identifier.
         *
         * This method deregisters an item, making its identifier available for future registrations.
         * If the identifier is invalid or the item has already been deregistered, a runtime_error exception is thrown.
         *
         * @param item_number The unique identifier of the item to be deregistered.
         * @throws cbeam::error::runtime_error if item_number is invalid or if the item has already been deregistered.
         */
        void deregister_item(std::size_t item_number)
        {
            if (_max_number_of_items && item_number >= _max_number_of_items)
            {
                throw cbeam::error::runtime_error("cbeam::lifecycle::item_registry::deregister_item: Invalid item number " + std::to_string(item_number) + " for deregistration.");
            }

            std::lock_guard<std::mutex> lock(_registry_mutex);

            if (_available_numbers.find(item_number) != _available_numbers.end())
            {
                throw cbeam::error::runtime_error("cbeam::lifecycle::item_registry::deregister_item: Item number " + std::to_string(item_number) + " already deregistered.");
            }

            _available_numbers.insert(item_number);
        }

    private:
        const std::size_t     _max_number_of_items; ///< The maximum number of items that can be registered.
        std::size_t           _next_item_number{0};
        std::set<std::size_t> _available_numbers; ///< Set of available identifiers for item registration.
        std::mutex            _registry_mutex;    ///< Mutex for thread-safe operations on the registry.
    };
}
