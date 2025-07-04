#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include "assert.hpp"
#include "meta.hpp"
#include "platform.hpp"

namespace arc
{
    template<typename T, std::size_t BufferSize = 64, std::size_t Alignment = alignof(T)>
    class small_buffer;

    template<std::size_t BufferSize = 23>
    class small_string;

    namespace detail
    {
        template<typename T, std::size_t BufferSize, std::size_t Alignment>
        struct can_store_inline : std::bool_constant<
            sizeof(T) <= BufferSize && 
            alignof(T) <= Alignment &&
            std::is_nothrow_move_constructible_v<T>  // Important for exception safety
        > {};

        template<typename T, std::size_t BufferSize, std::size_t Alignment>
        inline constexpr bool can_store_inline_v = can_store_inline<T, BufferSize, Alignment>::value;
    }

    /**
    * @brief Small Buffer Optimization container
    * 
    * Stores objects up to BufferSize inline, otherwise heap allocates.
    */
    template<typename T, std::size_t BufferSize, std::size_t Alignment>
    class small_buffer
    {
        static_assert(BufferSize >= sizeof(void*), "Buffer too small");
        static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");

    public:
        using value_type = T;
        static constexpr std::size_t buffer_size = BufferSize;
        static constexpr std::size_t alignment = Alignment;

        // Constructors
        small_buffer() noexcept : m_is_inline(true) {}

        template<typename U>
        explicit small_buffer(U&& value) noexcept(detail::can_store_inline_v<std::decay_t<U>, BufferSize, Alignment> && std::is_nothrow_constructible_v<std::decay_t<U>, U>)
        {
            construct(std::forward<U>(value));
        }

        small_buffer(const small_buffer& other)
        {
            if (other.m_is_inline)
            {
                other.visit([this](const auto& value)
                {
                    construct(value);
                });
            }
            else
            {
                other.visit([this](const auto& value)
                {
                    construct(value);
                });
            }
        }

        small_buffer(small_buffer&& other) noexcept
        {
            if (other.m_is_inline)
            {
                other.visit([this](auto& value)
                {
                    construct(std::move(value));
                });
            }
            else
            {
                // Steal the pointer
                m_heap_ptr = other.m_heap_ptr;
                m_type_ops = other.m_type_ops;
                m_is_inline = false;
                other.m_heap_ptr = nullptr;
                other.m_is_inline = true;
            }
        }

        ~small_buffer()
        {
            destroy();
        }

        small_buffer& operator=(const small_buffer& other)
        {
            if (this != &other)
            {
                destroy();
                other.visit([this](const auto& value)
                {
                    construct(value);
                });
            }
            return *this;
        }

        small_buffer& operator=(small_buffer&& other) noexcept
        {
            if (this != &other)
            {
                destroy();
                if (other.m_is_inline)
                {
                    other.visit([this](auto& value)
                    {
                        construct(std::move(value));
                    });
                }
                else
                {
                    m_heap_ptr = other.m_heap_ptr;
                    m_type_ops = other.m_type_ops;
                    m_is_inline = false;
                    other.m_heap_ptr = nullptr;
                    other.m_is_inline = true;
                }
            }
            return *this;
        }

        template<typename U>
        void emplace(U&& value)
        {
            destroy();
            construct(std::forward<U>(value));
        }

        void reset() noexcept
        {
            destroy();
            m_is_inline = true;
        }

        [[nodiscard]] bool has_value() const noexcept
        {
            return m_is_inline ? m_type_ops != nullptr : m_heap_ptr != nullptr;
        }

        [[nodiscard]] bool is_inline() const noexcept
        {
            return m_is_inline;
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return has_value();
        }

        // Visitor pattern removed due to type safety issues with type erasure
        // The original implementation had a type confusion bug and proper
        // implementation would require a redesign of the type erasure mechanism

        template<typename U>
        [[nodiscard]] U* get_if() noexcept
        {
            if (!has_value() || m_type_ops != &type_ops_for<U>)
                return nullptr;
            return static_cast<U*>(get_storage());
        }

        template<typename U>
        [[nodiscard]] const U* get_if() const noexcept
        {
            if (!has_value() || m_type_ops != &type_ops_for<U>)
                return nullptr;
            return static_cast<const U*>(get_storage());
        }

    private:
        struct type_ops_base
        {
            void (*destroy)(void*) noexcept;
            void (*copy_construct)(void*, const void*);
            void (*move_construct)(void*, void*) noexcept;
        };

