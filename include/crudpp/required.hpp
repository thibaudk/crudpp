#pragma once

#include <concepts>
#include <string>

namespace crudpp
{
template <typename T>
concept r_table = requires()
{
    { T::table() } -> std::same_as<const char*>;
};

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

template <typename T, typename Value_T>
concept r_value = requires(T t)
{
    { t.value } -> std::same_as<Value_T>;
};

template <typename T, typename Value_T>
concept r_c_name_and_value = requires(T t)
{
    r_c_name<T>;
    r_value<T, Value_T>(t);
};

template <typename T>
concept r_primary_key = requires()
{
    std::is_class<decltype(T::primary_key)>();
};

template <typename T>
concept r_session_id = requires()
{
    std::is_class<decltype(T::session_id)>();
};

template <typename T>
concept r_username = requires()
{
    std::is_class<decltype(T::username)>();
};

template <typename T>
concept r_password = requires()
{
    std::is_class<decltype(T::password)>();
};

template <typename T, typename Agg>
concept is_primary_key = std::same_as<T, decltype(Agg::primary_key)>;

template <typename T>
concept authenticates = r_primary_key<T> &&
                        r_username<T> &&
                        r_password<T>;

} // namespace crudpp
