#pragma once

#include <cstdint>
#include <iosfwd>
#include <limits>
#include <ostream>
#include <type_traits>

#include "platform.hpp"

#if defined(__SIZEOF_INT128__)
    #define ARC_HAS_NATIVE_INT128 1
#endif

#if defined(ARC_COMPILER_MSVC) && defined(ARC_ARCH_X64)
    #include <intrin.h>
#define ARC_HAS_MSVC_INTRINSICS 1
    #pragma intrinsic(_umul128)
    #pragma intrinsic(_udiv128)
#endif

namespace arc
{
    class int128;
    class uint128;

    namespace detail
    {
        struct uint128_storage;
        struct int128_storage;
    }

    class uint128
    {
    public:
        constexpr uint128() noexcept = default;
        constexpr uint128(int v) noexcept;
        constexpr uint128(unsigned int v) noexcept;
        constexpr uint128(long v) noexcept;
        constexpr uint128(unsigned long v) noexcept;
        constexpr uint128(long long v) noexcept;
        constexpr uint128(unsigned long long v) noexcept;
        constexpr explicit uint128(int128 v) noexcept;

        constexpr explicit operator bool() const noexcept;
        constexpr explicit operator char() const noexcept;
        constexpr explicit operator signed char() const noexcept;
        constexpr explicit operator unsigned char() const noexcept;
        constexpr explicit operator char16_t() const noexcept;
        constexpr explicit operator char32_t() const noexcept;
        constexpr explicit operator short() const noexcept;
        constexpr explicit operator unsigned short() const noexcept;
        constexpr explicit operator int() const noexcept;
        constexpr explicit operator unsigned int() const noexcept;
        constexpr explicit operator long() const noexcept;
        constexpr explicit operator unsigned long() const noexcept;
        constexpr explicit operator long long() const noexcept;
        constexpr explicit operator unsigned long long() const noexcept;

        uint128& operator+=(uint128 other) noexcept;
        uint128& operator-=(uint128 other) noexcept;
        uint128& operator*=(uint128 other) noexcept;
        uint128& operator/=(uint128 other) noexcept;
        uint128& operator%=(uint128 other) noexcept;
        uint128& operator&=(uint128 other) noexcept;
        uint128& operator|=(uint128 other) noexcept;
        uint128& operator^=(uint128 other) noexcept;
        uint128& operator<<=(int shift) noexcept;
        uint128& operator>>=(int shift) noexcept;

        uint128& operator++() noexcept;
        uint128& operator--() noexcept;
        uint128 operator++(int) noexcept;
        uint128 operator--(int) noexcept;

        [[nodiscard]] constexpr uint64_t low() const noexcept;
        [[nodiscard]] constexpr uint64_t high() const noexcept;

        [[nodiscard]] static constexpr uint128 from_parts(uint64_t high, uint64_t low) noexcept;

    private:
#ifdef ARC_HAS_NATIVE_INT128
        unsigned __int128 data_ = 0;

        constexpr uint128(unsigned __int128 v) noexcept : data_(v) {}
#else
        uint64_t m_lo = 0;
        uint64_t m_hi = 0;

        constexpr uint128(uint64_t high, uint64_t low) noexcept : m_lo(low), m_hi(high) {}
#endif

        friend class int128;
        friend struct std::hash<uint128>;
        friend struct detail::uint128_storage;
        friend struct detail::int128_storage;
    };

    class int128
    {
    public:
        constexpr int128() noexcept = default;
        constexpr int128(int v) noexcept;
        constexpr int128(unsigned int v) noexcept;
        constexpr int128(long v) noexcept;
        constexpr int128(unsigned long v) noexcept;
        constexpr int128(long long v) noexcept;
        constexpr int128(unsigned long long v) noexcept;
        constexpr explicit int128(uint128 v) noexcept;

        constexpr explicit operator bool() const noexcept;
        constexpr explicit operator char() const noexcept;
        constexpr explicit operator signed char() const noexcept;
        constexpr explicit operator unsigned char() const noexcept;
        constexpr explicit operator char16_t() const noexcept;
        constexpr explicit operator char32_t() const noexcept;
        constexpr explicit operator short() const noexcept;
        constexpr explicit operator unsigned short() const noexcept;
        constexpr explicit operator int() const noexcept;
        constexpr explicit operator unsigned int() const noexcept;
        constexpr explicit operator long() const noexcept;
        constexpr explicit operator unsigned long() const noexcept;
        constexpr explicit operator long long() const noexcept;
        constexpr explicit operator unsigned long long() const noexcept;

