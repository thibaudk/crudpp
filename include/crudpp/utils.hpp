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
    using type = decltype(T::primary_key.value);
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
        if constexpr(r_c_name<decltype(T::primary_key)>)
            return T::primary_key::c_name();
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
                                   if constexpr(is_primary_key<decltype(f), T>)
                                       pk_index = i;
                               });
    return pk_index;
}

// FIXME
// possibly replacable with avendish introspection lib
// iterate over size_t template arguments
// copied from https://stackoverflow.com/a/49319521/14999126
// --

template <size_t ...Is, typename F>
void for_each_index(std::index_sequence<Is...>, F&& f)
{
    int dummy[] = {0, (static_cast<void>(f(std::integral_constant<size_t, Is>())), 0)...};
    static_cast<void>(dummy);
}

template <size_t N, typename F>
void for_each_index(F&& f)
{
    for_each_index(std::make_index_sequence<N>(), std::forward<F>(f));
}
// --
} // namespace crudpp
