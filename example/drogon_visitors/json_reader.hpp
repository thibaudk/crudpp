#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include <json/value.h>

#include <required.hpp>

namespace crudpp
{
namespace visitor
{
template<bool omit_primary> // true
struct json_updater
{
    void mark_updated(auto& f)
    {
        if constexpr(is_primary_key<decltype(f)>)
            return;

        if constexpr(r_updated<decltype(f)>)
            f.updated = true;
    }
};

template<>
struct json_updater<false>
{
    void mark_updated(auto& f)
    {
        if constexpr(r_updated<decltype(f)>)
            f.updated = true;
    }
};

template<bool omit_primary = false>
struct json_reader
{
    const Json::Value& json;
    json_updater<omit_primary> u{};

    void operator()(r_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        if (json.isMember(f.name))
        {
            if (!json[f.name].isNull())
                f.value = json[f.name].asInt64();

            u.mark_updated(f);
        }
    }

    void operator()(r_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        if (json.isMember(f.name))
        {
            if (!json[f.name].isNull())
                f.value = json[(std::string)f.name].as<std::string>();

            u.mark_updated(f);
        }
    }
};

template<bool omit_primary = false>
struct m_vector_reader
{
    const Json::Value& json;
    const std::vector<std::string>& m_vector;
    ssize_t index = 0;
    json_updater<omit_primary> u{};

    void operator()(r_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        if (!m_vector[index].empty() && json.isMember(m_vector[index]))
        {
            if (!json[f.name].isNull())
                f.value = json[m_vector[index]].asInt64();

            u.mark_updated(f);
        }

        index++;
    }

    void operator()(r_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        if (!m_vector[index].empty() && json.isMember(m_vector[index]))
        {
            if (!json[f.name].isNull())
                f.value = json[m_vector[index]].as<std::string>();

            u.mark_updated(f);
        }

        index++;
    }
};
} // namespace visitor
} // namespace crudpp

#endif // JSON_READER_HPP
