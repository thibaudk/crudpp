#include <crudpp/bindings/qt/utils.hpp>

namespace qt
{
QDateTime to_qt(const std::chrono::sys_seconds& v)
{
    return QDateTime::fromSecsSinceEpoch(std::chrono::duration_cast<std::chrono::seconds>(
                                             v.time_since_epoch()).count());
}

QDateTime to_qt(const std::chrono::sys_time<std::chrono::milliseconds>& v)
{
    return QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(
                                              v.time_since_epoch()).count());
}

QJsonValue to_qjson(const QDate&& v) { return v.toString(Qt::ISODate); }
QJsonValue to_qjson(const QDateTime&& v) { return v.toString(Qt::ISODateWithMs); }

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
