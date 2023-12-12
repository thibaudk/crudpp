#pragma once

#include <chrono>

#include <QVariant>
#include <QDate>

namespace qt
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

template <typename T>
QString to_qt(const T& v)
    requires std::is_same_v<T, std::string>
{ return QString::fromStdString(v); }

// FIXME : workaround conversion from year_month_day to QDate

template <typename T>
QDate to_qt(const T& v)
    requires std::is_same_v<T, std::chrono::year_month_day>
{
    return {static_cast<int>(v.year()),
            static_cast<int>(unsigned(v.month())),
            static_cast<int>(unsigned(v.day()))};
}

template <typename T>
QJsonValue to_qjson(const T&& v) { return v; }

template <>
QJsonValue to_qjson(const QDate&& d) { return d.toString(Qt::ISODate); }

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

// FIXME : workaround conversion from QDate to year_month_day

std::chrono::year_month_day from_qdate(const QDate&& v)
{
    return {std::chrono::year{v.year()},
            std::chrono::month{unsigned(v.month())},
            std::chrono::day{unsigned(v.day())}};
}

template <typename T>
T from_qt(const QVariant& v)
    requires std::is_same_v<T, std::chrono::year_month_day>
{
    return from_qdate(v.toDate());
}

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

} // namespace qt
