#pragma once

#include <chrono>

#include <json/value.h>

#include <crudpp/required.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

namespace drgn
{
using namespace crudpp;

struct json_reader
{
    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), bool>
    {
        f.value = json[f.c_name()].asBool();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), uint64_t>
    {
        f.value = json[f.c_name()].asLargestUInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires(std::unsigned_integral<decltype(f.value)> &&
                 different_than<decltype(f.value), bool, uint64_t>)
    {
        f.value = json[f.c_name()].asUInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int64_t>
    {
        f.value = json[f.c_name()].asLargestInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires(std::signed_integral<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), int64_t>)
    {
        f.value = json[f.c_name()].asInt();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), double>
    {
        f.value = json[f.c_name()].asDouble();
    }

    void operator()(r_c_name auto& f) noexcept
        requires(std::floating_point<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), double>)
    {
        f.value = json[f.c_name()].asFloat();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(json[f.c_name()].asInt());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = json[f.c_name()].asString();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::convertible_to<decltype(f.value), std::chrono::sys_days>
    {
        f.value = from_drgn_date(json[f.c_name()].asString());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_seconds>
    {
        f.value = from_drgn_time(json[f.c_name()].asString());
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_time<std::chrono::milliseconds>>
    {
        f.value = from_drgn_time_ms(json[f.c_name()].asString());
    }

    const Json::Value& json;
};
} // namespace drgn
