#pragma once

#include <cassert>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace arc
{
    template<typename E>
    struct err_t
    {
        E value;

        constexpr explicit err_t(const E& e) : value(e) {}
        constexpr explicit err_t(E&& e) : value(std::move(e)) {}

        template<typename... Args>
        constexpr explicit err_t(std::in_place_t, Args&&... args) : value(std::forward<Args>(args)...) {}
    };

    template<typename E>
    constexpr err_t<std::decay_t<E>> err(E&& e)
    {
        return err_t<std::decay_t<E>>(std::forward<E>(e));
    }

    struct ok_tag {};
    inline constexpr ok_tag ok{};

    class bad_result_access : public std::runtime_error
    {
    public:
        explicit bad_result_access(const std::string& what) : std::runtime_error(what) {}
    };

    template<typename T, typename E>
    class result
    {
        static_assert(!std::is_reference_v<T>, "T cannot be a reference type");
        static_assert(!std::is_reference_v<E>, "E cannot be a reference type");

    public:
        using value_type = T;
        using error_type = E;

        constexpr result() noexcept(std::is_nothrow_default_constructible_v<T>) : m_has_value(true)
        {
            construct_value();
        }

        constexpr result(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) : m_has_value(true)
        {
            construct_value(value);
        }

        constexpr result(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) : m_has_value(true)
        {
            construct_value(std::move(value));
        }

        constexpr result(const err_t<E>& err) noexcept(std::is_nothrow_copy_constructible_v<E>) : m_has_value(false)
        {
            construct_error(err.value);
        }

        constexpr result(err_t<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>) : m_has_value(false)
        {
            construct_error(std::move(err.value));
        }

        constexpr result(const result& other) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<E>) : m_has_value(other.m_has_value)
        {
            if (m_has_value)
            {
                construct_value(other.value());
            }
            else
            {
                construct_error(other.error());
            }
        }

        constexpr result(result&& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>) : m_has_value(other.m_has_value)
        {
            if (m_has_value)
            {
                construct_value(std::move(other).value());
            }
            else
            {
                construct_error(std::move(other).error());
            }
        }

        ~result()
        {
            destroy();
        }

        constexpr result& operator=(const result& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_assignable_v<E>)
        {
            if (this != &other)
            {
                if (m_has_value && other.m_has_value)
                {
                    value() = other.value();
                }
                else if (!m_has_value && !other.m_has_value)
                {
                    error() = other.error();
                }
                else
                {
                    destroy();
                    m_has_value = other.m_has_value;
                    if (m_has_value)
                    {
                        construct_value(other.value());
                    }
                    else
                    {
                        construct_error(other.error());
                    }
                }
            }
            return *this;
        }

        constexpr result& operator=(result&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_assignable_v<E>)
        {
            if (this != &other)
            {
                if (m_has_value && other.m_has_value)
                {
                    value() = std::move(other).value();
                }
                else if (!m_has_value && !other.m_has_value)
                {
                    error() = std::move(other).error();
                }
                else
                {
                    destroy();
                    m_has_value = other.m_has_value;
                    if (m_has_value)
                    {
                        construct_value(std::move(other).value());
                    }
                    else
                    {
                        construct_error(std::move(other).error());
                    }
                }
            }
            return *this;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return m_has_value; }
        [[nodiscard]] constexpr bool is_ok() const noexcept { return m_has_value; }
        [[nodiscard]] constexpr bool is_err() const noexcept { return !m_has_value; }
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return m_has_value; }

        constexpr T& value() &
        {
            assert(m_has_value);
            return *val_ptr();
        }

        constexpr const T& value() const&
        {
            assert(m_has_value);
            return *val_ptr();
        }

        constexpr T&& value() &&
        {
            assert(m_has_value);
            return std::move(*val_ptr());
        }

        constexpr const T&& value() const&&
        {
            assert(m_has_value);
            return std::move(*val_ptr());
        }

        constexpr E& error() &
        {
            assert(!m_has_value);
            return *err_ptr();
        }

        constexpr const E& error() const&
        {
            assert(!m_has_value);
            return *err_ptr();
        }

        constexpr E&& error() &&
        {
            assert(!m_has_value);
            return std::move(*err_ptr());
        }

        constexpr const E&& error() const&&
        {
            assert(!m_has_value);
            return std::move(*err_ptr());
        }

        [[nodiscard]] T& unwrap() &
        {
            if (!m_has_value)
                throw bad_result_access("unwrap() called on result with error");
            return *val_ptr();
        }

        [[nodiscard]] const T& unwrap() const&
        {
            if (!m_has_value)
                throw bad_result_access("unwrap() called on result with error");
            return *val_ptr();
        }

        [[nodiscard]] T&& unwrap() &&
        {
            if (!m_has_value)
                throw bad_result_access("unwrap() called on result with error");
            return std::move(*val_ptr());
        }

        [[nodiscard]] const T&& unwrap() const&&
        {
            if (!m_has_value)
                throw bad_result_access("unwrap() called on result with error");
            return std::move(*val_ptr());
        }

        [[nodiscard]] T& expect(const std::string& msg) &
        {
            if (!m_has_value)
                throw bad_result_access(msg);
            return *val_ptr();
        }

        [[nodiscard]] const T& expect(const std::string& msg) const&
        {
            if (!m_has_value)
                throw bad_result_access(msg);
            return *val_ptr();
        }

        [[nodiscard]] T&& expect(const std::string& msg) &&
        {
            if (!m_has_value)
                throw bad_result_access(msg);
            return std::move(*val_ptr());
        }

        [[nodiscard]] const T&& expect(const std::string& msg) const&&
        {
            if (!m_has_value)
                throw bad_result_access(msg);
            return std::move(*val_ptr());
        }

        constexpr T& operator*() &
        {
            assert(m_has_value);
            return value();
        }

        constexpr const T& operator*() const&
        {
            assert(m_has_value);
            return value();
        }

        constexpr T&& operator*() &&
        {
            assert(m_has_value);
            return std::move(*this).value();
        }

        constexpr const T&& operator*() const&&
        {
            assert(m_has_value);
            return std::move(*this).value();
        }

        constexpr T* operator->() noexcept
        {
            assert(m_has_value);
            return val_ptr();
        }

        constexpr const T* operator->() const noexcept
        {
            assert(m_has_value);
            return val_ptr();
        }

        template<typename U>
        [[nodiscard]] constexpr T value_or(U&& default_value) const&
        {
            return m_has_value ? value() : static_cast<T>(std::forward<U>(default_value));
        }

        template<typename U>
        [[nodiscard]] constexpr T value_or(U&& default_value) &&
        {
            return m_has_value ? std::move(*this).value() : static_cast<T>(std::forward<U>(default_value));
        }

        template<typename... Args>
        constexpr void emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        {
            destroy();
            construct_value(std::forward<Args>(args)...);
            m_has_value = true;
        }

        template<typename... Args>
        constexpr void emplace_error(Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        {
            destroy();
            construct_error(std::forward<Args>(args)...);
            m_has_value = false;
        }

    private:
        template<typename... Args>
        constexpr void construct_value(Args&&... args)
        {
            std::construct_at(val_ptr(), std::forward<Args>(args)...);
        }

        template<typename... Args>
        constexpr void construct_error(Args&&... args)
        {
            std::construct_at(err_ptr(), std::forward<Args>(args)...);
        }

        constexpr void destroy()
        {
            if (m_has_value)
            {
                std::destroy_at(val_ptr());
            }
            else
            {
                std::destroy_at(err_ptr());
            }
        }

        T* val_ptr() noexcept
        {
            return std::launder(reinterpret_cast<T*>(&m_storage));
        }

        const T* val_ptr() const noexcept
        {
            return std::launder(reinterpret_cast<const T*>(&m_storage));
        }

        E* err_ptr() noexcept
        {
            return std::launder(reinterpret_cast<E*>(&m_storage));
        }

        const E* err_ptr() const noexcept
        {
            return std::launder(reinterpret_cast<const E*>(&m_storage));
        }

        static constexpr size_t storage_size = std::max(sizeof(T), sizeof(E));
        static constexpr size_t storage_align = std::max(alignof(T), alignof(E));
        alignas(storage_align) std::byte m_storage[storage_size];
        bool m_has_value;
    };

    template<typename E>
    class result<void, E>
    {
        static_assert(!std::is_reference_v<E>, "E cannot be a reference type");

    public:
        using value_type = void;
        using error_type = E;

        constexpr result() noexcept : m_has_value(true) {}

        constexpr result(ok_tag) noexcept : m_has_value(true) {}

        constexpr result(const err_t<E>& err) noexcept(std::is_nothrow_copy_constructible_v<E>) : m_has_value(false)
        {
            construct_error(err.value);
        }

        constexpr result(err_t<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>) : m_has_value(false)
        {
            construct_error(std::move(err.value));
        }

        constexpr result(const result& other) noexcept(std::is_nothrow_copy_constructible_v<E>) : m_has_value(other.m_has_value)
        {
            if (!m_has_value)
                construct_error(other.error());
        }

        constexpr result(result&& other) noexcept(std::is_nothrow_move_constructible_v<E>) : m_has_value(other.m_has_value)
        {
            if (!m_has_value)
                construct_error(std::move(other).error());
        }

        ~result()
        {
            if (!m_has_value)
                destroy();
        }

        constexpr result& operator=(const result& other) noexcept(std::is_nothrow_copy_assignable_v<E>)
        {
            if (this != &other)
            {
                if (!m_has_value && !other.m_has_value)
                {
                    error() = other.error();
                }
                else if (m_has_value && !other.m_has_value)
                {
                    construct_error(other.error());
                    m_has_value = false;
                }
                else if (!m_has_value && other.m_has_value)
                {
                    destroy();
                    m_has_value = true;
                }
            }
            return *this;
        }

        constexpr result& operator=(result&& other) noexcept(std::is_nothrow_move_assignable_v<E>)
        {
            if (this != &other)
            {
                if (!m_has_value && !other.m_has_value)
                {
                    error() = std::move(other).error();
                }
                else if (m_has_value && !other.m_has_value)
                {
                    construct_error(std::move(other).error());
                    m_has_value = false;
                }
                else if (!m_has_value && other.m_has_value)
                {
                    destroy();
                    m_has_value = true;
                }
            }
            return *this;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return m_has_value; }
        [[nodiscard]] constexpr bool is_ok() const noexcept { return m_has_value; }
        [[nodiscard]] constexpr bool is_err() const noexcept { return !m_has_value; }
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return m_has_value; }

        constexpr void value() const& { assert(m_has_value); }
        constexpr void value() && { assert(m_has_value); }

        constexpr E& error() & 
        { 
            assert(!m_has_value); 
            return *err_ptr();
        }

        constexpr const E& error() const& 
        { 
            assert(!m_has_value); 
            return *err_ptr();
        }

        constexpr E&& error() && 
        { 
            assert(!m_has_value); 
            return std::move(*err_ptr());
        }

        constexpr const E&& error() const&& 
        { 
            assert(!m_has_value); 
            return std::move(*err_ptr());
        }

        void unwrap() const
        {
            if (!m_has_value)
                throw bad_result_access("unwrap() called on result with error");
        }

        void expect(const std::string& msg) const
        {
            if (!m_has_value)
                throw bad_result_access(msg);
        }

        constexpr void operator*() const noexcept
        {
            assert(m_has_value);
        }

        template<typename... Args>
        constexpr void emplace_error(Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        {
            if (!m_has_value)
                destroy();
            construct_error(std::forward<Args>(args)...);
            m_has_value = false;
        }

    private:
        template<typename... Args>
        constexpr void construct_error(Args&&... args)
        {
            ::new (static_cast<void*>(&m_storage)) E(std::forward<Args>(args)...);
        }

        constexpr void destroy()
        {
            err_ptr()->~E();
        }

        alignas(E) std::byte m_storage[sizeof(E)];
        bool m_has_value;

        E* err_ptr() noexcept { return std::launder(reinterpret_cast<E*>(&m_storage)); }
        const E* err_ptr() const noexcept { return std::launder(reinterpret_cast<const E*>(&m_storage)); }
    };

    template<typename T>
    result(T) -> result<T, std::exception>;

    template<typename E>
    result(err_t<E>) -> result<void, E>;

    template<typename T, typename E>
    [[nodiscard]] constexpr bool operator==(const result<T, E>& lhs, const result<T, E>& rhs)
    {
        if (lhs.has_value() != rhs.has_value()) return false;
        if (lhs.has_value())
        {
            if constexpr (!std::is_void_v<T>)
            {
                return lhs.value() == rhs.value();
            }
            else
            {
                return true;
            }
        }
        return lhs.error() == rhs.error();
    }

    template<typename T, typename E>
    [[nodiscard]] constexpr bool operator!=(const result<T, E>& lhs, const result<T, E>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, typename E, typename U>
    [[nodiscard]] constexpr bool operator==(const result<T, E>& lhs, const U& rhs)
    {
        return lhs.has_value() && lhs.value() == rhs;
    }

    template<typename T, typename E, typename U>
    [[nodiscard]] constexpr bool operator==(const U& lhs, const result<T, E>& rhs)
    {
        return rhs == lhs;
    }

    template<typename T, typename E, typename U>
    [[nodiscard]] constexpr bool operator!=(const result<T, E>& lhs, const U& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, typename E, typename U>
    [[nodiscard]] constexpr bool operator!=(const U& lhs, const result<T, E>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, typename E>
    [[nodiscard]] constexpr bool operator==(const result<T, E>& lhs, const err_t<E>& rhs)
    {
        return !lhs.has_value() && lhs.error() == rhs.value;
    }

    template<typename T, typename E>
    [[nodiscard]] constexpr bool operator==(const err_t<E>& lhs, const result<T, E>& rhs)
    {
        return rhs == lhs;
    }

    template<typename T, typename E>
    [[nodiscard]] constexpr bool operator!=(const result<T, E>& lhs, const err_t<E>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, typename E>
    [[nodiscard]] constexpr bool operator!=(const err_t<E>& lhs, const result<T, E>& rhs)
    {
        return !(lhs == rhs);
    }

} // namespace arc

  // Standard library specializations
namespace std
{
    template<typename T, typename E>
    struct hash<arc::result<T, E>>
    {
        static constexpr size_t hash_mix = 0x9e3779b97f4a7c15ULL;

        size_t operator()(const arc::result<T, E>& r) const noexcept
        {
            if (r.has_value())
            {
                if constexpr (!std::is_void_v<T>)
                {
                    return std::hash<T>{}(r.value());
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return std::hash<E>{}(r.error()) ^ hash_mix;
            }
        }
    };

    template<typename E>
    struct hash<arc::result<void, E>>
    {
        static constexpr size_t hash_mix = 0x9e3779b97f4a7c15ULL;

        size_t operator()(const arc::result<void, E>& r) const noexcept
        {
            if (r.has_value())
            {
                return 0;
            }
            else
            {
                return std::hash<E>{}(r.error()) ^ hash_mix;
            }
        }
    };
} // namespace std
