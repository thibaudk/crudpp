#ifndef ROW_VISITOR_HPP
#define ROW_VISITOR_HPP

#include <drogon/orm/Field.h>
#include <drogon/orm/Row.h>

#include <required.hpp>

namespace crudpp
{
namespace visitor
{
struct row_reader
{
    const drogon::orm::Row& row;

    void operator()(name_and_value<int32_t> auto& f) noexcept
    {
        if (row[f.name].isNull()) return;
        f.value = row[f.name].asInt64();
    }

    void operator()(name_and_value<std::string> auto& f) noexcept
    {
        if (row[f.name].isNull()) return;
        f.value = row[f.name].asStrig();
    }

};

struct offset_row_reader
{
    const drogon::orm::Row& row;
    const ssize_t offset;
    ssize_t index = offset;

    void operator()(name_and_value<int32_t> auto& f) noexcept
    {
        if (!row.at(index).isNull())
            f.value = row.at(index).as<int32_t>();

        index++;
    }

    void operator()(name_and_value<std::string> auto& f) noexcept
    {
        if (!row.at(index).isNull())
            f.value = row.at(index).as<std::string>();

        index++;
    }
};
} // namespace visitor
} // namespace crudpp

#endif // ROW_VISITOR_HPP
