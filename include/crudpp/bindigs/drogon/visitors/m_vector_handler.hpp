#pragma once

#include "m_vector_reader.hpp"

namespace drgn
{
template <typename T>
struct m_vector_handler
{
    explicit m_vector_handler(T* agg,
                              const Json::Value& pJson,
                              const std::vector<std::string>& pMasqueradingVector)
        : aggregate{agg}
        , vis{.json = pJson, .m_vector = pMasqueradingVector}
    {}

    void operator()(const auto i) noexcept
    {
        auto& f{boost::pfr::get<i()>(*aggregate)};

        if (!vis.m_vector[vis.index].empty() && vis.json.isMember(vis.m_vector[vis.index]))
        {
            if (!vis.json[f.c_name()].isNull())
                vis(f);
        }

        vis.index++;
    }

    T* aggregate;
    m_vector_reader vis;
};
} // namespace drgn
