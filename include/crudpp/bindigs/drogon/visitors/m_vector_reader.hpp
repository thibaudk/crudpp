#pragma once

#include <json/value.h>

#include <crudpp/concepts.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

using namespace crudpp;

namespace drgn
{
struct m_vector_reader
{
    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), bool>
    {
        f.value = json[m_vector[index]].asBool();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), uint64_t>
    {
        f.value = json[m_vector[index]].asLargestUInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires(std::unsigned_integral<decltype(f.value)> &&
                 different_than<decltype(f.value), bool, uint64_t>)
    {
        f.value = json[m_vector[index]].asUInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int64_t>
    {
        f.value = json[m_vector[index]].asLargestInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires(std::signed_integral<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), int64_t>)
    {
        f.value = json[m_vector[index]].asInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), double>
    {
        f.value = json[m_vector[index]].asDouble();
    }

    void operator()(r_c_name auto& f) noexcept
        requires(std::floating_point<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), double>)
    {
        f.value = json[m_vector[index]].asFloat();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(json[m_vector[index]].asInt());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = json[m_vector[index]].asString();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::convertible_to<decltype(f.value), std::chrono::sys_days>
    {
        f.value = from_drgn_date(json[m_vector[index]].asString());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_seconds>
    {
        f.value = from_drgn_time(json[m_vector[index]].asString());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_time<std::chrono::milliseconds>>
    {
        f.value = from_drgn_time_ms(json[m_vector[index]].asString());
    }

    const Json::Value& json;
    const std::vector<std::string>& m_vector;
    ssize_t index{0};
};
} // namespace drgn
