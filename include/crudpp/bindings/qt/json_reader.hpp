#pragma once

#include <QJsonObject>

#include <crudpp/concepts/required.hpp>
#include <crudpp/bindings/qt/utils.hpp>

namespace qt
{
struct json_reader
{
    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), bool>
    {
        f.value = json[std::remove_reference_t<decltype(f)>::c_name()].toBool();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires(std::is_integral_v<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), bool>)
    {
        f.value = json[std::remove_reference_t<decltype(f)>::c_name()].toInt();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::floating_point<decltype(f.value)>
    {
        f.value = json[std::remove_reference_t<decltype(f)>::c_name()].toDouble();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(json[std::remove_reference_t<decltype(f)>::c_name()].toInt());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = json[std::remove_reference_t<decltype(f)>::c_name()].toString().toStdString();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::convertible_to<decltype(f.value), std::chrono::sys_days>
    {
        f.value = from_qdate(QDate::fromString(json[std::remove_reference_t<decltype(f)>::c_name()].toString(), Qt::ISODate));
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_seconds>
    {
        f.value = from_qdate_time(QDateTime::fromString(json[std::remove_reference_t<decltype(f)>::c_name()].toString(), Qt::ISODate));
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_time<std::chrono::milliseconds>>
    {
        f.value = from_qdate_time_ms(QDateTime::fromString(json[std::remove_reference_t<decltype(f)>::c_name()].toString(),
                                                           Qt::ISODateWithMs));
    }

    const QJsonObject& json;
};
} // namespace qt
