#pragma once

#include <boost/pfr/core.hpp>

#include <QJsonObject>

#include <crudpp/utils.hpp>
#include <crudpp/required.hpp>
#include <crudpp/bindigs/qt/visitors/json_reader.hpp>
#include "utils.hpp"

namespace crudpp
{
template <typename T>
struct base_wrapper
{
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

                                       obj[f.c_name()] = to_qjson(to_qt(f.value));
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

protected:
    // all true by default to set all fields upon insert
    bool dirtyFlag_[boost::pfr::tuple_size<T>::value] = { true };
    T aggregate{};
    bool loading{false};
};
} //crudpp
