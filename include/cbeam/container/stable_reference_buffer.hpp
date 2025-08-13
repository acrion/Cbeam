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

#include <cbeam/concurrency/process.hpp>               // for cbeam::concurrency::get_current_process_id
#include <cbeam/container/buffer.hpp>                  // for cbeam::container::buffer
#include <cbeam/container/stable_interprocess_map.hpp> // for cbeam::container::stable_interprocess_map
#include <cbeam/convert/string.hpp>                    // for cbeam::convert::to_string
#include <cbeam/error/bad_alloc.hpp>                   // for cbeam::error::bad_alloc
#include <cbeam/error/logic_error.hpp>                 // for cbeam::error::logic_error
#include <cbeam/error/runtime_error.hpp>               // for cbeam::error::runtime_error
#include <cbeam/lifecycle/singleton.hpp>               // for cbeam::lifecycle::singleton
#include <cbeam/logging/log_manager.hpp>               // for CBEAM_LOG_DEBUG, CBEAM_LOG

#include <stdlib.h> // for free

#include <cassert> // for assert
#include <cstddef> // for size_t, std::size_t
#include <cstdint> // for uint8_t

#include <memory> // for std::shared_ptr, std::__shared_ptr_access, std::allocator
#include <set>    // for std::operator!=, std::set
#include <string> // for std::operator+, std::to_string, std::char_traits, std::basic_string, std::string

namespace cbeam::container
{
    /**
     * @brief Manages memory buffers with stable reference counting, optimized for shared library contexts.
     * @details Extends dynamic memory buffer functionality with a robust reference counting mechanism, essential in multi-library environments.
     * The `cbeam::stable_interprocess_map` ensures a consistent and unique count for each buffer instance, even across different shared libraries.
     * This approach solves the common problem of duplicated reference counts and ensures memory buffer management remains reliable and predictable
     * in varied compiler environments. The interprocess memory is exclusively for reference counting, while the buffer itself uses standard memory.
     *
     * - **Reference Counting in Shared Libraries**: Utilizes interprocess memory for a consistent reference count, separate from the buffer's standard memory.
     * - **Memory Management**: Employs efficient memory resizing techniques using malloc and free, with uint8_t as the internal data type.
     *
     * ### Usage
     * Suitable for complex memory management scenarios involving multiple shared libraries within a single process.
     */
    class stable_reference_buffer : public buffer
    {
    public:
        /**
         * @brief Manages delayed deallocation of memory blocks in shared library contexts.
         * @details Ensures that memory blocks remain valid even if their reference count drops to zero within the scope of this class.
         * This capability is critical when shared libraries need to pass raw pointers back and forth, especially in cases where a library
         * might be unloaded, but the memory it allocated is still in use. The `stable_reference_buffer::safe_get()` method complements this by safely
         * retrieving raw pointers, ensuring that they are valid within the `delay_deallocation` scope, and returning `nullptr` if not to prevent unsafe access.
         *
         * - **Scope-Based Management**: Maintains memory buffer validity within `delay_deallocation` scope, allowing safe pointer access.
         * - **Use Case**: Preserves the reference count of buffers even after the associated `stable_reference_buffer` instances are destructed,
         * especially critical in scenarios involving the unloading of shared libraries.
         * - **Examples**: Consult `DelayDeallocationTest` for practical implementations.
         *
         * ### Best Practices
         * - Use `delay_deallocation` where `stable_reference_buffer` raw pointers are needed, especially when the original buffer instance may have been destructed.
         * - Rely on `stable_reference_buffer::safe_get()` for secure pointer retrieval, ensuring it's within an active `delay_deallocation` scope.
         */
        class delay_deallocation
        {
        public:
            delay_deallocation()
                : _use_count{get_use_count()}
                , _initial_on_entry{get_initial_use_count()} // remember entry state
            {
                auto lock = _use_count->get_lock_guard();
                _use_count->foreach ([this](auto kv)
                                     {
            _old_entries.insert(kv.first);
            return true; });
                // increase initial use count for allocations created in this scope
                _use_count->insert(nullptr, _initial_on_entry + 1);
            }

