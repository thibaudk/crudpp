#pragma once

namespace crudpp
{
template <class C, typename T>
T get_mp_type(T C::*v);

// adapted from drogon's Mapper.h
template <typename T, bool has_primary_key = true>
struct trait
{
    using pk_type = decltype(get_mp_type(T::primary_key()));
    using pk_v_type = decltype(pk_type::value);
};

template <typename T>
struct trait<T, false>
{
    using pk_type = void;
    using pk_v_type = void;
};

} // namespace crudpp
