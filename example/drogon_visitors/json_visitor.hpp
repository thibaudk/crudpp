#ifndef JSON_VISITOR_HPP
#define JSON_VISITOR_HPP

#include <json/value.h>

#include <required.hpp>

namespace crudpp
{
namespace visitor
{
struct json_reader
{
    const Json::Value& json;

    void operator()(name_and_value<int32_t> auto& f) noexcept
    {
        if (json.isMember(f.name))
        {
            if constexpr(requires { f.updated = true; })
                f.updated = true;

            if (!json[f.name].isNull())
                f.value = json[f.name].asInt64();
        }
    }

    void operator()(name_and_value<std::string> auto& f) noexcept
    {
        if (json.isMember(f.name))
        {
            if constexpr(requires { f.updated = true; })
                f.updated = true;

            if (!json[f.name].isNull())
                f.value = json[f.name].asStrig();
        }
    }
};

struct m_vector_reader
{
    const Json::Value& json;
    const std::vector<std::string>& m_vector;
    ssize_t index = 0;

    void operator()(name_and_value<int32_t> auto& f) noexcept
    {
        if (!m_vector[index].empty() && json.isMember(m_vector[index]))
        {
            if constexpr(requires { f.updated = true; })
                f.updated = true;

            if (!json[f.name].isNull())
                f.value = json[m_vector[index]].asInt64();
        }

        index++;
    }

    void operator()(name_and_value<std::string> auto& f) noexcept
    {
        if (!m_vector[index].empty() && json.isMember(m_vector[index]))
        {
            if constexpr(requires { f.updated = true; })
                f.updated = true;

            if (!json[f.name].isNull())
                f.value = json[m_vector[index]].asString();
        }

        index++;
    }
};
} // namespace visitor
} // namespace crudpp

#endif // JSON_VISITOR_HPP
