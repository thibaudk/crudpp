#ifndef JSON_HANDLER_OMIT_PRIMARY_HPP
#define JSON_HANDLER_OMIT_PRIMARY_HPP

#include "drogon_visitors/json_reader.hpp"
#include <concepts/required.hpp>

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
        if constexpr(!is_primary_key<decltype(f), T>)
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

#endif // JSON_HANDLER_OMIT_PRIMARY_HPP
