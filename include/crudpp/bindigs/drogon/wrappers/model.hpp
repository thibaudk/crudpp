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

#include <crudpp/utils.hpp>
#include <crudpp/concepts.hpp>
#include <crudpp/type_traits.hpp>
#include <crudpp/bindigs/drogon/visitors/json_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/json_handler_omit_primary.hpp>
#include <crudpp/bindigs/drogon/visitors/m_vector_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/m_vector_handler_omit_primary.hpp>
#include <crudpp/bindigs/drogon/visitors/row_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/offset_row_handler.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

using namespace drogon::orm;

namespace drgn
{
template <crudpp::r_table T>
class model
{
public:
    static const constexpr std::string tableName = T::table();
    static const constexpr bool hasPrimaryKey = crudpp::r_primary_key<T>;
    static const crudpp::t_trait<T>::pk_n_type primaryKeyName;
    using PrimaryKeyType = typename crudpp::t_trait<T>::pk_v_type;

    const internal::Traits<model<T>, crudpp::r_primary_key<T>>::type getPrimaryKey() const
    { return crudpp::t_trait<T>::pk_value(aggregate); }

    explicit model(const Row& r, const ssize_t indexOffset = 0) noexcept
    {
        using namespace boost::pfr;

        if (indexOffset < 0)
            for_each_field(aggregate, row_handler{r});
        else
        {
            if (getColumnNumber() + indexOffset > r.size())
            {
                LOG_FATAL << "Invalid SQL result for this model";
                return;
            }

            for_each_field(aggregate, offset_row_handler{r, indexOffset});
        }
    }

    explicit model(const Json::Value& pJson) noexcept(false)
    {
        crudpp::for_each_index<T>(json_handler{&aggregate, pJson});
    }

    model(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
    {
        if(pMasqueradingVector.size() != getColumnNumber())
        {
            LOG_ERROR << "Bad masquerading vector";
            return;
        }

        crudpp::for_each_index<T>(m_vector_handler{&aggregate, pJson, pMasqueradingVector});
    }

    model() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false)
    {
        // TODO: verify if the distinction with simple json_handler is waranted
        crudpp::for_each_index<T>(json_handler_omit_primary{&aggregate,
                                                            &prev_agg,
                                                            pJson});
    }

