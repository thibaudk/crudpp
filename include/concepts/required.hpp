#ifndef REQUIRED_HPP
#define REQUIRED_HPP

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
    T::id::property() == "primary";
};

template <typename T>
concept is_primary_key = requires()
{
    T::property() == "primary";
};
} // namespace crudpp

#endif // REQUIRED_HPP
