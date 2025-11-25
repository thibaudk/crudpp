#pragma once

#include <type_traits>

// adapted from avendish flags
// https://celtera.github.io/avendish/development/flags.html

namespace crudpp
{
template <typename T>
concept has_permission_func =
    requires { std::is_function_v<decltype(T::permission)>; };

template <typename T>
concept has_none_flag = requires { T::none; } ||
                        requires { sizeof(typename T::none*); };

template <typename T>
concept has_readonly_flag = requires { T::readonly; } ||
                            requires { sizeof(typename T::readonly*); };

template <typename T>
concept has_writeonly_flag = requires { T::writeonly; } ||
                             requires { sizeof(typename T::writeonly*); };

template <typename T>
concept has_all_flag = requires { T::all; } ||
                       requires { sizeof(typename T::all*); };
}
