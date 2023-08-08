#ifndef JSON_READER_HPP
#define JSON_READER_HPP

#include <json/value.h>

#include <concepts/required.hpp>

namespace crudpp
{
namespace visitor
{
struct json_reader
{
    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        f.value = json[f.c_name()].asInt64();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), std::string>
    {
        f.value = json[f.c_name()].asString();
    }

    const Json::Value& json;
};
} // namespace visitor
} // namespace crudpp

#endif // JSON_READER_HPP
