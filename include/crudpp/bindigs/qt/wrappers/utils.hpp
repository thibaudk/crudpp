#pragma once

#include <QVariant>

namespace crudpp
{
template <typename T>
class controller;

template <typename T>
T to_qt(T v)
    requires std::is_integral_v<T>
{ return v; }

template <typename T>
int to_qt(T v)
    requires std::is_enum_v<T>
{ return (int)v; }

QString to_qt(const std::string& s)
{ return QString::fromStdString(s); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::is_integral_v<T>
{ return v.toInt(); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::is_enum_v<T>
{ return T(v.toInt()); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::is_same_v<T, std::string>
{ return v.toString().toStdString(); }

template <typename T>
std::string make_uri()
{
    std::string s{'Q'};
    s += T::table();
    return s;
}

template <typename ...Ts>
std::tuple<controller<Ts>...> make_ctls()
{ return std::tuple<controller<Ts>...>{}; }

} // namespace crudpp
