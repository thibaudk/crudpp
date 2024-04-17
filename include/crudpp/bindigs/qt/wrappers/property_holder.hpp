#pragma once

// adapted from verdigris cpp_tutorial
// https://github.com/woboq/verdigris/blob/master/tutorial/cpp_tutorial.cpp

#include <boost/pfr/core.hpp>

#include <QObject>
#include <QVariant>

#include <wobjectcpp.h>
#include <wobjectimpl.h>

#include <crudpp/required.hpp>
#include <crudpp/bindigs/qt/utils.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include "base_wrapper.hpp"
#include "model.hpp"
#include "list_model.hpp"

namespace qt
{
class net_manager;

template <crudpp::r_c_name T>
constexpr auto get_property_name() -> w_cpp::StringView
{
    return {T::c_name(),
            T::c_name() + (std::char_traits<char>::length(T::c_name()))};
}

// constexpr char* concatenation adapted from https://stackoverflow.com/a/53190554/14999126
// http://coliru.stacked-crooked.com/a/3f557060cc18719e

template <size_t N>
struct String
{
    char c[N];
};

template <crudpp::r_c_name T>
constexpr auto get_property_changed_name()
{
    constexpr size_t length{std::char_traits<char>::length(T::c_name()) + 7};
    String<length + 1> result{};
    result.c[length] = '\0';

    char* dst = result.c;

    const char* src{T::c_name()};
    for (; *src != '\0'; src++, dst++)
        *dst = *src;

    const char* changed{"Changed"};
    for (; *changed != '\0'; changed++, dst++)
        *dst = *changed;

    return result;
}

template <typename T>
class property_holder : public base_wrapper<T>
                      , public QObject
{
    W_OBJECT(property_holder)

    void flaggedChanged()
    W_SIGNAL(flaggedChanged)

    W_PROPERTY(bool, flagged_for_update READ flagged_for_update NOTIFY flaggedChanged)

    template <size_t I>
    using property_at = std::remove_reference_t<decltype(boost::pfr::get<I>(std::declval<T>()))>;

public:
    property_holder(QObject* parent = nullptr)
        : QObject{parent}
    {}

    void read(const QJsonObject& obj)
    {
        crudpp::for_each_index<T>(
            [&obj, this] (const auto i)
            {
                json_reader vis{.json = obj};
                auto& field{boost::pfr::get<i()>(this->aggregate)};

                if (vis.json.contains(field.c_name()))
                {
                    if (!vis.json[field.c_name()].isNull())
                    {
                        vis(field);

                        if (field.value != boost::pfr::get<i()>(this->prev_agg).value)
                            property_changed<i()>();
                    }
                }
            }
            );

        reset_flags();
        this->m_inserted = true;

        emit flaggedChanged();
    }

    void get()
    {
        net_manager::instance().getFromKey(key(),
                                           [this](const QByteArray& bytes)
                                           { read(bytes); });
    }

    void from_item(model<T>& item)
    {
        const auto& agg{item.get_aggregate()};

        crudpp::for_each_index<T>
            ([this, agg] (const auto i)
             {
                 const auto val{QVariant::fromValue(to_qt(boost::pfr::get<i()>(agg).value))};
                 set_property_value<i()>(val);
             }
             );

        this->m_inserted = item.inserted();
        this->prev_agg = item.get_prev_agg();
        emit flaggedChanged();
    }

    void from_list(list_model<T>* list, int index)
    {
        from_item(list->item_at(index));
    }
    W_INVOKABLE(from_list, (list_model<T>*, int))

    void from_list_by(list_model<T>* list, const QByteArray& roleName, const QVariant& value)
    {
        int role{model<T>::roleNames().key(roleName)};
        int i{0};

        for (const auto& item : list->items())
        {
            if (item.data(role) == value)
            {
                from_item(list->item_at(i));
                return;
            }

            i++;
        }

        clear();
    }
    W_INVOKABLE(from_list_by, (list_model<T>*, const QByteArray&, const QVariant&))

    void clear()
    {
        crudpp::for_each_index<T>
            ([this] (const auto i)
             {
                 const std::remove_reference_t<decltype(boost::pfr::get<i()>(this->aggregate))> init{};
                 set_property_value<i()>(QVariant::fromValue(to_qt(init.value)));
             }
             );

        reset_flags();
        this->m_inserted = false;
    }
    W_INVOKABLE(clear)

    void save()
    {
        set_loading(true);

        if (!this->flagged_for_update())
        {
            set_loading(false);
            return;
        }

        QJsonObject obj{};
        this->write(obj);

        // update if the item was already inserted
        if (this->inserted())
        {
            net_manager::instance().putToKey(key().c_str(),
                QJsonDocument{obj}.toJson(),
                [this] (const QJsonObject& rep)
                {
                    const auto id{this->aggregate.primary_key.value};

                    // FIXME: replace with static vector of pointers to all instances ?
                    auto objects{bridge::instance().engine
                                     ->rootObjects()[0]
                                     ->findChildren<list_model<T>*>()};

                    for (auto* m : objects)
                    {
                        for (int i{0}; i < m->size(); i++)
                        {
                            auto& item{m->item_at(i)};

                            if (item.get_aggregate().primary_key.value == id)
                            {
                                item.set(this->aggregate);
                                m->dataChangedAt(i);
                                break;
                            }
                        }
                    }

                    reset_flags();
                    set_loading(false);
                },
                "save error",
                [this] ()
                { set_loading(false); });
        }
        else // insert otherwise
        {
            net_manager::instance().postToKey(T::table(),
                QJsonDocument{obj}.toJson(),
                [this, obj] (const QJsonObject& rep)
                {
                    read(rep);

                    auto map{rep.toVariantMap()};
                    map.insert(obj.toVariantMap());
                    const auto json{QJsonObject::fromVariantMap(map)};

                    // FIXME: replace with static vector of pointers to all instances ?
                    auto objects{bridge::instance().engine
                                     ->rootObjects()[0]
                                     ->findChildren<list_model<T>*>()};

                    for (auto* m : objects)
                        m->append(model<T>{json});

                    set_loading(false);
                },
                "save error",
                [this] ()
                { set_loading(false); });
        }
    }
    W_INVOKABLE(save)

    void remove()
    {
        set_loading(true);

        // delete on the server if it exists
        if (this->inserted())
        {
            net_manager::instance().deleteToKey(key().c_str(),
                [this](const QJsonValue& rep)
                {
                    const auto id{this->get_aggregate().primary_key.value};

                    // FIXME: replace with static vector of pointers to all instances ?
                    auto objects{bridge::instance().engine
                                     ->rootObjects()[0]
                                     ->findChildren<list_model<T>*>()};

                    for (auto* m : objects)
                    {
                        for (int i{0}; i < m->size(); i++)
                        {
                            auto& item{m->item_at(i)};

                            if (item.get_aggregate().primary_key.value == id)
                            {
                                m->removeItem(i);
                                break;
                            }
                        }
                    }

                    clear();
                    set_loading(false);
                },
                "Remove Error",
                [this] ()
                { set_loading(false); });

            return;
        }

        // only remove localy otherwise
        clear();
        set_loading(false);
    }
    W_INVOKABLE(remove)

    void loadingChanged()
    W_SIGNAL(loadingChanged)

private:
    bool get_loading() const { return base_wrapper<T>::loading; }
    W_PROPERTY(bool, loading READ get_loading NOTIFY loadingChanged)

    void set_loading(bool l)
    {
        if (l == base_wrapper<T>::loading) return;

        base_wrapper<T>::loading = l;
        emit loadingChanged();
    }

    void reset_flags()
    {
        base_wrapper<T>::reset_flags();
        emit flaggedChanged();
    }

    const std::string key()
    {
        return make_key(std::move(this->get_aggregate()));
    }

    template <size_t I>
    void property_changed()
    {
        W_CPP_SIGNAL_IMPL(decltype(&property_holder::property_changed<I>),
                          property_changed_signals,
                          I,
                          0);
    }

    template<size_t I, class = std::enable_if_t<(I < boost::pfr::tuple_size_v<T>)>>
    struct property_changed_signals
    {
        constexpr static auto base_name{get_property_changed_name<property_at<I>>()};
        constexpr static auto signal{w_cpp::makeSignalBuilder(w_cpp::viewLiteral(base_name.c),
                                                              &property_holder::property_changed<I>)
                                                             .build()};
    };
    W_CPP_SIGNAL(property_changed_signals)

    template <size_t I>
    auto get_property_value() const -> QVariant
    {
        const auto& property{boost::pfr::get<I>(this->aggregate)};
        return QVariant::fromValue(to_qt(property.value));
    }

    template <size_t I>
    void set_property_value(QVariant variant)
    {
        auto& property{boost::pfr::get<I>(this->aggregate)};
        property.value = from_qt<decltype(property.value)>(variant);
        property_changed<I>();
        emit flaggedChanged();
    }

    template <size_t I, class = std::enable_if_t<(I < boost::pfr::tuple_size_v<T>)>>
    struct register_properties
    {
        constexpr static auto property{w_cpp::makeProperty<QVariant>(
                                           get_property_name<property_at<I>>(),
                                           w_cpp::viewLiteral("QVariant")
                                           ).setGetter(&property_holder::get_property_value<I>)
                                           .setSetter(&property_holder::set_property_value<I>)
                                           .setNotify(&property_holder::property_changed<I>)};
    };
    W_CPP_PROPERTY(register_properties)
};
} // namespace qt

W_OBJECT_IMPL_INLINE(qt::property_holder<T>, template <typename T>)
