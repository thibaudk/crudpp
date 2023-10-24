#pragma once

// adapted from verdigris cpp_tutorial
// https://github.com/woboq/verdigris/blob/master/tutorial/cpp_tutorial.cpp

#include <boost/pfr/core.hpp>

#include <QObject>
#include <QVariant>

#include <wobjectcpp.h>
#include <wobjectimpl.h>

#include <crudpp/required.hpp>
#include "base_wrapper.hpp"
#include "utils.hpp"

namespace crudpp
{
template <r_c_name T>
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

template <r_c_name T>
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

    template <size_t I>
    using property_at = std::remove_reference_t<decltype(boost::pfr::get<I>(std::declval<T>()))>;

public:
    property_holder(QObject* parent = nullptr)
        : QObject{parent}
    {}

    void read(const QJsonObject& obj)
    {
        boost::pfr::for_each_field(this->aggregate, crudpp::visitor::json_reader{.json = obj});
        this->reset_flags();
        for_each_index<boost::pfr::tuple_size_v<T> - 1>([this](const auto i){ property_changed<i()>(); });
    }

private:
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
    }

    template <size_t I, class = std::enable_if_t<(I < boost::pfr::tuple_size_v<T>)>>
    struct register_properties
    {
        constexpr static auto property{w_cpp::makeProperty<QVariant>(get_property_name<property_at<I>>(),
                                                                     w_cpp::viewLiteral("QVariant"))
                                           .setGetter(&property_holder::get_property_value<I>)
                                           .setSetter(&property_holder::set_property_value<I>)
                                           .setNotify(&property_holder::property_changed<I>)};
    };
    W_CPP_PROPERTY(register_properties)
};
} // namespace crudpp

W_OBJECT_IMPL_INLINE(crudpp::property_holder<T>, template <typename T>)
