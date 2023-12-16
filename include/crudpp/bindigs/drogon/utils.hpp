#pragma once

#include <format>

#include <json/value.h>
#include <trantor/utils/Date.h>

namespace drgn
{
// FIXME : workaround conversion from string to date

std::chrono::year_month_day from_drgn_date(const std::string&& str)
{
    const auto date{trantor::Date::fromDbStringLocal(str).tmStruct()};
    return {std::chrono::year{date.tm_year + 1900},
            std::chrono::month{unsigned(date.tm_mon + 1)},
            std::chrono::day{unsigned(date.tm_mday)}};
}

std::chrono::time_point<std::chrono::system_clock> from_drgn_time(const std::string&& str)
{
    auto t{trantor::Date::fromDbStringLocal(str).tmStruct()};
    return std::chrono::system_clock::from_time_t(mktime(&t));
}

template <typename T>
T to_drgn(const T& v) { return v; }

std::string to_drgn(const std::chrono::year_month_day& d)
{ return std::format("%Y-%m-%d", d); }

std::string to_drgn(const std::chrono::time_point<std::chrono::system_clock>& d)
{ return std::format("%Y-%m-%d %H:%M:%S", d); }
} // namespace drgn