        int128& operator+=(int128 other) noexcept;
        int128& operator-=(int128 other) noexcept;
        int128& operator*=(int128 other) noexcept;
        int128& operator/=(int128 other) noexcept;
        int128& operator%=(int128 other) noexcept;
        int128& operator&=(int128 other) noexcept;
        int128& operator|=(int128 other) noexcept;
        int128& operator^=(int128 other) noexcept;
        int128& operator<<=(int amount) noexcept;
        int128& operator>>=(int amount) noexcept;

        int128& operator++() noexcept;
        int128& operator--() noexcept;
        int128 operator++(int) noexcept;
        int128 operator--(int) noexcept;

        [[nodiscard]] constexpr uint64_t low() const noexcept;
        [[nodiscard]] constexpr int64_t high() const noexcept;

        [[nodiscard]] static constexpr int128 from_parts(int64_t high, uint64_t low) noexcept;

    private:
#ifdef ARC_HAS_NATIVE_INT128
        __int128 data_ = 0;

        constexpr int128(__int128 v) noexcept : data_(v) {}
#else
        uint64_t m_lo = 0;
        int64_t m_hi = 0;

        constexpr int128(int64_t high, uint64_t low) noexcept : m_lo(low), m_hi(high) {}
#endif

        friend struct std::hash<int128>;
        friend struct detail::uint128_storage;
        friend struct detail::int128_storage;
    };

