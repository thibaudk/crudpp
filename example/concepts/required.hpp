#ifndef REQUIRED_HPP
#define REQUIRED_HPP

#include <concepts>

namespace crudpp
{
template <typename T, typename Value_T>
concept name_and_value = requires(T t)
{
    { t.name } -> std::same_as<const char*>;
    { t.value } -> std::same_as<Value_T>;
};

template <typename T>
concept has_primary_key = requires()
{
    T::id::property == "primary";
};
} // namespace crudpp

#endif // REQUIRED_HPP