            ~delay_deallocation() noexcept
            {
                try
                {
                    auto lock = _use_count->get_lock_guard();

                    // Collect new entries under lock
                    std::vector<uint8_t*> new_entries;
                    _use_count->foreach ([&](auto kv)
                                         {
                auto ptr = kv.first;
                if (ptr != nullptr && _old_entries.count(ptr) == 0) {
                    new_entries.push_back(ptr);
                }
                return true; });

                    // Now decrement those new entries.
                    for (auto* ptr : new_entries)
                    {
                        int updated = 0;
                        try
                        {
                            updated = _use_count->update(ptr, [](int& c)
                                                         { --c; });
                        }
                        catch (...)
                        {
                            // If the key vanished meanwhile, just continue; this scope no longer owns it.
                            continue;
                        }

                        if (updated == 0)
                        {
                            CBEAM_LOG_DEBUG("... deallocating " + convert::to_string((void*)ptr)
                                            + " when leaving delay_deallocation scope");
                            free(ptr);
                            _use_count->erase(ptr);
                        }
                        else if (updated < 0)
                        {
                            CBEAM_LOG("stable_reference_buffer::delay_deallocation: negative refcount detected");
                            assert(false);
                        }
                    }

                    // Restore initial use count to its previous value.
                    _use_count->insert(nullptr, _initial_on_entry);
                }
                catch (...)
                {
                    // never throw from noexcept dtor
                    CBEAM_LOG("stable_reference_buffer::delay_deallocation: unexpected exception in destructor");
                    assert(false);
                }
            }

        private:
            std::set<uint8_t*>                                      _old_entries;
            std::shared_ptr<stable_interprocess_map<uint8_t*, int>> _use_count;
            int                                                     _initial_on_entry{1}; // FIX: remember starting value
        };

        stable_reference_buffer() ///< Will not create any memory block. Use `append` to create one or append bytes to an existing one.
            : buffer()
        {
        }

        /// \brief Create a managed memory block with optional element size.
        /// \param size Number of elements to allocate. Defaults to allocating single bytes if size_of_type is not specified.
        /// \param size_of_type (Optional) Byte size of each element to allocate. Defaults to 1 if not specified, allowing for byte-sized handling similar to std::memset.
        stable_reference_buffer(const std::size_t size, const size_t size_of_type = 1)
            : buffer(size, size_of_type)
        {
            _use_count->update_or_insert(
                _buffer, [](int& count)
                { ++count; },
                get_initial_use_count());

            CBEAM_LOG_DEBUG("cbeam::container::container::stable_reference_buffer: Allocated   " + convert::to_string((void*)_buffer) + " with useCount=" + std::to_string(get_initial_use_count()));
        }

        stable_reference_buffer(const buffer& base)
            : buffer(base)
        {
            _use_count->update_or_insert(
                _buffer, [](int& count)
                { ++count; },
                get_initial_use_count());

            CBEAM_LOG_DEBUG("cbeam::container::container::stable_reference_buffer: Allocated   " + convert::to_string((void*)_buffer) + " with useCount=" + std::to_string(get_initial_use_count()));
        }

        /// \brief deallocate the managed memory block, in case this instance olds the last reference to it. Otherwise, do nothing.
        ~stable_reference_buffer() noexcept override
        {
            stable_reference_buffer::reset();
        }

        /// \brief create a managed memory block from address. If this address is a known address, it only increases the reference count
        /// \details Different from std::shared_ptr<T>(T*), because it checks if pointer is managed by another instance. Note that
        /// size() will be zero for instances the are constructed in this way. Appending to such an instance will throw.
        explicit stable_reference_buffer(const void* address)
        {
            _buffer = (uint8_t*)address;

            _use_count->update(
                _buffer, [](int& count)
                { ++count; },
                "cbeam::container::container::stable_reference_buffer: memory address " + convert::to_string((void*)address) + " was not created by cbeam::stable_reference_buffer");

            CBEAM_LOG_DEBUG("cbeam::container::container::stable_reference_buffer: " + std::to_string(_use_count->at(_buffer)) + ". reference to " + convert::to_string((void*)_buffer) + " (added from raw pointer)");
        }

