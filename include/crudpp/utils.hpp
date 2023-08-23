#pragma once

#include <string>

#include <crudpp/required.hpp>

namespace crudpp
{
// adapted from drogon's Mapper.h
template <typename T, bool hasPrimaryKey = true>
struct trait
{
    using type = decltype((T{}.*T::primary_key()).value);
};

template <typename T>
struct trait<T, false>
{
    using type = void;
};

template <typename T>
const constexpr std::string get_primary_key_name()
{
    if constexpr(has_primary_key<T>)
    {
        const auto m_primary_key{T{}.*T::primary_key()};

        if constexpr(r_c_name<decltype(m_primary_key)>)
            return m_primary_key.c_name();
    }
    return "";
}
}
