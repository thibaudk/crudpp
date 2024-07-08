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
#include <crudpp/concepts/required.hpp>
#include <crudpp/type_traits.hpp>
#include <crudpp/bindings/drogon/visitors/json_handler.hpp>
#include <crudpp/bindings/drogon/visitors/row_handler.hpp>
#include <crudpp/bindings/drogon/visitors/offset_row_handler.hpp>
#include <crudpp/bindings/drogon/utils.hpp>

using namespace drogon::orm;

namespace drgn
{
template <crudpp::r_table T>
class model
{
    using t_trait = crudpp::t_trait<T, std::string, std::vector<std::string>>;

public:
    static const constexpr std::string tableName = T::table();
    static const constexpr bool hasPrimaryKey = crudpp::r_primary_key<T>;
    static const t_trait::pk_n_type primaryKeyName;
    using PrimaryKeyType = typename t_trait::pk_v_type;

    const constexpr internal::Traits<model<T>, crudpp::r_primary_key<T>>::type getPrimaryKey() const
    { return t_trait::pk_value(aggregate); }

    explicit model(const Row& r, const ssize_t indexOffset = 0) noexcept
    {
        using namespace boost::pfr;

        if (indexOffset > 0)
        {
            if (getColumnNumber() + indexOffset > r.size())
            {
                LOG_FATAL << "Invalid SQL result for this model";
                return;
            }

            for_each_field(aggregate, offset_row_handler{r, indexOffset});
        }
        else
            for_each_field(aggregate, row_handler{r});

        prev_agg = aggregate;
    }

    explicit model(const Json::Value& pJson) noexcept(false)
    {
        boost::pfr::for_each_field(aggregate, json_handler{dirtyFlag_, pJson});
        prev_agg = aggregate;
    }

    model() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false)
    {
        boost::pfr::for_each_field(aggregate, json_handler{dirtyFlag_, pJson});
        prev_agg = aggregate;
    }

    //    TODO : re-impelement checks
    //    static bool validJsonOfField(size_t index,
    //                                 const std::string &fieldName,
    //                                 const Json::Value &pJson,
    //                                 std::string &err,
    //                                 bool isForCreation){ return true; }
    //    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err){ return true; }

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
                                   { ret[std::remove_reference_t<decltype(f)>::c_name()] = to_drgn(f.value); });
        return ret;
    }

    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const
    { return toJson(); }

    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string str = "select * from " + tableName + " where " +
                                       sql_pk_criteria(primaryKeyName);
        return str;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string str = "delete from " + tableName + " where " +
                                       sql_pk_criteria(primaryKeyName);
        return str;
    }

    std::string sqlForInserting(bool& needSelection) const
    {
        using namespace crudpp;

        if constexpr (r_primary_key<T>) needSelection = true;

        // BUG: parametersCount increments but never evaluates true for > 0
        // user sql.size() as a workaround
        // size_t parametersCount = 0;
        std::string sql = "insert into " + tableName + " (";
        std::string values = " values (";

        boost::pfr::for_each_field(aggregate,
                                   [this, &sql, &values/*, &parametersCount*/]
                                   (const r_c_name auto& f, size_t i)
                                   {
                                       if constexpr(is_single_primary_key<decltype(f), T>)
                                           return;

                                       if (!dirtyFlag_[i])
                                           return;

                                       sql += std::remove_reference_t<decltype(f)>::c_name();
                                       sql += ',';
                                       values.append("?,");
                                       // ++parametersCount;
                                   });

        // if (parametersCount > 0)
        if (sql[sql.length()-1] == ',')
        {
            sql[sql.length()-1]=')';
            values[values.length()-1]=')';
        }
        else
        {
            sql += ')';
            values += ')';
        }

        sql += values;
        LOG_TRACE << sql;
        return sql;
    }

    // FIXME : can't be a consteval for some reason
    // static consteval std::vector<std::string> insertColumns()
    static const constexpr std::vector<std::string> insertColumns()
    {
        using namespace std;
        using namespace boost;

        return [] <size_t... I> (T&& t, index_sequence<I...>)
               -> vector<string>
        { return {remove_reference_t<decltype(pfr::get<I>(t))>::c_name() ...}; }
        (T{},
         make_index_sequence<pfr::tuple_size_v<T>>{});
    }

    T& get_aggregate() { return aggregate; }
    void reset_flags()
    {
        using namespace boost::pfr;

        crudpp::for_each_index<T>([this] (const auto i)
                                  {
                                      if (dirtyFlag_[i])
                                          return;

                                      auto& f{get<i()>(aggregate)};
                                      auto& pf{get<i()>(prev_agg)};

                                      if (f.value != pf.value)
                                          dirtyFlag_[i] = true;
                                  }
                                  );
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
                                   [this, &ret] (const auto& f, size_t i)
                                   {
                                       if constexpr(crudpp::is_primary_key<decltype(f), T>)
                                           return;

                                       if (dirtyFlag_[i])
                                           ret.push_back(std::remove_reference_t<decltype(f)>::c_name());
                                   });
        return ret;
    }

    void outputArgs(internal::SqlBinder& binder) const
    {
        using namespace crudpp;

        boost::pfr::for_each_field(aggregate,
                                   [this, &binder] (const auto& f, size_t i)
                                   {
                                       if constexpr(is_primary_key<decltype(f), T>)
                                           return;

                                       if (!dirtyFlag_[i])
                                           return;

                                       if constexpr(is_foreign_key<decltype(f)>)
                                           if (!valid_key<decltype(f)>(f))
                                           {
                                               binder << nullptr;
                                               return;
                                           }

                                       binder << to_drgn(f.value);
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

    bool dirtyFlag_[boost::pfr::tuple_size<T>::value] = { false };
    T aggregate{};
    T prev_agg{};
};

template <crudpp::r_table T>
const model<T>::t_trait::pk_n_type model<T>::primaryKeyName = model<T>::t_trait::pk_name();
} // namespace drgn
