#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "meta.hpp"
#include "reflect.hpp"
#include "result.hpp"

namespace arc
{
    namespace customize
    {
        inline constexpr int default_enum_min = -128;
        inline constexpr int default_enum_max = 128;

        template<typename E>
        struct enum_range
        {
            static_assert(std::is_enum_v<E>, "arc::customize::enum_range requires enum type");

            // Default range values
            static constexpr int min = default_enum_min;
            static constexpr int max = default_enum_max;

            // Optional: mark as flags enum
            static constexpr bool is_flags = false;
        };

    } // namespace customize

      // Enum-specific meta utilities
    namespace meta
    {
        // Enum underlying type extraction
        template<typename T, typename = void>
        struct enum_underlying
        {
            using type = void;
        };

        template<typename T>
        struct enum_underlying<T, std::enable_if_t<std::is_enum_v<T>>>
        {
            using type = std::underlying_type_t<T>;
        };

        template<typename T>
        using enum_underlying_t = typename enum_underlying<T>::type;

        template<typename Enum>
        [[nodiscard]] constexpr auto to_underlying(Enum value) noexcept -> std::enable_if_t<std::is_enum_v<Enum>, std::underlying_type_t<Enum>>
        {
            return static_cast<std::underlying_type_t<Enum>>(value);
        }

        template<typename Enum, typename T>
        [[nodiscard]] constexpr Enum to_enum(T value) noexcept
        {
            static_assert(std::is_enum_v<Enum>, "Target type must be an enum");
            return static_cast<Enum>(value);
        }

        template<typename T>
        struct decay_enum
        {
            using type = std::conditional_t<std::is_enum_v<T>, std::underlying_type_t<T>, T>;
        };

        template<typename T>
        using decay_enum_t = typename decay_enum<T>::type;

        template<typename T>
        [[nodiscard]] constexpr auto decay_enum_v(T value) noexcept
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

        template<typename T>
        struct is_signed_enum : std::bool_constant<std::is_enum_v<T> && std::is_signed_v<std::underlying_type_t<T>>> {};

        template<typename T>
        inline constexpr bool is_signed_enum_v = is_signed_enum<T>::value;

        template<typename T>
        struct is_unsigned_enum : std::bool_constant<std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type_t<T>>> {};

        template<typename T>
        inline constexpr bool is_unsigned_enum_v = is_unsigned_enum<T>::value;

        template<typename T>
        struct is_scoped_enum : std::bool_constant<std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>> {};

        template<typename T>
        inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

        template<typename T>
        struct is_unscoped_enum : std::bool_constant<std::is_enum_v<T> && std::is_convertible_v<T, std::underlying_type_t<T>>> {};

        template<typename T>
        inline constexpr bool is_unscoped_enum_v = is_unscoped_enum<T>::value;

        template<typename Enum, typename T>
        [[nodiscard]] constexpr bool is_valid_enum_value(T value) noexcept
        {
            static_assert(std::is_enum_v<Enum>, "First template parameter must be an enum");
            using underlying = std::underlying_type_t<Enum>;

            if constexpr (std::is_integral_v<T>)
            {
                constexpr auto min = std::numeric_limits<underlying>::min();
                constexpr auto max = std::numeric_limits<underlying>::max();

                if constexpr (std::is_signed_v<T> == std::is_signed_v<underlying>)
                {
                    return value >= min && value <= max;
                }
                else if constexpr (std::is_signed_v<T> && !std::is_signed_v<underlying>)
                {
                    return value >= 0 && static_cast<std::make_unsigned_t<T>>(value) <= max;
                }
                else
                {
                    return value <= static_cast<std::make_unsigned_t<underlying>>(max);
                }
            }
            else
            {
                return false;
            }
        }

        template<typename Enum, typename T>
        [[nodiscard]] constexpr std::pair<Enum, bool> try_to_enum(T value) noexcept
        {
            if (is_valid_enum_value<Enum>(value))
            {
                return {static_cast<Enum>(value), true};
            }
            else
            {
                return {Enum{}, false};
            }
        }

        template<typename Enum>
        [[nodiscard]] constexpr bool has_flag(Enum value, Enum flag) noexcept
        {
            static_assert(std::is_enum_v<Enum>, "Parameters must be enum types");
            using underlying = std::underlying_type_t<Enum>;
            return (static_cast<underlying>(value) & static_cast<underlying>(flag)) == static_cast<underlying>(flag);
        }

