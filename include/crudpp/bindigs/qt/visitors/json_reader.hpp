#pragma once

#include <QJsonObject>

#include <crudpp/required.hpp>

namespace crudpp
{
namespace visitor
{
struct json_reader
{
    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        f.value = json[f.c_name()].toInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int8_t>
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

    const QJsonObject& json;
};
} // namespace visitor
} // namespace crudpp
