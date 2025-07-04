#pragma once

#include <string_view>
#include <array>
#include <utility>
#include <algorithm>
#include <cstddef>
#include "meta.hpp"
#include "platform.hpp"

namespace arc::reflect
{
    template<typename T>
    struct reflect_traits
    {
        static constexpr bool is_reflectable = false;
    };

    namespace detail
    {
        constexpr std::string_view trim_whitespace(std::string_view str) noexcept
        {
            auto first = str.find_first_not_of(" \t\n\r");
            if (first == std::string_view::npos)
                return {};
            auto last = str.find_last_not_of(" \t\n\r");
            return str.substr(first, last - first + 1);
        }

        constexpr bool is_identifier_char(char c) noexcept
        {
            return (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   (c >= '0' && c <= '9') ||
                    c == '_';
        }

        constexpr std::string_view extract_identifier_reverse(std::string_view str) noexcept
        {
            if (str.empty())
                return {};

            size_t end = str.size();
            while (end > 0 && !is_identifier_char(str[end - 1]))
            {
                --end;
            }
            if (end == 0)
                return {};

            size_t start = end;
            while (start > 0 && is_identifier_char(str[start - 1]))
            {
                --start;
            }

            return str.substr(start, end - start);
        }

#if defined(ARC_COMPILER_CLANG)
        // Clang: constexpr auto arc::reflect::detail::type_name() [T = TypeName]
        template<typename T>
        constexpr std::string_view type_name() noexcept
        {
            constexpr std::string_view function = ARC_FUNCTION_NAME;
            constexpr auto prefix = std::string_view{"[T = "};
            constexpr auto suffix = std::string_view{"]"};

            constexpr auto start_pos = function.find(prefix);
            if (start_pos == std::string_view::npos)
                return {};

            constexpr auto name_start = start_pos + prefix.size();
            constexpr auto name_end = function.find(suffix, name_start);
            if (name_end == std::string_view::npos)
                return {};

            return function.substr(name_start, name_end - name_start);
        }

        // Clang: constexpr auto arc::reflect::detail::value_name() [V = EnumType::Value]
        template<auto V>
        constexpr std::string_view value_name() noexcept
        {
            constexpr std::string_view function = ARC_FUNCTION_NAME;
            constexpr auto prefix = std::string_view{"[V = "};
            constexpr auto suffix = std::string_view{"]"};

            constexpr auto start_pos = function.find(prefix);
            if (start_pos == std::string_view::npos)
                return {};

            constexpr auto name_start = start_pos + prefix.size();
            constexpr auto name_end = function.find(suffix, name_start);
            if (name_end == std::string_view::npos)
                return {};

            constexpr auto full_name = function.substr(name_start, name_end - name_start);

            constexpr auto last_colon = full_name.rfind("::");
            if (last_colon != std::string_view::npos && last_colon + 2 < full_name.size())
                return full_name.substr(last_colon + 2);
        
            return extract_identifier_reverse(full_name);
        }

#elif defined(ARC_COMPILER_GCC)
        // GCC: constexpr std::string_view arc::reflect::detail::type_name() [with T = TypeName]
        template<typename T>
        constexpr std::string_view type_name() noexcept
        {
            constexpr std::string_view function = ARC_FUNCTION_NAME;
            constexpr auto prefix = std::string_view{"[with T = "};
            constexpr auto suffix = std::string_view{"; std::"};

            constexpr auto start_pos = function.find(prefix);
            if (start_pos == std::string_view::npos)
                return {};

            constexpr auto name_start = start_pos + prefix.size();
            constexpr auto name_end = function.find(suffix, name_start);
            if (name_end == std::string_view::npos)
            {
                constexpr auto alt_suffix = std::string_view{"]"};
                constexpr auto alt_end = function.find(alt_suffix, name_start);
                if (alt_end == std::string_view::npos) return {};
                return function.substr(name_start, alt_end - name_start);
            }

            return function.substr(name_start, name_end - name_start);
        }

        template<auto V>
        constexpr std::string_view value_name() noexcept
        {
            constexpr std::string_view function = ARC_FUNCTION_NAME;
            constexpr auto prefix = std::string_view{"[with auto V = "};
            constexpr auto suffix = std::string_view{"; std::"};

            constexpr auto start_pos = function.find(prefix);
            if (start_pos == std::string_view::npos)
                return {};

            constexpr auto name_start = start_pos + prefix.size();
            constexpr auto name_end = function.find(suffix, name_start);
            if (name_end == std::string_view::npos)
            {
                constexpr auto alt_suffix = std::string_view{"]"};
                constexpr auto alt_end = function.find(alt_suffix, name_start);
                if (alt_end == std::string_view::npos)
                    return {};
                constexpr auto full_name = function.substr(name_start, alt_end - name_start);
                return extract_identifier_reverse(full_name);
            }

            constexpr auto full_name = function.substr(name_start, name_end - name_start);
            return extract_identifier_reverse(full_name);
        }

#elif defined(ARC_COMPILER_MSVC)
        // MSVC: auto __cdecl arc::reflect::detail::type_name<TypeName>(void)
        template<typename T>
        constexpr std::string_view type_name() noexcept
        {
            constexpr std::string_view function = ARC_FUNCTION_NAME;
            constexpr auto prefix = std::string_view{"type_name<"};
            constexpr auto suffix = std::string_view{">(void)"};

            constexpr auto start_pos = function.find(prefix);
            if (start_pos == std::string_view::npos)
                return {};

            constexpr auto name_start = start_pos + prefix.size();
            constexpr auto name_end = function.find(suffix, name_start);
            if (name_end == std::string_view::npos)
                return {};

            return function.substr(name_start, name_end - name_start);
        }

        template<auto V>
        constexpr std::string_view value_name() noexcept
        {
            constexpr std::string_view function = ARC_FUNCTION_NAME;
            constexpr auto prefix = std::string_view{"value_name<"};
            constexpr auto suffix = std::string_view{">(void)"};

            constexpr auto start_pos = function.find(prefix);
            if (start_pos == std::string_view::npos)
                return {};

            constexpr auto name_start = start_pos + prefix.size();
            constexpr auto name_end = function.find(suffix, name_start);
            if (name_end == std::string_view::npos)
                return {};

            constexpr auto full_name = function.substr(name_start, name_end - name_start);
            return extract_identifier_reverse(full_name);
        }
#else
        // Unsupported compiler
        template<typename T>
        constexpr std::string_view type_name() noexcept { return {}; }

        template<auto V>
        constexpr std::string_view value_name() noexcept { return {}; }
#endif

        constexpr bool is_valid_name(std::string_view name) noexcept
        {
            if (name.empty())
                return false;

            if (!((name[0] >= 'a' && name[0] <= 'z') ||
                  (name[0] >= 'A' && name[0] <= 'Z') ||
                   name[0] == '_'))
            {
                return false;
            }

            for (char c : name)
            {
                if (!is_identifier_char(c))
                {
                    return false;
                }
            }

            if (name.find('(') != std::string_view::npos)
                return false;

            return true;
        }
    } // namespace detail

