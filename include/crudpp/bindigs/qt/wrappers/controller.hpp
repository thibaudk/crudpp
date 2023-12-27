#pragma once

#include <QObject>

#include <wobjectdefs.h>

#include <crudpp/utils.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include "list.hpp"
#include "property_holder.hpp"

namespace qt
{
template <typename T, bool has_primary_key = false>
class controller : QObject
{
    W_OBJECT(controller)

public:
    controller() : QObject{}
    {
        const auto uri{make_uri<T>()};
        qmlRegisterUncreatableType<T>(uri.c_str(), 1, 0, uri.c_str(), "");

        QString str{T::table()};

        str += "_list";
        bridge::instance().context()->setContextProperty(str, m_list);

        connect(m_list,
                &list<T>::addWith,
                [this] (const QJsonObject& obj)
                {
                    net_manager::instance().postToKey(T::table(),
                        QJsonDocument{obj}.toJson(),
                        [](const QJsonObject& rep)
                        { m_list->append(model<T>{rep}); },
                        "AddWith error");
                });

        // not sure why the regular syntax for connection does not work here
        // connect(m_list, &list<T>::get, &controller<T>::clear_get);
        connect(m_list, &list<T>::get, [this](){ get(); });

        connect(m_list,
                &list<T>::save,
                [this] (int row)
                {
                    QJsonObject obj{};
                    auto& item{m_list->item_at(row)};
                    item.write(obj);

                    // update if primary key is set
                    if (obj.contains(to_qt(crudpp::get_primary_key_name<T>())))
                    {
                        // skip if nothing needs updating
                        if (obj.size() == 1) return;

                        net_manager::instance().putToKey(make_key(item).c_str(),
                            QJsonDocument{obj}.toJson(),
                            [&item, row](const QJsonObject& rep)
                            {
                                item.reset_flags();
                                emit m_list->loaded(row);
                            },
                            "save error",
                            [row]()
                            { emit m_list->loaded(row); });
                    }
                    else // insert otherwise
                    {
                        net_manager::instance().postToKey(T::table(),
                            QJsonDocument{obj}.toJson(),
                            [&item, row](const QJsonObject& rep)
                            {
                                item.read(rep);
                                emit m_list->loaded(row);
                            },
                            "save error",
                            [row]()
                            { emit m_list->loaded(row); });
                    }
                });

        connect(m_list,
                &list<T>::remove,
                [this] (int row)
                {
                    auto& item{m_list->item_at(row)};

                    // delete on the server if it exists
                    if (item.inserted())
                    {
                        net_manager::instance().deleteToKey(make_key(item).c_str(),
                            [this, row](const QJsonValue& rep)
                            {
                                m_list->removeItem(row);
                                emit m_list->loaded(row);
                            },
                            "Remove Error",
                            [this, row]()
                            { emit m_list->loaded(row); });

                        return;
                    }

                    // only remove localy otherwise
                    m_list->removeItem(row);
                });
    }

private:
    void get()
    {
        net_manager::instance().getFromKey(T::table(),
                                           [this](const QByteArray& bytes)
                                           { m_list->read(bytes); });
    }
    W_SLOT(get)

    const std::string make_key(model<T>&& item) const
    {
        return make_key(std::move(item));
    }

    const std::string make_key(model<T>& item) const
    {
        std::string key{T::table()};
        key += '/';

        if constexpr (crudpp::r_primary_key<T>)
            key += std::to_string(item.get_aggregate().primary_key.value);

        return key;
    }

    static list<T>* m_list;
};

template <typename T>
struct controller<T, true> : public controller<T, false>
{
    controller() : controller<T, false>{}
    {
        const auto uri{make_uri<T>()};

        auto holder_uri{uri};
        holder_uri += "_holder";
        qmlRegisterUncreatableType<property_holder<T>>(uri.c_str(), 1, 0, holder_uri.c_str(), "");

        QString str{T::table()};
        str.prepend("current_");
        bridge::instance().context()->setContextProperty(str, m_holder);
    }

private:
    static property_holder<T>* m_holder;
};

template <typename T, bool has_primary_key>
list<T>* controller<T, has_primary_key>::m_list{new list<T>{}};

template <typename T>
property_holder<T>* controller<T, true>::m_holder{new property_holder<T>{}};

} // namespace qt

#include <wobjectimpl.h>
W_OBJECT_IMPL((qt::controller<T, has_primary_key>), template <typename T, bool has_primary_key>)
