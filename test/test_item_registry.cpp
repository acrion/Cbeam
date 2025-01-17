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

#include <cbeam/lifecycle/item_registry.hpp>

#include <gtest/gtest.h>

#include <string> // for std::allocator, std::string
#include <vector> // for std::vector

namespace cbeam::lifecycle
{
    TEST(ItemRegistryTest, RegisterNewItem)
    {
        item_registry registry;

        auto id = registry.register_item();
        EXPECT_GE(id, 0);
    }

    TEST(ItemRegistryTest, DeregisterAndReuseId)
    {
        item_registry registry;

        auto id1 = registry.register_item();
        registry.deregister_item(id1);

        auto id2 = registry.register_item();
        EXPECT_EQ(id1, id2);
    }

    TEST(ItemRegistryTest, ReachMaximumItems)
    {
        item_registry registry;

        item_registry limited_registry(1);
        limited_registry.register_item();

        EXPECT_THROW(limited_registry.register_item(), cbeam::error::overflow_error);
    }

    TEST(ItemRegistryTest, RecycleIdentifier)
    {
        item_registry registry;

        auto id1 = registry.register_item();
        registry.deregister_item(id1);
        auto id2 = registry.register_item();

        EXPECT_EQ(id1, id2);
    }
}
