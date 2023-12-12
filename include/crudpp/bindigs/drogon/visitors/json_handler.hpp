#pragma once

#include "json_reader.hpp"

namespace drgn
{
struct json_handler
{
    explicit json_handler(bool* flag_ptr, const Json::Value& pJson)
        : flags{flag_ptr}
        , vis{.json = pJson}
    {}

    void operator()(crudpp::r_c_name auto& f) noexcept
    {
        if (vis.json.isMember(f.c_name()))
        {
            *flags = true;
            if (!vis.json[f.c_name()].isNull())
                vis(f);
        }

        flags++;
    }

    bool* flags;
    json_reader vis;
};
} // namespace drgn
