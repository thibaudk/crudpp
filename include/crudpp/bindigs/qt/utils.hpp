#pragma once

#include <chrono>

#include <QVariant>
#include <QDate>
#include <QJsonValue>

#include <crudpp/concepts.hpp>
#include <crudpp/type_traits.hpp>

namespace qt
{
template <typename T, bool>
class controller;

template <typename T>
class property_holder;

template <std::integral T>
T to_qt(T v)
{ return v; }

template <typename T>
    requires std::is_enum_v<T>
int to_qt(T v)
{ return (int)v; }

template <std::floating_point T>
T to_qt(const T& v)
{ return v; }

template <std::same_as<std::string> T>
QString to_qt(const T& v)
{ return QString::fromStdString(v); }

// FIXME : workaround conversion from year_month_day to QDate
template <std::convertible_to<std::chrono::sys_days> T>
QDate to_qt(const T& v)
{
    const std::chrono::year_month_day d{v};
    return {static_cast<int>(d.year()),
            static_cast<int>(unsigned(d.month())),
            static_cast<int>(unsigned(d.day()))};
}

QDateTime to_qt(const std::chrono::sys_seconds& v);
QDateTime to_qt(const std::chrono::sys_time<std::chrono::milliseconds>& v);

template <std::signed_integral T>
QJsonValue to_qjson(T v) { return v; }

template <std::same_as<bool> T>
QJsonValue to_qjson(T v) { return v; }

template <typename T>
    requires(std::unsigned_integral<T> && !std::same_as<T, bool>)
QJsonValue to_qjson(T v) { return (int)v; }

template <typename T>
    requires(std::same_as<T, QString> || std::floating_point<T>)
QJsonValue to_qjson(const T&& v) { return v; }

QJsonValue to_qjson(const QDate&& v);
QJsonValue to_qjson(const QDateTime&& v);

template <std::integral T>
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

template <std::same_as<double> T>
T from_qt(const QVariant& v)
{ return v.toDouble(); }

template <std::same_as<std::string> T>
T from_qt(const QVariant& v)
{ return v.toString().toStdString(); }

std::chrono::sys_days from_qdate(const QDate&& v);
std::chrono::sys_seconds from_qdate_time(const QDateTime&& v);
std::chrono::sys_time<std::chrono::milliseconds> from_qdate_time_ms(const QDateTime&& v);

template <std::convertible_to<std::chrono::sys_days> T>
T from_qt(const QVariant& v)
{ return from_qdate(v.toDate()); }

template <std::same_as<std::chrono::sys_seconds> T>
T from_qt(const QVariant& v)
{ return from_qdate_time(v.toDateTime()); }

template <std::same_as<std::chrono::sys_time<std::chrono::milliseconds>> T>
T from_qt(const QVariant& v)
{ return from_qdate_time_ms(v.toDateTime()); }

template <crudpp::r_table T>
std::string make_uri()
{
    std::string s{'Q'};
    s += T::table();
    return s;
}

template <crudpp::r_table T>
const std::string make_key(const T&& agg)
{
    using namespace crudpp;
    std::string key{T::table()};

    if constexpr (r_single_primary_key<T>)
    {
        key += '/';

        if constexpr (std::same_as<typename trait<T>::pk_v_type, std::string>)
            key += (agg.*T::primary_key()).value;
        else
            key += std::to_string((agg.*T::primary_key()).value);
    }
    else if constexpr (r_composite_primary_key<T>)
    {
        key += '&';


    }

    return key;
}

} // namespace qt
