#pragma once

#include "json_reader.hpp"
#include <crudpp/required.hpp>

namespace crudpp
{
namespace visitor
{
template<typename T>
struct json_handler_omit_primary
{
    explicit json_handler_omit_primary(bool* flag_ptr, const Json::Value& pJson)
        : flags{flag_ptr}
        , vis{.json = pJson}
    {}

    void operator()(r_c_name auto& f) noexcept
    {
        if (vis.json.isMember(f.c_name()))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);

            if constexpr(!is_primary_key<decltype(f), T>)
                *flags = true;
        }

        flags++;
    }

    bool* flags;
    json_reader vis;
};
} // namespace visitor
} // namespace crudpp
