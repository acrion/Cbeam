#pragma once

/// \brief The root namespace for the Cbeam library.
/// This namespace unifies cross-platform utilities for concurrency, memory management, file handling, logging, serialization, and more. Its purpose is to provide a cohesive collection of modern C++ components that address common system-level and application-level tasks, while promoting thread-safety, interprocess communication, and stable resource handling across varied platform APIs.
namespace cbeam { }

/// \brief Provides concurrency primitives and abstractions for multithreaded programming.
/// It features the powerful `message_manager` class for managing asynchronous message queues with flexible ordering (FIFO, FILO, or RANDOM), as well as `threaded_object`, which uses CRTP to encapsulate worker-thread logic with built-in synchronization. These classes simplify threaded operations, interprocess synchronization, and message dispatch under varying concurrency scenarios.
namespace cbeam::concurrency { }

/// \brief Offers advanced container types with unique approaches to stability and interprocess sharing.
/// Besides standard helpers like `buffer` and `circular_buffer`, it includes innovative classes such as `stable_interprocess_container` and `stable_interprocess_map`. These support shared-memory usage, ensuring consistent serialization and robust data exchange across process boundaries or differing compiler environments.
namespace cbeam::container { }

/// \brief Provides a specialized variant type (`xpod::type`) for simple data exchange, supporting integer, floating-point, boolean, pointers, and strings.
/// This namespace is designed for compactness and serializability, letting you store fundamental data types in a variant structure. By including additional headers, the variant gains transparent serialization capabilities without sacrificing performance.
namespace cbeam::container::xpod { }

/// \brief Defines index constants for the xpod::type variant (e.g., integer, number, boolean, pointer, string).
/// These constants allow you to refer to a specific type index in the underlying variant, simplifying code that checks or extracts a particular xpod type.
namespace cbeam::container::xpod::type_index { }

/// \brief Contains conversion utilities to transform data between different formats and types.
/// Functions here handle string conversions, encoding manipulations, and the creation of string representations for various C++ types. They help ensure consistent serialization and text output across the library.
namespace cbeam::convert { }

/// \brief Focuses on UTF-8 checks, character handling, and encoding-specific validations.
/// It includes lightweight helpers like `is_valid_utf8` and `has_utf8_specific_encoding`, which verify the correctness and properties of strings under UTF-8 constraints.
namespace cbeam::encoding { }

/// \brief Defines Cbeam-specific exception types that behave like their standard counterparts.
/// For example, `runtime_error` and `system_error` derive from `base_error` and std::runtime_error (or std::system_error), unifying error handling under a single hierarchy and ensuring consistent cross-platform throw/catch logic.
namespace cbeam::error { }

/// \brief Facilitates file I/O, path normalization, and directory operations in a cross-platform manner.
/// Its classes and functions simplify reading, writing, creating, or deleting files and directories, handling discrepancies in path formats, symbolic links, and OS-specific peculiarities.
namespace cbeam::filesystem { }

/// \brief Provides JSON-style and nested-map serialization features.
/// It offers methods to convert a wide range of container types (maps, nested_maps) and strings into JSON-like structures, with an emphasis on composability and integration with other Cbeam utilities. The goal is to offer concise, type-safe serialization for both simple and nested data.
namespace cbeam::json { }

/// \brief Manages the lifecycle of singletons, item registries, and scoped variables.
/// This namespace introduces powerful constructs such as `singleton_control` and `item_registry` for explicitly managing the instantiation and teardown of resources, minimizing global-state risks and encouraging well-defined initialization flows.
namespace cbeam::lifecycle { }

/// \brief Offers flexible logging mechanisms to record messages with timestamps and thread information.
/// The `log_manager` can create or reinitialize log files at runtime, appending text from multiple threads safely, while `log` handles file writes with concurrency considerations. Debug-logging macros are also provided for conditional compilation.
namespace cbeam::logging { }

/// \brief Houses abstractions for shared-memory and interprocess data exchange.
/// This includes `interprocess_shared_memory` for OS-level memory segments and unified resource sharing, along with helper classes in other namespaces. It highlights stable reference counting and safe pointer conversions for robust cross-process operations.
namespace cbeam::memory { }

/// \brief Groups platform-specific helpers for Windows, Linux, and macOS.
/// These utilities detect CPU architecture, manage COM initialization on Windows, resolve runtime paths, handle system directories, and unify OS-level functionality behind well-defined C++ APIs.
namespace cbeam::platform { }

/// \brief Collects random number generation tools for multithreaded environments.
/// It includes a default thread-local generator and utilities to produce random integers, uniform distributions, or random strings. This promotes convenient generation of secure or test-oriented random values without external dependencies.
namespace cbeam::random { }

/// \brief Implements traits-based serialization for complex data types, including standard containers and custom structures.
/// It relies on specialized traits and buffer logic to convert objects to and from raw memory blocks, enabling stable interprocess data transport and straightforward integration with advanced container classes and xpod variants.
namespace cbeam::serialization { }
