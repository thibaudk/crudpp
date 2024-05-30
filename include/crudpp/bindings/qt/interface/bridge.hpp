#pragma once

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <wobjectdefs.h>

#include <crudpp/concepts.hpp>
#include <crudpp/bindings/qt/utils.hpp>

class QQmlContext;

namespace qt
{
template <typename T>
class list_model;

class bridge final : public QObject
{
    W_OBJECT(bridge)

public:
    static bridge& instance()
    {
        static bridge instance;
        return instance;
    }

    void init();

    template <typename ...Types>
    void registerQml() { (registerSingleQml<Types>(), ...); }

    bridge(bridge const&) = delete;
    void operator = (bridge const&) = delete;

    QQmlContext* context() { return engine->rootContext(); }

    void onLogin(bool success, const QString& errorString) const;

    void onException(const QString& prefix,
                     const QString& errorString) const;

#ifndef EMSCRIPTEN
    void setHost(const QString& newHost) const;
    W_INVOKABLE(setHost)
#endif

    void authenticate(const QString& username, const QString& password) const;
    W_INVOKABLE(authenticate)

    void updatePwd(const QString& newPwd) const;
    W_INVOKABLE(updatePwd)

    void resetPwd(int id) const;
    W_INVOKABLE(resetPwd)

    void setQmlObject(QObject* obj) noexcept { qmlObject = obj; }

    bool hasFlag(int value, int flag) const noexcept { return value & flag; }
    W_INVOKABLE(hasFlag)

    void logout() const
    W_SIGNAL(logout)

    void loaded() const
    W_SIGNAL(loaded)

    float getDownloadProgress() const { return downloadProgress; }
    void setDownloadProgress(float newDownloadProgress);
    void downloadProgressChanged()
    W_SIGNAL(downloadProgressChanged)

    W_PROPERTY(float, downloadProgress
                      READ getDownloadProgress
                      WRITE setDownloadProgress
                      NOTIFY downloadProgressChanged)

    QQmlApplicationEngine* engine{new QQmlApplicationEngine{}};

private:
    bridge() {}

    QObject* qmlObject;

    float downloadProgress{-1.f};

    void changePwd(const char* key, const QJsonObject& json) const;

    template <typename T>
    void registerSingleQml()
    {
        const auto uri{make_uri<T>()};
        const auto listModelName{uri + "ListModel"};

        // TODO: handle lists of objects without primary key
        qmlRegisterType<list_model<T>>(uri.c_str(), 1, 0, listModelName.c_str());

        if constexpr(crudpp::r_primary_key<T>)
        {
            const auto singleName{"Single" + uri};
            qmlRegisterType<property_holder<T>>(uri.c_str(), 1, 0, singleName.c_str());
        }
    }
};

} // namespace qt
