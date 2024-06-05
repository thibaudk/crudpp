#pragma once

#include "concepts.hpp"

namespace crudpp
{
// adapted from https://stackoverflow.com/q/22825512
template <class C, typename T>
T get_mp_type(T C::*v);

template <typename T>
const constexpr std::size_t pk_size()
{
    if constexpr (r_composite_primary_key<T>)
        return std::tuple_size_v<decltype(T::primary_key())>;
    else if (r_single_primary_key<T>)
        return 1;
    else
        return 0;
}

// adapted from drogon's Mapper.h
template <typename T,
         typename Name_t,
         typename Container_t,
         bool single_primary_key = true>
struct base_trait
{
    using pk_type = decltype(get_mp_type(T::primary_key()));
    using pk_n_type = Name_t;
    using pk_v_type = decltype(pk_type::value);

    static constexpr auto pk_name()
    {
        if constexpr(r_c_name<pk_type>)
            return pk_type::c_name();

        return "";
    }

    static const constexpr pk_v_type pk_value(const T& t)
    { return (t.*T::primary_key()).value; }
};

template <typename T,
         typename Name_t,
         typename Container_t>
struct base_trait<T, Name_t, Container_t, false>
{
    using pk_n_type = Container_t;

    static const constexpr pk_n_type pk_name()
    {
        using namespace std;

        return [] <size_t... I> (auto&& tp, index_sequence<I...>)
               -> pk_n_type
        // FIXME: simingly useless decltype ?
        { return {decltype(get_mp_type(get<I>(tp)))::c_name() ...}; }
        (T::primary_key(),
         make_index_sequence<pk_size<T>()>{});
    }

    static const constexpr auto pk_value(const T& t)
    {
        using namespace std;

        return [&t] <size_t... I> (auto&& tp, index_sequence<I...>)
        { return std::make_tuple((t.*get<I>(tp)).value ...); }
        (T::primary_key(),
         make_index_sequence<pk_size<T>()>{});
    }

    using pk_v_type = decltype(pk_value(T{}));

private:
    static const constexpr auto pk_type_()
    {
        using namespace std;

        return [] <size_t... I> (auto&& tp, index_sequence<I...>)
        { return std::make_tuple((get_mp_type(get<I>(tp))) ...); }
        (T::primary_key(),
         make_index_sequence<pk_size<T>()>{});
    }

public:
    using pk_type = decltype(pk_type_());
};

template <typename T,
         typename Name_t = const char*,
         typename Container_t = std::array<Name_t, pk_size<T>()>,
         bool has_primary_key = true>
struct trait : base_trait<T, Name_t, Container_t, r_single_primary_key<T>>
{
};

template <typename T,
         typename Name_t,
         typename Container_t>
struct trait<T, Name_t, Container_t, false>
{
    using pk_type = void;
    using pk_n_type = Name_t;
    using pk_v_type = void;

    static const constexpr auto pk_name() { return ""; };
    static const constexpr void pk_value(const T&) {};
};

template <typename T,
         typename Name_t = const char*,
         typename Container_t = std::array<Name_t, pk_size<T>()>>
using t_trait = trait<T, Name_t, Container_t, r_primary_key<T>>;

} // namespace crudpp
