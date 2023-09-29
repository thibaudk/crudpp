#pragma once

// adapted from verdigris cpp_tutorial
// https://github.com/woboq/verdigris/blob/master/tutorial/cpp_tutorial.cpp

#include <boost/pfr/core.hpp>

#include <QObject>
#include <QVariant>

#include <wobjectcpp.h>
#include <wobjectimpl.h>

#include <crudpp/required.hpp>

namespace crudpp
{
template <r_c_name T>
constexpr auto get_property_name()
{
}

//template <r_c_name T>
//constexpr auto get_property_changed_name()
//{
//    std::string str{"Changed"};
//    str.insert(0, T::c_name());
//    return w_cpp::viewLiteral(cstr.c_str());
//}

template <typename T>
class property_holder : public QObject
{
    W_OBJECT(property_holder)

    template <size_t I>
    using property_at =
        std::remove_reference_t<decltype(boost::pfr::get<I>(std::declval<T>()))>;

public:
    property_holder(QObject* parent = nullptr)
        : QObject{parent}
    {}

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
        constexpr static auto len{std::char_traits<char>::length(property_at<I>::c_name()) - 1};
        constexpr static w_cpp::StringView name{property_at<I>::c_name(),
                                                property_at<I>::c_name() + len};
        constexpr static auto signal{w_cpp::makeSignalBuilder(name,
                                                              &property_holder::property_changed<I>)
                                                             .build()};
    };
    W_CPP_SIGNAL(property_changed_signals)

    template <size_t I>
    auto get_property_value() const -> QVariant
    {
        const auto& property{boost::pfr::get<I>(aggregate)};
        return QVariant::fromValue(property.value);
    }

    template <size_t I>
    void set_property_value(QVariant variant)
    {
        auto& property{boost::pfr::get<I>(aggregate)};
        using value_t = decltype(std::declval<property_at<I>>().value);
        property.value = variant.value<value_t>();
        property_changed<I>();
    }

    template <size_t I, class = std::enable_if_t<(I < boost::pfr::tuple_size_v<T>)>>
    struct register_properties
    {
        constexpr static auto len{std::char_traits<char>::length(property_at<I>::c_name()) - 1};
        constexpr static w_cpp::StringView name{property_at<I>::c_name(),
                                                property_at<I>::c_name() + len};
        constexpr static auto property{w_cpp::makeProperty<QVariant>(name,
                                                                     w_cpp::viewLiteral("QVariant"))
                                           .setGetter(&property_holder::get_property_value<I>)
                                           .setGetter(&property_holder::get_property_value<I>)
                                           .setGetter(&property_holder::get_property_value<I>)};
    };
    W_CPP_PROPERTY(register_properties)

    T aggregate{};
};
} // namespace crudpp


W_OBJECT_IMPL_INLINE(crudpp::property_holder<T>, template <typename T>)
