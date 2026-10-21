#pragma once

#include <utility>
#include <variant>

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
        : m_Value(value) {}
    Expected(T&& value)
        : m_Value(std::move(value)) {}
    Expected(const Unexpected<E>& err)
        : m_Value(std::in_place_index<1>, err.err) {}
    Expected(Unexpected<E>&& err)
        : m_Value(std::in_place_index<1>, std::move(err.err)) {}

    inline T& Value() { return std::get<0>(m_Value); }
    inline E& Error() { return std::get<1>(m_Value); }
    inline const T& Value() const { return std::get<0>(m_Value); }
    inline const E& Error() const { return std::get<1>(m_Value); }
    inline bool IsError() const { return m_Value.index() == 1; }
    inline T Or(T& value) {
        if(IsError())
            return value;
        else
            return Value();
    }
    inline T Or(T&& value) {
        if(IsError())
            return value;
        else
            return Value();
    }

    explicit operator bool() const {
        return !IsError();
    }
    T& operator*() { return Value(); }
    const T& operator*() const { return Value(); }

private:
    std::variant<T, E> m_Value;
};
