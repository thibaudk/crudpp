#pragma once

#include <chrono>

#include <drogon/orm/Field.h>
#include <drogon/orm/Row.h>

#include <crudpp/concepts/required.hpp>
#include <crudpp/bindings/drogon/utils.hpp>

namespace drgn
{
struct row_reader
{
    const drogon::orm::Row& row;

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::is_arithmetic_v<decltype(f.value)>
    {
        f.value = row[std::remove_reference_t<decltype(f)>::c_name()].template as<decltype(f.value)>();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(row[std::remove_reference_t<decltype(f)>::c_name()].template as<int>());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = row[std::remove_reference_t<decltype(f)>::c_name()].template as<std::string>();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::convertible_to<decltype(f.value), std::chrono::sys_days>
    {
        f.value = from_drgn_date(row[std::remove_reference_t<decltype(f)>::c_name()].template as<std::string>());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_seconds>
    {
        f.value = from_drgn_time(row[std::remove_reference_t<decltype(f)>::c_name()].template as<std::string>());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_time<std::chrono::milliseconds>>
    {
        f.value = from_drgn_time_ms(row[std::remove_reference_t<decltype(f)>::c_name()].template as<std::string>());
    }
};
} // namespace drgn
