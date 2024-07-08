#pragma once

// adapted from verdigris cpp_tutorial
// https://github.com/woboq/verdigris/blob/master/tutorial/cpp_tutorial.cpp

#include <boost/pfr/core.hpp>

#include <QObject>
#include <QVariant>

#include <wobjectcpp.h>

#include <crudpp/concepts/required.hpp>
#include <crudpp/bindings/qt/utils.hpp>
#include <crudpp/bindings/qt/interface/bridge.hpp>
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
consteval auto get_property_changed_name()
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

    template <size_t I>
    using property_at = std::remove_reference_t<decltype(boost::pfr::get<I>(std::declval<T>()))>;

public:
    // VERIFY : should replace by std::deque to avoid realocation problemes ?
    static std::vector<property_holder<T>*> instances;

    explicit property_holder(QObject* parent = nullptr)
        : QObject{parent}
    {
        instances.emplace_back(this);
    }

    ~ property_holder()
    {
        instances.erase(std::remove(instances.begin(),
                                    instances.end(),
                                    this),
                        instances.end());
    }

    property_holder(const property_holder &) = delete;
    void operator = (const property_holder &) = delete;

    void read(const QByteArray& bytes)
    {
        const auto doc{QJsonDocument::fromJson(bytes)};
        read(doc.object());
    }

    void read(const QJsonObject& obj)
    {
        crudpp::for_each_index<T>(
            [&obj, this] (const auto i)
            {
                using namespace boost;
                auto& field{pfr::get<i()>(this->aggregate)};
                using f_type = std::remove_reference_t<decltype(field)>;
                if constexpr(crudpp::has_writeonly_flag<f_type>) return;

                json_reader vis{.json = obj};
                const auto name = f_type::c_name();

                if (vis.json.contains(name))
                {
                    if (!vis.json[name].isNull())
                    {
                        vis(field);

                        if (field.value != pfr::get<i()>(this->prev_agg).value)
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
        setLoading(true);

        net_manager::instance().getFromKey(key().c_str(),
            [this](const QByteArray& bytes)
            {
                read(bytes);
                setLoading(false);
            },
            QString::fromStdString(std::string{T::table()} + " get error"),
            [this] ()
            { setLoading(false); }
            );
    }
    W_INVOKABLE(get)

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
        if (!this->flagged_for_update())
            return;

        // update if the item was already inserted
        this->inserted() ? update() : insert();
    }
    W_INVOKABLE(save)

    void save_queued()
    {
        if (!this->flagged_for_update())
        {
            bridge::instance().dequeue();
            return;
        }

        if (this->inserted())
        {
            update();
            // updates are not considered relevant in the QML queue
            // therefore dequeue is called directly
            // outside of the callback
            bridge::instance().dequeue();
        }
        else
            insert(true);
    }
    W_INVOKABLE(save_queued)

    void remove()
    {
        // delete on the server if it exists
        if (this->inserted())
        {
            del();
            return;
        }
        // only remove localy otherwise
        clear();
    }
    W_INVOKABLE(remove)

    void remove_queued()
    {
        // delete on the server if it exists
        if (this->inserted())
        {
            del(true);
            return;
        }
        // only remove localy otherwise
        clear();
        bridge::instance().dequeue();
    }
    W_INVOKABLE(remove_queued)

    void loadingChanged()
    W_SIGNAL(loadingChanged)

private:
    void flaggedChanged()
    W_SIGNAL(flaggedChanged)

    void insertedChanged()
    W_SIGNAL(insertedChanged)

    W_PROPERTY(bool, flagged_for_update READ flagged_for_update NOTIFY flaggedChanged)
    W_PROPERTY(bool, inserted READ inserted NOTIFY insertedChanged)

    bool getLoading() const { return base_wrapper<T>::loading; }
    W_PROPERTY(bool, loading READ getLoading NOTIFY loadingChanged)

    void setLoading(bool l)
    {
        if (l == base_wrapper<T>::loading) return;

        base_wrapper<T>::loading = l;
        emit loadingChanged();
    }

    void from_oter(property_holder<T>* item)
    {
        const auto& agg{item->get_aggregate()};

        crudpp::for_each_index<T>
            ([this, agg] (const auto i)
             {
                 const auto val{QVariant::fromValue(to_qt(boost::pfr::get<i()>(agg).value))};
                 set_property_value<i()>(val);
             }
             );

        this->m_inserted = item->inserted();
        this->prev_agg = item->get_prev_agg();
        emit flaggedChanged();
    }

    void reset_flags()
    {
        base_wrapper<T>::reset_flags();
        emit flaggedChanged();
    }

    const std::string key()
    {
        return crudpp::make_key(std::move(this->get_aggregate()));
    }

    void insert(bool queued = false)
    {
        bridge::instance().increment_load();

        QJsonObject obj{};
        this->write(obj);

        net_manager::instance().postToKey(T::table(),
            QJsonDocument{obj}.toJson(),
            [this/*, obj*/, queued] (const QJsonObject& rep)
            {
                read(rep);

                // VERIFY: is this necessary ?
                // auto map{rep.toVariantMap()};
                // map.insert(obj.toVariantMap());
                // const auto json{QJsonObject::fromVariantMap(map)};

                // for (auto* m : list_model<T>::instances)
                //     m->append(model<T>{json});

                bridge::instance().decrement_load();
                if (queued) bridge::instance().dequeue();
            },
            "save error");
    }

    void update()
    {
        setLoading(true);

        QJsonObject obj{};
        this->write(obj);

        net_manager::instance().putToKey(key().c_str(),
            QJsonDocument{obj}.toJson(),
            [this] (const QJsonObject& rep)
            {
                using namespace crudpp;

                const auto id{t_trait<T>::pk_value(this->aggregate)};

                for (auto* p : instances)
                    if (t_trait<T>::pk_value(p->get_aggregate()) == id)
                        p->from_oter(this);

                for (auto* m : list_model<T>::instances)
                {
                    for (int i{0}; i < m->size(); i++)
                    {
                        auto& item{m->item_at(i)};

                        if (t_trait<T>::pk_value(item.get_aggregate()) == id)
                        {
                            item.set(this->aggregate);
                            m->dataChangedAt(i);
                            break;
                        }
                    }
                }

                reset_flags();
                setLoading(false);
            },
            "save error",
            [this] ()
            { setLoading(false); });
    }

    void del(bool queued = false)
    {
        bridge::instance().increment_load();

        net_manager::instance().deleteToKey(key().c_str(),
            [this, queued](const QJsonValue& rep)
            {
                using namespace crudpp;

                const auto id{t_trait<T>::pk_value(this->aggregate)};

                for (auto* p : instances)
                    if (t_trait<T>::pk_value(p->get_aggregate()) == id)
                        p->clear();

                for (auto* m : list_model<T>::instances)
                {
                    for (int i{0}; i < m->size(); i++)
                    {
                        auto& item{m->item_at(i)};

                        if (t_trait<T>::pk_value(item.get_aggregate()) == id)
                        {
                            m->removeItem(i);
                            break;
                        }
                    }
                }

                clear();
                bridge::instance().decrement_load();
                if (queued) bridge::instance().dequeue();
            },
            "Remove Error");
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

template <typename T>
std::vector<qt::property_holder<T>*> qt::property_holder<T>::instances{};

#include <wobjectimpl.h>
W_OBJECT_IMPL_INLINE(qt::property_holder<T>, template <typename T>)
