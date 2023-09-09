#pragma once

#include <string>

#include <boost/pfr/core.hpp>

#include <drogon/orm/Row.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/BaseBuilder.h>
#ifdef __cpp_impl_coroutine
#include <drogon/orm/CoroMapper.h>
#endif
#include <json/value.h>

#include <crudpp/required.hpp>
#include <crudpp/utils.hpp>
#include <crudpp/bindigs/drogon/visitors/row_reader.hpp>
#include <crudpp/bindigs/drogon/visitors/json_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/json_handler_omit_primary.hpp>
#include <crudpp/bindigs/drogon/visitors/m_vector_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/m_vector_handler_omit_primary.hpp>
#include <crudpp/bindigs/drogon/visitors/row_handler.hpp>

namespace crudpp
{
namespace wrapper
{
using namespace drogon::orm;

template <r_table T>
class model
{
public:
    static const constexpr std::string tableName = T::table();
    static const constexpr bool hasPrimaryKey = has_primary_key<T>;
    static const constexpr std::string primaryKeyName = get_primary_key_name<T>();
    using PrimaryKeyType = typename trait<T, has_primary_key<T>>::type;

    const typename internal::Traits<model<T>, has_primary_key<T>>::type getPrimaryKey() const
    {
        if constexpr(has_primary_key<T>)
            if constexpr(requires { r_value<T, PrimaryKeyType>; })
            {
                const auto id = aggregate.*T::primary_key();
                return id.value;
            }

        assert(false);
        return 0;
    }

    explicit model(const Row& r, const ssize_t indexOffset = 0) noexcept
    {
        using namespace boost::pfr;

        if(indexOffset < 0)
            for_each_field(aggregate, visitor::row_reader{.row = r});
        else
        {
            if(getColumnNumber() + indexOffset > r.size())
            {
                LOG_FATAL << "Invalid SQL result for this model";
                return;
            }

            for_each_field(aggregate, visitor::row_handler{r, indexOffset});
        }
    }

    explicit model(const Json::Value& pJson) noexcept(false)
    {
        using namespace visitor;
        boost::pfr::for_each_field(aggregate, json_handler{dirtyFlag_, pJson});
    }

    model(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
    {
        if(pMasqueradingVector.size() != getColumnNumber())
        {
            LOG_ERROR << "Bad masquerading vector";
            return;
        }

        boost::pfr::for_each_field(aggregate,
                                   visitor::m_vector_handler{dirtyFlag_, pJson, pMasqueradingVector});
    }

    model() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false)
    {
        boost::pfr::for_each_field(aggregate, visitor::json_handler_omit_primary<T>{dirtyFlag_, pJson});
    }

    void updateByMasqueradedJson(const Json::Value &pJson,
                                 const std::vector<std::string> &pMasqueradingVector) noexcept(false)
    {
        if(pMasqueradingVector.size() != getColumnNumber())
        {
            LOG_ERROR << "Bad masquerading vector";
            return;
        }

        boost::pfr::for_each_field(aggregate,
                                   visitor::m_vector_handler_omit_primary<T>{dirtyFlag_,
                                                                             pJson,
                                                                             pMasqueradingVector});
    }

//    TODO : re-impelement checks
//    static bool validJsonOfField(size_t index,
//                                 const std::string &fieldName,
//                                 const Json::Value &pJson,
//                                 std::string &err,
//                                 bool isForCreation){ return true; }
//    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err){ return true; }
//    static bool validateMasqueradedJsonForCreation(const Json::Value &,
//                                                   const std::vector<std::string> &pMasqueradingVector,
//                                                   std::string &err){ return true; }
//    static bool validateMasqueradedJsonForUpdate(const Json::Value & pJson,
//                                                 const std::vector<std::string> &pMasqueradingVector,
//                                                 std::string &err){ return true; }

    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err)
    {
        if constexpr (has_primary_key<T>)
        {
            if (!pJson.isMember(primaryKeyName))
            {
                err = "The value of primary key must be set in the json object for update";
                return false;
            }

            // TODO : check primary key type

            if (pJson.size() < 2)
            {
                err = "No values to update";
                return false;
            }
        }
        else
            if (pJson.empty())
            {
                err = "No values to update";
                return false;
            }

        return true;
    }

