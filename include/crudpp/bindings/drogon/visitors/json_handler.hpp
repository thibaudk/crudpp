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
        const auto name{std::remove_reference_t<decltype(f)>::c_name()};

        if (vis.json.isMember(name))
        {
            *flags = true;
            if (!vis.json[name].isNull())
                vis(f);
        }

        flags++;
    }

    bool* flags;
    json_reader vis;
};
} // namespace drgn
