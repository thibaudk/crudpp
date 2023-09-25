#pragma once

#include <string>

#include <boost/pfr/core.hpp>

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
    if constexpr(r_primary_key<T>)
    {
        const auto m_primary_key{T{}.*T::primary_key()};

        if constexpr(r_c_name<decltype(m_primary_key)>)
            return m_primary_key.c_name();
    }

    return "";
}

template <typename T>
const constexpr size_t get_primary_key_index()
{
    size_t pk_index{};

    boost::pfr::for_each_field(T{},
                               [&pk_index](const r_c_name auto& f, size_t i)
                               {
                                   if constexpr (is_primary_key<decltype(f), T>)
                                       pk_index = i;
                               });
    return pk_index;
}

} // namespace crudpp