    static size_t getColumnNumber() noexcept { return boost::pfr::tuple_size<T>::value; }

    Json::Value toJson() const
    {
        Json::Value ret;

        boost::pfr::for_each_field(aggregate,
                                   [&ret](const r_c_name auto& f) { ret[f.c_name()] = f.value; });
        return ret;
    }

    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const
    {
        Json::Value ret;

        if(pMasqueradingVector.size() == getColumnNumber())
        {
            boost::pfr::for_each_field(aggregate,
                                       [&ret, &pMasqueradingVector](const r_c_name auto& f, size_t i)
                                       {
                                           if(!pMasqueradingVector[i].empty())
                                               ret[pMasqueradingVector[i]] = f.value;
                                       });
            return ret;
        }

        LOG_ERROR << "Masquerade failed";
        return toJson();
    }

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

    constexpr std::string sqlForInserting(bool& needSelection) const
    {
        std::string sql="insert into " + tableName + " (";
        size_t parametersCount = 0;
        needSelection = false;

        using namespace boost::pfr;

        for_each_field(aggregate,
                       [this, &sql, &parametersCount](const r_c_name auto& f, size_t i)
                       {
                           if constexpr(!is_primary_key<decltype(f), T>)
                               if (!dirtyFlag_[i]) return;

                           sql += f.c_name();
                           sql += ',';
                           ++parametersCount;
                       });

        if constexpr (has_primary_key<T>)
            needSelection = true;

        if(parametersCount > 0)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        for_each_field(aggregate,
                       [this, &sql](const auto& f, size_t i)
                       {
                           if constexpr(is_primary_key<decltype(f), T>)
                               sql +="default,";

                           if (dirtyFlag_[i])
                               sql.append("?,");
                       });

        if(parametersCount > 0)
            sql.resize(sql.length() - 1);

        sql.append(1, ')');
        LOG_TRACE << sql;
        return sql;
    }

    T& get_aggregate() { return aggregate; }

    static const std::vector<std::string>& insertColumns() noexcept
    {
        static std::vector<std::string> inCols;

        if (!inCols.empty())
            return inCols;

        boost::pfr::for_each_field(T{},
                                   [](const r_c_name auto& f)
                                   { inCols.push_back(f.c_name()); });
        return inCols;
    }

private:
    friend Mapper<model<T>>;
    friend BaseBuilder<model<T>, true, true>;
    friend BaseBuilder<model<T>, true, false>;
    friend BaseBuilder<model<T>, false, true>;
    friend BaseBuilder<model<T>, false, false>;
#ifdef __cpp_impl_coroutine
    friend CoroMapper<model<T>>;
#endif

    const std::vector<std::string> updateColumns() const
    {
        std::vector<std::string> ret;

        boost::pfr::for_each_field(aggregate,
                                   [this, &ret](r_c_name auto& f, size_t i)
                                   {
                                       if constexpr(is_primary_key<decltype(f), T>)
                                           return;

                                       if (dirtyFlag_[i])
                                           ret.push_back(f.c_name());
                                   });
        return ret;
    }

    void outputArgs(internal::SqlBinder& binder) const
    {
        boost::pfr::for_each_field(aggregate,
                                   [this, &binder](auto& f, size_t i)
                                   {
                                       if constexpr(is_primary_key<decltype(f), T>)
                                           return;

                                       if (dirtyFlag_[i])
                                           binder << f.value;
                                   });
    }

    void updateArgs(internal::SqlBinder& binder) const { outputArgs(binder); }

    ///For mysql or sqlite3
    void updateId(const uint64_t id)
    {
        if constexpr(has_primary_key<T>)
        {
            aggregate.id.value = id;
            return;
        }

        assert(false);
    }

    bool dirtyFlag_[boost::pfr::tuple_size<T>::value] = { false };
    T aggregate{};
};
} // namespace wrapper
} // namespace crudpp
