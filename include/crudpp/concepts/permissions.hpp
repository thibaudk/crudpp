#pragma once

// adapted from avendish flags
// https://celtera.github.io/avendish/development/flags.html

namespace crudpp
{
template <typename T>
concept has_readonly_flag = requires { T::readonly; } ||
                            requires { sizeof(typename T::readonly*); };

template <typename T>
concept has_writeonly_flag = requires { T::writeonly; } ||
                             requires { sizeof(typename T::writeonly*); };
}
