#pragma once

#include "json_reader.hpp"

namespace qt
{
struct json_handler
{
    explicit json_handler(const QJsonObject& pJson)
        : vis{.json = pJson}
    {}

    void operator()(crudpp::r_c_name auto& f) noexcept
    {
        if (vis.json.contains(f.c_name()))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);
        }
    }

    json_reader vis;
};
} // namespace qt
