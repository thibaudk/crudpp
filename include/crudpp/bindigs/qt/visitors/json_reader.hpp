#pragma once

#include <QJsonObject>

#include <crudpp/required.hpp>
#include <crudpp/bindigs/qt/wrappers/utils.hpp>

namespace crudpp
{
namespace visitor
{
struct json_reader
{
    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), bool>
    {
        f.value = json[f.c_name()].toBool();
    }

    void operator()(r_c_name auto& f) noexcept
        requires satisfies_and_different<decltype(f.value), std::is_integral, bool>
    {
        f.value = json[f.c_name()].toInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(json[f.c_name()].toInt());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = json[f.c_name()].toString().toStdString();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::year_month_day>
    {
        f.value = from_qdate(QDate::fromString(json[f.c_name()].toString(), Qt::ISODate));
    }

    const QJsonObject& json;
};
} // namespace visitor
} // namespace crudpp