    template<std::size_t N>
    struct static_string
    {
        char data[N + 1]{};

        constexpr static_string() = default;

        constexpr static_string(std::string_view str) noexcept
        {
            for (std::size_t i = 0; i < N && i < str.size(); ++i)
            {
                data[i] = str[i];
            }
        }

        constexpr operator std::string_view() const noexcept
        {
            return {data, N};
        }

        constexpr const char* c_str() const noexcept
        {
            return data;
        }

        constexpr std::size_t length() const noexcept
        {
            return N;
        }
    };

    template<>
    struct static_string<0>
    {
        constexpr static_string() = default;
        constexpr static_string(std::string_view) noexcept {}
        constexpr operator std::string_view() const noexcept { return {}; }
        constexpr const char* c_str() const noexcept { return ""; }
        constexpr std::size_t length() const noexcept { return 0; }
    };

    template<typename T>
    class type_info
    {
    public:
        using type = T;

        static constexpr std::string_view name() noexcept
        {
            return name_storage;
        }

        static constexpr bool is_reflectable() noexcept
        {
            return reflect_traits<T>::is_reflectable;
        }

        static constexpr std::size_t size() noexcept
        {
            return sizeof(T);
        }

        static constexpr std::size_t alignment() noexcept
        {
            return alignof(T);
        }

    private:
        static constexpr auto name_view = detail::type_name<T>();
        static constexpr auto name_storage = static_string<name_view.size()>{name_view};
    };

    template<auto V, typename T = std::decay_t<decltype(V)>>
    class value_info
    {
    public:
        using value_type = T;
        static constexpr T value = V;

        static constexpr std::string_view name() noexcept
        {
            return name_storage;
        }

        static constexpr bool has_valid_name() noexcept
        {
            return detail::is_valid_name(name());
        }

    private:
        static constexpr auto name_view = detail::value_name<V>();
        static constexpr auto name_storage = static_string<name_view.size()>{name_view};
    };

    template<typename T, T Min, T Max>
    struct range
    {
        using value_type = T;
        static constexpr T min = Min;
        static constexpr T max = Max;

        static constexpr bool contains(T value) noexcept
        {
            return value >= min && value <= max;
        }

        static constexpr std::size_t size() noexcept 
            requires std::is_integral_v<T>
        {
            if constexpr (std::is_signed_v<T>)
            {
                return static_cast<std::size_t>(static_cast<std::make_unsigned_t<T>>(max) - static_cast<std::make_unsigned_t<T>>(min) + 1);
            }
            else
            {
                return static_cast<std::size_t>(max - min + 1);
            }
        }

        template<typename F>
        static constexpr void for_each(F&& f) noexcept 
            requires std::is_integral_v<T>
        {
            for (T i = min; i <= max; ++i)
            {
                f(i);
            }
        }
    };
} // namespace arc
