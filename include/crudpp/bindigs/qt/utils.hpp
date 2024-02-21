#pragma once

#include <chrono>

#include <QVariant>
#include <QDate>
#include <QJsonValue>

#include <crudpp/required.hpp>

namespace qt
{
template <typename T, bool>
class controller;

template <typename T>
class property_holder;

template <typename T>
    requires std::integral<T>
T to_qt(T v)
{ return v; }

template <typename T>
    requires std::is_enum_v<T>
int to_qt(T v)
{ return (int)v; }

template <typename T>
    requires std::floating_point<T>
T to_qt(const T& v)
{ return v; }

template <typename T>
    requires std::same_as<T, std::string>
QString to_qt(const T& v)
{ return QString::fromStdString(v); }

// FIXME : workaround conversion from year_month_day to QDate

template <typename T>
    requires std::convertible_to<T, std::chrono::sys_days>
QDate to_qt(const T& v)
{
    const std::chrono::year_month_day d{v};
    return {static_cast<int>(d.year()),
            static_cast<int>(unsigned(d.month())),
            static_cast<int>(unsigned(d.day()))};
}

QDateTime to_qt(const std::chrono::sys_seconds& v);
QDateTime to_qt(const std::chrono::sys_time<std::chrono::milliseconds>& v);

template <typename T>
    requires std::signed_integral<T>
QJsonValue to_qjson(T v) { return v; }

template <typename T>
    requires std::same_as<T, bool>
QJsonValue to_qjson(T v) { return v; }

template <typename T>
    requires(std::unsigned_integral<T> && !std::same_as<T, bool>)
QJsonValue to_qjson(T v) { return (int)v; }

template <typename T>
    requires(std::same_as<T, QString> || std::floating_point<T>)
QJsonValue to_qjson(const T&& v) { return v; }

QJsonValue to_qjson(const QDate&& v);
QJsonValue to_qjson(const QDateTime&& v);

template <typename T>
    requires std::integral<T>
T from_qt(const QVariant& v)
{ return v.toInt(); }

template <typename T>
    requires std::is_enum_v<T>
T from_qt(const QVariant& v)
{ return T(v.toInt()); }

template <typename T>
    requires(std::floating_point<T> && !std::same_as<T, double>)
T from_qt(const QVariant& v)
{ return v.toFloat(); }

template <typename T>
    requires std::same_as<T, double>
T from_qt(const QVariant& v)
{ return v.toDouble(); }

template <typename T>
    requires std::same_as<T, std::string>
T from_qt(const QVariant& v)
{ return v.toString().toStdString(); }

std::chrono::sys_days from_qdate(const QDate&& v);
std::chrono::sys_seconds from_qdate_time(const QDateTime&& v);
std::chrono::sys_time<std::chrono::milliseconds> from_qdate_time_ms(const QDateTime&& v);

template <typename T>
    requires std::convertible_to<T, std::chrono::sys_days>
T from_qt(const QVariant& v)
{
    return from_qdate(v.toDate());
}

template <typename T>
    requires std::same_as<T, std::chrono::sys_seconds>
T from_qt(const QVariant& v)
{
    return from_qdate_time(v.toDateTime());
}

template <typename T>
    requires std::same_as<T, std::chrono::sys_time<std::chrono::milliseconds>>
T from_qt(const QVariant& v)
{
    return from_qdate_time_ms(v.toDateTime());
}

template <typename T>
std::string make_uri()
{
    std::string s{'Q'};
    s += T::table();
    return s;
}

template <typename T>
void make_clt()
{
    const auto uri{make_uri<T>()};
    std::string qml_name{T::table()};
    qml_name.insert(0, "Single_");
    qmlRegisterType<property_holder<T>>(uri.c_str(), 1, 0, qml_name.c_str());
}

template <typename ...Ts>
std::tuple<controller<Ts, crudpp::r_primary_key<Ts>>...> make_ctls()
{
    (make_clt<Ts>(), ...);

    return std::tuple<controller<Ts, crudpp::r_primary_key<Ts>>...>{};
}

} // namespace qt
