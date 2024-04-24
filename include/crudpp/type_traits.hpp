#pragma once

#include "concepts.hpp"

namespace crudpp
{
// adapted from https://stackoverflow.com/q/22825512
template <class C, typename T>
T get_mp_type(T C::*v);

// adapted from drogon's Mapper.h
template <typename T, bool single_primary_key = true>
struct base_trait
{
    using pk_type = decltype(get_mp_type(T::primary_key()));
    using pk_n_type = std::string;
    using pk_v_type = decltype(pk_type::value);

    static const constexpr std::string pk_name()
    {
        if constexpr(r_c_name<pk_type>)
            return pk_type::c_name();

        return "";
    };
};

template <typename T>
struct base_trait<T, false>
{
    using pk_n_type = std::vector<std::string>;
    // using pk_v_type = std::vector<std::string>;

    static consteval std::vector<std::string> pk_name()
    {
        using namespace std;

        return [] <size_t... I> (auto&& tp, index_sequence<I...>)
        { return vector<string>{(decltype(get_mp_type(get<I>(tp)))::c_name()) ...}; }
        (T::primary_key(),
         make_index_sequence<tuple_size_v<decltype(T::primary_key())>>{});
    }

private:
    static consteval auto pk_type_()
    {
        using namespace std;

        return [] <size_t... I> (auto&& tp, index_sequence<I...>)
        { return std::make_tuple((get_mp_type(get<I>(tp))) ...); }
        (T::primary_key(),
         make_index_sequence<tuple_size_v<decltype(T::primary_key())>>{});
    }

    static consteval auto pk_v_type_()
    {
        using namespace std;

        return [] <size_t... I> (auto&& tp, index_sequence<I...>)
        { return std::make_tuple((get_mp_type(get<I>(tp))).value ...); }
        (T::primary_key(),
         make_index_sequence<tuple_size_v<decltype(T::primary_key())>>{});
    }

public:
    using pk_type = decltype(pk_type_());
    using pk_v_type = decltype(pk_v_type_());
};

template <typename T, bool has_primary_key = true>
struct trait : base_trait<T, r_single_primary_key<T>>
{
};

template <typename T>
struct trait<T, false>
{
    using pk_type = void;
    using pk_n_type = std::string;
    using pk_v_type = void;

    static consteval std::string pk_name() { return ""; };
};

template <typename T>
using trait_t = trait<T, r_primary_key<T>>;

} // namespace crudpp
