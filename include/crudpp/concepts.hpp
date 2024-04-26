#pragma once

#include <concepts>
#include <string>
#include <utility>
#include <tuple>

namespace crudpp
{
template <typename T, bool>
struct trait;

template <typename T, typename ...Exc_T>
concept same_as = (std::same_as<T, Exc_T> || ...);

template <typename T, typename ...Exc_T>
concept different_than = !same_as<T, Exc_T...>;

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

template <typename T>
concept r_table = requires()
{
    { T::table() } -> std::same_as<const char*>;
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

template <typename T, typename Agg>
concept is_single_primary_key = std::same_as<
    std::remove_cvref_t<T>,
    std::remove_cvref_t<typename trait<Agg, true>::pk_type>
    >;

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
concept is_foreign_key = std::is_member_object_pointer_v<decltype(T::foreign_key())>;

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