        /// \brief copy construction means that the copied instance increases the reference count of the managed memory address
        stable_reference_buffer(const stable_reference_buffer& other)
        {
            if (other._buffer == nullptr)
            {
                throw cbeam::error::runtime_error("cbeam::container::container::stable_reference_buffer: copy constructor has been passed a default constructed (therefore invalid) instance of Memory");
            }

            _buffer = other._buffer;
            _size   = other._size;
            _use_count->update(_buffer, [this, &other](int& count)
                               { ++count; });

            CBEAM_LOG_DEBUG("cbeam::container::container::stable_reference_buffer: " + std::to_string(_use_count->at(_buffer)) + ". reference to " + convert::to_string((void*)_buffer) + " (added from copy constructor)");
        }

        /// \brief append the given buffer to the end of the current buffer. If there is no current buffer yet, it will be allocated. Memory will be copied if required, otherwise expanded.
        void append(const void* data, const size_t len) override
        {
            auto lock = _use_count->get_lock_guard();

            if (_size == 0 && _buffer != nullptr)
            {
                throw cbeam::error::logic_error(
                    "stable_reference_buffer::append: instance was created from a raw pointer without a known size");
            }

            if (_buffer && _use_count->at_or_default(_buffer, 0) > 1)
            {
                // ---- Copy-on-Write path: other instances still reference the old block ----
                const size_t new_size = _size + len;
                uint8_t*     new_buf  = static_cast<uint8_t*>(std::malloc(new_size));
                if (!new_buf)
                {
                    CBEAM_LOG("stable_reference_buffer::append (COW): Out of RAM (" + std::to_string(new_size) + ")");
                    throw cbeam::error::bad_alloc();
                }
                // copy old data + append new bytes
                std::memcpy(new_buf, _buffer, _size);
                std::memcpy(new_buf + _size, data, len);

                // drop our reference to the old block
                int remaining = 0;
                try
                {
                    remaining = _use_count->update(_buffer, [](int& c)
                                                   { --c; });
                }
                catch (...)
                {
                    remaining = 0; // if the key vanished, treat as zero
                }
                if (remaining == 0)
                {
                    std::free(_buffer);
                    _use_count->erase(_buffer);
                }
                else if (remaining < 0)
                {
                    CBEAM_LOG("stable_reference_buffer::append: negative refcount after decrement");
                    assert(false);
                }

                _buffer = new_buf;
                _size   = new_size;

                // newly created block: start with current initial count (respects delay_deallocation scopes)
                _use_count->insert(_buffer, get_initial_use_count());
                return;
            }

            // ---- Exclusive owner (or no buffer yet): realloc is safe ----
            uint8_t* old       = _buffer;
            int      old_count = (old ? _use_count->at_or_default(old, get_initial_use_count()) : 0);

            buffer::append(data, len); // may move and free 'old'

            if (_buffer != old)
            {
                if (old)
                {
                    // preserve the previous refcount when moving the address
                    _use_count->insert(_buffer, old_count);
                    _use_count->erase(old);
                }
                else
                {
                    _use_count->insert(_buffer, get_initial_use_count());
                }
            }
        }

