#pragma once

#include <boost/pfr/core.hpp>

#include <QJsonObject>

#include <crudpp/utils.hpp>
#include <crudpp/concepts/permissions.hpp>
#include <crudpp/concepts/required.hpp>
#include <crudpp/bindings/qt/utils.hpp>
#include <crudpp/bindings/qt/json_reader.hpp>

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
                auto& field{boost::pfr::get<i()>(aggregate)};
                using f_type = std::remove_reference_t<decltype(field)>;
                if constexpr(crudpp::has_writeonly_flag<f_type>) return;

                json_reader vis{.json = obj};
                const auto name{f_type::c_name()};

                if (vis.json.contains(name))
                    if (!vis.json[name].isNull())
                        vis(field);
            });

        reset_flags();
        m_inserted = true;
    }

    void write(QJsonObject& obj)
    {
        using namespace crudpp;

        for_each_index<T>(
            [&obj, this] (const auto i)
            {
                using namespace boost::pfr;
                const auto field{get<i()>(aggregate)};
                using f_type = std::remove_reference_t<decltype(field)>;
                if constexpr(crudpp::has_readonly_flag<f_type>) return;

                if constexpr(is_single_primary_key<f_type, T>)
                {
                    // skip auto incremented primary key for insert
                    if (!m_inserted) return;
                }
                else
                {
                    const auto prev_field{get<i()>(prev_agg)};
                    // skip non primary key values that have not been updated
                    if (field.value == prev_field.value) return;
                }

                obj[f_type::c_name()] = to_qjson(to_qt(field.value));
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
                using namespace boost::pfr;

                return get<i()>(aggregate).value
                       == get<i()>(prev_agg).value;
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
