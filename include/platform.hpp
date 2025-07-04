#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#if defined(_MSC_VER)
    #define ARC_COMPILER_MSVC 1
    #define ARC_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
    #define ARC_COMPILER_CLANG 1
    #define ARC_COMPILER_VERSION (__clang_major__ * 100 + __clang_minor__)
#elif defined(__GNUC__)
    #define ARC_COMPILER_GCC 1
    #define ARC_COMPILER_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
    #error "Unsupported compiler"
#endif

#if defined(_WIN32)
    #define ARC_PLATFORM_WINDOWS 1
    #if defined(_WIN64)
        #define ARC_PLATFORM_WIN64 1
    #else
        #define ARC_PLATFORM_WIN32 1
    #endif
#elif defined(__linux__)
    #define ARC_PLATFORM_LINUX 1
#elif defined(__unix__)
    #define ARC_PLATFORM_UNIX 1
#else
    #error "Unsupported platform"
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define ARC_ARCH_X64 1
    #define ARC_ARCH_BITS 64
#elif defined(__i386__) || defined(_M_IX86)
    #define ARC_ARCH_X86 1
    #define ARC_ARCH_BITS 32
#else
    #error "Unsupported architecture"
#endif

#if defined(__BYTE_ORDER__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define ARC_LITTLE_ENDIAN 1
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define ARC_BIG_ENDIAN 1
    #endif
#elif defined(ARC_COMPILER_MSVC)
    // Windows is always little-endian
    #define ARC_LITTLE_ENDIAN 1
#else
    #error "Cannot detect endianness"
#endif

#if defined(__cpp_lib_hardware_interference_size)
    #include <new>
    #define ARC_HARDWARE_INTERFERENCE_SIZE 1
#endif

namespace arc
{
    inline constexpr std::size_t constructive_interference_size = 
#if defined(ARC_HARDWARE_INTERFERENCE_SIZE)
        std::hardware_constructive_interference_size;
#else
        64;
#endif

    inline constexpr std::size_t destructive_interference_size = 
#if defined(ARC_HARDWARE_INTERFERENCE_SIZE)
        std::hardware_destructive_interference_size;
#else
        64;
#endif

    inline constexpr std::size_t cache_line_size = destructive_interference_size;

    template<std::size_t Size, std::size_t Alignment = alignof(std::max_align_t)>
    struct alignas(Alignment) aligned_storage
    {
        static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");
        static_assert(Size > 0, "Size must be greater than 0");

        std::byte data[Size];

        template<typename T>
        [[nodiscard]] T* as() noexcept
        {
            static_assert(sizeof(T) <= Size, "Type too large for storage");
            static_assert(alignof(T) <= Alignment, "Type alignment requirement exceeds storage alignment");
            return std::launder(reinterpret_cast<T*>(data));
        }

        template<typename T>
        [[nodiscard]] const T* as() const noexcept
        {
            static_assert(sizeof(T) <= Size, "Type too large for storage");
            static_assert(alignof(T) <= Alignment, "Type alignment requirement exceeds storage alignment");
            return std::launder(reinterpret_cast<const T*>(data));
        }

        [[nodiscard]] void* ptr() noexcept { return data; }
        [[nodiscard]] const void* ptr() const noexcept { return data; }
    };

    template<typename... Types>
    struct variant_storage
    {
        static constexpr std::size_t size = std::max({sizeof(Types)...});
        static constexpr std::size_t alignment = std::max({alignof(Types)...});

        using storage_type = aligned_storage<size, alignment>;
        storage_type storage;

        template<typename T>
        [[nodiscard]] T* as() noexcept
        {
            static_assert((std::is_same_v<T, Types> || ...), "Type not in variant");
            return storage.template as<T>();
        }

        template<typename T>
        [[nodiscard]] const T* as() const noexcept
        {
            static_assert((std::is_same_v<T, Types> || ...), "Type not in variant");
            return storage.template as<T>();
        }
    };

    template<typename T>
    struct alignas(cache_line_size) cache_aligned
    {
        T value;

        constexpr cache_aligned() = default;
        constexpr cache_aligned(const T& v) : value(v) {}
        constexpr cache_aligned(T&& v) : value(std::move(v)) {}

        constexpr operator T&() noexcept { return value; }
        constexpr operator const T&() const noexcept { return value; }
        constexpr T* operator->() noexcept { return &value; }
        constexpr const T* operator->() const noexcept { return &value; }
    };

