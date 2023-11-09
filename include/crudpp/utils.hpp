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

// FIXME
// very ineficiant !! replace with avendish introspection lib
// --
template <typename T>
const constexpr size_t get_primary_key_index()
{
    size_t pk_index{};

    boost::pfr::for_each_field(T{},
                               [&pk_index](const auto& f, size_t i)
                               {
                                   if constexpr(is_primary_key<decltype(f), T>)
                                       pk_index = i;
                               });
    return pk_index;
}

template <typename F, typename Agg>
const constexpr size_t get_field_index()
{
    size_t index{};

    boost::pfr::for_each_field(Agg{},
                               [&index](const auto& f, size_t i)
                               {
                                   if constexpr(is_field<F, decltype(f)>)
                                       index = i;
                               });
    return index;
}

template <typename F, typename Agg>
const constexpr size_t get_field_index(const Agg& agg)
{
    size_t index{};

    boost::pfr::for_each_field(agg,
                               [&index](const auto& f, size_t i)
                               {
                                   if constexpr(is_field<F, decltype(f)>)
                                       index = i;
                               });
    return index;
}
// --

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

enum permissions
{
    none,
    read,
    write
};

// member function trait adapted from https://stackoverflow.com/a/9779391/14999126
// and https://stackoverflow.com/a/9065203/14999126

template <typename T>
struct member_function_traits;

template <typename T, typename R>
struct member_function_traits<R (T::*)()>
{
    static const consteval size_t n_args() { return 0; }
    using return_type = R;
    using arg_type = void;
};

template <typename T, typename R, typename Arg>
struct member_function_traits<R (T::*)(Arg)>
{
    static const consteval size_t n_args() { return 1; }
    using return_type = R;
    using arg_type = Arg;
};

template <typename T, typename R, typename ...Args>
struct member_function_traits<R (T::*)(Args...)>
{
    static const consteval size_t n_args() { return sizeof...(Args); }
    using return_type = R;
    using arg_type = std::tuple<Args...>;
};
} // namespace crudpp
