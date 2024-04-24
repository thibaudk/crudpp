#pragma once

#include <string>

#include <boost/pfr/core.hpp>

#include <crudpp/concepts.hpp>
#include <crudpp/type_traits.hpp>

namespace crudpp
{
// template <typename T>
// const constexpr std::string get_primary_key_name()
// {
//     if constexpr(r_single_primary_key<T>)
//     {
//         if constexpr(r_c_name<typename trait<T>::pk_type>)
//             return trait<T>::pk_type::c_name();
//     }
//     else if constexpr(r_composite_primary_key<T>) {}

//     return "";
// }

template <r_single_primary_key T>
const constexpr std::string get_primary_key_name()
{
    if constexpr(r_c_name<typename trait<T>::pk_type>)
        return trait<T>::pk_type::c_name();

    return "";
}

template <r_single_primary_key T,
         typename ...Ts,
         std::tuple<Ts ...> = T::primary_key()>
const constexpr std::vector<std::string> get_primary_key_name()
{
    return {(trait<Ts>::pk_type::c_name(), ...)};
}

// iterate over size_t template arguments
// adapted from both https://www.fluentcpp.com/2021/03/05/stdindex_sequence-and-its-improvement-in-c20/
// and https://stackoverflow.com/a/49319521/14999126
template <class Aggregate, class F>
constexpr void for_each_index(F&& f)
{
    [] <size_t... I>
        (F&& f, std::index_sequence<I...>)
    { (f(std::integral_constant<size_t, I>()), ...); }
    (std::forward<F>(f),
     std::make_index_sequence<boost::pfr::tuple_size_v<Aggregate>>{});
}

// short circuit the above fold expression
// adapted from https://github.com/celtera/avendish/blob/main/include/avnd/common/for_nth.hpp#L62
template <class Aggregate, class F>
constexpr void for_nth_index(int index, F&& f)
{
    [index] <size_t... I>
        (F&& f, std::index_sequence<I...>)
    { ((void)(I == index && (f(std::integral_constant<size_t, I>()), true)), ...); }
    (std::forward<F>(f),
     std::make_index_sequence<boost::pfr::tuple_size_v<Aggregate>>{});
}

template <class Aggregate, class F>
constexpr bool for_each_index_until(F&& f)
{
    return [] <size_t... I>
        (F&& f, std::index_sequence<I...>)
    { return (f(std::integral_constant<size_t, I>()) && ...); }
    (std::forward<F>(f),
     std::make_index_sequence<boost::pfr::tuple_size_v<Aggregate>>{});
}

template <typename T>
bool valid_key(const auto& f)
{
    if constexpr(std::integral<decltype(f.value)>)
        if (f.value > 0) return true;

    if constexpr(std::same_as<decltype(f.value), std::string>)
        if (!f.value.empty()) return true;

    return false;
}

} // namespace crudpp
