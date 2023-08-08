#ifndef ROW_HANDLER_HPP
#define ROW_HANDLER_HPP

#include "row_reader.hpp"

namespace crudpp
{
namespace visitor
{
struct row_handler
{
    explicit row_handler(const drogon::orm::Row& r, const ssize_t indexOffset)
        : vis{.row = r, .offset = indexOffset}
    {}

    void operator()(auto& f) noexcept
    {
        vis(f);
        vis.index++;
    }

    offset_row_reader vis;
};
} // namespace visitor
} // namespace crudpp
#endif // ROW_HANDLER_HPP