        template<typename U>
        static constexpr type_ops_base type_ops_for =
        {
            // destroy
            [](void* ptr) noexcept
            {
                std::destroy_at(static_cast<U*>(ptr));
            },
            // copy_construct
            [](void* dst, const void* src)
            {
                std::construct_at(static_cast<U*>(dst), *static_cast<const U*>(src));
            },
            // move_construct
            [](void* dst, void* src) noexcept
            {
                std::construct_at(static_cast<U*>(dst), std::move(*static_cast<U*>(src)));
            }
        };

        template<typename U>
        void construct(U&& value)
        {
            using DecayU = std::decay_t<U>;

            if constexpr (detail::can_store_inline_v<DecayU, BufferSize, Alignment>)
            {
                std::construct_at(reinterpret_cast<DecayU*>(&m_buffer), std::forward<U>(value));
                m_type_ops = &type_ops_for<DecayU>;
                m_is_inline = true;
            }
            else
            {
                m_heap_ptr = std::make_unique<DecayU>(std::forward<U>(value)).release();
                m_type_ops = &type_ops_for<DecayU>;
                m_is_inline = false;
            }
        }

        void destroy() noexcept
        {
            if (has_value())
            {
                if (m_is_inline)
                {
                    m_type_ops->destroy(&m_buffer);
                }
                else
                {
                    m_type_ops->destroy(m_heap_ptr);
                    ::operator delete(m_heap_ptr);
                    m_heap_ptr = nullptr;
                }
                m_type_ops = nullptr;
            }
        }

        [[nodiscard]] void* get_storage() noexcept
        {
            return m_is_inline ? &m_buffer : m_heap_ptr;
        }

        [[nodiscard]] const void* get_storage() const noexcept
        {
            return m_is_inline ? &m_buffer : m_heap_ptr;
        }

        union
        {
            aligned_storage<BufferSize, Alignment> m_buffer;
            void* m_heap_ptr;
        };

        const type_ops_base* m_type_ops = nullptr;
        bool m_is_inline = true;
    };

    /**
    * @brief Small String Optimization
    * 
    * Default size of 23 allows for 22 chars + null terminator in 24 bytes.
    */
    template<std::size_t BufferSize>
    class small_string
    {
        static_assert(BufferSize >= 7, "Buffer too small for SSO");

    public:
        using value_type = char;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = char&;
        using const_reference = const char&;
        using pointer = char*;
        using const_pointer = const char*;
        using iterator = char*;
        using const_iterator = const char*;

        small_string() noexcept
        {
            m_data.local.size = 0;
            m_data.local.data[0] = '\0';
        }

        small_string(const char* str)
        {
            if (str)
            {
                assign(str, std::strlen(str));
            }
            else
            {
                m_data.local.size = 0;
                m_data.local.data[0] = '\0';
            }
        }

        small_string(const char* str, size_type len)
        {
            assign(str, len);
        }

        small_string(std::string_view sv)
        {
            assign(sv.data(), sv.size());
        }

        small_string(const small_string& other)
        {
            if (other.is_inline())
            {
                std::memcpy(this, &other, sizeof(small_string));
            }
            else
            {
                assign(other.data(), other.size());
            }
        }

        small_string(small_string&& other) noexcept
        {
            if (other.is_inline())
            {
                std::memcpy(this, &other, sizeof(small_string));
            }
            else
            {
                // Steal heap allocation
                m_data.heap = other.m_data.heap;
                other.m_data.local.size = 0;
                other.m_data.local.data[0] = '\0';
            }
        }

        ~small_string()
        {
            if (!is_inline())
            {
                std::free(m_data.heap.data);
            }
        }

        small_string& operator=(const small_string& other)
        {
            if (this != &other)
            {
                assign(other.data(), other.size());
            }
            return *this;
        }

        small_string& operator=(small_string&& other) noexcept
        {
            if (this != &other)
            {
                if (!is_inline())
                {
                    std::free(m_data.heap.data);
                }

                if (other.is_inline())
                {
                    std::memcpy(this, &other, sizeof(small_string));
                }
                else
                {
                    m_data.heap = other.m_data.heap;
                    other.m_data.local.size = 0;
                    other.m_data.local.data[0] = '\0';
                }
            }
            return *this;
        }

        small_string& operator=(const char* str)
        {
            assign(str, str ? std::strlen(str) : 0);
            return *this;
        }

        small_string& operator=(std::string_view sv)
        {
            assign(sv.data(), sv.size());
            return *this;
        }

