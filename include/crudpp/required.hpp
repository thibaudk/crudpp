#pragma once

#include <concepts>
#include <string>
#include <utility>
//#include <crudpp/utils.hpp>

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
concept r_c_name_and_value = requires(T t)
{
    r_c_name<T>;
    r_value<T, Value_T>(t);
};

template <typename T>
concept r_primary_key = requires()
{
    std::is_class<decltype(T::primary_key)>();
};

template <typename T>
concept r_session_id = requires()
{
    std::is_class<decltype(T::session_id)>();
};

template <typename T>
concept r_username = requires()
{
    std::is_class<decltype(T::username)>();
};

template <typename T>
concept r_password = requires()
{
    std::is_class<decltype(T::password)>();
};

template <typename T, typename F>
concept is_field = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<F>>;

template <typename T, typename Agg>
concept is_primary_key = is_field<T, decltype(Agg::primary_key)>;

template <typename T>
concept authenticates = r_primary_key<T> &&
                        r_username<T> &&
                        r_password<T>;

//template <typename T, typename ...Args>
//concept r_permission = requires(Args... args)
//{
//    { T::permission(args) } -> std::same_as<std::to_undelying(utils::permissions)>;
//};

template <typename T, typename ...Exc_T>
concept is_same = (std::same_as<T, Exc_T> || ...);

template <typename T, typename ...Exc_T>
concept is_different = !is_same<T, Exc_T...>;

// adapted from https://stackoverflow.com/a/44616039/14999126

template<typename T, template<typename> typename... Concept>
concept satisfies_all = (... && Concept<T>::value);

template<typename T, template<typename> typename Concept, typename ...Same>
concept satisfies_and_same = Concept<T>::value &&
                             is_same<T, Same...>;

template<typename T, template<typename> typename Concept, typename ...Diff>
concept satisfies_and_different = Concept<T>::value &&
                                  is_different<T, Diff...>;

template<typename T, typename ...Diff>
concept signed_integral_different = std::signed_integral<T> &&
                                    is_different<T, Diff...>;

template<typename T, typename ...Diff>
concept unsigned_integral_different = std::unsigned_integral<T> &&
                                      is_different<T, Diff...>;

} // namespace crudpp
