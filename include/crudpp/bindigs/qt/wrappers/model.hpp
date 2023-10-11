#pragma once

#include <QJsonObject>
#include "qnamespace.h"

#include <boost/pfr/core.hpp>

#include <crudpp/required.hpp>
#include <crudpp/utils.hpp>

#include "utils.hpp"
#include <crudpp/bindigs/qt/visitors/json_reader.hpp>

namespace crudpp
{
template <typename T>
struct model final
{
    static const constexpr auto table() { return T::table(); }

    model(const QJsonObject& json) { read(json); }
    model() = default;

    static const constexpr int flagged_for_update_role() { return boost::pfr::tuple_size<T>::value + Qt::UserRole; }
    static const constexpr int now_loading_role() { return flagged_for_update_role() + 1; }

    static QHash<int, QByteArray> roleNames()
    {
        static QHash<int, QByteArray> rn{};

        if (!rn.empty())
            return rn;

        boost::pfr::for_each_field(T{},
                                   [](const r_c_name auto& f, size_t i)
                                   { rn[i + Qt::UserRole] = f.c_name(); });

        rn[flagged_for_update_role()] = "flagged_for_update";
        rn[now_loading_role()] = "now_loading";

        return rn;
    }

    // FIXME
    // very ineficiant !! replace with avendish introspection lib
    // --
    QVariant data(int role) const
    {
        QVariant v{};

        if (role == flagged_for_update_role())
        {
            for (bool f : dirtyFlag_)
            {
                if (f)
                {
                    v = true;
                    break;
                }
            }

            if (!v.isValid()) v = false;
        }
        else if (role == now_loading_role())
        {
            v = loading;
        }
        else
        {
            boost::pfr::for_each_field(aggregate,
                                       [&v, role](const auto& f, size_t i)
                                       {
                                           if (role - Qt::UserRole == i)
                                               v = to_qt(f.value);
                                       });
        }

        return v;
    }

    void setData(const QVariant& v, int role)
    {
        boost::pfr::for_each_field(aggregate,
                                   [&v, role, this](auto& f, size_t i)
                                   {
                                       // prevent from manually setting primary key
                                       if constexpr(is_primary_key<decltype(f), T>)
                                           return;

                                       if (role - Qt::UserRole == i)
                                       {
                                           const auto new_val{from_qt<decltype(f.value)>(v)};

                                           if (f.value != new_val)
                                           {
                                               f.value = new_val;
                                               dirtyFlag_[i] = true;
                                           }
                                       }
                                   });

        if (role == now_loading_role())
            loading = v.toBool();
    }
    // --

    void read(const QJsonObject& obj)
    {
        boost::pfr::for_each_field(aggregate, crudpp::visitor::json_reader{.json = obj});
        reset_flags();
    }

    void write(QJsonObject& obj)
    {
        boost::pfr::for_each_field(aggregate,
                                   [&obj, this](const r_c_name auto& f, size_t i)
                                   {
                                       if constexpr(is_primary_key<decltype(f), T>)
                                       {
                                           // skip primary key for insert
                                           // ie. when it's flag is true (default)
                                           if (dirtyFlag_[i]) return;
                                       }
                                       else
                                       {
                                           // skip non primary key values that have not been updated
                                           if (!dirtyFlag_[i]) return;
                                       }

                                       obj[f.c_name()] = to_qt(f.value);
                                   });
    }

    // set all flags to false
    void reset_flags()
    {
        for (bool& v : dirtyFlag_)
            if (v) v = false;
    }

    // check if the item was inserted in the database
    // ie. if it's primary key is not flagged
    bool inserted() { return !dirtyFlag_[get_primary_key_index<T>()]; }

    T& get_aggregate() { return aggregate; }

private:
    // all true by default to set all fields upon insert
    bool dirtyFlag_[boost::pfr::tuple_size<T>::value] = { true };
    T aggregate{};
    bool loading{false};
};

} // namespace crudpp
