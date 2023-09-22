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
concept r_name_and_value = requires(T t)
{
    r_name<T>;
    r_value<T, Value_T>(t);
};

template <typename T>
concept has_primary_key = requires()
{
    std::is_aggregate<decltype(T::primary_key())>();
};

template <typename T>
concept has_session_id = requires()
{
    std::is_aggregate<decltype(T::session_id())>();
};

template <typename T>
concept has_identifier = requires()
{
    std::is_aggregate<decltype(T::identifier())>();
};

template <typename T, typename Agg>
concept is_primary_key = requires(const Agg& a)
{
    { a.*Agg::primary_key() } -> std::same_as<T>;
};

template <typename T>
concept authenticates = requires()
{
    has_primary_key<T>;
    has_identifier<T>;
    { T::authenticate() } -> std::same_as<bool>;
};

} // namespace crudpp
