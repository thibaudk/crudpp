#pragma once

#include "m_vector_reader.hpp"
#include <crudpp/required.hpp>

namespace crudpp
{
namespace visitor
{
template<typename T>
struct m_vector_handler_omit_primary
{
    explicit m_vector_handler_omit_primary(bool* flag_ptr,
                                           const Json::Value& pJson,
                                           const std::vector<std::string>& pMasqueradingVector)
        : flags{flag_ptr}
        , vis{.json = pJson, .m_vector = pMasqueradingVector}
    {}

    void operator()(r_c_name auto& f) noexcept
    {
        if constexpr(!is_primary_key<std::remove_cvref_t<decltype(f)>, T>)
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
} // namespace visitor
} // namespace crudpp
