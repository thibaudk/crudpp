#ifndef M_VECTOR_READER_HPP
#define M_VECTOR_READER_HPP

#include <json/value.h>

#include <concepts/required.hpp>

namespace crudpp
{
namespace visitor
{
struct m_vector_reader
{
    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int32_t>
    {
        f.value = json[m_vector[index]].asInt64();
    }

    void operator()(r_c_name auto& f) noexcept
        requires std::same_as<decltype(f.value), int8_t>
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

    const Json::Value& json;
    const std::vector<std::string>& m_vector;
    ssize_t index{0};
};
} // namespace visitor
} // namespace crudpp

#endif // M_VECTOR_READER_HPP
