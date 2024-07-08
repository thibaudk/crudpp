#pragma once

#include "types.hpp"

namespace crudpp
{
template <typename T>
concept r_table = requires()
{
    { T::table() } -> std::convertible_to<const char*>;
};

template <typename T>
concept r_single_primary_key = std::is_member_object_pointer_v<decltype(T::primary_key())>;

template <typename T>
concept r_composite_primary_key = requires()
{
    { T::primary_key() } -> mop_tuple_like;
};

template <typename T>
concept r_primary_key = r_composite_primary_key<T> || r_single_primary_key<T>;

template <typename T>
concept r_c_name = requires()
{
    { T::c_name() } -> std::same_as<const char*>;
};

template <typename T>
concept r_name = requires()
{
    { T::name() } -> std::same_as<const char*>;
};

template <typename T>
concept r_value = requires(T t)
{
    { t.value } -> suported;
};

template <typename T>
concept r_c_n_v = r_c_name<T> && r_value<T>;

template <typename T>
concept r_session_id = std::is_class<decltype(T::session_id)>();

template <typename T>
concept r_username = std::is_class_v<decltype(T::username)>;

template <typename T>
concept r_password = std::is_class_v<decltype(T::password)>;

template <typename T>
concept authenticates = r_primary_key<T> &&
                        r_username<T> &&
                        r_password<T>;

} // namespace crudpp
