[**Cbeam**](https://cbeam.org) is a **modern, cross-platform, header-only C++ library** covering essential functionality in:

- [Concurrency](https://cbeam.org/doxygen/namespacecbeam_1_1concurrency.html)
- [Containers](https://cbeam.org/doxygen/namespacecbeam_1_1container.html)
- [Conversion](https://cbeam.org/doxygen/namespacecbeam_1_1convert.html)
- [Encoding](https://cbeam.org/doxygen/namespacecbeam_1_1encoding.html)
- [Filesystem interaction](https://cbeam.org/doxygen/namespacecbeam_1_1filesystem.html)
- [JSON](https://cbeam.org/doxygen/namespacecbeam_1_1json.html)
- [Lifecycle management](https://cbeam.org/doxygen/namespacecbeam_1_1lifecycle.html)
- [Logging](https://cbeam.org/doxygen/namespacecbeam_1_1logging.html)
- [Memory](https://cbeam.org/doxygen/namespacecbeam_1_1memory.html)
- [Platform abstraction](https://cbeam.org/doxygen/namespacecbeam_1_1platform.html)
- [Random generation](https://cbeam.org/doxygen/namespacecbeam_1_1random.html)
- [Serialization](https://cbeam.org/doxygen/namespacecbeam_1_1serialization.html)

While it draws inspiration from Boost’s coding style and extensive use of templates, **Cbeam** imposes **no Boost dependency**, making it lightweight to integrate into projects on **Linux**, **macOS**, and **Windows** alike. Comprehensive Doxygen documentation and thorough unit tests ensure clarity and reliability.

---

## Spotlight Features

### 1. Asynchronous Message Dispatching
The [`message_manager`](https://cbeam.org/message_manager) simplifies multithreading by letting you send messages (with FIFO, FILO, or RANDOM order) to one or more handler threads:
```cpp
cbeam::concurrency::message_manager<int> manager;
manager.add_handler(1, [](int msg) { /* handle msg */ });
manager.send_message(1, 42); 
// Thread-safe, flexible, and easy to wait_until_empty() for a clean finish
```
Designed with performance, customizable ordering, and thread-safe enqueuing in mind—particularly beneficial for distributing complex workloads.

### 2. Cross-Platform Shared Memory Without Hassle
[`interprocess_shared_memory`](https://cbeam.org/interprocess_shared_memory) unifies Windows and Unix APIs in a simple header-only approach:
```cpp
cbeam::memory::interprocess_shared_memory shm("UniqueName", 4096);
// Windows: Uses native file mapping (avoids "C:\ProgramData\boost_interprocess")
// Linux/Unix: Uses ephemeral POSIX shm_open without leftover kernel objects
```
No leftover kernel objects or complicated file permission setups. Once all processes detach, the memory is automatically cleaned up.

### 3. Stable Reference Counting Across Shared Libraries
If you’ve ever battled `std::shared_ptr` mismatches across DLLs or plugins, [`stable_reference_buffer`](https://cbeam.org/stable_reference_buffer) solves the problem:
```cpp
cbeam::container::stable_reference_buffer buf(10, sizeof(int));
{
    auto copy = buf;
    // Both references track the same block, 
    // and the refcount remains correct even if libraries differ in compiler/ABI
}
```
The reference count is stored in a safe, process-wide map, preventing double-free or missing deletions in plugin systems.

### 4. Interoperable Containers Across Compiler Boundaries
A family of "stable interprocess" containers (e.g., [`stable_interprocess_map`](https://cbeam.org/interprocess_containers)) lets you store data structures in shared memory while bridging incompatible ABIs. Automatic serialization keeps the container stable even when libraries use different compilers:
```cpp
cbeam::container::stable_interprocess_map<std::string, int> sharedMap;
// Insert data from one shared library...
sharedMap["answer"] = 42;
// ...read from another library built with a different toolchain
```
No more binary compatibility nightmares—Cbeam’s [Interprocess Containers](https://cbeam.org/interprocess_containers) abstract the underlying complexity and ensure consistent data exchange.

---

## Additional Highlights

- **Thread Management**: CRTP-based [`threaded_object`](https://cbeam.org/threaded_object) spares you from boilerplate, giving a well-defined worker thread lifecycle.  
- **Nested Maps and JSON**: Easily manage hierarchical data with [`nested_map`](https://cbeam.org/nested_map) and optional JSON or binary serialization.
- **Logging & Lifecycle**: Central logging, plus robust singletons that can be reset at will ([`singleton_control`](https://cbeam.org/singleton)) to handle complex initialization or teardown scenarios.

---

## Comprehensive Documentation and Testing

All classes and functions are thoroughly documented with Doxygen, providing up-to-date usage details. An extensive suite of unit tests and concurrency stress tests backs every feature. 

---

## Licensing

Cbeam is dual-licensed:

1. **AGPL v3 or later** for open-source use,
2. **Commercial licensing** for closed-source integration or special requirements.

We would like to emphasize that offering a dual license does not restrict users of the normal open-source license (including commercial users). The dual licensing model is designed to support both open-source collaboration and commercial integration needs. For commercial licensing inquiries, please contact us at [https://acrion.ch/sales](https://acrion.ch/sales).

---

**Discover how Cbeam can streamline your C++ concurrency, cross-library data sharing, and memory management.** Explore the linked pages for deeper insights into each component. Enjoy clean code, minimal dependencies, and a reliable, modern C++ experience.