        template<typename Enum>
        [[nodiscard]] constexpr Enum set_flag(Enum value, Enum flag) noexcept
        {
            static_assert(std::is_enum_v<Enum>, "Parameters must be enum types");
            using underlying = std::underlying_type_t<Enum>;
            return static_cast<Enum>(static_cast<underlying>(value) | static_cast<underlying>(flag));
        }

        template<typename Enum>
        [[nodiscard]] constexpr Enum clear_flag(Enum value, Enum flag) noexcept
        {
            static_assert(std::is_enum_v<Enum>, "Parameters must be enum types");
            using underlying = std::underlying_type_t<Enum>;
            return static_cast<Enum>(static_cast<underlying>(value) & ~static_cast<underlying>(flag));
        }

        template<typename Enum>
        [[nodiscard]] constexpr Enum toggle_flag(Enum value, Enum flag) noexcept
        {
            static_assert(std::is_enum_v<Enum>, "Parameters must be enum types");
            using underlying = std::underlying_type_t<Enum>;
            return static_cast<Enum>(static_cast<underlying>(value) ^ static_cast<underlying>(flag));
        }

        template<typename T>
        struct enable_enum_bitwise : std::false_type {};

        template<typename T>
        inline constexpr bool enable_enum_bitwise_v = enable_enum_bitwise<T>::value;
    } // namespace meta

    namespace detail
    {

        template<typename E>
        constexpr auto get_enum_range() noexcept
        {
            using U = std::underlying_type_t<E>;

            if constexpr (customize::enum_range<E>::is_flags)
            {
                return std::pair{0, std::min(customize::enum_range<E>::max, static_cast<int>(std::numeric_limits<U>::digits - 1))};
            }
            else
            {
                constexpr auto custom_min = customize::enum_range<E>::min;
                constexpr auto custom_max = customize::enum_range<E>::max;

                constexpr auto type_min = static_cast<int>(std::numeric_limits<U>::min());
                constexpr auto type_max = static_cast<int>(std::numeric_limits<U>::max());

                return std::pair{std::max(custom_min, type_min), std::min(custom_max, type_max)};
            }
        }

        template<typename E, auto V>
        constexpr bool enum_value_has_name() noexcept
        {
            using U = std::underlying_type_t<E>;

            if constexpr (customize::enum_range<E>::is_flags)
            {
                // For flags, V is the bit position
                if constexpr (V >= 0 && V < std::numeric_limits<U>::digits)
                {
                    constexpr U flag_value = U(1) << V;
                    using value_info = reflect::value_info<static_cast<E>(flag_value), E>;
                    return value_info::has_valid_name();
                }
                else
                {
                    return false;
                }
            }
            else
            {
                constexpr auto range = get_enum_range<E>();
                if constexpr (V >= 0 && V < (range.second - range.first + 1))
                {
                    constexpr auto actual_value = static_cast<U>(V + range.first);
                    using value_info = reflect::value_info<static_cast<E>(actual_value), E>;
                    return value_info::has_valid_name();
                }
                else
                {
                    return false;
                }
            }
        }

        template<typename E, int... Values>
        constexpr std::size_t count_valid_values(std::integer_sequence<int, Values...>) noexcept
        {
            return ((enum_value_has_name<E, Values>() ? 1 : 0) + ...);
        }

        template<typename E, typename U, std::size_t N, int... Values>
        constexpr std::array<E, N> generate_values(std::integer_sequence<int, Values...>) noexcept
        {
            std::array<E, N> result{};
            std::size_t index = 0;

            if constexpr (customize::enum_range<E>::is_flags)
            {
                // For flags, Values are bit positions
                ((enum_value_has_name<E, Values>() ? (result[index++] = static_cast<E>(U(1) << Values), true) : false), ...);
            }
            else
            {
                // For regular enums, Values are offsets from range.first
                constexpr auto range = get_enum_range<E>();
                ((enum_value_has_name<E, Values>() ? (result[index++] = static_cast<E>(Values + range.first), true) : false), ...);
            }

            return result;
        }

