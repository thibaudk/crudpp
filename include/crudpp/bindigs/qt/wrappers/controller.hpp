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
        const auto uri{make_uri<T>()};

        auto holder_uri{uri};
        holder_uri += "_holder";
        qmlRegisterUncreatableType<property_holder<T>>(uri.c_str(), 1, 0, holder_uri.c_str(), "");

        QString str{T::table()};
        str.prepend("current_");
        bridge::instance().context()->setContextProperty(str, m_holder);
        qDebug() << str << ": " << bridge::instance().context()->contextProperty(str);

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
                                         if (item.get_aggregate().primary_key.value ==
                                             m_holder->get_aggregate().primary_key.value)
                                             m_holder->read(obj);

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

        QObject::connect(m_holder,
                         &property_holder<T>::save,
                         [this] ()
                         {
                             m_holder->set_loading(true);

                             QJsonObject obj{};
                             m_holder->write(obj);

                             // update if the item was already inserted
                             if (m_holder->inserted())
                             {
                                 // skip if nothing needs updating
                                 // ie. if only the primary key was writen to json
                                 if (obj.size() == 1)
                                 {
                                     m_holder->set_loading(false);
                                     return;
                                 };

                                 net_manager::instance().putToKey(make_key().c_str(),
                                     QJsonDocument{obj}.toJson(),
                                     [obj, this](const QJsonObject& rep)
                                     {
                                         const auto id{m_holder->get_aggregate().primary_key.value};

                                         int i{0};

                                         for (auto& item : this->m_list->get_list())
                                         {
                                             if (item.get_aggregate().primary_key.value == id)
                                             {
                                                 item.read(obj);
                                                 emit this->m_list->dataChangedAt(i);
                                                 break;
                                             }

                                             i++;
                                         }

                                         m_holder->reset_flags();
                                         m_holder->set_loading(false);
                                     },
                                     "save error",
                                     [this] ()
                                     { m_holder->set_loading(false); });
                             }
                             else // insert otherwise
                             {
                                 net_manager::instance().postToKey(T::table(),
                                     QJsonDocument{obj}.toJson(),
                                     [this, obj](const QJsonObject& rep)
                                     {
                                         m_holder->read(rep);

                                         auto map{rep.toVariantMap()};
                                         map.insert(obj.toVariantMap());
                                         const auto json{QJsonObject::fromVariantMap(map)};

                                         this->m_list->append(model<T>{json});

                                         m_holder->set_loading(false);
                                     },
                                     "save error",
                                     [this] ()
                                     { m_holder->set_loading(false); });
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

                                         if (item.get_aggregate().primary_key.value ==
                                             m_holder->get_aggregate().primary_key.value)
                                             m_holder->clear();

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

        QObject::connect(m_holder,
                         &property_holder<T>::remove,
                         [this] ()
                         {
                             m_holder->set_loading(true);

                             // delete on the server if it exists
                             if (m_holder->inserted())
                             {
                                 net_manager::instance().deleteToKey(make_key().c_str(),
                                     [this](const QJsonValue& rep)
                                     {
                                         const auto id{m_holder->get_aggregate().primary_key.value};

                                         int i{0};

                                         for (auto& item : this->m_list->get_list())
                                         {
                                             if (item.get_aggregate().primary_key.value == id)
                                             {
                                                 this->m_list->removeItem(i);
                                                 break;
                                             }

                                             i++;
                                         }

                                         m_holder->clear();
                                         m_holder->set_loading(false);
                                     },
                                     "Remove Error",
                                     [this] ()
                                     { m_holder->set_loading(false); });

                                 return;
                             }

                             // only remove localy otherwise
                             m_holder->clear();
                             m_holder->set_loading(false);
                         });

        QObject::connect(this->m_list,
                         &list<T>::select,
                         [this] (int row)
                         { m_holder->set(this->m_list->item_at(row)); });

        QObject::connect(this->m_list,
                         &list<T>::select_by,
                         [this] (const QByteArray& roleName, const QVariant& value)
                         {
                             int role{model<T>::roleNames().key(roleName)};
                             int i{0};

                             for (const auto& item : this->m_list->get_list())
                             {
                                 if (item.data(role) == value)
                                 {
                                     m_holder->set(this->m_list->item_at(i));
                                     return;
                                 }

                                 i++;
                             }

                             m_holder->clear();
                         });
    }

private:
    const std::string make_key(model<T>& item) const
    {
        return make_key(std::move(item.get_aggregate()));
    }

    const std::string make_key() const
    {
        return make_key(std::move(m_holder->get_aggregate()));
    }

    const std::string make_key(const T&& aggregate) const
    {
        std::string key{T::table()};

        key += '/';
        key += std::to_string(aggregate.primary_key.value);

        return key;
    }

    static property_holder<T>* m_holder;
};

template <typename T, bool has_primary_key>
list<T>* controller<T, has_primary_key>::m_list{new list<T>{}};

template <typename T>
property_holder<T>* controller<T, true>::m_holder{new property_holder<T>{}};

} // namespace qt

#include <wobjectimpl.h>
W_OBJECT_IMPL((qt::controller<T, has_primary_key>), template <typename T, bool has_primary_key>)
