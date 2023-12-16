#pragma once

#include "row_reader.hpp"

namespace drgn
{
struct offset_row_reader
{
    const drogon::orm::Row& row;
    const ssize_t offset;
    ssize_t index = offset;

    void operator()(auto& f) noexcept
        requires std::is_arithmetic_v<decltype(f.value)>
    {
        f.value = row.at(index).as<decltype(f.value)>();
    }

    void operator()(auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(row.at(index).as<int>());
    }

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = row.at(index).as<std::string>();
    }

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::year_month_day>
    {
        f.value = from_drgn_date(row.at(index).as<std::string>());
    }

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::time_point<std::chrono::system_clock>>
    {
        f.value = from_drgn_time(row.at(index).as<std::string>());
    }
};
} // namespace drgn