        template<typename E, std::size_t N, std::size_t... I>
        constexpr std::array<std::string_view, N> generate_names(const std::array<E, N>& values, std::index_sequence<I...>) noexcept
        {
            return {{reflect::value_info<values[I], E>::name()...}};
        }

        template<typename E, typename U = meta::enum_underlying_t<E>>
        constexpr bool is_flags_enum(const auto& values) noexcept
        {
            if constexpr (customize::enum_range<E>::is_flags)
                return true;  // Explicitly marked as flags
        
            if (values.size() == 0)
                return false;

            for (auto value : values)
            {
                auto v = static_cast<U>(value);
                if (v != 0 && (v & (v - 1)) != 0)
                {
                    return false;
                }
            }
            return true;
        }

        // Binary search for enum value
        template<typename E, std::size_t N>
        constexpr std::optional<std::size_t> find_value(const std::array<E, N>& values, E value) noexcept
        {
            // Use linear search for small arrays
            if constexpr (N <= 8)
            {
                for (std::size_t i = 0; i < N; ++i)
                {
                    if (values[i] == value)
                    {
                        return i;
                    }
                }
                return std::nullopt;
            }
            else
            {
                std::size_t left = 0;
                std::size_t right = N;

                while (left < right)
                {
                    std::size_t mid = left + (right - left) / 2;
                    if (values[mid] == value)
                    {
                        return mid;
                    }
                    else if (values[mid] < value)
                    {
                        left = mid + 1;
                    }
                    else
                    {
                        right = mid;
                    }
                }
                return std::nullopt;
            }
        }
    } // namespace detail

    template<typename E>
        requires std::is_enum_v<E>
    struct range_of
    {
    private:
        static constexpr auto range = detail::get_enum_range<E>();
    public:
        static constexpr int min = range.first;
        static constexpr int max = range.second;
    };

    template<typename E>
        requires std::is_enum_v<E>
    struct enum_data
    {
        using enum_type = E;
        using underlying_type = meta::enum_underlying_t<E>;

        static constexpr auto range = detail::get_enum_range<E>();

        static constexpr auto detection_range = []()
        {
            if constexpr (customize::enum_range<E>::is_flags)
            {
                // For flags, create sequence from 0 to max bit position
                return std::make_integer_sequence<int, range.second + 1>{};
            }
            else
            {
                constexpr auto size = range.second - range.first + 1;
                if constexpr (size > 0 && size < 10000)
                {
                    return std::make_integer_sequence<int, size>{};
                }
                else
                {
                    return std::make_integer_sequence<int, 0>{};
                }
            }
            }();

        static constexpr std::size_t count = detail::count_valid_values<E>(detection_range);

        static constexpr auto values = detail::generate_values<E, underlying_type, count>(detection_range);
        static constexpr auto names = detail::generate_names<E>(values, std::make_index_sequence<count>{});

        static constexpr bool is_flags = detail::is_flags_enum<E>(values);

        static constexpr auto type_name = reflect::type_info<E>::name();
    };

    template<typename E>
        requires std::is_enum_v<E>
    class enum_reflect
    {
    public:
        using enum_type = E;
        using underlying_type = meta::enum_underlying_t<E>;
        using data = enum_data<E>;

        static constexpr std::string_view type_name() noexcept
        {
            return data::type_name;
        }

        static constexpr std::size_t count() noexcept
        {
            return data::count;
        }

        static constexpr bool is_flags() noexcept
        {
            return data::is_flags;
        }

        static constexpr auto values() noexcept -> const auto&
        {
            return data::values;
        }

        static constexpr auto names() noexcept -> const auto&
        {
            return data::names;
        }

        static constexpr result<std::string_view, std::errc> to_string(E value) noexcept
        {
            if (auto index = detail::find_value(data::values, value))
                return data::names[*index];
            return err(std::errc::invalid_argument);
        }

        static constexpr result<E, std::errc> from_string(std::string_view name) noexcept
        {
            auto it = std::find(data::names.begin(), data::names.end(), name);
            if (it != data::names.end())
            {
                auto index = std::distance(data::names.begin(), it);
                return data::values[index];
            }
            return err(std::errc::invalid_argument);
        }

        static constexpr bool contains(E value) noexcept
        {
            return detail::find_value(data::values, value).has_value();
        }

        static constexpr bool contains(underlying_type value) noexcept
        {
            return contains(static_cast<E>(value));
        }

