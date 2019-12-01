#pragma once

//#include <iostream>

// This structure is used to distinguish between simple size_t and column index.
template <typename T,
          typename std::enable_if<std::is_literal_type<T>::value>::type * = nullptr>
class TypeWrapper_t
{
public:
    // Only allow explicit conversion from T.
             constexpr TypeWrapper_t()           noexcept : m_val(0) {} //= default;
    explicit constexpr TypeWrapper_t(T const& v) noexcept : m_val(v) {}
    explicit constexpr TypeWrapper_t(T&& v     ) noexcept : m_val(std::move(v)) {}

    // Allow passing this type to functions that take a T.
              operator T&()      noexcept { return m_val; }
    constexpr operator T() const noexcept { return m_val; }

    // Allow access to the value.
                    T& val()       noexcept { return m_val; }
    constexpr const T& val() const noexcept { return m_val; }

private:
    T m_val;
};
typedef TypeWrapper_t<size_t> ColumnIdx;

//inline std::ostream& operator << (std::ostream& ostr, const TypeWrapper_t& str)
//{
//    return ostr << str.val();
//}