    [[nodiscard]] constexpr uint128 operator+(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr uint128 operator-(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] uint128 operator*(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] uint128 operator/(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] uint128 operator%(uint128 lhs, uint128 rhs) noexcept;

    [[nodiscard]] constexpr uint128 operator&(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr uint128 operator|(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr uint128 operator^(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr uint128 operator~(uint128 val) noexcept;
    [[nodiscard]] constexpr uint128 operator<<(uint128 lhs, int amount) noexcept;
    [[nodiscard]] constexpr uint128 operator>>(uint128 lhs, int amount) noexcept;

    [[nodiscard]] constexpr bool operator==(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator!=(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator<(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator>(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator<=(uint128 lhs, uint128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator>=(uint128 lhs, uint128 rhs) noexcept;

    [[nodiscard]] constexpr uint128 operator+(uint128 val) noexcept;
    [[nodiscard]] constexpr uint128 operator-(uint128 val) noexcept;
    [[nodiscard]] constexpr bool operator!(uint128 val) noexcept;

    [[nodiscard]] constexpr int128 operator+(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr int128 operator-(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] int128 operator*(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] int128 operator/(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] int128 operator%(int128 lhs, int128 rhs) noexcept;

    [[nodiscard]] constexpr int128 operator&(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr int128 operator|(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr int128 operator^(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr int128 operator~(int128 val) noexcept;
    [[nodiscard]] constexpr int128 operator<<(int128 lhs, int amount) noexcept;
    [[nodiscard]] constexpr int128 operator>>(int128 lhs, int amount) noexcept;

    [[nodiscard]] constexpr bool operator==(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator!=(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator<(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator>(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator<=(int128 lhs, int128 rhs) noexcept;
    [[nodiscard]] constexpr bool operator>=(int128 lhs, int128 rhs) noexcept;

    [[nodiscard]] constexpr int128 operator+(int128 val) noexcept;
    [[nodiscard]] constexpr int128 operator-(int128 val) noexcept;
    [[nodiscard]] constexpr bool operator!(int128 val) noexcept;

    std::ostream& operator<<(std::ostream& os, uint128 v);
    std::ostream& operator<<(std::ostream& os, int128 v);

    namespace detail
    {
        struct uint128_storage
        {
#ifdef ARC_HAS_NATIVE_INT128
            static constexpr unsigned __int128 get(uint128 v) noexcept { return v.data_; }
            static constexpr uint128 make(unsigned __int128 v) noexcept { return uint128(v); }
#else
            static constexpr uint64_t low(uint128 v) noexcept { return v.m_lo; }
            static constexpr uint64_t high(uint128 v) noexcept { return v.m_hi; }
            static constexpr uint128 make(uint64_t hi, uint64_t lo) noexcept { return uint128(hi, lo); }
#endif
        };

        struct int128_storage
        {
#ifdef ARC_HAS_NATIVE_INT128
            static constexpr __int128 get(int128 v) noexcept { return v.data_; }
            static constexpr int128 make(__int128 v) noexcept { return int128(v); }
#else
            static constexpr uint64_t low(int128 v) noexcept { return v.m_lo; }
            static constexpr int64_t high(int128 v) noexcept { return v.m_hi; }
            static constexpr int128 make(int64_t hi, uint64_t lo) noexcept { return int128(hi, lo); }
#endif
        };

        inline uint128 multiply_portable(uint64_t lhs_high, uint64_t lhs_low, uint64_t rhs_high, uint64_t rhs_low) noexcept
        {
            uint64_t a32 = lhs_low >> 32;
            uint64_t a00 = lhs_low & 0xffffffff;
            uint64_t b32 = rhs_low >> 32;
            uint64_t b00 = rhs_low & 0xffffffff;

            uint128 result = uint128::from_parts(lhs_high * rhs_low + lhs_low * rhs_high + a32 * b32, a00 * b00);

            result += uint128(a32 * b00) << 32;
            result += uint128(a00 * b32) << 32;
            return result;
        }

        inline uint128 divide_portable(uint128 lhs, uint128 rhs) noexcept
        {
            if (rhs == 0 || lhs < rhs)
                return 0;
            
            uint128 quotient = 0;
            uint128 remainder = lhs;

            int shift = 0;
            uint128 temp = rhs;
            while (temp <= remainder && temp.high() < (1ULL << 63))
            {
                temp = temp << 1;
                ++shift;
            }

            for (int i = shift; i >= 0; i--)
            {
                uint128 shifted_divisor = rhs << i;
                if (remainder >= shifted_divisor)
                {
                    remainder = remainder - shifted_divisor;
                    quotient = quotient | (uint128(1) << i);
                }
            }

            return quotient;
        }
    } // namespace detail

    constexpr uint128::uint128(int v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(static_cast<uint64_t>(v)),
        m_hi(v < 0 ? static_cast<uint64_t>(-1) : 0) {}
#endif

    constexpr uint128::uint128(unsigned int v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(v), m_hi(0) {}
#endif

    constexpr uint128::uint128(long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(static_cast<uint64_t>(v)),
        m_hi(v < 0 ? static_cast<uint64_t>(-1) : 0) {}
#endif

    constexpr uint128::uint128(unsigned long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(v), m_hi(0) {}
#endif

    constexpr uint128::uint128(long long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(static_cast<uint64_t>(v)),
        m_hi(v < 0 ? static_cast<uint64_t>(-1) : 0) {}
#endif

    constexpr uint128::uint128(unsigned long long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(v), m_hi(0) {}
#endif

    constexpr uint128::uint128(int128 v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(static_cast<unsigned __int128>(detail::int128_storage::get(v))) {}
#else
        : m_lo(v.low()), m_hi(static_cast<uint64_t>(v.high())) {}
#endif

    constexpr int128::int128(int v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(static_cast<uint64_t>(v)), m_hi(v < 0 ? -1 : 0) {}
#endif

    constexpr int128::int128(unsigned int v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(v), m_hi(0) {}
#endif

    constexpr int128::int128(long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(static_cast<uint64_t>(v)), m_hi(v < 0 ? -1 : 0) {}
#endif

    constexpr int128::int128(unsigned long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(v), m_hi(0) {}
#endif

    constexpr int128::int128(long long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(static_cast<uint64_t>(v)), m_hi(v < 0 ? -1 : 0) {}
#endif

    constexpr int128::int128(unsigned long long v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(v) {}
#else
        : m_lo(v), m_hi(0) {}
#endif

    constexpr int128::int128(uint128 v) noexcept
#ifdef ARC_HAS_NATIVE_INT128
        : data_(static_cast<__int128>(detail::uint128_storage::get(v))) {}
#else
        : m_lo(v.low()), m_hi(static_cast<int64_t>(v.high())) {}
#endif

    constexpr uint64_t uint128::low() const noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return static_cast<uint64_t>(data_);
#else
        return m_lo;
#endif
    }

    constexpr uint64_t uint128::high() const noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return static_cast<uint64_t>(data_ >> 64);
#else
        return m_hi;
#endif
    }

    constexpr uint64_t int128::low() const noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return static_cast<uint64_t>(data_);
#else
        return m_lo;
#endif
    }

    constexpr int64_t int128::high() const noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return static_cast<int64_t>(data_ >> 64);
#else
        return m_hi;
#endif
    }

    constexpr uint128 uint128::from_parts(uint64_t high, uint64_t low) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return uint128((static_cast<unsigned __int128>(high) << 64) | low);
#else
        return uint128(high, low);
#endif
    }

    constexpr int128 int128::from_parts(int64_t high, uint64_t low) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return int128((static_cast<__int128>(high) << 64) | low);
#else
        return int128(high, low);
#endif
    }

    constexpr uint128::operator bool() const noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return data_ != 0;
#else
        return m_lo || m_hi;
#endif
    }

    constexpr uint128::operator char() const noexcept { return static_cast<char>(low()); }
    constexpr uint128::operator signed char() const noexcept { return static_cast<signed char>(low()); }
    constexpr uint128::operator unsigned char() const noexcept { return static_cast<unsigned char>(low()); }
    constexpr uint128::operator char16_t() const noexcept { return static_cast<char16_t>(low()); }
    constexpr uint128::operator char32_t() const noexcept { return static_cast<char32_t>(low()); }
    constexpr uint128::operator short() const noexcept { return static_cast<short>(low()); }
    constexpr uint128::operator unsigned short() const noexcept { return static_cast<unsigned short>(low()); }
    constexpr uint128::operator int() const noexcept { return static_cast<int>(low()); }
    constexpr uint128::operator unsigned int() const noexcept { return static_cast<unsigned int>(low()); }
    constexpr uint128::operator long() const noexcept { return static_cast<long>(low()); }
    constexpr uint128::operator unsigned long() const noexcept { return static_cast<unsigned long>(low()); }
    constexpr uint128::operator long long() const noexcept { return static_cast<long long>(low()); }
    constexpr uint128::operator unsigned long long() const noexcept { return static_cast<unsigned long long>(low()); }

    constexpr int128::operator bool() const noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return data_ != 0;
#else
        return m_lo || m_hi;
#endif
    }

    constexpr int128::operator char() const noexcept { return static_cast<char>(low()); }
    constexpr int128::operator signed char() const noexcept { return static_cast<signed char>(low()); }
    constexpr int128::operator unsigned char() const noexcept { return static_cast<unsigned char>(low()); }
    constexpr int128::operator char16_t() const noexcept { return static_cast<char16_t>(low()); }
    constexpr int128::operator char32_t() const noexcept { return static_cast<char32_t>(low()); }
    constexpr int128::operator short() const noexcept { return static_cast<short>(low()); }
    constexpr int128::operator unsigned short() const noexcept { return static_cast<unsigned short>(low()); }
    constexpr int128::operator int() const noexcept { return static_cast<int>(low()); }
    constexpr int128::operator unsigned int() const noexcept { return static_cast<unsigned int>(low()); }
    constexpr int128::operator long() const noexcept { return static_cast<long>(low()); }
    constexpr int128::operator unsigned long() const noexcept { return static_cast<unsigned long>(low()); }
    constexpr int128::operator long long() const noexcept { return static_cast<long long>(low()); }
    constexpr int128::operator unsigned long long() const noexcept { return static_cast<unsigned long long>(low()); }

    constexpr uint128 operator+(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) + detail::uint128_storage::get(rhs));
#else
        uint128 result = uint128::from_parts(lhs.high() + rhs.high(), lhs.low() + rhs.low());
        if (result.low() < lhs.low())
            return uint128::from_parts(result.high() + 1, result.low());
        return result;
#endif
    }

    constexpr uint128 operator-(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) - detail::uint128_storage::get(rhs));
#else
        uint128 result = uint128::from_parts(lhs.high() - rhs.high(), lhs.low() - rhs.low());
        if (lhs.low() < rhs.low())
            return uint128::from_parts(result.high() - 1, result.low());
        return result;
#endif
    }

    inline uint128 operator*(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) * detail::uint128_storage::get(rhs));
#elif defined(ARC_HAS_MSVC_INTRINSICS)
        uint64_t high;
        uint64_t low = _umul128(lhs.low(), rhs.low(), &high);
        return uint128::from_parts(high + lhs.low() * rhs.high() + lhs.high() * rhs.low(), low);
#else
        return detail::multiply_portable(lhs.high(), lhs.low(), rhs.high(), rhs.low());
#endif
    }

    inline uint128 operator/(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        if (detail::uint128_storage::get(rhs) == 0) return 0;
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) / detail::uint128_storage::get(rhs));
#elif defined(ARC_HAS_MSVC_INTRINSICS) && 0  // _udiv128 is tricky to use correctly
        // MSVC optimization path would go here, but _udiv128 requires
        // the dividend to be less than divisor * 2^64, making it complex
        return detail::divide_portable(lhs, rhs);
#else
        return detail::divide_portable(lhs, rhs);
#endif
    }

    inline uint128 operator%(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        if (detail::uint128_storage::get(rhs) == 0) return 0;
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) % detail::uint128_storage::get(rhs));
#else
        if (rhs == 0) return 0;
        return lhs - (lhs / rhs) * rhs;
#endif
    }

    constexpr uint128 operator&(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) & detail::uint128_storage::get(rhs));
#else
        return uint128::from_parts(lhs.high() & rhs.high(), lhs.low() & rhs.low());
#endif
    }

    constexpr uint128 operator|(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) | detail::uint128_storage::get(rhs));
#else
        return uint128::from_parts(lhs.high() | rhs.high(), lhs.low() | rhs.low());
#endif
    }

    constexpr uint128 operator^(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) ^ detail::uint128_storage::get(rhs));
#else
        return uint128::from_parts(lhs.high() ^ rhs.high(), lhs.low() ^ rhs.low());
#endif
    }

    constexpr uint128 operator~(uint128 val) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(~detail::uint128_storage::get(val));
#else
        return uint128::from_parts(~val.high(), ~val.low());
#endif
    }

    constexpr uint128 operator<<(uint128 lhs, int amount) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) << amount);
#else
        if (amount == 0) return lhs;
        if (amount >= 128) return 0;
        if (amount >= 64) return uint128::from_parts(lhs.low() << (amount - 64), 0);
        return uint128::from_parts((lhs.high() << amount) | (lhs.low() >> (64 - amount)), lhs.low() << amount);
#endif
    }

    constexpr uint128 operator>>(uint128 lhs, int amount) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(detail::uint128_storage::get(lhs) >> amount);
#else
        if (amount == 0) return lhs;
        if (amount >= 128) return 0;
        if (amount >= 64) return uint128::from_parts(0, lhs.high() >> (amount - 64));
        return uint128::from_parts(lhs.high() >> amount, (lhs.low() >> amount) | (lhs.high() << (64 - amount)));
#endif
    }

    constexpr bool operator==(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::get(lhs) == detail::uint128_storage::get(rhs);
#else
        return lhs.low() == rhs.low() && lhs.high() == rhs.high();
#endif
    }

    constexpr bool operator!=(uint128 lhs, uint128 rhs) noexcept
    {
        return !(lhs == rhs);
    }

    constexpr bool operator<(uint128 lhs, uint128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::get(lhs) < detail::uint128_storage::get(rhs);
#else
        return (lhs.high() == rhs.high()) ? (lhs.low() < rhs.low()) : (lhs.high() < rhs.high());
#endif
    }

    constexpr bool operator>(uint128 lhs, uint128 rhs) noexcept
    {
        return rhs < lhs;
    }

    constexpr bool operator<=(uint128 lhs, uint128 rhs) noexcept
    {
        return !(rhs < lhs);
    }

    constexpr bool operator>=(uint128 lhs, uint128 rhs) noexcept
    {
        return !(lhs < rhs);
    }

    constexpr uint128 operator+(uint128 val) noexcept
    {
        return val;
    }

    constexpr uint128 operator-(uint128 val) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::uint128_storage::make(-detail::uint128_storage::get(val));
#else
        return uint128::from_parts(~val.high() + (val.low() == 0 ? 1 : 0), ~val.low() + 1);
#endif
    }

