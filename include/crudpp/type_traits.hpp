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
    using pk_v_type = decltype(pk_type::value);
};

template <typename T, bool has_primary_key = true>
struct trait : base_trait<T, crudpp::r_single_primary_key<T>>
{
};

template <typename T>
struct trait<T, false>
{
    using pk_type = void;
    using pk_v_type = void;
};

template <typename T>
using trait_t = trait<T, r_primary_key<T>>;

} // namespace crudpp
