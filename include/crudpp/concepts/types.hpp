#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <chrono>
#include <tuple>

namespace crudpp
{
template <typename T,
         typename Name_t,
         typename Container_t,
         bool>
struct trait;

template <typename T, typename ...Exc_T>
concept same_as = (std::same_as<T, Exc_T> || ...);

template <typename T>
concept suported = std::is_arithmetic_v<std::remove_reference_t<T>> ||
                   std::is_enum_v<std::remove_reference_t<T>> ||
                   same_as<std::remove_reference_t<T>,
                           std::string,
                           std::chrono::sys_days,
                           std::chrono::sys_seconds,
                           std::chrono::sys_seconds,
                           std::chrono::sys_time<std::chrono::milliseconds>>;

template <typename T, typename ...Exc_T>
concept different = !same_as<T, Exc_T...>;

template <template <std::size_t> typename T>
concept is_size_template = requires
{ typename T<0>; };

// adapted from https://stackoverflow.com/a/68444475
// --
template<class T, std::size_t N>
concept has_tuple_element = requires(T t)
{
    typename std::tuple_element_t<N, std::remove_const_t<T>>;
    { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
};

template<class T>
concept tuple_like = requires()
{ typename std::tuple_size<T>::type; } &&
[] <std::size_t... N> (std::index_sequence<N...>)
{
    return (has_tuple_element<T, N> && ...);
} (std::make_index_sequence<std::tuple_size_v<T>>());
// --

template<class T>
concept mop_tuple_like = requires()
{ typename std::tuple_size<T>::type; } &&
[] <std::size_t... N> (T&& t, std::index_sequence<N...>)
{
    using namespace std;
    return ((is_member_object_pointer_v<
                 remove_cvref_t<decltype(get<N>(t))>
                 > &&
             has_tuple_element<T, N>) && ...);
} (T{}, std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T, typename Agg>
concept is_single_primary_key = std::same_as<
    std::remove_cvref_t<T>,
    std::remove_cvref_t<typename trait<Agg, const char*, void, true>::pk_type>>;

template <typename T, typename Agg>
concept is_composite_primary_key = requires()
{ typename std::tuple_size<T>::type; } &&
[] <typename ...Ts, std::size_t... N> (std::tuple<Ts...>&&, std::index_sequence<N...>)
{
    using namespace std;
    return ((same_as<remove_cvref_t<T>, remove_cvref_t<Ts>>) || ...);
} (T::primary_key(),
  std::make_index_sequence<std::tuple_size_v<decltype(T::primary_key())>>());

template <typename T, typename Agg>
concept is_primary_key = is_single_primary_key<T, Agg> || is_composite_primary_key<T, Agg>;

template <typename T>
concept is_foreign_key = std::is_member_object_pointer_v<
    decltype(std::remove_cvref_t<T>::foreign_key())>;

} // namespace crudpp