    constexpr bool operator!(uint128 val) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return !detail::uint128_storage::get(val);
#else
        return !val.high() && !val.low();
#endif
    }

    constexpr int128 operator+(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) + detail::int128_storage::get(rhs));
#else
        return int128(uint128(lhs) + uint128(rhs));
#endif
    }

    constexpr int128 operator-(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) - detail::int128_storage::get(rhs));
#else
        return int128(uint128(lhs) - uint128(rhs));
#endif
    }

    inline int128 operator*(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) * detail::int128_storage::get(rhs));
#else
        return int128(uint128(lhs) * uint128(rhs));
#endif
    }

    inline int128 operator/(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        if (detail::int128_storage::get(rhs) == 0) return 0;
        return detail::int128_storage::make(detail::int128_storage::get(lhs) / detail::int128_storage::get(rhs));
#else
        bool negative_result = (lhs.high() < 0) != (rhs.high() < 0);
        uint128 abs_dividend = lhs.high() < 0 ? -uint128(lhs) : uint128(lhs);
        uint128 abs_divisor = rhs.high() < 0 ? -uint128(rhs) : uint128(rhs);

        uint128 result = abs_dividend / abs_divisor;
        return negative_result ? -int128(result) : int128(result);
#endif
    }

    inline int128 operator%(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        if (detail::int128_storage::get(rhs) == 0) return 0;
        return detail::int128_storage::make(detail::int128_storage::get(lhs) % detail::int128_storage::get(rhs));
#else
        bool negative_result = lhs.high() < 0;
        uint128 abs_dividend = lhs.high() < 0 ? -uint128(lhs) : uint128(lhs);
        uint128 abs_divisor = rhs.high() < 0 ? -uint128(rhs) : uint128(rhs);

        uint128 result = abs_dividend % abs_divisor;
        return negative_result ? -int128(result) : int128(result);
#endif
    }

    constexpr int128 operator&(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) & detail::int128_storage::get(rhs));
