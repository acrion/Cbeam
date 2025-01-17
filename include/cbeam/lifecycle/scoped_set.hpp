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

#include <atomic> // for std::atomic

namespace cbeam::lifecycle
{
    /**
     * @brief Extracts the underlying value type `T` from either `T` or `std::atomic<T>`.
     *
     * @tparam T The type to inspect.
     * If `T` is `std::atomic<U>`, then `type` is `U`. Otherwise `type` is `T`.
     */
    template <typename T>
    struct extract_atomic_value_type
    {
        using type = T;
    };

    template <typename T>
    struct extract_atomic_value_type<std::atomic<T>>
    {
        using type = T;
    };

    /**
     * @brief A helper that sets the given variable to a new value, and restores the original value on destruction.
     *
     * This class can handle both normal and `std::atomic` variables. On construction,
     * it stores the old value, then sets the variable to the new value. On destruction,
     * it reverts the variable back to the original value.
     *
     * @tparam T The type of the managed variable (normal or atomic).
     */
    template <typename T>
    class scoped_set
    {
    public:
        using ValueType = typename extract_atomic_value_type<T>::type;

        /**
         * @brief Constructs a `scoped_set` and assigns `newVal` to `instance`.
         *
         * Stores the previous value, which is restored on destruction.
         *
         * @param instance A reference to the variable to manage.
         * @param newVal The new value to set temporarily.
         */
        scoped_set(T& instance, ValueType newVal)
            : _instance(instance)
            , _old_value(instance)
        {
            _instance = newVal;
        }

        /**
         * @brief Destructor that restores the original value of the managed variable.
         */
        virtual ~scoped_set() noexcept
        {
            _instance = _old_value;
        }

    private:
        T&        _instance;
        ValueType _old_value;
    };
}
