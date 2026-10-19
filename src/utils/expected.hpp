#pragma once

#include <utility>

template <typename E>
struct Unexpected {
    E err;

    Unexpected(const E& e) : err(e) {}
    Unexpected(E&& e) : err(std::move(e)) {}
};
template <typename E>
Unexpected(E) -> Unexpected<E>;

// Similar to std::expected<T, E> from C++20, and Result<T, E> in Rust
// C++20 is not fully supported on most compilers
template <typename T, typename E>
class Expected {
public:
    Expected(const T& value)
        : m_IsError(false), m_Value(value) {}
    Expected(T&& value)
        : m_IsError(false), m_Value(std::move(value)) {}
    Expected(const Unexpected<E>& err)
        : m_IsError(true), m_Value(err.err) {}
    Expected(Unexpected<E>&& err)
        : m_IsError(true), m_Value(std::move(err.err)) {}
    Expected(const Expected& e) = delete;

    ~Expected() {
        if(m_IsError)
            m_Value.error.~E();
        else
            m_Value.value.~T();
    }

    inline T& Value() { return m_Value.value; }
    inline E& Error() { return m_Value.error; }
    inline const T& Value() const { return m_Value.value; }
    inline const E& Error() const { return m_Value.error; }
    inline bool IsError() { return m_IsError; }

    explicit operator bool() const {
        return !m_IsError;
    }
    T& operator*() { return m_Value.value; }
    const T& operator*() const { return m_Value.value; }

private:
    bool m_IsError;
    union U {
        T value;
        E error;
        U() {}
        U(const T& v) : value(v) {}
        U(T&& v) : value(std::move(v)) {}
        U(const E& e) : error(e) {}
        U(E&& e) : error(std::move(e)) {}
        ~U() {}
    } m_Value;
};
