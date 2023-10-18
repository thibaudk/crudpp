#pragma once

#include "qnamespace.h"

#include "base_wrapper.hpp"

namespace crudpp
{
template <typename T>
struct model final : public base_wrapper<T>
{
    base_wrapper(const QJsonObject& json) { read(json); }
    base_wrapper() = default;

    void read(const QJsonObject& obj)
    {
        boost::pfr::for_each_field(aggregate, crudpp::visitor::json_reader{.json = obj});
        reset_flags();
    }

    static const constexpr int flagged_for_update_role()
    { return boost::pfr::tuple_size<T>::value + Qt::UserRole; }

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
            for (bool f : this->dirtyFlag_)
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
            v = this->loading;
        }
        else
        {
            boost::pfr::for_each_field(this->aggregate,
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
        boost::pfr::for_each_field(this->aggregate,
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
                                               this->dirtyFlag_[i] = true;
                                           }
                                       }
                                   });

        if (role == now_loading_role())
            this->loading = v.toBool();
    }
    // --
};

} // namespace crudpp
