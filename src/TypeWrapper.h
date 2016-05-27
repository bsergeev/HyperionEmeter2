#pragma once

#include <iostream>

// This structure is used to distinguish between simple T and column index.
template <typename T>
struct TypeWrapper_t
{
public:
    // Only allow explicit conversion from T.
    TypeWrapper_t() = default;
	explicit TypeWrapper_t(T const& v) : m_val(v) {}
	explicit TypeWrapper_t(T&& v) : m_val(std::move(v)) {}

    // Allow passing this type to functions that take a T.
    operator T&()      { return m_val; }
    operator T() const { return m_val; }

    // Allow access to the value.
    T& val() { return m_val; }
    const T& val() const { return m_val; }

private:
	T m_val;
};
typedef TypeWrapper_t<size_t> ColumnIdx;

//inline std::ostream& operator << (std::ostream& ostr, const TypeWrapper_t& str)
//{
//    return ostr << str.val();
//}