    void updateByMasqueradedJson(const Json::Value &pJson,
                                 const std::vector<std::string> &pMasqueradingVector) noexcept(false)
    {
        if(pMasqueradingVector.size() != getColumnNumber())
        {
            LOG_ERROR << "Bad masquerading vector";
            return;
        }

        crudpp::for_each_index<T>(m_vector_handler_omit_primary<T>{&aggregate,
                                                                   &prev_agg,
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
    // static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err)
    // {
    //     if constexpr (r_primary_key<T>)
    //     {
    //         if (!pJson.isMember(primaryKeyName))
    //         {
    //             err = "The value of primary key must be set in the json object for update";
    //             return false;
    //         }

    //         // TODO : check primary key type

    //         if (pJson.size() < 2)
    //         {
    //             err = "No values to update";
    //             return false;
    //         }
    //     }
    //     else
    //         if (pJson.empty())
    //         {
    //             err = "No values to update";
    //             return false;
    //         }

    //     return true;
    // }

    static size_t getColumnNumber() noexcept { return boost::pfr::tuple_size<T>::value; }

    Json::Value toJson() const
    {
        Json::Value ret;

        boost::pfr::for_each_field(aggregate,
                                   [&ret](const crudpp::r_c_n_v auto& f)
                                   { ret[f.c_name()] = to_drgn(f.value); });
        return ret;
    }

    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const
    {
        Json::Value ret;

        if(pMasqueradingVector.size() == getColumnNumber())
        {
            boost::pfr::for_each_field(aggregate,
                                       [&ret, &pMasqueradingVector]
                                       (const crudpp::r_c_n_v auto& f, size_t i)
                                       {
                                           if (!pMasqueradingVector[i].empty())
                                                   ret[pMasqueradingVector[i]] = to_drgn(f.value);
                                       });
            return ret;
        }

        LOG_ERROR << "Masquerade failed";
        return toJson();
    }

    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string sql="select * from " + tableName + " where " +
                                       sql_pk_criteria(primaryKeyName);
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="delete from " + tableName + " where " +
                                       sql_pk_criteria(primaryKeyName);
        return sql;
    }

    std::string sqlForInserting(bool& needSelection) const
    {
        using namespace crudpp;

        if constexpr (r_primary_key<T>) needSelection = true;

        // BUG: parametersCount increments but never evaluates true for > 0
        // user sql.size() as a workaround
        // size_t parametersCount = 0;
        std::string sql="insert into " + tableName + " (";

        using namespace boost::pfr;

        for_each_index<T>([this, &sql/*, &parametersCount*/] (const auto i)
                          {
                              auto& f{get<i()>(aggregate)};

                              if constexpr(is_single_primary_key<decltype(f), T>)
                                  return;

                              auto& pf{get<i()>(prev_agg)};

                              if (f.value == pf.value)
                                  return;

                              sql += f.c_name();
                              sql += ',';
                              // ++parametersCount;
                          });

        // if (parametersCount > 0)
        if (sql.size() > 21) // initial length
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        for_each_index<T>([this, &sql] (const auto i)
                          {
                              auto& f{get<i()>(aggregate)};

                              if constexpr(is_single_primary_key<decltype(f), T>)
                                  return;

                              auto& pf{get<i()>(prev_agg)};

                              if (f.value == pf.value)
                                  sql.append("?,");
                          });

        // if (parametersCount > 0)
        if(sql.size() > 31) // initial length + ") valuees ("
            sql.resize(sql.length() - 1);

        sql.append(1, ')');
        LOG_TRACE << sql;
        return sql;
    }

    static const constexpr std::vector<std::string> insertColumns()
    {
        using namespace std;
        using namespace boost;

        return [] <size_t... I> (T&& t, index_sequence<I...>)
               -> vector<string>
        { return {pfr::get<I>(t).c_name() ...}; }
               (T{},
                make_index_sequence<pfr::tuple_size_v<T>>{});
    }

    T& get_aggregate() { return aggregate; }

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

        using namespace boost::pfr;

        crudpp::for_each_index<T>([this, &ret] (const auto i)
                                  {
                                      auto& f{get<i()>(aggregate)};

                                      if constexpr(crudpp::is_primary_key<decltype(f), T>)
                                          return;

                                      auto& pf{get<i()>(prev_agg)};

                                      if (f.value == pf.value)
                                          ret.push_back(f.c_name());
                                  });
        return ret;
    }

    void outputArgs(internal::SqlBinder& binder) const
    {
        using namespace boost::pfr;
        using namespace crudpp;

        for_each_index<T>([this, &binder] (const auto i)
                          {
                              auto& f{get<i()>(aggregate)};

                              if constexpr(is_primary_key<decltype(f), T>)
                                  return;

                              auto& pf{get<i()>(prev_agg)};

                              if (f.value == pf.value)
                              {
                                  if constexpr(is_foreign_key<decltype(f)>)
                                      if (!valid_key<decltype(f)>(f))
                                      {
                                          binder << nullptr;
                                          return;
                                      }

                                  binder << to_drgn(f.value);
                              }
                          });
    }

    void updateArgs(internal::SqlBinder& binder) const { outputArgs(binder); }

    ///For mysql or sqlite3
    void updateId(const uint64_t id)
    {
        if constexpr(crudpp::r_single_primary_key<T>)
            (aggregate.*T::primary_key()).value = id;

        return; // VERIFY: composite keys never auto increment ?
        // assert(false);
    }

    T aggregate{};
    T prev_agg{};
};

template <crudpp::r_table T>
const crudpp::t_trait<T>::pk_n_type model<T>::primaryKeyName = crudpp::t_trait<T>::pk_name();
} // namespace drgn
