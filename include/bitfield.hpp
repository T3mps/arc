#pragma once

#include <cassert>
#include <utility>

#include "enum.hpp"
#include "int128.hpp"
#include "meta.hpp"

namespace arc
{
    template<typename Tag, uint8_t Bits, typename ValueType = void>
    struct bit_spec
    {
        static_assert(Bits > 0 && Bits <= 128, "Field must have 1-128 bits");

        using tag_type = Tag;
        using specified_type = ValueType;
        static constexpr uint8_t bits = Bits;

        static constexpr auto max_value = []()
        {
            if constexpr (Bits == 128)
            {
                return arc::uint128::from_parts(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max());
            }
            else if constexpr (Bits > 64)
            {
                return (arc::uint128(1) << Bits) - 1;
            }
            else if constexpr (Bits == 64)
            {
                return std::numeric_limits<uint64_t>::max();
            }
            else
            {
                return (1ULL << Bits) - 1;
            }
            }();
    };

    namespace detail
    {
        template<typename Tag, typename... Fields>
        struct field_traits_impl
        {
            template<typename Field>
            struct matches : std::bool_constant<std::is_same_v<Tag, typename Field::tag_type>> {};

            using finder = meta::find_if<matches, Fields...>;
            static constexpr bool found = finder::found;
            static constexpr size_t index = finder::index;

            using field = typename finder::type;
            using value_type = std::conditional_t<
                !found, void,
                std::conditional_t<
                std::is_void_v<typename field::specified_type>,
                meta::auto_uint_t<field::bits>,
                typename field::specified_type>>;

            template<size_t... Is>
            static constexpr uint8_t calc_shift(std::index_sequence<Is...>)
            {
                return ((Is > index ? meta::type_at<Is, Fields...>::bits : 0) + ...);
            }

            static constexpr uint8_t bits = found ? field::bits : 0;
            static constexpr uint8_t shift = found ? calc_shift(std::make_index_sequence<sizeof...(Fields)>{}) : 0;

            static constexpr auto mask = []()
            {
                if constexpr (!found)
                {
                    return uint64_t(0);
                }
                else
                {
                    constexpr auto total_bits = (Fields::bits + ...);
                    if constexpr (total_bits > 64)
                    {
                        if constexpr (bits == 128)
                        {
                            return arc::uint128::from_parts(std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max());
                        }
                        else if constexpr (bits >= 64)
                        {
                            return (arc::uint128(1) << bits) - 1;
                        }
                        else
                        {
                            return ((arc::uint128(1) << bits) - 1) << shift;
                        }
                    }
                    else
                    {
                        if constexpr (bits == 64)
                        {
                            return std::numeric_limits<uint64_t>::max();
                        }
                        else
                        {
                            return ((1ULL << bits) - 1) << shift;
                        }
                    }
                }
            }();
        };

        template<typename Tag, typename... Fields>
        using field_type_t = typename field_traits_impl<Tag, Fields...>::value_type;

        template<typename... Fields>
        struct field_info
        {
            static constexpr uint16_t total_bits = (Fields::bits + ...);
            static constexpr uint8_t field_count = sizeof...(Fields);
        };
    }

    template<typename... Fields>
    class bitfield
    {
        static_assert(sizeof...(Fields) > 0, "Bitfield must have at least one field");
        static_assert(detail::field_info<Fields...>::total_bits <= 128, "Total bits must not exceed 128");
        static_assert(!meta::has_duplicates_v<typename Fields::tag_type...>, "Duplicate field tags detected");

    public:
        using storage_type = meta::auto_uint_t<detail::field_info<Fields...>::total_bits>;
        static constexpr uint16_t total_bits = detail::field_info<Fields...>::total_bits;
        static constexpr uint8_t field_count = detail::field_info<Fields...>::field_count;

        constexpr bitfield() noexcept : m_value(0) {}
        constexpr explicit bitfield(storage_type value) noexcept : m_value(value) {}

        template<typename Tag>
        constexpr void set(detail::field_type_t<Tag, Fields...> value)
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            auto raw_value = static_cast<storage_type>(meta::decay_enum_v(value));

            if constexpr (std::is_same_v<storage_type, arc::uint128>)
            {
                assert(raw_value <= traits::field::max_value && "Value exceeds field capacity");
                m_value = (m_value & ~traits::mask) | ((raw_value << traits::shift) & traits::mask);
            }
            else
            {
                assert(static_cast<uint64_t>(raw_value) <= static_cast<uint64_t>(traits::field::max_value) && "Value exceeds field capacity");
                m_value = (m_value & ~traits::mask) | ((raw_value << traits::shift) & traits::mask);
            }
        }