        /**
         * @brief Provides safe access to the raw pointer of the managed memory buffer.
         * @details This method is a convenience function designed for typical use cases where the safety of accessing the raw pointer
         * depends on the lifecycle management of the `stable_reference_buffer` instance. It checks the reference count of the buffer
         * and returns a raw pointer only if the buffer is not the sole reference. This ensures safety in contexts where instances of
         * `stable_reference_buffer::delay_deallocation` are used to manage memory lifecycles and avoid premature deallocation.
         *
         * The method is safe under the condition that the raw pointer is not used outside the scope of the `delay_deallocation` instance.
         * The direct use of `get()` can be safe as well, provided the caller guarantees that the `stable_reference_buffer` instance
         * is not destructed while the raw pointer is in use. Conversely, `safe_get()` can be unsafe if the raw pointer is passed beyond
         * the scope of `delay_deallocation`.
         *
         * ### Best Practices
         * - Use `safe_get()` inside scope of an instance of `delay_deallocation`.
         * - Do not pass the raw pointer outside the scope of `delay_deallocation`.
         *
         * @return void* A raw pointer to the memory buffer if safe, otherwise `nullptr`.
         * @note The safety of using the returned raw pointer depends on the scope of `delay_deallocation`.
         */
        void* safe_get() const noexcept
        {
            if (use_count() <= 1)
            {
                CBEAM_LOG("Error: Attempt to access the raw pointer via cbeam::container::stable_reference_buffer::safe_get() without adequate reference count. This operation is blocked and returns a nullptr to prevent unsafe memory access, as the buffer could be invalidated upon destruction of this instance.");
                return nullptr;
            }

            return get();
        }

        /// \brief copy assignment means that the copied instance increases the reference count of the managed memory address
        stable_reference_buffer& operator=(const stable_reference_buffer& other)
        {
            if (this != &other)
            {
                if (other._buffer == nullptr)
                {
                    throw cbeam::error::runtime_error("cbeam::container::container::stable_reference_buffer: copy assignment operator has been passed a default constructed (therefore invalid) instance of Memory");
                }

                auto lock = _use_count->get_lock_guard();
                reset();

                assert(is_known(other._buffer));
                _buffer = other._buffer;
                _size   = other._size;
                _use_count->update(_buffer, [this, &other](auto& count)
                                   { ++count; });
                CBEAM_LOG_DEBUG("cbeam::container::container::stable_reference_buffer: " + std::to_string(_use_count->at(_buffer)) + ". reference to " + convert::to_string((void*)_buffer) + " (added from copy assignment operator)");
            }
            return *this;
        }

        stable_reference_buffer& operator=(const buffer& other) override
        {
            if (this == &other)
            {
                return *this;
            }

            auto lock = _use_count->get_lock_guard();
            reset();
            buffer::operator=(other);

            if (is_known(_buffer))
            {
                _use_count->insert(_buffer, _use_count->at(_buffer) + 1);
            }
            else
            {
                _use_count->insert(_buffer, get_initial_use_count());
            }

            return *this;
        }

        /// \brief return if the given address is one of the managed addresses
        static bool is_known(const void* address)
        {
            if (!address)
            {
                return false; // performance optimization for nullptr
            }

            auto use_count = get_use_count();
            auto lock      = use_count->get_lock_guard();
            return use_count->count((uint8_t*)address) == 1;
        }

        /// \brief Get the use count of the managed memory block
        /// \return Number of shared_buffer instances managing this block
        size_t use_count() const
        {
            if (_buffer == nullptr)
            {
                return 0;
            }
            return _use_count->at_or_default(_buffer, 0); // return 0 if the buffer is unknown
        }

        /// \brief Resets the shared_buffer instance, potentially deallocating the managed memory block
        void reset() noexcept override
        {
            try
            {
                auto lock = _use_count->get_lock_guard();

                if (is_known(_buffer))
                {
                    auto updated_value = _use_count->update(_buffer, [](auto& count)
                                                            { --count; });

                    if (updated_value == 0)
                    {
                        CBEAM_LOG_DEBUG("Deallocating " + convert::to_string((void*)_buffer));
                        _use_count->erase(_buffer);
                        buffer::reset();
                    }
                    else
                    {
                        CBEAM_LOG_DEBUG("Removed reference to " + convert::to_string((void*)_buffer) + " (" + std::to_string(_use_count->at(_buffer)) + " left)");

                        if (updated_value < 0)
                        {
                            CBEAM_LOG("cbeam::stable_reference_buffer::reset: Detected invalid pointer to " + convert::to_string((void*)_buffer));
                            assert(false);
                        }
                    }
                }
                _buffer = nullptr;
                _size   = 0;
            }
            catch (const std::exception& ex)
            {
                CBEAM_LOG("cbeam::container::container::stable_reference_buffer: ");
                assert(false);
            }
        }