#else
        return int128::from_parts(lhs.high() & rhs.high(), lhs.low() & rhs.low());
#endif
    }

    constexpr int128 operator|(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) | detail::int128_storage::get(rhs));
#else
        return int128::from_parts(lhs.high() | rhs.high(), lhs.low() | rhs.low());
#endif
    }

    constexpr int128 operator^(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) ^ detail::int128_storage::get(rhs));
#else
        return int128::from_parts(lhs.high() ^ rhs.high(), lhs.low() ^ rhs.low());
#endif
    }

    constexpr int128 operator~(int128 val) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(~detail::int128_storage::get(val));
#else
        return int128::from_parts(~val.high(), ~val.low());
#endif
    }

    constexpr int128 operator<<(int128 lhs, int amount) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) << amount);
#else
        if (amount == 0) return lhs;
        if (amount >= 128) return 0;
        if (amount >= 64) return int128::from_parts(lhs.low() << (amount - 64), 0);
        return int128::from_parts((lhs.high() << amount) | (lhs.low() >> (64 - amount)), lhs.low() << amount);
#endif
    }

    constexpr int128 operator>>(int128 lhs, int amount) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(detail::int128_storage::get(lhs) >> amount);
#else
        // Arithmetic right shift
        if (amount == 0) return lhs;
        if (amount >= 128) return lhs.high() < 0 ? int128(-1) : int128(0);
        if (amount >= 64)
        {
            return int128::from_parts(lhs.high() < 0 ? -1 : 0, static_cast<uint64_t>(lhs.high() >> (amount - 64)));
        }
        return int128::from_parts(lhs.high() >> amount, (lhs.low() >> amount) | (static_cast<uint64_t>(lhs.high()) << (64 - amount)));
