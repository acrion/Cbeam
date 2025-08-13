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
#include <cbeam/logging/log_manager.hpp> // for CBEAM_LOG_DEBUG

#include <cassert> // for assert

#include <map>     // for std::map, std::operator!=, std::_Rb_tree_iterator
#include <memory>  // for std::unique_ptr, std::allocator, std::shared_ptr, std::make_shared, std::make_unique
#include <mutex>   // for std::mutex, std::lock_guard
#include <string>  // for std::string, std::operator+, std::basic_string, std::char_traits
#include <utility> // for std::pair, std::forward
#include <vector>  // for std::vector

namespace cbeam::lifecycle
{
    /**
     * @class singleton_control
     * @brief Base class for controlling the lifecycle of all `singleton` instances across different types.
     *
     * `singleton_control` allows a coordinated reset or shutdown of multiple resources that are
     * each managed by `singleton<T, ...>`. This is achieved by storing all instances in
     * a static map and providing methods to reset or to set the operational state.
     */
    struct singleton_control
    {
    public:
        virtual ~singleton_control() = default;

        /**
         * @brief Resets (shuts down) all stored singleton instances.
         *
         * After calling `reset()`, new calls to `singleton<T>::get(...)` will return `nullptr`
         * until `set_operational()` is called. This is a global, collective approach for
         * explicitly destroying all singletons and freeing their resources in a controlled manner.
         */
        static void reset()
        {
            CBEAM_LOG_DEBUG("cbeam::lifecycle::singleton_control::reset");

            std::lock_guard<std::mutex> lock(_mutex);
            _shutdown = true;
            for (auto& instance : _instances)
            {
                instance.second->release_instance();
                instance.second.reset();
            }
            _instances.clear();
        }

        /**
         * @brief Ends the shut-down state and allows singletons to be created again.
         *
         * Particularly useful in test scenarios where you want to re-initialize singletons
         * after a global teardown.
         */
        static void set_operational()
        {
            CBEAM_LOG_DEBUG("cbeam::lifecycle::singleton_control::set_operational");
            _shutdown = false;
        }

    protected:
        /**
         * @brief Releases the internal resource of the derived singleton.
         * Called during a global reset.
         */
        virtual void release_instance() = 0;

        static inline std::map<std::string, std::unique_ptr<singleton_control>> _instances;
        static inline std::mutex                                                _mutex;
        static inline bool                                                      _shutdown{false};
    };

    /**
     * @brief Manages a single, shared resource of type T with controlled and explicit lifecycle management.
     *
     * ### Background and Motivation
     *
     * In traditional singleton implementations, the singleton instance is often released at program
     * termination by the runtime (via static object destruction). This can lead to issues if the singleton's
     * destructor depends on resources that have already been destroyed earlier — for example, if other
     * global or static objects that the singleton relies on have already been torn down.
     *
     * By contrast, `singleton` and its base class `singleton_control` enable the creation and, most importantly,
     * the explicit destruction of singleton objects without relying on the undefined order
     * of static deinitialization. This ensures that resources can be released in a well-defined sequence
     * at a consciously chosen point in time, which is particularly useful in scenarios with complex dependencies.
     *
     * ### How It Works
     *
     * - `singleton<T>::get(name, args...)` either creates a new instance or returns the existing one,
     *   keyed by a string `name`.
     * - All singleton instances, regardless of their types, can be collectively destroyed via
     *   `singleton_control::reset()`. Once reset, no further singletons can be created until
     *   `set_operational()` is called again (commonly used in tests).
     * - For a fine-grained removal, you can call `singleton<T>::release(name)`, which removes only
     *   the singleton with that specific name.
     *
     * ### Note on `myResource.reset()`
     *
     * Depending on the use case, either use the `std::shared_ptr` interface, e.g. `myResource.get().reset()`
     * (also see SingletonTest::ResourceRelease) or remove the singleton globally via `singleton<T>::release(name)`.
     *
     * @example cbeam/lifecycle/singleton.hpp
     * @code{.cpp}
     * // Create or retrieve the "Example" singleton of type MyClass
     * auto myResource = singleton<MyClass, int, std::string>::get("Example", 5, "Hello");
     *
     * // We can get a std::shared_ptr from it directly
     * std::shared_ptr<MyClass> ptr = myResource; // Just another reference
     *
     * // Release only our local pointer. The object remains alive as long as any shared_ptr references it.
     * ptr.reset();
     *
     * // Explicitly remove the singleton from the global map if desired
     * singleton<MyClass>::release("Example");
     *
     * // Or perform a full global destruction of all singletons:
     * singleton_control::reset();
     * @endcode
     *
     * @tparam T    The resource type to manage.
     * @tparam Args Constructor parameter types for T.
     */
    template <typename T, typename... Args>
    class singleton : public singleton_control
    {
    private:
        struct private_construction_tag
        {
        };

