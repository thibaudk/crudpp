#pragma once

#include <json/value.h>

#include <crudpp/required.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

namespace drgn
{
using namespace crudpp;

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
                 is_different<decltype(f.value), bool, uint64_t>)
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
        requires std::same_as<decltype(f.value), std::chrono::year_month_day>
    {
        f.value = from_drgn(json[f.c_name()].asString());
    }

    const Json::Value& json;
    const std::vector<std::string>& m_vector;
    ssize_t index{0};
};
} // namespace drgn