        [[nodiscard]] reference operator[](size_type pos) noexcept
        {
            assert(pos <= size());  // <= to allow access to null terminator
            return data()[pos];
        }

        [[nodiscard]] const_reference operator[](size_type pos) const noexcept
        {
            assert(pos <= size());
            return data()[pos];
        }

        [[nodiscard]] reference at(size_type pos)
        {
            if (pos >= size())
                throw std::out_of_range("small_string::at");
            return data()[pos];
        }

        [[nodiscard]] const_reference at(size_type pos) const
        {
            if (pos >= size())
                throw std::out_of_range("small_string::at");
            return data()[pos];
        }

        [[nodiscard]] reference front() noexcept
        {
            assert(!empty());
            return data()[0];
        }

        [[nodiscard]] const_reference front() const noexcept
        {
            assert(!empty());
            return data()[0];
        }

        [[nodiscard]] reference back() noexcept
        {
            assert(!empty());
            return data()[size() - 1];
        }

        [[nodiscard]] const_reference back() const noexcept
        {
            assert(!empty());
            return data()[size() - 1];
        }

        [[nodiscard]] char* data() noexcept
        {
            return is_inline() ? m_data.local.data : m_data.heap.data;
        }

        [[nodiscard]] const char* data() const noexcept
        {
            return is_inline() ? m_data.local.data : m_data.heap.data;
        }

        [[nodiscard]] const char* c_str() const noexcept
        {
            return data();
        }

        [[nodiscard]] operator std::string_view() const noexcept
        {
            return {data(), size()};
        }

        [[nodiscard]] iterator begin() noexcept { return data(); }
        [[nodiscard]] const_iterator begin() const noexcept { return data(); }
        [[nodiscard]] const_iterator cbegin() const noexcept { return data(); }

        [[nodiscard]] iterator end() noexcept { return data() + size(); }
        [[nodiscard]] const_iterator end() const noexcept { return data() + size(); }
        [[nodiscard]] const_iterator cend() const noexcept { return data() + size(); }

        [[nodiscard]] bool empty() const noexcept
        {
            return size() == 0;
        }

        [[nodiscard]] size_type size() const noexcept
        {
            return is_inline() ? m_data.local.size : (m_data.heap.capacity & ~flag_bit);
        }

        [[nodiscard]] size_type length() const noexcept
        {
            return size();
        }

        [[nodiscard]] size_type capacity() const noexcept
        {
            return is_inline() ? inline_capacity : m_data.heap.capacity & ~flag_bit;
        }

        [[nodiscard]] bool is_inline() const noexcept
        {
            return (m_data.heap.capacity & flag_bit) == 0;
        }

        void reserve(size_type new_cap)
        {
            if (new_cap > capacity())
            {
                grow(new_cap);
            }
        }

        void shrink_to_fit()
        {
            if (!is_inline() && size() <= inline_capacity)
            {
                heap_data temp = m_data.heap;
                m_data.local.size = static_cast<uint8_t>(temp.capacity & ~flag_bit);
                std::memcpy(m_data.local.data, temp.data, m_data.local.size + 1);
                std::free(temp.data);
            }
        }

        void clear() noexcept
        {
            if (is_inline())
            {
                m_data.local.size = 0;
                m_data.local.data[0] = '\0';
            }
            else
            {
                m_data.heap.data[0] = '\0';
                m_data.heap.capacity = flag_bit;
            }
        }

        void push_back(char ch)
        {
            size_type sz = size();
            if (sz + 1 > capacity())
            {
                grow(sz + 1);
            }

            data()[sz] = ch;
            data()[sz + 1] = '\0';
            set_size(sz + 1);
        }

        void pop_back() noexcept
        {
            assert(!empty());
            size_type sz = size() - 1;
            data()[sz] = '\0';
            set_size(sz);
        }

        small_string& append(const char* str, size_type len)
        {
            size_type old_size = size();
            size_type new_size = old_size + len;

            if (new_size > capacity())
            {
                grow(new_size);
            }

            std::memcpy(data() + old_size, str, len);
            data()[new_size] = '\0';
            set_size(new_size);

            return *this;
        }

        small_string& append(std::string_view sv)
        {
            return append(sv.data(), sv.size());
        }

        small_string& operator+=(const small_string& str)
        {
            return append(str.data(), str.size());
        }

        small_string& operator+=(std::string_view sv)
        {
            return append(sv.data(), sv.size());
        }

