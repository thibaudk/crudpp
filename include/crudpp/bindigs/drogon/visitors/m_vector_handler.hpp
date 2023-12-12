#pragma once

#include "m_vector_reader.hpp"

namespace drgn
{
struct m_vector_handler
{
    explicit m_vector_handler(bool* flag_ptr,
                              const Json::Value& pJson,
                              const std::vector<std::string>& pMasqueradingVector)
        : flags{flag_ptr}
        , vis{.json = pJson, .m_vector = pMasqueradingVector}
    {}

    void operator()(crudpp::r_c_name auto& f) noexcept
    {
        if (!vis.m_vector[vis.index].empty() && vis.json.isMember(vis.m_vector[vis.index]))
        {
            *flags = true;
            if (!vis.json[f.c_name()].isNull())
                vis(f);
        }

        flags++;
        vis.index++;
    }

    bool* flags;
    m_vector_reader vis;
};
} // namespace drgn
