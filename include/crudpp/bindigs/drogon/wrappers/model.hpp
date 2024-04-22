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
#include <crudpp/bindigs/drogon/visitors/json_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/json_handler_omit_primary.hpp>
#include <crudpp/bindigs/drogon/visitors/m_vector_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/m_vector_handler_omit_primary.hpp>
#include <crudpp/bindigs/drogon/visitors/row_handler.hpp>
#include <crudpp/bindigs/drogon/visitors/offset_row_handler.hpp>
#include <crudpp/bindigs/drogon/utils.hpp>

namespace drgn
{
using namespace drogon::orm;

template <crudpp::r_table T>
class model
{
public:
    static const constexpr std::string tableName = T::table();
    static const constexpr bool hasPrimaryKey = r_primary_key<T>;
    static const constexpr std::string primaryKeyName = get_primary_key_name<T>();
    using PrimaryKeyType = typename trait<T, r_primary_key<T>>::type;

    const typename internal::Traits<model<T>, r_primary_key<T>>::type getPrimaryKey() const
    {
        if constexpr(r_primary_key<T>)
            if constexpr(requires { r_value<T, PrimaryKeyType>; })
            {
                return aggregate.primary_key.value;
            }

        assert(false);
        return 0;
    }

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

    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err)
    {
        if constexpr (r_primary_key<T>)
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
                                   [&ret](const r_c_name auto& f)
                                   { ret[f.c_name()] = to_drgn(f.value); });
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
        static const std::string sql="select * from " + tableName + " where " + primaryKeyName + " = ?";
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="delete from " + tableName + " where " + primaryKeyName + " = ?";
        return sql;
    }

    std::string sqlForInserting(bool& needSelection) const
    {
        if constexpr (r_primary_key<T>) needSelection = true;

        // BUG: parametersCount increments but never evaluates true for > 0
        // user sql.size() as a workaround
        // size_t parametersCount = 0;
        std::string sql="insert into " + tableName + " (";

        using namespace boost::pfr;

        for_each_index<T>([this, &sql/*, &parametersCount*/] (const auto i)
                          {
                              auto& f{boost::pfr::get<i()>(aggregate)};

                              if constexpr(is_primary_key<decltype(f), T>)
                                  return;

                              auto& pf{boost::pfr::get<i()>(prev_agg)};

                              if (f.value == pf.value)
                                  return;

                              sql += f.c_name();
                              sql += ',';
                              // ++parametersCount;
                          });

        // if (parametersCount > 0)
        if (sql.size() > 21)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        crudpp::for_each_index<T>([this, &sql] (const auto i)
                                  {
                                      auto& f{boost::pfr::get<i()>(aggregate)};

                                      if constexpr(is_primary_key<decltype(f), T>)
                                          return;

                                      auto& pf{boost::pfr::get<i()>(prev_agg)};

                                      if (f.value == pf.value)
                                          sql.append("?,");
                                  });

        // if (parametersCount > 0)
        if(sql.size() > 31)
            sql.resize(sql.length() - 1);

        sql.append(1, ')');
        LOG_TRACE << sql;
        return sql;
    }

    static const std::vector<std::string>& insertColumns() noexcept
    {
        static std::vector<std::string> inCols;

        if (!inCols.empty())
            return inCols;

        boost::pfr::for_each_field(T{},
                                   [] (const r_c_name auto& f)
                                   { inCols.push_back(f.c_name()); });
        return inCols;
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

        crudpp::for_each_index<T>([this, &ret] (const auto i)
                                  {
                                      auto& f{boost::pfr::get<i()>(aggregate)};

                                      if constexpr(is_primary_key<decltype(f), T>)
                                          return;

                                      auto& pf{boost::pfr::get<i()>(prev_agg)};

                                      if (f.value == pf.value)
                                          ret.push_back(f.c_name());
                                  });
        return ret;
    }

    void outputArgs(internal::SqlBinder& binder) const
    {
        crudpp::for_each_index<T>([this, &binder] (const auto i)
                                  {
                                      auto& f{boost::pfr::get<i()>(aggregate)};

                                      if constexpr(is_primary_key<decltype(f), T>)
                                          return;

                                      auto& pf{boost::pfr::get<i()>(prev_agg)};

                                      if (f.value == pf.value)
                                      {
                                          if constexpr(is_foreign_key<decltype(f)>)
                                              if (!crudpp::valid_key<decltype(f)>(f))
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
        if constexpr(r_primary_key<T>)
        {
            aggregate.primary_key.value = id;
            return;
        }

        assert(false);
    }

    T aggregate{};
    T prev_agg{};
};
} // namespace drgn