        small_string& operator+=(char ch)
        {
            push_back(ch);
            return *this;
        }

        [[nodiscard]] int compare(std::string_view sv) const noexcept
        {
            size_type sz = size();
            int result = std::memcmp(data(), sv.data(), std::min(sz, sv.size()));
            if (result == 0)
            {
                if (sz < sv.size()) return -1;
                if (sz > sv.size()) return 1;
            }
            return result;
        }

        [[nodiscard]] bool operator==(std::string_view sv) const noexcept
        {
            return size() == sv.size() && std::memcmp(data(), sv.data(), size()) == 0;
        }

        [[nodiscard]] bool operator!=(std::string_view sv) const noexcept
        {
            return !(*this == sv);
        }

        [[nodiscard]] bool operator<(std::string_view sv) const noexcept
        {
            return compare(sv) < 0;
        }

        [[nodiscard]] bool operator<=(std::string_view sv) const noexcept
        {
            return compare(sv) <= 0;
        }

        [[nodiscard]] bool operator>(std::string_view sv) const noexcept
        {
            return compare(sv) > 0;
        }

        [[nodiscard]] bool operator>=(std::string_view sv) const noexcept
        {
            return compare(sv) >= 0;
        }

    private:
        static constexpr std::size_t inline_capacity = BufferSize;
        static constexpr std::size_t flag_bit = std::size_t(1) << (sizeof(std::size_t) * 8 - 1);

        void assign(const char* str, size_type len)
        {
            if (len <= inline_capacity)
            {
                if (!is_inline())
                {
                    std::free(m_data.heap.data);
                }
                m_data.local.size = static_cast<uint8_t>(len);
                std::memcpy(m_data.local.data, str, len);
                m_data.local.data[len] = '\0';
            }
            else
            {
                if (is_inline() || len > capacity())
                {
                    char* new_data = static_cast<char*>(std::malloc(len + 1));
                    if (!new_data)
                        throw std::bad_alloc();

                    if (!is_inline())
                    {
                        std::free(m_data.heap.data);
                    }

                    m_data.heap.data = new_data;
                    m_data.heap.capacity = len | flag_bit;
                }

                std::memcpy(m_data.heap.data, str, len);
                m_data.heap.data[len] = '\0';
                m_data.heap.capacity = len | flag_bit;
            }
        }

        void grow(size_type new_cap)
        {
            // Round up to next power of 2 or reasonable increment
            size_type new_capacity = capacity();
            while (new_capacity < new_cap)
            {
                new_capacity = new_capacity + new_capacity / 2;
            }

            char* new_data = static_cast<char*>(std::malloc(new_capacity + 1));
            if (!new_data)
                throw std::bad_alloc();

            size_type sz = size();
            std::memcpy(new_data, data(), sz + 1);

            if (!is_inline())
            {
                std::free(m_data.heap.data);
            }

            m_data.heap.data = new_data;
            m_data.heap.capacity = sz | flag_bit;
        }

        void set_size(size_type new_size) noexcept
        {
            if (is_inline())
            {
                m_data.local.size = static_cast<uint8_t>(new_size);
            }
            else
            {
                m_data.heap.capacity = new_size | flag_bit;
            }
        }

        struct local_data
        {
            uint8_t size;
            char data[BufferSize];
        };

        struct heap_data
        {
            char* data;
            size_type capacity; // High bit indicates heap storage
        };

        union storage
        {
            local_data local;
            heap_data heap;
        } m_data;
    };

    using sbo_any = small_buffer<void*, 64>;
    using sso24 = small_string<23>;    // 24 bytes total
    
    template<typename T>
    small_buffer(T) -> small_buffer<T, sizeof(T) <= 64 ? 64 : sizeof(T)>;

} // namespace arc

  // Hash support
namespace std
{
    template<typename T, size_t BufferSize, size_t Alignment>
    struct hash<arc::small_buffer<T, BufferSize, Alignment>>
    {
        size_t operator()(const arc::small_buffer<T, BufferSize, Alignment>& sb) const
        {
            size_t h = 0;
            sb.visit([&h](const auto& value)
            {
                using ValueType = std::decay_t<decltype(value)>;
                h = std::hash<ValueType>{}(value);
            });
            return h;
        }
    };

    template<size_t BufferSize>
    struct hash<arc::small_string<BufferSize>>
    {
        size_t operator()(const arc::small_string<BufferSize>& str) const noexcept
        {
            return std::hash<std::string_view>{}(str);
        }
    };
} // namespace std