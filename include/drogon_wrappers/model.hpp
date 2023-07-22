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
#include <json/value.h>

#include <concepts/required.hpp>
#include <drogon_visitors/row_reader.hpp>
#include <drogon_visitors/json_handler.hpp>
#include <drogon_visitors/m_vector_handler.hpp>

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

template <r_table T>
class model
{
    static const constexpr std::string get_primary_key_name()
    {
        if constexpr(has_primary_key<T>)
            if constexpr(requires { r_c_name<typename T::id>; })
                return T::id::c_name();

        return "";
    }

public:
    static const constexpr std::string tableName = T::table();
    static const constexpr bool hasPrimaryKey = has_primary_key<T>;
    static const constexpr std::string primaryKeyName = get_primary_key_name();
    using PrimaryKeyType = typename internal::trait<T, has_primary_key<T>>::type;

    const PrimaryKeyType getPrimaryKey() const
    {
        if constexpr(has_primary_key<T>)
            return aggregate.id.value;

        assert(false);
        return 0;
    }

    explicit model(const drogon::orm::Row& r, const ssize_t indexOffset = 0) noexcept
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

            for_each_field(aggregate, visitor::offset_row_reader{.row = r, .offset = indexOffset});
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
        boost::pfr::for_each_field(aggregate, visitor::json_handler<true>{dirtyFlag_, pJson});
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
                                   visitor::m_vector_handler<true>{dirtyFlag_,
                                                                   pJson,
                                                                   pMasqueradingVector});
    }

//    TODO : re-impelement checks
    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err){ return true; }
    static bool validateMasqueradedJsonForCreation(const Json::Value &,
                                                   const std::vector<std::string> &pMasqueradingVector,
                                                   std::string &err){ return true; }
    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err){ return true; }
    static bool validateMasqueradedJsonForUpdate(const Json::Value &,
                                                 const std::vector<std::string> &pMasqueradingVector,
                                                 std::string &err){ return true; }
    static bool validJsonOfField(size_t index,
                                 const std::string &fieldName,
                                 const Json::Value &pJson,
                                 std::string &err,
                                 bool isForCreation){ return true; }

    static size_t getColumnNumber() noexcept { return boost::pfr::tuple_size<T>::value; }

    Json::Value toJson() const
    {
        Json::Value ret;

        boost::pfr::for_each_field(aggregate,
                                   [&ret](r_c_name auto& f) { ret[f.c_name()] = f.value; });
        return ret;
    }

    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const
    {
        Json::Value ret;

        if(pMasqueradingVector.size() == getColumnNumber())
        {
            boost::pfr::for_each_field(aggregate,
                                       [&ret, &pMasqueradingVector](r_c_name auto& f, size_t i)
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
                       [this, &sql, &parametersCount](r_c_name auto& f, size_t i)
                       {
                           if constexpr(is_primary_key<decltype(f)>)
                               return;

                           if (dirtyFlag_[i])
                           {
                               sql += f.c_name();
                               ++parametersCount;
                           }

                       });

        needSelection = true;

        if(parametersCount > 0)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        sql +="default,";

        for_each_field(aggregate,
                       [this, &sql](auto& f, size_t i)
                       {
                           if constexpr(is_primary_key<decltype(f)>)
                               return;

                           if (dirtyFlag_[i])
                               sql.append("?,");
                       });

        if(parametersCount > 0)
            sql.resize(sql.length() - 1);

        sql.append(1, ')');
        LOG_TRACE << sql;
        return sql;
    }

    T& get_aggregate() { return aggregate; };

private:
    friend drogon::orm::Mapper<model<T>>;
    friend drogon::orm::BaseBuilder<model<T>, true, true>;
    friend drogon::orm::BaseBuilder<model<T>, true, false>;
    friend drogon::orm::BaseBuilder<model<T>, false, true>;
    friend drogon::orm::BaseBuilder<model<T>, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<model<T>>;
#endif
    static const std::vector<std::string>& insertColumns() noexcept
    {
        static std::vector<std::string> inCols;
        boost::pfr::for_each_field(T{},
                                   [](r_c_name auto& f)
                                   { inCols.push_back(f.c_name()); });
        return inCols;
    }

    const std::vector<std::string> updateColumns() const
    {
        std::vector<std::string> ret;

        boost::pfr::for_each_field(aggregate,
                                   [this, &ret](r_c_name auto& f, size_t i)
                                   {
                                       if constexpr(is_primary_key<decltype(f)>)
                                           return;

                                       if (dirtyFlag_[i])
                                           ret.push_back(f.c_name());
                                   });
        return ret;
    }

    void outputArgs(drogon::orm::internal::SqlBinder& binder) const
    {
        boost::pfr::for_each_field(aggregate,
                                   [this, &binder](auto& f, size_t i)
                                   {
                                       if constexpr(is_primary_key<decltype(f)>)
                                           return;

                                       if (dirtyFlag_[i])
                                           binder << f.value;
                                   });
    }

    void updateArgs(drogon::orm::internal::SqlBinder& binder) const { outputArgs(binder); }

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

#endif // MODEL_WRAPPER_HPP
