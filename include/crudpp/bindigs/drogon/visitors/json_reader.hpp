#pragma once

#include <chrono>

#include <json/value.h>

#include <crudpp/concepts.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

namespace drgn
{
struct json_reader
{
    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), bool>
    {
        f.value = json[f.c_name()].asBool();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), uint64_t>
    {
        f.value = json[f.c_name()].asLargestUInt();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires(std::unsigned_integral<decltype(f.value)> &&
                 crudpp::different<decltype(f.value), bool, uint64_t>)
    {
        f.value = json[f.c_name()].asUInt();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), int64_t>
    {
        f.value = json[f.c_name()].asLargestInt();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires(std::signed_integral<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), int64_t>)
    {
        f.value = json[f.c_name()].asInt();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), double>
    {
        f.value = json[f.c_name()].asDouble();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires(std::floating_point<decltype(f.value)> &&
                 !std::same_as<decltype(f.value), double>)
    {
        f.value = json[f.c_name()].asFloat();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::is_enum_v<decltype(f.value)>
    {
        f.value = decltype(f.value)(json[f.c_name()].asInt());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = json[f.c_name()].asString();
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::convertible_to<decltype(f.value), std::chrono::sys_days>
    {
        f.value = from_drgn_date(json[f.c_name()].asString());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_seconds>
    {
        f.value = from_drgn_time(json[f.c_name()].asString());
    }

    void operator()(crudpp::r_c_n_v auto& f) noexcept
        requires std::same_as<decltype(f.value), std::chrono::sys_time<std::chrono::milliseconds>>
    {
        f.value = from_drgn_time_ms(json[f.c_name()].asString());
    }

    const Json::Value& json;
};
} // namespace drgn
