#ifndef JSON_HANDLER_HPP
#define JSON_HANDLER_HPP

#include "drogon_visitors/json_reader.hpp"

namespace crudpp
{
namespace visitor
{
struct json_handler
{
    explicit json_handler(bool* flag_ptr, const Json::Value& pJson)
        : flags{flag_ptr}
        , vis{.json = pJson}
    {}

    void operator()(r_c_name auto& f) noexcept
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
} // namespace visitor
} // namespace crudpp

#endif // JSON_HANDLER_HPP
