#pragma once

// function traits adapted from https://stackoverflow.com/a/9065203/14999126

#include <type_traits>
#include <tuple>

template <typename T>
    requires std::is_function_v<T>
struct static_function_traits;

template <typename R>
struct static_function_traits<R ()>
{
    static consteval size_t n_args() { return 0; }
    using return_type = R;
    using arg_type = void;
};

template <typename R, typename Arg>
struct static_function_traits<R (Arg)>
{
    static consteval size_t n_args() { return 1; }
    using return_type = R;
    using arg_type = Arg;
};

template <typename R, typename ...Args>
struct static_function_traits<R (Args...)>
{
    static consteval size_t n_args() { return sizeof...(Args); }
    using return_type = R;
    using arg_type = std::tuple<Args...>;
};
