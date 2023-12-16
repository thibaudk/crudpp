#pragma once

#include <chrono>

#include <drogon/orm/Field.h>
#include <drogon/orm/Row.h>

#include <crudpp/required.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

namespace drgn
{
struct row_reader
{
    const drogon::orm::Row& row;

    void operator()(crudpp::r_c_name auto& f) noexcept
        requires std::is_arithmetic_v<decltype(f.value)>
    {
        f.value = row[f.c_name()].template as<decltype(f.value)>();
    }

    void operator()(crudpp::r_c_name auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(row[f.c_name()].template as<int>());
    }

    void operator()(crudpp::r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = row[f.c_name()].template as<std::string>();
    }

    void operator()(crudpp::r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::year_month_day>
    {
        f.value = from_drgn_date(row[f.c_name()].template as<std::string>());
    }

    void operator()(crudpp::r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::time_point<std::chrono::system_clock>>
    {
        f.value = from_drgn_time(row[f.c_name()].template as<std::string>());
    }
};
} // namespace drgn
