#pragma once

#include <cstdint>
#include <type_traits>
#include <limits>
#include <utility>
#include "int128.hpp"

namespace arc::meta
{
    // Auto-sizing integer types based on bit count
    template<uint8_t Bits>
    using auto_uint_t = std::conditional_t<Bits <= 8, uint8_t,
        std::conditional_t<Bits <= 16, uint16_t,
        std::conditional_t<Bits <= 32, uint32_t,
        std::conditional_t<Bits <= 64, uint64_t, arc::uint128>>>>;

    template<uint8_t Bits>
    using auto_int_t = std::conditional_t<Bits <= 8, int8_t,
        std::conditional_t<Bits <= 16, int16_t,
        std::conditional_t<Bits <= 32, int32_t,
        std::conditional_t<Bits <= 64, int64_t, arc::int128>>>>;

    // Bit size calculation
    template<typename T>
    struct bit_size : std::integral_constant<size_t, sizeof(T) * 8> {};

    template<>
    struct bit_size<arc::uint128> : std::integral_constant<size_t, 128> {};

    template<>
    struct bit_size<arc::int128> : std::integral_constant<size_t, 128> {};

    template<typename T>
    inline constexpr size_t bit_size_v = bit_size<T>::value;

    // Type detection including arc integer types
    template<typename T>
    struct is_arc_integer : std::bool_constant<
        std::is_integral_v<T> || 
        std::is_same_v<T, arc::uint128> || 
        std::is_same_v<T, arc::int128>> {};

    template<typename T>
    inline constexpr bool is_arc_integer_v = is_arc_integer<T>::value;

    // Type list operations
    template<size_t Index, typename... Types>
    struct type_at_impl;

    template<typename First, typename... Rest>
    struct type_at_impl<0, First, Rest...>
    {
        using type = First;
    };

    template<size_t Index, typename First, typename... Rest>
    struct type_at_impl<Index, First, Rest...>
    {
        using type = typename type_at_impl<Index - 1, Rest...>::type;
    };

    template<size_t Index, typename... Types>
    using type_at = typename type_at_impl<Index, Types...>::type;

    // Find type matching predicate
    template<template<typename> class Predicate, typename... Types>
    struct find_if;

    template<template<typename> class Predicate>
    struct find_if<Predicate>
    {
        using type = void;
        static constexpr bool found = false;
        static constexpr size_t index = 0;
    };

    template<template<typename> class Predicate, typename First, typename... Rest>
    struct find_if<Predicate, First, Rest...>
    {
        static constexpr bool match = Predicate<First>::value;
        using type = std::conditional_t<match, First, typename find_if<Predicate, Rest...>::type>;
        static constexpr bool found = match || find_if<Predicate, Rest...>::found;
        static constexpr size_t index = match ? 0 : 1 + find_if<Predicate, Rest...>::index;
    };

    // Check if type list contains a type
    template<typename T, typename... Types>
    struct contains : std::bool_constant<(std::is_same_v<T, Types> || ...)> {};

    // Check for duplicate types
    template<typename... Types>
    struct has_duplicates;

    template<>
    struct has_duplicates<> : std::false_type {};

    template<typename First, typename... Rest>
    struct has_duplicates<First, Rest...> : std::bool_constant<contains<First, Rest...>::value || has_duplicates<Rest...>::value> {};

    template<typename... Types>
    inline constexpr bool has_duplicates_v = has_duplicates<Types...>::value;

    // Additional type trait utilities
    template<typename T>
    struct always_false : std::false_type {};

    template<typename T>
    inline constexpr bool always_false_v = always_false<T>::value;

    // Function traits
    template<typename T>
    struct function_traits;

    template<typename R, typename... Args>
    struct function_traits<R(Args...)> {
        using return_type = R;
        using argument_types = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
    };

    template<typename R, typename... Args>
    struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> {};

    template<typename R, typename... Args>
    struct function_traits<R(&)(Args...)> : function_traits<R(Args...)> {};

    // Check if all types satisfy a predicate
    template<template<typename> class Predicate, typename... Types>
    struct all_of : std::bool_constant<(Predicate<Types>::value && ...)> {};

    template<template<typename> class Predicate, typename... Types>
    inline constexpr bool all_of_v = all_of<Predicate, Types...>::value;

    // Check if any type satisfies a predicate
    template<template<typename> class Predicate, typename... Types>
    struct any_of : std::bool_constant<(Predicate<Types>::value || ...)> {};

    template<template<typename> class Predicate, typename... Types>
    inline constexpr bool any_of_v = any_of<Predicate, Types...>::value;

    // Count types that satisfy a predicate
    template<template<typename> class Predicate, typename... Types>
    struct count_if : std::integral_constant<size_t, (size_t(Predicate<Types>::value) + ...)> {};

    template<template<typename> class Predicate, typename... Types>
    inline constexpr size_t count_if_v = count_if<Predicate, Types...>::value;
} // namespace arc::meta