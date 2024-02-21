#pragma once

#include "qnamespace.h"

#include "base_wrapper.hpp"

namespace qt
{
template <typename T>
struct model final : public base_wrapper<T>
{
    model(const QJsonObject& json) { this->read(json); }
    model() = default;

    static const constexpr int flagged_for_update_role()
    { return boost::pfr::tuple_size<T>::value + Qt::UserRole; }

    static const constexpr int loading_role() { return flagged_for_update_role() + 1; }

    static QHash<int, QByteArray> roleNames()
    {
        static QHash<int, QByteArray> rn{};

        if (!rn.empty())
            return rn;

        boost::pfr::for_each_field(T{},
                                   [](const crudpp::r_c_name auto& f, size_t i)
                                   { rn[i + Qt::UserRole] = f.c_name(); });

        rn[flagged_for_update_role()] = "flagged_for_update";
        rn[loading_role()] = "loading";

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
            v = this->flagged_for_update();
        }
        else if (role == loading_role())
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
                                       if constexpr(crudpp::is_primary_key<decltype(f), T>)
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

        if (role == loading_role())
            this->loading = v.toBool();
    }
    // --

    bool get_inserted() const { return this->m_inserted; }
};

} // namespace qt
