#ifndef ROW_READER_HPP
#define ROW_READER_HPP

#include <drogon/orm/Field.h>
#include <drogon/orm/Row.h>

#include <concepts/required.hpp>

namespace crudpp
{
namespace visitor
{
struct row_reader
{
    const drogon::orm::Row& row;

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        if (!row[f.c_name()].isNull())
            f.value = row[f.c_name()].template as<int32_t>();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int8_t>
    {
        if (!row[f.c_name()].isNull())
            f.value = row[f.c_name()].template as<int8_t>();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        if (!row[f.c_name()].isNull())
            f.value = decltype(f.value)(row[f.c_name()].template as<int>());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        if (!row[f.c_name()].isNull())
            f.value = row[f.c_name()].template as<std::string>();
    }
};

struct offset_row_reader
{
    const drogon::orm::Row& row;
    const ssize_t offset;
    ssize_t index = offset;

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        if (!row.at(index).isNull())
            f.value = row.at(index).as<int32_t>();
    }

    void operator()(auto& f) noexcept
        requires std::same_as<decltype(f.value), int8_t>
    {
        if (!row.at(index).isNull())
            f.value = row.at(index).as<int8_t>();
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
};
} // namespace visitor
} // namespace crudpp

#endif // ROW_READER_HPP
