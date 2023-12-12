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
        if (!row.at(index).isNull())
            f.value = row.at(index).as<decltype(f.value)>();
    }

    void operator()(auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        if (!row.at(index).isNull())
            f.value = decltype(f.value)(row.at(index).as<int>());
    }

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        if (!row.at(index).isNull())
            f.value = row.at(index).as<std::string>();
    }

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::year_month_day>
    {
        if (!row.at(index).isNull())
        {
            f.value = from_drgn(row.at(index).as<std::string>());
        }
    }
};
} // namespace drgn
