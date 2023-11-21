#pragma once

#include "json_reader.hpp"

namespace crudpp
{
namespace visitor
{
struct json_handler
{
    explicit json_handler(const Json::Value& pJson)
        : vis{.json = pJson}
    {}

    void operator()(r_c_name auto& f) noexcept
    {
//        if constexpr ()
        if (vis.json.isMember(f.c_name()))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);
        }
    }

    json_reader vis;
};
} // namespace visitor
} // namespace crudpp