#endif
    }

    constexpr bool operator==(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::get(lhs) == detail::int128_storage::get(rhs);
#else
        return lhs.low() == rhs.low() && lhs.high() == rhs.high();
#endif
    }

    constexpr bool operator!=(int128 lhs, int128 rhs) noexcept
    {
        return !(lhs == rhs);
    }

    constexpr bool operator<(int128 lhs, int128 rhs) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::get(lhs) < detail::int128_storage::get(rhs);
#else
        return (lhs.high() == rhs.high()) ? (lhs.low() < rhs.low()) : (lhs.high() < rhs.high());
#endif
    }

    constexpr bool operator>(int128 lhs, int128 rhs) noexcept
    {
        return rhs < lhs;
    }

    constexpr bool operator<=(int128 lhs, int128 rhs) noexcept
    {
        return !(rhs < lhs);
    }

    constexpr bool operator>=(int128 lhs, int128 rhs) noexcept
    {
        return !(lhs < rhs);
    }

    constexpr int128 operator+(int128 val) noexcept
    {
        return val;
    }

    constexpr int128 operator-(int128 val) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return detail::int128_storage::make(-detail::int128_storage::get(val));
#else
        return int128::from_parts(~val.high() + (val.low() == 0 ? 1 : 0),
            ~val.low() + 1);
