#pragma once

#include <chrono>

#include <QVariant>
#include <QDate>

#include <crudpp/required.hpp>

namespace qt
{
template <typename T>
class controller;

template <typename T>
T to_qt(T v)
    requires std::integral<T>
{ return v; }

template <typename T>
int to_qt(T v)
    requires std::is_enum_v<T>
{ return (int)v; }

template <typename T>
T to_qt(const T& v)
    requires std::floating_point<T>
{ return v; }

template <typename T>
QString to_qt(const T& v)
    requires std::same_as<T, std::string>
{ return QString::fromStdString(v); }

// FIXME : workaround conversion from year_month_day to QDate

template <typename T>
QDate to_qt(const T& v)
    requires std::convertible_to<T, std::chrono::sys_days>
{
    const std::chrono::year_month_day d{v};
    return {static_cast<int>(d.year()),
            static_cast<int>(unsigned(d.month())),
            static_cast<int>(unsigned(d.day()))};
}

template <typename T>
QDateTime to_qt(const T& v)
    requires std::same_as<T, std::chrono::sys_seconds>
{
    return QDateTime::fromSecsSinceEpoch(std::chrono::duration_cast<std::chrono::seconds>(
                                             v.time_since_epoch()).count());
}

template <typename T>
QDateTime to_qt(const T& v)
    requires std::same_as<T, std::chrono::sys_time<std::chrono::milliseconds>>
{
    return QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(
                                              v.time_since_epoch()).count());
}

template <typename T>
    requires std::signed_integral<T>
QJsonValue to_qjson(T v) { return v; }

template <typename T>
    requires std::unsigned_integral<T>
QJsonValue to_qjson(T v) { return (int)v; }

template <typename T>
    requires(std::same_as<T, QString> ||
             std::floating_point<T>)
QJsonValue to_qjson(const T&& v) { return v; }

QJsonValue to_qjson(const QDate&& d) { return d.toString(Qt::ISODate); }

QJsonValue to_qjson(const QDateTime&& d) { return d.toString(Qt::ISODateWithMs); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::integral<T>
{ return v.toInt(); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::is_enum_v<T>
{ return T(v.toInt()); }

template <typename T>
T from_qt(const QVariant& v)
    requires(std::floating_point<T> &&
             !std::same_as<T, double>)
{ return v.toFloat(); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::same_as<T, double>
{ return v.toDouble(); }

template <typename T>
T from_qt(const QVariant& v)
    requires std::same_as<T, std::string>
{ return v.toString().toStdString(); }

// FIXME : workaround conversion from QDate to year_month_day

std::chrono::sys_days from_qdate(const QDate&& v)
{
    return std::chrono::year_month_day{std::chrono::year{v.year()},
                                       std::chrono::month{unsigned(v.month())},
                                       std::chrono::day{unsigned(v.day())}};
}

template <typename T>
T from_qt(const QVariant& v)
    requires std::convertible_to<T, std::chrono::sys_days>
{
    return from_qdate(v.toDate());
}

std::chrono::sys_seconds from_qdate_time(const QDateTime&& v)
{
    auto epoch{v.toSecsSinceEpoch()};
    return std::chrono::sys_seconds{std::chrono::duration<qint64>{epoch}};
}

std::chrono::sys_time<std::chrono::milliseconds> from_qdate_time_ms(const QDateTime&& v)
{
    auto epoch{v.toMSecsSinceEpoch()};
    return std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::duration<qint64>{epoch}};
}

template <typename T>
T from_qt(const QVariant& v)
    requires std::same_as<T, std::chrono::sys_seconds>
{
    return from_qdate_time(v.toDateTime());
}

template <typename T>
T from_qt(const QVariant& v)
    requires std::same_as<T, std::chrono::sys_time<std::chrono::milliseconds>>
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

template <typename ...Ts>
std::tuple<controller<Ts>...> make_ctls()
{ return std::tuple<controller<Ts>...>{}; }

} // namespace qt
