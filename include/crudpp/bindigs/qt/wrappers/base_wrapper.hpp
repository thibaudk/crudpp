#pragma once

#include <boost/pfr/core.hpp>

#include <QJsonObject>

#include <crudpp/utils.hpp>
#include <crudpp/concepts.hpp>
#include <crudpp/bindigs/qt/utils.hpp>
#include <crudpp/bindigs/qt/json_reader.hpp>

namespace qt
{
template <typename T>
struct base_wrapper
{
    void set(const T& agg)
    {
        aggregate = agg;
        prev_agg = agg;
        m_inserted = true;
    }

    void read(const QJsonObject& obj)
    {
        crudpp::for_each_index<T>(
            [&obj, this] (const auto i)
            {
                json_reader vis{.json = obj};
                auto& field{boost::pfr::get<i()>(aggregate)};

                if (vis.json.contains(field.c_name()))
                    if (!vis.json[field.c_name()].isNull())
                        vis(field);
            });

        reset_flags();
        m_inserted = true;
    }

    void write(QJsonObject& obj)
    {
        crudpp::for_each_index<T>(
            [&obj, this] (const auto i)
            {
                const auto field{boost::pfr::get<i()>(aggregate)};

                if constexpr(crudpp::is_primary_key<decltype(field), T>)
                {
                    // skip primary key for insert
                    if (!m_inserted) return;
                }
                else
                {
                    const auto prev_field{boost::pfr::get<i()>(prev_agg)};
                    // skip non primary key values that have not been updated
                    if (field.value == prev_field.value) return;
                }

                obj[field.c_name()] = to_qjson(to_qt(field.value));
            });
    }

    void reset_flags() { prev_agg = aggregate; }

    // check if the item was inserted in the database
    // ie. if the default false value is now true
    bool inserted() const { return m_inserted; }

    bool flagged_for_update() const
    {
        return !crudpp::for_each_index_until<T>(
            [this] (const auto i)
            {
                return boost::pfr::get<i()>(aggregate).value
                       == boost::pfr::get<i()>(prev_agg).value;
            }
            );
    }

    T& get_aggregate() { return aggregate; }
    const T& get_prev_agg() { return prev_agg; }

protected:
    base_wrapper() = default;

    bool m_inserted{false};
    bool loading{false};
    T aggregate{};
    T prev_agg{};
};

} //crudpp