        using buffer::swap;

        /// \brief Swaps the contents of this shared_buffer with another shared_buffer
        /// \param other Another shared_buffer to swap with
        void swap(stable_reference_buffer& other)
        {
            auto lock = _use_count->get_lock_guard();
            buffer::swap(other);
        }

    private:
        /**
         * @brief Manages the reference count for memory buffers, specifically designed for shared library contexts.
         * @details Unlike typical memory containers, this class uses interprocess memory for the reference counter to
         * effectively share and manage memory across different shared libraries within the same process. This design ensures
         * that memory allocated by one library can be safely and correctly managed by another, without duplicating reference counts.
         *
         * - **Shared Library Memory Management**: Addresses the challenge of managing memory across shared libraries in a single process.
         * - **Non-Interprocess Memory for Buffers**: While the reference count uses interprocess memory, the actual container memory is
         *   standard and accessible to all shared libraries within the same process context.
         */
        static std::shared_ptr<stable_interprocess_map<uint8_t*, int>> get_use_count()
        {
            std::size_t bytes = 1ull << 16; // 64 KiB default (maximum during heavy loads in acrionphoto: 2 KiB)

            if (const char* env = std::getenv("CBEAM_SRB_MAP_BYTES"))
            {
                char*              end = nullptr;
                unsigned long long v   = std::strtoull(env, &end, 10);
                if (end && *end == '\0' && v >= 1024)
                {
                    bytes = static_cast<std::size_t>(v);
                }
            }

            return lifecycle::singleton<
                stable_interprocess_map<uint8_t*, int>,
                std::string,
                std::size_t>::get("cbeam::memory::stable_reference_buffer::_use_count",
                                  std::to_string(concurrency::get_current_process_id()) + ".srb.cbeam",
                                  bytes);
        }

        /// \brief get the initial value of the reference counter for a newly created buffer
        /// \details This may be adapted by instances of class delay_deallocation.
        static int get_initial_use_count()
        {
            return get_use_count()->at_or_default(nullptr, 1); // we use the key nullptr to store the initial use count
        }

        /**
         * @brief Points to the single instance of an interprocess-capable reference counter for memory blocks.
         * @details The `_use_count` member is a shared pointer to a `stable_interprocess_map`, mapping memory blocks (uint8_t*) to their respective reference counts (int). This reference counter is used for
         * managing the lifecycle of memory buffers in shared library contexts. The use of a shared pointer for `_use_count`, and the decision against a static design, are based on the following considerations:
         *
         * - **Global Reference Counting**: Despite each instance of `stable_reference_buffer` utilizing a shared pointer, `_use_count` consistently points to a singular, global instance of `stable_interprocess_map`.
         * This architecture guarantees that every memory block is associated with a unique and uniform reference count, irrespective of the number of instances or the diversity of shared libraries. This approach
         * effectively circumvents the typical duplication issues encountered in traditional designs employing a static singleton instance, especially in complex multi-library environments.
         * - **Integration with `lifecycle::singleton`**: The approach of using a non-static instance of `stable_interprocess_map` aligns with the architecture of the `lifecycle::singleton` class.
         * This class allows managing singleton instances without static initializations by using unique identifiers (strings) for accessing singleton objects.
         * - **Efficient Resource Management**: By employing a `std::shared_ptr` for `_use_count`, the class benefits from automatic and safe memory management. This facilitates resource handling and ensures
         * that the reference counter is properly cleaned up when the last reference to it is released.
         *
         * In summary, `_use_count` as a non-static member enables efficient and robust management of memory blocks across different shared libraries in the same process, in line with the innovative design of `lifecycle::singleton`.
         */
        std::shared_ptr<stable_interprocess_map<uint8_t*, int>> _use_count{get_use_count()};
    };
}
