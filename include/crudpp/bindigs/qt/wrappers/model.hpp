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
            crudpp::for_nth_index<T>(role - Qt::UserRole,
                                     [&v, role, this] (const auto i)
                                     { v = to_qt(boost::pfr::get<i()>(this->aggregate).value); }
                                     );
        }

        return v;
    }

    void setData(const QVariant& v, int role)
    {
        if (role == loading_role())
            this->loading = v.toBool();
        else
            crudpp::for_nth_index<T>(role - Qt::UserRole,
                                     [&v, role, this] (const auto i)
                                     {
                                         auto& field{boost::pfr::get<i()>(this->aggregate)};

                                         // prevent from manually setting primary key
                                         if constexpr(crudpp::is_primary_key<decltype(field), T>)
                                             return;

                                         const auto new_val{from_qt<decltype(field.value)>(v)};

                                         if (field.value != new_val) field.value = new_val;
                                     });
    }
};

} // namespace qt
