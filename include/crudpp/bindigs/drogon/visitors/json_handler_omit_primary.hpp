#pragma once

#include "json_reader.hpp"
#include <crudpp/required.hpp>

namespace drgn
{
template<typename T>
struct json_handler_omit_primary
{
    explicit json_handler_omit_primary(T* agg, T* prev, const Json::Value& pJson)
        : aggregate{agg}
        , prev_agg{prev}
        , vis{.json = pJson}
    {}

    void operator()(const auto i) noexcept
    {
        using namespace boost::pfr;

        auto& f{get<i()>(*aggregate)};

        if (vis.json.isMember(f.c_name()))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);

            if constexpr(!crudpp::is_primary_key<decltype(f), T>)
            {
                auto& prev{get<i()>(*prev_agg)};
                prev.value = f.value;
            }
        }
    }

    T* aggregate;
    T* prev_agg;
    json_reader vis;
};
} // namespace drgn