    template<typename T>
    struct no_false_sharing
    {
        alignas(cache_line_size) T value;
        char padding[cache_line_size - sizeof(T)];
    };

#ifndef ARC_PAGE_SIZE
    #define ARC_PAGE_SIZE 4096
#endif

    inline constexpr std::size_t page_size = ARC_PAGE_SIZE;

    inline constexpr std::size_t large_page_size = 
#if defined(ARC_PLATFORM_WINDOWS)
        2 * 1024 * 1024; // 2MB on Windows
#else
        2 * 1024 * 1024; // 2MB on Linux (typical)
#endif

    namespace simd
    {
#if defined(__SSE__)
        inline constexpr bool has_sse = true;
#else
        inline constexpr bool has_sse = false;
#endif

#if defined(__SSE2__)
        inline constexpr bool has_sse2 = true;
#else
        inline constexpr bool has_sse2 = false;
#endif

#if defined(__SSE3__)
        inline constexpr bool has_sse3 = true;
#else
        inline constexpr bool has_sse3 = false;
#endif

#if defined(__SSSE3__)
        inline constexpr bool has_ssse3 = true;
#else
        inline constexpr bool has_ssse3 = false;
#endif

#if defined(__SSE4_1__)
        inline constexpr bool has_sse4_1 = true;
#else
        inline constexpr bool has_sse4_1 = false;
#endif

#if defined(__SSE4_2__)
        inline constexpr bool has_sse4_2 = true;
#else
        inline constexpr bool has_sse4_2 = false;
#endif

#if defined(__AVX__)
        inline constexpr bool has_avx = true;
#else
        inline constexpr bool has_avx = false;
#endif

#if defined(__AVX2__)
        inline constexpr bool has_avx2 = true;
#else
        inline constexpr bool has_avx2 = false;
#endif

#if defined(__AVX512F__)
        inline constexpr bool has_avx512 = true;
#else
        inline constexpr bool has_avx512 = false;
#endif
    } // namespace simd
} // namespace arc

#define ARC_LIKELY [[likely]]
#define ARC_UNLIKELY [[unlikely]]

#if defined(ARC_COMPILER_MSVC)
    #define ARC_FORCE_INLINE __forceinline
#elif defined(ARC_COMPILER_GCC) || defined(ARC_COMPILER_CLANG)
    #define ARC_FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define ARC_FORCE_INLINE inline
#endif

#if defined(ARC_COMPILER_MSVC)
    #define ARC_NO_INLINE __declspec(noinline)
#elif defined(ARC_COMPILER_GCC) || defined(ARC_COMPILER_CLANG)
    #define ARC_NO_INLINE __attribute__((noinline))
#else
    #define ARC_NO_INLINE
#endif

#if defined(ARC_COMPILER_MSVC)
    #define ARC_RESTRICT __restrict
#elif defined(ARC_COMPILER_GCC) || defined(ARC_COMPILER_CLANG)
    #define ARC_RESTRICT __restrict__
#else
    #define ARC_RESTRICT
#endif

#if defined(ARC_COMPILER_MSVC)
    #define ARC_UNREACHABLE() __assume(false)
#elif defined(ARC_COMPILER_GCC) || defined(ARC_COMPILER_CLANG)
    #define ARC_UNREACHABLE() __builtin_unreachable()
#else
    #define ARC_UNREACHABLE() ((void)0)
#endif

#if defined(ARC_COMPILER_MSVC)
    #define ARC_DEBUG_BREAK() __debugbreak()
#elif defined(ARC_COMPILER_GCC) || defined(ARC_COMPILER_CLANG)
    #if defined(ARC_ARCH_X64) || defined(ARC_ARCH_X86)
        #define ARC_DEBUG_BREAK() __asm__ __volatile__("int3")
    #else
        #define ARC_DEBUG_BREAK() ((void)0)
    #endif
#else
    #define ARC_DEBUG_BREAK() ((void)0)
#endif

#if defined(ARC_COMPILER_MSVC)
    #define ARC_FUNCTION_NAME __FUNCSIG__
#elif defined(ARC_COMPILER_GCC) || defined(ARC_COMPILER_CLANG)
    #define ARC_FUNCTION_NAME __PRETTY_FUNCTION__
#else
    #define ARC_FUNCTION_NAME __func__
#endif
