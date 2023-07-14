#ifndef MODEL_WRAPPER_HPP
#define MODEL_WRAPPER_HPP

#include <string>

#include <boost/pfr/core.hpp>

#include <drogon/orm/Row.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/BaseBuilder.h>
#ifdef __cpp_impl_coroutine
#include <drogon/orm/CoroMapper.h>
#endif
#include <drogon/orm/Mapper.h>
#include <json/value.h>

#include <required.hpp>
#include <json_visitor.hpp>
#include <row_visitor.hpp>

namespace crudpp
{
namespace wrapper
{
namespace internal // adapted from drogon's Mapper.h
{
template <typename T, bool hasPrimaryKey = true>
struct trait
{
    using type = decltype(T::id.value);
};
template <typename T>
struct trait<T, false>
{
    using type = void;
};
} // namespace intenal

template <typename T>
class model
{
public:
    static const constexpr std::string tableName = T::table;
    static const constexpr bool hasPrimaryKey = has_primary_key<T>;
    static const constexpr std::string primaryKeyName = has_primary_key<T> ? T::id::name : "";

    using PrimaryKeyType = typename internal::trait<T, has_primary_key<T>>::type;

    const PrimaryKeyType& getPrimaryKey() const
    {
        if (has_primary_key<T>)
            return instance.id.value();
        else
        {
            assert(false);
            return 0;
        }
    }

    explicit model(const drogon::orm::Row& r, const ssize_t indexOffset = 0) noexcept
    {
        using namespace boost::pfr;

        if(indexOffset < 0)
            for_each_field(instance, visitor::row_reader{.row = r});
        else
        {
            if(getColumnNumber() + indexOffset > r.size())
            {
                LOG_FATAL << "Invalid SQL result for this model";
                return;
            }

            for_each_field(instance, visitor::offset_row_reader{.row = r, .offset = indexOffset});
        }
    }

    explicit model(const Json::Value& pJson) noexcept(false)
    {
        boost::pfr::for_each_field(instance, crudpp::visitor::json_reader{.json = pJson});
    }

    model(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
    {
        if(pMasqueradingVector.size() != getColumnNumber())
        {
            LOG_ERROR << "Bad masquerading vector";
            return;
        }

        boost::pfr::for_each_field(instance,
                                   visitor::m_vector_reader{.json = pJson,
                                                            .m_vector = pMasqueradingVector});
    }

    model() = default;

    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string sql="select * from " + tableName + " where id = ?";
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="delete from " + tableName + " where id = ?";
        return sql;
    }

    static size_t getColumnNumber() noexcept { return boost::pfr::tuple_size<T>::value; }

    T& get_instance() { return instance; };

private:
    friend drogon::orm::Mapper<model<T>>;
    friend drogon::orm::BaseBuilder<model<T>, true, true>;
    friend drogon::orm::BaseBuilder<model<T>, true, false>;
    friend drogon::orm::BaseBuilder<model<T>, false, true>;
    friend drogon::orm::BaseBuilder<model<T>, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<model<T>>;
#endif

    T instance{};
};
} // namespace wrapper
} // namespace crudpp

#endif // MODEL_WRAPPER_HPP
