#include <crudpp/bindigs/qt/utils.hpp>

namespace qt
{
// FIXME : workaround conversion from QDate to year_month_day
std::chrono::sys_days from_qdate(const QDate&& v)
{
    return std::chrono::year_month_day{std::chrono::year{v.year()},
                                       std::chrono::month{unsigned(v.month())},
                                       std::chrono::day{unsigned(v.day())}};
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

} // qt
