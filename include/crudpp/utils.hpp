#pragma once

#include <string>

#include <boost/pfr/core.hpp>

#include <crudpp/concepts.hpp>
#include <crudpp/type_traits.hpp>

namespace crudpp
{
// iterate over size_t template arguments
// adapted from both https://www.fluentcpp.com/2021/03/05/stdindex_sequence-and-its-improvement-in-c20/
// and https://stackoverflow.com/a/49319521/14999126
template <class Aggregate, class F>
constexpr void for_each_index(F&& f)
{
    using namespace std;

    [] <size_t... I>
        (F&& f, index_sequence<I...>)
    { (f(integral_constant<size_t, I>()), ...); }
    (std::forward<F>(f),
     make_index_sequence<boost::pfr::tuple_size_v<Aggregate>>{});
}

// short circuit the above fold expression
// adapted from https://github.com/celtera/avendish/blob/main/include/avnd/common/for_nth.hpp#L62
template <class Aggregate, class F>
constexpr void for_nth_index(int index, F&& f)
{
    using namespace std;

    [index] <size_t... I>
        (F&& f, index_sequence<I...>)
    { ((void)(I == index && (f(integral_constant<size_t, I>()), true)), ...); }
    (std::forward<F>(f),
     make_index_sequence<boost::pfr::tuple_size_v<Aggregate>>{});
}

template <class Aggregate, class F>
constexpr bool for_each_index_until(F&& f)
{
    using namespace std;

    return [] <size_t... I>
        (F&& f, index_sequence<I...>)
    { return (f(integral_constant<size_t, I>()) && ...); }
    (std::forward<F>(f),
     make_index_sequence<boost::pfr::tuple_size_v<Aggregate>>{});
}

template <typename T>
bool valid_key(const auto& f)
{
    using namespace std;

    if constexpr(integral<decltype(f.value)>)
    {
        if (f.value > 0) return true;
    }
    else if constexpr(same_as<decltype(f.value), string>)
    {
        if (!f.value.empty()) return true;
    }

    return false;
}

} // namespace crudpp