        static constexpr std::optional<E> at(std::size_t index) noexcept
        {
            if (index < data::count)
                return data::values[index];
            return std::nullopt;
        }

        static constexpr std::optional<std::size_t> index_of(E value) noexcept
        {
            return detail::find_value(data::values, value);
        }

        static constexpr result<std::array<E, 64>, std::errc> decompose_flags(E value) noexcept 
            requires (data::is_flags) 
        {
            std::array<E, 64> flags{};
            std::size_t count = 0;

            auto bits = meta::to_underlying(value);
            for (auto flag : data::values)
            {
                auto flag_bit = meta::to_underlying(flag);
                if (flag_bit != 0 && (bits & flag_bit) == flag_bit)
                {
                    if (count >= 64)
                    {
                        return err(std::errc::value_too_large);
                    }
                    flags[count++] = flag;
                    bits &= ~flag_bit;
                }
            }

            if (bits != 0)
                return err(std::errc::invalid_argument);
        
            std::array<E, 64> result{};
            std::copy_n(flags.begin(), count, result.begin());
            return result;
        }

        template<typename F>
        static constexpr void for_each(F&& f)
        {
            for (std::size_t i = 0; i < data::count; ++i)
            {
                f(data::values[i], data::names[i]);
            }
        }
    };

    template<typename E>
    concept ReflectableEnum = std::is_enum_v<E> && enum_data<E>::count > 0;

    template<typename E>
    concept FlagsEnum = ReflectableEnum<E> && enum_data<E>::is_flags;

    template<ReflectableEnum E>
    constexpr auto enum_to_string(E value) noexcept
    {
        return enum_reflect<E>::to_string(value);
    }

    template<ReflectableEnum E>
    constexpr auto string_to_enum(std::string_view name) noexcept
    {
        return enum_reflect<E>::from_string(name);
    }

    template<ReflectableEnum E>
    constexpr auto enum_values() noexcept
    {
        return enum_reflect<E>::values();
    }

    template<ReflectableEnum E>
    constexpr auto enum_names() noexcept
    {
        return enum_reflect<E>::names();
    }

    template<ReflectableEnum E>
    constexpr auto enum_count() noexcept
    {
        return enum_reflect<E>::count();
    }

} // namespace arc

template<typename Enum>
[[nodiscard]] constexpr auto operator|(Enum lhs, Enum rhs) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum>
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template<typename Enum>
[[nodiscard]] constexpr auto operator&(Enum lhs, Enum rhs) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum>
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

template<typename Enum>
[[nodiscard]] constexpr auto operator^(Enum lhs, Enum rhs) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum>
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
}

template<typename Enum>
[[nodiscard]] constexpr auto operator~(Enum value) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum>
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(~static_cast<underlying>(value));
}

template<typename Enum>
constexpr auto operator|=(Enum& lhs, Enum rhs) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum&>
{
    lhs = lhs | rhs;
    return lhs;
}

template<typename Enum>
constexpr auto operator&=(Enum& lhs, Enum rhs) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum&>
{
    lhs = lhs & rhs;
    return lhs;
}

template<typename Enum>
constexpr auto operator^=(Enum& lhs, Enum rhs) noexcept -> std::enable_if_t<std::is_enum_v<Enum> && arc::meta::enable_enum_bitwise_v<Enum>, Enum&>
{
    lhs = lhs ^ rhs;
    return lhs;
}

// Macros for easy customization
#define ARC_ENABLE_ENUM_BITWISE(EnumType) \
    template<> \
    struct arc::meta::enable_enum_bitwise<EnumType> : std::true_type {}

#define ARC_ENUM_RANGE(EnumType, MinValue, MaxValue) \
    template<> \
    struct arc::customize::enum_range<EnumType> { \
        static constexpr int min = MinValue; \
        static constexpr int max = MaxValue; \
        static constexpr bool is_flags = false; \
    }

#define ARC_FLAGS_ENUM(EnumType, MaxBits) \
    ARC_ENABLE_ENUM_BITWISE(EnumType); \
    template<> \
    struct arc::customize::enum_range<EnumType> { \
        static constexpr int min = 0; \
        static constexpr int max = MaxBits; \
        static constexpr bool is_flags = true; \
    }
