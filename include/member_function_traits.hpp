#pragma once

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
