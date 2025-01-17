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

#include <memory>   // for std::shared_ptr
#include <stddef.h> // for size_t

namespace cbeam::container
{
    /**
     * @brief Creates a `std::shared_ptr` that manages a dynamically allocated array of size `size`.
     *
     * This function returns a `std::shared_ptr<T>` that holds an array of type `T`.
     * The custom deleter ensures the array is deleted with `delete[]`.
     *
     * @tparam T The array element type.
     * @param size The number of elements in the array.
     * @return std::shared_ptr<T> A smart pointer to a dynamically allocated array.
     */
    template <typename T>
    inline std::shared_ptr<T> make_shared_array(const size_t size)
    {
        return std::shared_ptr<T>(new T[size], [](const T* p)
                                  { delete[] p; });
    }
}
