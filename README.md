# ARC - Advanced Runtime Components

<div align="center">

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Header-only](https://img.shields.io/badge/header--only-✓-green.svg)](https://github.com/starworks/arc)
[![License](https://img.shields.io/badge/license-MPL2.0-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-windows%20|%20linux%20|%20macos-lightgrey.svg)](https://github.com/starworks/arc)

**A modern C++20 header-only library providing high-performance runtime utilities with zero-overhead abstractions**

[Features](#-features) •
[Quick Start](#-quick-start) •
[Components](#-components) •
[Examples](#-examples) •
[Building](#-building) •
[Documentation](#-documentation)

</div>

## 🎯 Overview

ARC (Advanced Runtime Components) is a cutting-edge C++20 library that brings modern programming paradigms to systems-level development. With a focus on compile-time computation and zero-overhead abstractions, ARC provides essential utilities for high-performance applications.

### ✨ Key Highlights

- 🚀 **Zero-overhead abstractions** - Pay only for what you use
- 📦 **Header-only** - Simple integration, no build configuration needed
- 🔧 **C++20** - Leverages concepts, if-constexpr, and modern features
- 🎯 **Type-safe** - Strong typing with compile-time guarantees
- 🔍 **Reflection** - Compile-time type and enum introspection
- 🌍 **Cross-platform** - Works on Windows, Linux, and macOS

## 🚀 Quick Start

```cpp
#include <arc/arc.hpp>

// Automatic enum reflection
enum class Status 
{ 
    Success, 
    Failed, 
    Pending 
};
auto name = arc::enum_to_string(Status::Success);  // "Success"

// Type-safe error handling
arc::result<int, std::string> divide(int a, int b) 
{
    if (b == 0) 
        return arc::err("Division by zero");
    return a / b;
}

// Efficient small string optimization
arc::sso24 message = "Hello, World!";  // Stack-allocated

// Type-safe bitfields
struct PixelTags 
{ 
    struct R {}; 
    struct G {}; 
    struct B {}; 
    struct A {}; 
};
using Pixel = arc::bitfield<
    arc::bit_spec<PixelTags::R, 8>,
    arc::bit_spec<PixelTags::G, 8>,
    arc::bit_spec<PixelTags::B, 8>,
    arc::bit_spec<PixelTags::A, 8>
>;
```

## 📚 Components

### 🔧 Platform Detection (`platform.hpp`)
Advanced platform and compiler detection with optimization hints.

```cpp
// Cache-line aligned types for false sharing prevention
arc::cache_aligned<std::atomic<int>> counter;

// Platform-specific optimizations
if constexpr (arc::platform::has_avx2) 
{
    // AVX2 optimized path
}
```

### 🔢 128-bit Integers (`int128.hpp`)
Portable 128-bit integer support with native performance when available.

```cpp
arc::uint128 large = arc::uint128{1} << 100;
arc::int128 signed_large = -large;
```

### 🏷️ Enum Utilities (`enum.hpp`)
Automatic enum↔string conversion and flag operations.

```cpp
enum class Permission 
{ 
    Read = 1, 
    Write = 2, 
    Execute = 4 
};
ARC_FLAGS_ENUM(Permission, 3);

auto perms = Permission::Read | Permission::Write;
auto names = arc::decompose_flags(perms);  // ["Read", "Write"]
```

### 🎯 Type-safe Bitfields (`bitfield.hpp`)
Tag-based bitfield implementation preventing name collisions.

```cpp
struct Tags 
{ 
    struct Field1 {}; 
    struct Field2 {}; 
};
using MyBits = arc::bitfield<
    arc::bit_spec<Tags::Field1, 4>,
    arc::bit_spec<Tags::Field2, 12>
>;
```

### ✅ Result Type (`result.hpp`)
Rust-style error handling without exceptions.

```cpp
arc::result<std::string, std::errc> read_file(const char* path) 
{
    if (auto* f = std::fopen(path, "r")) 
    {
        // ... read file ...
        return content;
    }
    return arc::err(std::errc::no_such_file_or_directory);
}

auto content = read_file("data.txt")
    .map([](auto& s) { return s.size(); })
    .unwrap_or(0);
```

### 📝 Small Buffer Optimization (`small.hpp`)
Efficient containers with inline storage.

```cpp
arc::small_string<32> path = "/usr/local/bin";  // Stack-allocated
path += "/program";  // Still on stack if fits

arc::small_buffer<std::vector<int>, 64> vec;
vec.emplace<std::vector<int>>({1, 2, 3});  // Inline storage
```

### 🔍 Compile-time Reflection (`reflect.hpp`)
Extract type and enum names at compile-time.

```cpp
constexpr auto type_name = arc::type_name<std::vector<int>>();
static_assert(type_name == "std::vector<int>");

enum class Color 
{ 
    Red, 
    Green, 
    Blue 
};
constexpr auto color_name = arc::enum_name<Color, Color::Red>();
static_assert(color_name == "Red");
```

### 🛡️ Advanced Assertions (`assert.hpp`)
Format-string assertions with custom handlers.

```cpp
ARC_ASSERT(ptr != nullptr);
ARC_ASSERT_IN_RANGE(index, 0, size, "Index {} out of bounds [0, {})", index, size);

// Custom assert handler
arc::set_assert_handler([](const arc::assert_info& info) 
{
    std::cerr << std::format("Assertion failed: {} at {}:{}\n", 
                            info.expression, info.file, info.line);
});
```

### 🧩 Template Metaprogramming (`meta.hpp`)
Advanced type manipulation utilities.

```cpp
// Automatically sized integers
using Int20 = arc::meta::auto_uint_t<20>;  // uint32_t
using Int70 = arc::meta::auto_uint_t<70>;  // arc::uint128

// Type list operations
using Types = arc::meta::type_list<int, float, double>;
constexpr bool has_float = arc::meta::contains_v<Types, float>;
```

## 💡 Examples

### Error Propagation with Result

```cpp
arc::result<Config, std::string> load_config(const std::string& path) 
{
    auto file_result = open_file(path);
    if (!file_result) 
    {
        return arc::err("Failed to open: " + file_result.error());
    }
    
    auto parse_result = parse_json(file_result.value());
    if (!parse_result) 
    {
        return arc::err("Parse error: " + parse_result.error());
    }
    
    return Config{parse_result.value()};
}

// Usage
auto config = load_config("settings.json")
    .map_err([](auto& e) { return "Configuration error: " + e; })
    .expect("Failed to load configuration");
```

### Custom Enum Ranges

```cpp
enum class Temperature : int8_t 
{ 
    AbsoluteZero = -273,
    Freezing = 0,
    Boiling = 100 
};

// Specify custom range for reflection
ARC_ENUM_RANGE(Temperature, -273, 100);

// Now enum reflection works across the full range
for (auto [value, name] : arc::enum_entries<Temperature>()) 
{
    std::cout << name << " = " << static_cast<int>(value) << "\n";
}
```

### High-Performance Bitfield Parsing

```cpp
// Network packet header
struct PacketTags 
{
    struct Version {};
    struct HeaderLength {};
    struct ServiceType {};
    struct TotalLength {};
};

using PacketHeader = arc::bitfield<
    arc::bit_spec<PacketTags::Version, 4>,
    arc::bit_spec<PacketTags::HeaderLength, 4>,
    arc::bit_spec<PacketTags::ServiceType, 8>,
    arc::bit_spec<PacketTags::TotalLength, 16>
>;

PacketHeader parse_header(uint32_t raw) 
{
    return PacketHeader::from_storage(raw);
}
```

## 🔨 Building

ARC is header-only, so no build step is required. Simply include the headers in your project:

```cmake
# CMakeLists.txt
target_include_directories(your_target PRIVATE path/to/arc/include)
```

For running tests or examples, use Premake5:

```bash
# Generate build files
premake5 vs2022    # Visual Studio 2022
premake5 gmake2    # GNU Make
premake5 xcode4    # Xcode

# Build
make config=release  # or use your IDE
```

### Requirements

- C++20 compliant compiler:
  - GCC 10+
  - Clang 11+
  - MSVC 2019 16.8+
- Optional: Premake5 for generating project files

## 📖 Documentation

### Design Philosophy

1. **Zero-overhead**: Abstractions compile away to optimal code
2. **Compile-time first**: Prefer compile-time computation over runtime
3. **Type safety**: Strong typing prevents common errors
4. **Modern C++**: Embrace C++20 features for better APIs
5. **Header-only**: Simple integration and inlining opportunities

### Best Practices

- Include only what you need - components are independent where possible
- Use `arc::result` for error handling instead of exceptions
- Leverage compile-time reflection to reduce boilerplate
- Prefer tag-based APIs (like bitfield) for type safety

### Performance Notes

- All reflection operations are compile-time with zero runtime cost
- Small buffer optimizations avoid heap allocations for common cases
- Platform detection enables optimal code paths automatically
- Bitfield operations compile to efficient bit manipulation instructions

## 🤝 Contributing

Contributions are welcome! Please ensure:

- Code follows the existing style (snake_case, header-only)
- New features include appropriate tests
- Compiler compatibility is maintained
- Documentation is updated

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Special thanks to the C++ community for pushing the boundaries of what's possible with modern C++.

---

<div align="center">
Made with ❤️ by the ARC contributors
</div>