#endif
    }

    constexpr bool operator!(int128 val) noexcept
    {
#ifdef ARC_HAS_NATIVE_INT128
        return !detail::int128_storage::get(val);
#else
        return !val.high() && !val.low();
#endif
    }

    inline uint128& uint128::operator+=(uint128 other) noexcept
    {
        *this = *this + other;
        return *this;
    }

    inline uint128& uint128::operator-=(uint128 other) noexcept
    {
        *this = *this - other;
        return *this;
    }

    inline uint128& uint128::operator*=(uint128 other) noexcept
    {
        *this = *this * other;
        return *this;
    }

    inline uint128& uint128::operator/=(uint128 other) noexcept
    {
        *this = *this / other;
        return *this;
    }

    inline uint128& uint128::operator%=(uint128 other) noexcept
    {
        *this = *this % other;
        return *this;
    }

    inline uint128& uint128::operator&=(uint128 other) noexcept
    {
        *this = *this & other;
        return *this;
    }

    inline uint128& uint128::operator|=(uint128 other) noexcept
    {
        *this = *this | other;
        return *this;
    }

    inline uint128& uint128::operator^=(uint128 other) noexcept
    {
        *this = *this ^ other;
        return *this;
    }

    inline uint128& uint128::operator<<=(int shift) noexcept
    {
        *this = *this << shift;
        return *this;
    }

    inline uint128& uint128::operator>>=(int shift) noexcept
    {
        *this = *this >> shift;
        return *this;
    }

    inline uint128& uint128::operator++() noexcept
    {
        *this += 1;
        return *this;
    }

    inline uint128& uint128::operator--() noexcept
    {
        *this -= 1;
        return *this;
    }

    inline uint128 uint128::operator++(int) noexcept
    {
        uint128 tmp(*this);
        ++*this;
        return tmp;
    }

    inline uint128 uint128::operator--(int) noexcept
    {
        uint128 tmp(*this);
        --*this;
        return tmp;
    }

    inline int128& int128::operator+=(int128 other) noexcept
    {
        *this = *this + other;
        return *this;
    }

    inline int128& int128::operator-=(int128 other) noexcept
    {
        *this = *this - other;
        return *this;
    }

    inline int128& int128::operator*=(int128 other) noexcept
    {
        *this = *this * other;
        return *this;
    }

    inline int128& int128::operator/=(int128 other) noexcept
    {
        *this = *this / other;
        return *this;
    }

    inline int128& int128::operator%=(int128 other) noexcept
    {
        *this = *this % other;
        return *this;
    }

    inline int128& int128::operator&=(int128 other) noexcept
    {
        *this = *this & other;
        return *this;
    }

    inline int128& int128::operator|=(int128 other) noexcept
    {
        *this = *this | other;
        return *this;
    }

    inline int128& int128::operator^=(int128 other) noexcept
    {
        *this = *this ^ other;
        return *this;
    }

    inline int128& int128::operator<<=(int amount) noexcept
    {
        *this = *this << amount;
        return *this;
    }

    inline int128& int128::operator>>=(int amount) noexcept
    {
        *this = *this >> amount;
        return *this;
    }

    inline int128& int128::operator++() noexcept
    {
        *this += 1;
        return *this;
    }

    inline int128& int128::operator--() noexcept
    {
        *this -= 1;
        return *this;
    }

    inline int128 int128::operator++(int) noexcept
    {
        int128 tmp(*this);
        ++*this;
        return tmp;
    }

    inline int128 int128::operator--(int) noexcept
    {
        int128 tmp(*this);
        --*this;
        return tmp;
    }

    inline std::ostream& operator<<(std::ostream& os, uint128 v)
    {
        if (v == 0)
            return os << '0';

        char buffer[40];
        int pos = 0;

        while (v != 0)
        {
            uint128 quotient = v / 10;
            uint128 remainder = v - quotient * 10;
            buffer[pos++] = '0' + static_cast<char>(remainder.low());
            v = quotient;
        }

        while (pos > 0)
        {
            os.put(buffer[--pos]);
        }

        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, int128 v)
    {
        if (v.high() < 0)
        {
            os.put('-');
            v = -v;
        }
        return operator<<(os, uint128(v));
    }

} // namespace arc

  // Standard library specializations
namespace std
{
    template<> struct is_integral<arc::int128> : true_type {};
    template<> struct is_integral<arc::uint128> : true_type {};
    template<> struct is_signed<arc::int128> : true_type {};
    template<> struct is_signed<arc::uint128> : false_type {};
    template<> struct is_unsigned<arc::int128> : false_type {};
    template<> struct is_unsigned<arc::uint128> : true_type {};

