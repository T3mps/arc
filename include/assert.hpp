#pragma once

#include <cstdio>
#include <cstdlib>
#include <format>
#include <source_location>
#include <string_view>
#include <utility>

#include "platform.hpp"

// Configuration macros
#ifndef ARC_ENABLE_ASSERTS
    #ifdef NDEBUG
        #define ARC_ENABLE_ASSERTS 0
    #else
        #define ARC_ENABLE_ASSERTS 1
    #endif
#endif

namespace arc
{
    namespace detail
    {
        // Assert handler function pointer - can be overridden by user
        inline void (*g_assert_handler)(const char* expr, const char* msg, std::source_location loc) = nullptr;

        // Default assert handler
        [[noreturn]] inline void default_assert_handler(const char* expr, const char* msg, std::source_location loc)
        {
            std::fprintf(stderr, "\n========== ASSERTION FAILED ==========\n");
            std::fprintf(stderr, "Expression: %s\n", expr);
            if (msg && msg[0] != '\0')
            {
                std::fprintf(stderr, "Message: %s\n", msg);
            }
            std::fprintf(stderr, "File: %s\n", loc.file_name());
            std::fprintf(stderr, "Function: %s\n", loc.function_name());
            std::fprintf(stderr, "Line: %u, Column: %u\n", loc.line(), loc.column());
            std::fprintf(stderr, "======================================\n\n");
            std::fflush(stderr);

            // Platform-specific break into debugger
            ARC_DEBUG_BREAK();

            // Abort if debugger didn't catch it
            std::abort();
        }

        // Core assert implementation
        [[noreturn]] ARC_FORCE_INLINE void assert_fail(const char* expr, const char* msg, std::source_location loc = std::source_location::current())
        {
            if (g_assert_handler)
            {
                g_assert_handler(expr, msg, loc);
            }
            else
            {
                default_assert_handler(expr, msg, loc);
            }

            // Ensure we never return
            ARC_UNREACHABLE();
            std::abort();
        }

        // Helper to format messages at compile time when possible
        template<typename... Args>
        ARC_FORCE_INLINE void assert_fail_formatted(const char* expr, std::string_view fmt, std::source_location loc, Args&&... args)
        {
            if constexpr (sizeof...(Args) == 0)
            {
                assert_fail(expr, fmt.data(), loc);
            }
            else
            {
                // Format the message
                auto msg = std::vformat(fmt, std::make_format_args(args...));
                assert_fail(expr, msg.c_str(), loc);
            }
        }
    } // namespace detail

    inline void set_assert_handler(void (*handler)(const char* expr, const char* msg, std::source_location loc))
    {
        arc::detail::g_assert_handler = handler;
    }
} // namespace arc

#define ARC_ASSERT_IMPL_1(cond)                 \
    do {                                        \
        if (!(cond)) [[unlikely]] {             \
            arc::detail::assert_fail(#cond, "", \
            std::source_location::current());   \
        }                                       \
    } while (0)

#define ARC_ASSERT_IMPL_2(cond, fmt, ...)       \
    do {                                        \
        if (!(cond)) [[unlikely]] {             \
            arc::detail::assert_fail_formatted( \
                #cond,                          \
                fmt,                            \
                std::source_location::current() \
                __VA_OPT__(,) __VA_ARGS__);     \
        }                                       \
    } while (0)

#define ARC_VERIFY_IMPL_1(expr)                 \
    do {                                        \
        if (!(expr)) [[unlikely]] {             \
            arc::detail::assert_fail(#expr, "", \
            std::source_location::current());   \
        }                                       \
    } while (0)

#define ARC_VERIFY_IMPL_2(expr, fmt, ...)       \
    do {                                        \
        if (!(expr)) [[unlikely]] {             \
            arc::detail::assert_fail_formatted( \
                #expr,                          \
                fmt,                            \
                std::source_location::current() \
                __VA_OPT__(,) __VA_ARGS__);     \
        } \
    } while (0)

#define ARC_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define ARC_ASSERT_CHOOSER(...) ARC_GET_MACRO(__VA_ARGS__,      \
    ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_2,    \
    ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_2,    \
    ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_2, ARC_ASSERT_IMPL_1)

#define ARC_VERIFY_CHOOSER(...) ARC_GET_MACRO(__VA_ARGS__,      \
    ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_2,    \
    ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_2,    \
    ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_2, ARC_VERIFY_IMPL_1)

#if ARC_ENABLE_ASSERTS

    #define ARC_ASSERT(...) ARC_ASSERT_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
    #define ARC_VERIFY(...) ARC_VERIFY_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
    #define ARC_STATIC_ASSERT(cond, ...) static_assert(cond __VA_OPT__(,) __VA_ARGS__)
#else
    #define ARC_ASSERT(...) ((void)0)
    #define ARC_VERIFY(...) ARC_VERIFY_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
    #define ARC_STATIC_ASSERT(cond, ...) static_assert(cond __VA_OPT__(,) __VA_ARGS__)

#endif // ARC_ENABLE_ASSERTS

// Assume hint for optimizer
#if defined(ARC_COMPILER_MSVC)
    #define ARC_ASSUME(cond) __assume(cond)
#elif defined(ARC_COMPILER_CLANG)
    #define ARC_ASSUME(cond) __builtin_assume(cond)
#elif defined(ARC_COMPILER_GCC) && (__GNUC__ >= 13)
    #define ARC_ASSUME(cond) __attribute__((assume(cond)))
#else
    #define ARC_ASSUME(cond) do { if (!(cond)) { ARC_UNREACHABLE(); } } while(0)
#endif

// Assert and assume - powerful optimization hint
#if ARC_ENABLE_ASSERTS
    #define ARC_ASSERT_ASSUME(cond) \
        do {                        \
            ARC_ASSERT(cond);       \
            ARC_ASSUME(cond);       \
        } while(0)
#else
    #define ARC_ASSERT_ASSUME(cond) ARC_ASSUME(cond)
#endif

#define ARC_ASSERT_NOT_NULL(ptr) ARC_ASSERT((ptr) != nullptr)

#define ARC_ASSERT_IN_RANGE(val, min, max) \
    ARC_ASSERT((val) >= (min) && (val) <= (max), \
        "Value {} is out of range [{}, {}]", (val), (min), (max))

#define ARC_ASSERT_ALIGNED(ptr, alignment) \
    ARC_ASSERT((reinterpret_cast<std::uintptr_t>(ptr) & ((alignment) - 1)) == 0, \
        "Pointer {:p} is not aligned to {} bytes", (void*)(ptr), (alignment))

#define ARC_NOT_IMPLEMENTED() ARC_ASSERT(false, "Not implemented: {}", ARC_FUNCTION_NAME)