    public:
        /**
         * @brief Internal constructor. Use get() to create a new instance.
         */
        singleton(private_construction_tag, const std::string& name, Args... args)
            : _name{name}
            , _instance{std::make_shared<T>(std::forward<Args>(args)...)}
        {
            CBEAM_LOG_DEBUG("cbeam::lifecycle::singleton construction: " + _name);
        }

        ~singleton() noexcept override
        {
        }

        /**
         * @brief Retrieves (or creates) the shared instance of type T by name.
         *
         * If `_shutdown` is true, returns `nullptr` immediately. Otherwise,
         * it either returns the existing instance (if present) or creates a new one.
         *
         * @param name  Unique name for the resource.
         * @param args  Forwarded constructor arguments for T if a new instance is created.
         * @return      A `std::shared_ptr<T>` to the managed resource, or `nullptr` if shut down.
         * @throws      cbeam::error::runtime_error if a type conflict arises (rare).
         */
        static std::shared_ptr<T> get(const std::string& name, Args... args)
        {
            if (_shutdown)
            {
                CBEAM_LOG_DEBUG("cbeam::lifecycle::singleton::get: " + name + ": refused to create an instance because singleton_control::reset() "
                                                                              "had been called. Use singleton_control::set_operational() to enable again.");
                return nullptr;
            }
            std::lock_guard<std::mutex> lock(_mutex);

            auto&      base_instance        = _instances[name];
            const bool create_base_instance = !base_instance;

            if (create_base_instance)
            {
                base_instance = std::make_unique<singleton>(private_construction_tag{}, name, std::forward<Args>(args)...);
            }

            auto derived_instance = dynamic_cast<singleton*>(base_instance.get());

            if (derived_instance)
            {
                return derived_instance->_instance;
            }
            else
            {
                if (create_base_instance)
                {
                    base_instance.reset();
                }
                throw cbeam::error::runtime_error("cbeam::lifecycle::singleton: incompatible singleton type requested.");
            }
        }

        /**
         * @brief Explicitly removes the named singleton instance from the global map.
         *
         * If you need a full reset of all singletons, use `singleton_control::reset()`.
         *
         * @param name  Name of the resource to remove.
         */
        static void release(const std::string& name)
        {
            CBEAM_LOG_DEBUG("cbeam::lifecycle::singleton::release: " + name);
            std::lock_guard<std::mutex> lock(_mutex);

            auto it = _instances.find(name);
            if (it != _instances.end())
            {
                it->second.reset();
                _instances.erase(it);
            }
        }

    protected:
        /**
         * @brief Called by `singleton_control::reset()` to release the managed resource.
         */
        void release_instance() override
        {
            CBEAM_LOG_DEBUG("cbeam::lifecycle::singleton::release_instance: " + _name);
            _instance.reset();
        }

    private:
        singleton(const singleton&)            = delete;
        singleton(singleton&&)                 = delete;
        singleton& operator=(const singleton&) = delete;
        singleton& operator=(singleton&&)      = delete;

        std::string        _name;
        std::shared_ptr<T> _instance;
    };
}