        template<typename Tag>
        [[nodiscard]] constexpr auto get() const noexcept -> detail::field_type_t<Tag, Fields...>
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            using return_type = detail::field_type_t<Tag, Fields...>;

            if constexpr (std::is_same_v<storage_type, arc::uint128>)
            {
                auto raw_value = (m_value & traits::mask) >> traits::shift;
                if constexpr (std::is_enum_v<return_type>)
                {
                    return static_cast<return_type>(static_cast<uint64_t>(raw_value));
                }
                else if constexpr (std::is_same_v<return_type, arc::uint128>)
                {
                    return raw_value;
                }
                else
                {
                    return static_cast<return_type>(raw_value);
                }
            }
            else
            {
                auto raw_value = (m_value & traits::mask) >> traits::shift;
                if constexpr (std::is_enum_v<return_type>)
                {
                    return static_cast<return_type>(raw_value);
                }
                else
                {
                    return static_cast<return_type>(raw_value);
                }
            }
        }

        template<typename Tag>
        constexpr result<void, std::errc> set_from_string(std::string_view name)
            requires (std::is_enum_v<detail::field_type_t<Tag, Fields...>> && 
        ReflectableEnum<detail::field_type_t<Tag, Fields...>>)
        {
            using value_type = detail::field_type_t<Tag, Fields...>;
            auto result = string_to_enum<value_type>(name);
            if (result)
            {
                set<Tag>(result.value());
                return ok;
            }
            return err(result.error());
        }

        template<typename Tag>
        [[nodiscard]] constexpr result<std::string_view, std::errc> get_as_string() const noexcept
            requires (std::is_enum_v<detail::field_type_t<Tag, Fields...>> && 
        ReflectableEnum<detail::field_type_t<Tag, Fields...>>)
        {
            using value_type = detail::field_type_t<Tag, Fields...>;
            auto value = get<Tag>();
            return enum_to_string(value);
        }

        template<typename Tag>
        [[nodiscard]] constexpr bool is_valid_enum_value() const noexcept
            requires (std::is_enum_v<detail::field_type_t<Tag, Fields...>> && 
        ReflectableEnum<detail::field_type_t<Tag, Fields...>>)
        {
            using value_type = detail::field_type_t<Tag, Fields...>;
            auto value = get<Tag>();
            return enum_reflect<value_type>::contains(value);
        }

        template<typename Tag>
        using field_value_t = detail::field_type_t<Tag, Fields...>;

        template<typename Tag>
        struct field_info
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static constexpr uint8_t bits = traits::bits;
            static constexpr uint8_t shift = traits::shift;
            static constexpr auto mask = traits::mask;
            static constexpr auto max_value = traits::field::max_value;
            using type = detail::field_type_t<Tag, Fields...>;
        };

        template<typename Tag>
        [[nodiscard]] static constexpr bool has_field() noexcept
        {
            return detail::field_traits_impl<Tag, Fields...>::found;
        }

        template<typename Tag>
        [[nodiscard]] constexpr bool is_value(detail::field_type_t<Tag, Fields...> value) const noexcept
        {
            return get<Tag>() == value;
        }

        constexpr void clear() noexcept { m_value = 0; }

        template<typename Tag>
        constexpr void clear() noexcept
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");
            m_value &= ~traits::mask;
        }

        template<typename Tag>
        [[nodiscard]] constexpr bool is_max() const noexcept
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            if constexpr (std::is_same_v<storage_type, arc::uint128>)
            {
                return ((m_value & traits::mask) >> traits::shift) == traits::field::max_value;
            }
            else
            {
                return ((m_value & traits::mask) >> traits::shift) == traits::field::max_value;
            }
        }

        template<typename Tag>
        [[nodiscard]] constexpr bool increment() noexcept
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            auto current = (m_value & traits::mask) >> traits::shift;
            if (current == traits::field::max_value)
            {
                m_value &= ~traits::mask;
                return true;
            }
            m_value = (m_value & ~traits::mask) | (((current + 1) << traits::shift) & traits::mask);
            return false;
        }

        template<typename Tag>
        [[nodiscard]] constexpr bool decrement() noexcept
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            auto current = (m_value & traits::mask) >> traits::shift;
            if (current == 0)
            {
                m_value = (m_value & ~traits::mask) | ((traits::field::max_value << traits::shift) & traits::mask);
                return true;
            }
            m_value = (m_value & ~traits::mask) | (((current - 1) << traits::shift) & traits::mask);
            return false;
        }

        [[nodiscard]] constexpr storage_type raw() const noexcept { return m_value; }
        constexpr void set_raw(storage_type value) noexcept { m_value = value; }

        [[nodiscard]] constexpr bool operator==(const bitfield& other) const noexcept { return m_value == other.m_value; }
        [[nodiscard]] constexpr bool operator!=(const bitfield& other) const noexcept { return m_value != other.m_value; }
        [[nodiscard]] constexpr bool operator<(const bitfield& other) const noexcept { return m_value < other.m_value; }
        [[nodiscard]] constexpr bool operator<=(const bitfield& other) const noexcept { return m_value <= other.m_value; }
        [[nodiscard]] constexpr bool operator>(const bitfield& other) const noexcept { return m_value > other.m_value; }
        [[nodiscard]] constexpr bool operator>=(const bitfield& other) const noexcept { return m_value >= other.m_value; }

        [[nodiscard]] constexpr bitfield operator&(const bitfield& other) const noexcept { return bitfield(m_value & other.m_value); }
        [[nodiscard]] constexpr bitfield operator|(const bitfield& other) const noexcept { return bitfield(m_value | other.m_value); }
        [[nodiscard]] constexpr bitfield operator^(const bitfield& other) const noexcept { return bitfield(m_value ^ other.m_value); }
        [[nodiscard]] constexpr bitfield operator~() const noexcept 
        { 
            if constexpr (std::is_same_v<storage_type, arc::uint128>)
            {
                if constexpr (total_bits == 128)
                {
                    return bitfield(~m_value);
                }
                else
                {
                    return bitfield(~m_value & ((arc::uint128(1) << total_bits) - 1));
                }
            }
            else
            {
                return bitfield(~m_value & ((1ULL << total_bits) - 1));
            }
        }

        constexpr bitfield& operator&=(const bitfield& other) noexcept { m_value &= other.m_value; return *this; }
        constexpr bitfield& operator|=(const bitfield& other) noexcept { m_value |= other.m_value; return *this; }
        constexpr bitfield& operator^=(const bitfield& other) noexcept { m_value ^= other.m_value; return *this; }

        template<typename Visitor>
        constexpr void for_each_field(Visitor&& visitor) const
        {
            for_each_field_impl<0>(std::forward<Visitor>(visitor), std::make_index_sequence<sizeof...(Fields)>{});
        }

    private:
        storage_type m_value;

        template<size_t Index, typename Visitor, size_t... Is>
        constexpr void for_each_field_impl(Visitor&& visitor, std::index_sequence<Is...>) const
        {
            ((Is == Index ? visitor_helper<meta::type_at<Is, Fields...>>(visitor) : void()), ...);
        }

        template<typename Field, typename Visitor>
        constexpr void visitor_helper(Visitor&& visitor) const
        {
            using tag = typename Field::tag_type;
            using value_type = detail::field_type_t<tag, Fields...>;
            auto value = get<tag>();

            if constexpr (std::is_enum_v<value_type> && ReflectableEnum<value_type>)
            {
                auto name_result = enum_to_string(value);
                visitor(reflect::type_info<tag>::name(), Field::bits, value, name_result.value_or("?"));
            }
            else
            {
                visitor(reflect::type_info<tag>::name(), Field::bits, value, std::string_view{});
            }
        }
    };

    template<typename E, typename Tag>
        requires ReflectableEnum<E>
    constexpr auto make_enum_bit_spec()
    {
        constexpr auto bits = []()
        {
            if constexpr (FlagsEnum<E>)
            {
                auto max_val = enum_reflect<E>::values().back();
                auto val = meta::to_underlying(max_val);
                uint8_t needed_bits = 0;
                while (val > 0)
                {
                    val >>= 1;
                    ++needed_bits;
                }
                return needed_bits;
            }
            else
            {
                constexpr auto count = enum_reflect<E>::count();
                uint8_t bits = 0;
                auto n = count - 1;
                while (n > 0)
                {
                    n >>= 1;
                    ++bits;
                }
                return bits > 0 ? bits : uint8_t(1);
            }
            }();

        return bit_spec<Tag, bits, E>{};
    }

} // namespace arc

// Standard library specializations
namespace std
{
    template<typename... Fields>
    struct hash<arc::bitfield<Fields...>>
    {
        size_t operator()(const arc::bitfield<Fields...>& bf) const noexcept
        {
            using storage_type = typename arc::bitfield<Fields...>::storage_type;
            if constexpr (std::is_same_v<storage_type, arc::uint128>)
            {
                auto value = bf.raw();
                return std::hash<uint64_t>{}(value.low()) ^ (std::hash<uint64_t>{}(value.high()) << 1);
            }
            else
            {
                return std::hash<storage_type>{}(bf.raw());
            }
        }
    };
} // namespace std
