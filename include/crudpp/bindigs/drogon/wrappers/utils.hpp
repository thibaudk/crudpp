#pragma once

#include <format>

#include <json/value.h>
#include <trantor/utils/Date.h>

namespace crudpp
{
// FIXME : workaround conversion from string to year_month_day

std::chrono::year_month_day from_drgn(const std::string&& str)
{
    const auto date{trantor::Date::fromDbStringLocal(str).tmStruct()};
    return {std::chrono::year{date.tm_year + 1900},
            std::chrono::month{unsigned(date.tm_mon + 1)},
            std::chrono::day{unsigned(date.tm_mday)}};
}

template <typename T>
T to_drgn(const T& v) { return v; }

std::string to_drgn(const std::chrono::year_month_day& d) { return std::format("%Y-%m-%d", d); }
} // namespace crudpp
