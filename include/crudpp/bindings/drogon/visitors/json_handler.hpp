#pragma once

#include "json_reader.hpp"

namespace drgn
{
template <typename T>
struct json_handler
{
    explicit json_handler(T* agg, const Json::Value& pJson)
        : aggregate{agg}
        , vis{.json = pJson}
    {}

    void operator()(const auto i) noexcept
    {
        auto& f{boost::pfr::get<i()>(*aggregate)};

        if (vis.json.isMember(f.c_name()))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);
        }
    }

    T* aggregate;
    json_reader vis;
};
} // namespace drgn
