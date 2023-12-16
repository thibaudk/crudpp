#pragma once

#include "offset_row_reader.hpp"

namespace drgn
{
struct offset_row_handler
{
    explicit offset_row_handler(const drogon::orm::Row& r, const ssize_t indexOffset)
        : vis{.row = r, .offset = indexOffset}
    {}

    void operator()(auto& f) noexcept
    {
        if (!vis.row.at(vis.index).isNull())
            vis(f);

        vis.index++;
    }

    offset_row_reader vis;
};
} // namespace drgn
