#pragma once

#include "m_vector_reader.hpp"
#include <crudpp/required.hpp>

namespace drgn
{
template<typename T>
struct m_vector_handler_omit_primary
{
    explicit m_vector_handler_omit_primary(T* agg,
                                           T* prev,
                                           const Json::Value& pJson,
                                           const std::vector<std::string>& pMasqueradingVector)
        : aggregate{agg}
        , prev_agg{prev}
        , vis{.json = pJson, .m_vector = pMasqueradingVector}
    {}

    void operator()(const auto i) noexcept
    {
        using namespace boost::pfr;

        auto& f{get<i()>(*aggregate)};

        if (!vis.m_vector[vis.index].empty() && vis.json.isMember(vis.m_vector[vis.index]))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);

            if constexpr(!crudpp::is_primary_key<decltype(f), T>)
            {
                auto& prev{get<i()>(*prev_agg)};
                prev.value = f.value;
            }
        }

        vis.index++;
    }

    T* aggregate;
    T* prev_agg;
    m_vector_reader vis;
};
} // namespace drgn
