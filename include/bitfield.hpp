#pragma once

#include <cassert>
#include <utility>

#include "meta.hpp"

namespace arc
{
    template<typename Tag, uint8_t Bits, typename ValueType = void>
    struct bit_spec
    {
        static_assert(Bits > 0 && Bits <= 64, "Field must have 1-64 bits");

        using tag_type = Tag;
        using specified_type = ValueType;
        static constexpr uint8_t bits = Bits;
        static constexpr uint64_t max_value = (1ULL << Bits) - 1;
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
            static constexpr uint64_t mask = found ? ((1ULL << bits) - 1) << shift : 0;
        };

        template<typename Tag, typename... Fields>
        using field_type_t = typename field_traits_impl<Tag, Fields...>::value_type;

        template<typename... Fields>
        struct field_info
        {
            static constexpr uint8_t total_bits = (Fields::bits + ...);
            static constexpr uint8_t field_count = sizeof...(Fields);
        };
    }

    template<typename... Fields>
    class bitfield
    {
        static_assert(sizeof...(Fields) > 0, "Bitfield must have at least one field");
        static_assert(detail::field_info<Fields...>::total_bits <= 64, "Total bits must not exceed 64");
        static_assert(!meta::has_duplicates_v<typename Fields::tag_type...>, "Duplicate field tags detected");

    public:
        using storage_type = meta::auto_uint_t<detail::field_info<Fields...>::total_bits>;
        static constexpr uint8_t total_bits = detail::field_info<Fields...>::total_bits;
        static constexpr uint8_t field_count = detail::field_info<Fields...>::field_count;

        constexpr bitfield() noexcept : m_value(0) {}
        constexpr explicit bitfield(storage_type value) noexcept : m_value(value) {}

        template<typename Tag>
        constexpr void set(detail::field_type_t<Tag, Fields...> value)
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            auto raw_value = static_cast<storage_type>(meta::unwrap_enum_v(value));
            assert(raw_value <= traits::field::max_value && "Value exceeds field capacity");
            m_value = (m_value & ~traits::mask) | ((raw_value << traits::shift) & traits::mask);
        }

        template<typename Tag>
        [[nodiscard]] constexpr auto get() const noexcept -> detail::field_type_t<Tag, Fields...>
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static_assert(traits::found, "Field with the specified tag type does not exist");

            using return_type = detail::field_type_t<Tag, Fields...>;
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

        template<typename Tag>
        using field_value_t = detail::field_type_t<Tag, Fields...>;

        template<typename Tag>
        struct field_info
        {
            using traits = detail::field_traits_impl<Tag, Fields...>;
            static constexpr uint8_t bits = traits::bits;
            static constexpr uint8_t shift = traits::shift;
            static constexpr uint64_t mask = traits::mask;
            static constexpr uint64_t max_value = (1ULL << bits) - 1;
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
            return ((m_value & traits::mask) >> traits::shift) == traits::field::max_value;
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
        [[nodiscard]] constexpr bitfield operator~() const noexcept { return bitfield(~m_value & ((1ULL << total_bits) - 1)); }

        constexpr bitfield& operator&=(const bitfield& other) noexcept { m_value &= other.m_value; return *this; }
        constexpr bitfield& operator|=(const bitfield& other) noexcept { m_value |= other.m_value; return *this; }
        constexpr bitfield& operator^=(const bitfield& other) noexcept { m_value ^= other.m_value; return *this; }

    private:
        storage_type m_value;
    };
} // namespace arc

// Standard library specializations
namespace std
{
    template<typename... Fields>
    struct hash<arc::bitfield<Fields...>>
    {
        size_t operator()(const arc::bitfield<Fields...>& bf) const noexcept
        {
            return std::hash<typename arc::bitfield<Fields...>::storage_type>{}(bf.raw());
        }
    };
} // namespace std
