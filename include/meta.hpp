#pragma once

#include <cstdint>
#include <type_traits>

namespace arc::meta
{
    template<uint8_t Bits>
    using auto_uint_t = std::conditional_t<Bits <= 8, uint8_t,
        std::conditional_t<Bits <= 16, uint16_t,
            std::conditional_t<Bits <= 32, uint32_t, uint64_t>>>;

    template<uint8_t Bits>
    using auto_int_t = std::conditional_t<Bits <= 8, int8_t,
        std::conditional_t<Bits <= 16, int16_t,
            std::conditional_t<Bits <= 32, int32_t, int64_t>>>;

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

    template<typename T, typename... Types>
    struct contains : std::bool_constant<(std::is_same_v<T, Types> || ...)> {};

    template<typename... Types>
    struct has_duplicates;

    template<>
    struct has_duplicates<> : std::false_type {};

    template<typename First, typename... Rest>
    struct has_duplicates<First, Rest...> : std::bool_constant<contains<First, Rest...>::value || has_duplicates<Rest...>::value> {};

    template<typename... Types>
    inline constexpr bool has_duplicates_v = has_duplicates<Types...>::value;

    template<typename T>
    [[nodiscard]] constexpr auto unwrap_enum_v(T value) noexcept
    {
        if constexpr (std::is_enum_v<T>)
        {
            return static_cast<std::underlying_type_t<T>>(value);
        }
        else
        {
            return value;
        }
    }
} // namespace arc::meta