    template<>
    class numeric_limits<arc::uint128>
    {
    public:
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = false;
        static constexpr bool is_integer = true;
        static constexpr bool is_exact = true;
        static constexpr bool has_infinity = false;
        static constexpr bool has_quiet_NaN = false;
        static constexpr bool has_signaling_NaN = false;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = true;
        static constexpr int digits = 128;
        static constexpr int digits10 = 38;
        static constexpr int max_digits10 = 0;
        static constexpr int radix = 2;
        static constexpr int min_exponent = 0;
        static constexpr int min_exponent10 = 0;
        static constexpr int max_exponent = 0;
        static constexpr int max_exponent10 = 0;
        static constexpr bool traps = numeric_limits<uint64_t>::traps;
        static constexpr bool tinyness_before = false;

        static constexpr arc::uint128 min() noexcept { return 0; }
        static constexpr arc::uint128 lowest() noexcept { return 0; }
        static constexpr arc::uint128 max() noexcept
        {
            return arc::uint128::from_parts((numeric_limits<uint64_t>::max)(), (numeric_limits<uint64_t>::max)());
        }
        static constexpr arc::uint128 epsilon() noexcept { return 0; }
        static constexpr arc::uint128 round_error() noexcept { return 0; }
        static constexpr arc::uint128 infinity() noexcept { return 0; }
        static constexpr arc::uint128 quiet_NaN() noexcept { return 0; }
        static constexpr arc::uint128 signaling_NaN() noexcept { return 0; }
        static constexpr arc::uint128 denorm_min() noexcept { return 0; }
    };

    template<>
    class numeric_limits<arc::int128>
    {
    public:
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = true;
        static constexpr bool is_exact = true;
        static constexpr bool has_infinity = false;
        static constexpr bool has_quiet_NaN = false;
        static constexpr bool has_signaling_NaN = false;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 127;
        static constexpr int digits10 = 38;
        static constexpr int max_digits10 = 0;
        static constexpr int radix = 2;
        static constexpr int min_exponent = 0;
        static constexpr int min_exponent10 = 0;
        static constexpr int max_exponent = 0;
        static constexpr int max_exponent10 = 0;
        static constexpr bool traps = numeric_limits<uint64_t>::traps;
        static constexpr bool tinyness_before = false;

        static constexpr arc::int128 min() noexcept
        {
            return arc::int128::from_parts((numeric_limits<int64_t>::min)(), 0);
        }
        static constexpr arc::int128 lowest() noexcept { return min(); }
        static constexpr arc::int128 max() noexcept
        {
            return arc::int128::from_parts((numeric_limits<int64_t>::max)(), (numeric_limits<uint64_t>::max)());
        }
        static constexpr arc::int128 epsilon() noexcept { return 0; }
        static constexpr arc::int128 round_error() noexcept { return 0; }
        static constexpr arc::int128 infinity() noexcept { return 0; }
        static constexpr arc::int128 quiet_NaN() noexcept { return 0; }
        static constexpr arc::int128 signaling_NaN() noexcept { return 0; }
        static constexpr arc::int128 denorm_min() noexcept { return 0; }
    };

    template<>
    struct hash<arc::uint128>
    {
        size_t operator()(arc::uint128 v) const noexcept
        {
#ifdef ARC_HAS_NATIVE_INT128
            return hash<uint64_t>{}(v.low()) ^ (hash<uint64_t>{}(v.high()) << 1);
#else
            return hash<uint64_t>{}(v.low()) ^ (hash<uint64_t>{}(v.high()) << 1);
#endif
        }
    };

    template<>
    struct hash<arc::int128>
    {
        size_t operator()(arc::int128 v) const noexcept
        {
            return hash<uint64_t>{}(v.low()) ^ (hash<uint64_t>{}(static_cast<uint64_t>(v.high())) << 1);
        }
    };

#ifdef ARC_HAS_NATIVE_INT128
    // Verify our assumptions about native int128
    static_assert(sizeof(arc::uint128) == 16);
    static_assert(alignof(arc::uint128) == 16);
    static_assert(sizeof(arc::int128) == 16);
    static_assert(alignof(arc::int128) == 16);
#else
    static_assert(sizeof(arc::uint128) == 16);
    static_assert(alignof(arc::uint128) == 8);
    static_assert(sizeof(arc::int128) == 16);
    static_assert(alignof(arc::int128) == 8);
#endif
}