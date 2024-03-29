#pragma once

#include <QObject>

#include <wobjectdefs.h>

#include <crudpp/utils.hpp>
#include "list.hpp"
#include "property_holder.hpp"

namespace qt
{
class bridge;

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
        qDebug() << str << ": " << bridge::instance().context()->contextProperty(str);

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

        connect(m_list,
                &list<T>::get,
                [this] ()
                {
                    net_manager::instance().getFromKey(T::table(),
                                                       [this](const QByteArray& bytes)
                                                       { m_list->read(bytes); });
                });
    }

protected:
    static list<T>* m_list;
};

template <typename T>
class controller<T, true> : public controller<T, false>
{
public:
    controller() : controller<T, false>{}
    {
        QObject::connect(this->m_list,
                         &list<T>::save,
                         [this] (int row)
                         {
                             QJsonObject obj{};
                             auto& item{this->m_list->item_at(row)};
                             item.write(obj);

                             // update if the item was already inserted
                             if (item.inserted())
                             {
                                 // skip if nothing needs updating
                                 // ie. if only the primary key was writen to json
                                 if (obj.size() == 1)
                                 {
                                     emit this->m_list->loaded(row);
                                     return;
                                 }

                                 net_manager::instance().putToKey(make_key(item).c_str(),
                                     QJsonDocument{obj}.toJson(),
                                     [&item, row, obj, this](const QJsonObject& rep)
                                     {
                                         // if (item.get_aggregate().primary_key.value ==
                                         //     m_holder->get_aggregate().primary_key.value)
                                         //     m_holder->read(obj);

                                         item.reset_flags();
                                         emit this->m_list->loaded(row);
                                     },
                                     "save error",
                                     [row, this]()
                                     { emit this->m_list->loaded(row); });
                             }
                             else // insert otherwise
                             {
                                 net_manager::instance().postToKey(T::table(),
                                     QJsonDocument{obj}.toJson(),
                                     [&item, row, this](const QJsonObject& rep)
                                     {
                                         item.read(rep);
                                         emit this->m_list->loaded(row);
                                     },
                                     "save error",
                                     [row, this]()
                                     { emit this->m_list->loaded(row); });
                             }
                         });

        QObject::connect(this->m_list,
                         &list<T>::remove,
                         [this] (int row)
                         {
                             auto& item{this->m_list->item_at(row)};

                             // delete on the server if it exists
                             if (item.inserted())
                             {
                                 net_manager::instance().deleteToKey(make_key(item).c_str(),
                                     [this, &item, row](const QJsonValue& rep)
                                     {
                                         this->m_list->removeItem(row);

                                         // if (item.get_aggregate().primary_key.value ==
                                         //     m_holder->get_aggregate().primary_key.value)
                                         //     m_holder->clear();

                                         emit this->m_list->loaded(row);
                                     },
                                     "Remove Error",
                                     [this, row] ()
                                     { emit this->m_list->loaded(row); });

                                 return;
                             }

                             // only remove localy otherwise
                             this->m_list->removeItem(row);
                         });
    }

private:
    const std::string make_key(model<T>& item) const
    {
        return make_key(std::move(item.get_aggregate()));
    }

    const std::string make_key(const T&& aggregate) const
    {
        std::string key{T::table()};

        key += '/';
        key += std::to_string(aggregate.primary_key.value);

        return key;
    }
};

template <typename T, bool has_primary_key>
list<T>* controller<T, has_primary_key>::m_list{new list<T>{}};

} // namespace qt

#include <wobjectimpl.h>
W_OBJECT_IMPL((qt::controller<T, has_primary_key>), template <typename T, bool has_primary_key>)
