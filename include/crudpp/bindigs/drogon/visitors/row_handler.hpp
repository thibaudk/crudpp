#pragma once

#include "row_reader.hpp"

namespace drgn
{
struct row_handler
{
    explicit row_handler(const drogon::orm::Row& r)
        : vis{.row = r}
    {}

    void operator()(auto& f) noexcept
    {
        if (!vis.row[f.c_name()].isNull())
            vis(f);
    }

    row_reader vis;
};
} // namespace drgn
