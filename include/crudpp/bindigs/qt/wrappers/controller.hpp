#pragma once

#include <QObject>

#include <wobjectdefs.h>

#include <crudpp/utils.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include "list.hpp"

namespace crudpp
{
template <typename T>
class controller final : QObject
{
    W_OBJECT(controller)

public:
    controller() : QObject{}
    {
        const auto uri{make_uri<T>()};

        qmlRegisterUncreatableType<T>(uri.c_str(), 1, 0, uri.c_str(), "");
        bridge::instance().context()->setContextProperty(T::table(), m_list);

        connect(m_list,
                &list<T>::save,
                [this] (int row)
                {
                    QJsonObject obj{};
                    auto& item{m_list->item_at(row)};
                    item.write(obj);

                    // update if primary key is set
                    if (obj.contains(to_qt(get_primary_key_name<T>())))
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
                            "Validate error",
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
                            "Add error");
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

//        this->connect(m_list,
//                      &list<T>::add,
//                      this,
//                      [this] ()
//                      {
//                          net_manager::instance().postToKey(this->inner->key,
//                              QByteArray{},
//                              [this](const Json::Value& rep)
//                              { this->inner->appendWith(rep); },
//                              "Add error");
//                      });

//        this->connect(this->inner,
//                      &Inner::addWith,
//                      this,
//                      [this] (const QJsonObject& obj)
//                      {
//                          QJsonDocument doc{obj};
//                          QByteArray data{doc.toJson()};

//                          Json::Value val;
//                          Json::Reader reader;
//                          reader.parse(data.toStdString(), val);

//                          Interface::netManager::instance().postToKey(this->inner->key,
//                              data,
//                              [this, val](const Json::Value& res)
//                              {
//                                  Json::Value concat{val};
//                                  concatenate(concat, res);
//                                  this->inner->appendWith(concat);
//                              },
//                              "addWith error");
//                      });
        get();
    }

    void get()
    {
        net_manager::instance().getFromKey(T::table(),
                                           [this](const QByteArray& bytes)
                                           { m_list->read(bytes); });
    }

    const std::string make_key(model<T>&& item) const
    {
        return make_key(std::move(item));
    }

    const std::string make_key(model<T>& item) const
    {
        std::string key{T::table()};
        key += '/';

        if constexpr (has_primary_key<T>)
            key += std::to_string((item.get_aggregate().*T::primary_key()).value);

        return key;
    }

    static list<T>* m_list;
};

template <typename T>
list<T>* controller<T>::m_list{new list<T>{}};

} // namespace crudpp

#include <wobjectimpl.h>
W_OBJECT_IMPL(crudpp::controller<T>, template <typename T>)
