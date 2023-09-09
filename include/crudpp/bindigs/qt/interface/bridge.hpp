#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <wobjectdefs.h>

#include <crudpp/bindigs/qt/wrappers/list_model.hpp>
#include "net_manager.hpp"

class QQmlContext;

namespace crudpp
{
class bridge final : public QObject
{
    W_OBJECT(bridge)

public:
    static bridge& instance()
    {
        static bridge instance;
        return instance;
    }

    void init()
    {
        connect(&net_manager::instance(),
                &net_manager::loggedIn,
                this,
                &bridge::onLogin);

        connect(&net_manager::instance(),
                &net_manager::replyError,
                this,
                &bridge::onException);
    }

    template <typename ...Types>
    void registerQml() { (registerSingleQml<Types>(), ...); }

    bridge(bridge const&) = delete;
    void operator = (bridge const&) = delete;

    QQmlContext* context() { return engine->rootContext(); }

    void onLogin(bool success, const QString& errorString) const
    {
        QMetaObject::invokeMethod(qmlObject,
                                  "onLogin",
                                  Q_ARG(bool, success),
                                  Q_ARG(QString, errorString));
    }

    void onException(const QString& prefix,
                     const QString& errorString) const
    {
        QMetaObject::invokeMethod(qmlObject,
                                  "onException",
                                  Q_ARG(QString, prefix),
                                  Q_ARG(QString, errorString));
    }

    void authenticate(const QString& username, const QString& password) const
    {
        net_manager::instance().authenticate(username, password);
    }
    W_INVOKABLE(authenticate, (const QString&, const QString&))

    void updatePwd(const QString& newPwd) const
    {
        QJsonObject json;
        json["password"] = newPwd;

        changePwd("changePassword", json);
    }
    W_INVOKABLE(updatePwd)

    void resetPwd(int id) const
    {
        QJsonObject json;
        json["id"] = id;

        changePwd("resetPassword", json);
    }
    W_INVOKABLE(resetPwd)

    void setQmlObject(QObject* obj) noexcept { qmlObject = obj; };

    bool hasFlag(int value, int flag) const noexcept
    {
        return value & flag;
    }
    W_INVOKABLE(hasFlag, (int, int))

    void logout() const
    W_SIGNAL(logout)

    void loaded() const
    W_SIGNAL(loaded)

    float getDownloadProgress() const { return downloadProgress; }
    void setDownloadProgress(float newDownloadProgress)
    {
        downloadProgress = newDownloadProgress;
        emit downloadProgressChanged();
    }
    void downloadProgressChanged()
    W_SIGNAL(downloadProgressChanged);

    W_PROPERTY(float, downloadProgress READ getDownloadProgress WRITE setDownloadProgress NOTIFY downloadProgressChanged)

    QQmlApplicationEngine* engine{new QQmlApplicationEngine{}};

private:
    bridge() {};

    QObject* qmlObject;

    float downloadProgress{-1.f};

    void changePwd(const char* key, const QJsonObject& json) const
    {
        net_manager::instance().putToKey(key,
            QJsonDocument(json).toJson(),
            [this] (const QJsonObject& rep)
            { emit loaded(); },
            "changePwd error");
    }

    template <typename T>
    void registerSingleQml()
    {
        const auto uri{make_uri<T>()};
        const auto qmlName{uri + "ListModel"};

        qmlRegisterType<list_model<T>>(uri.c_str(), 1, 0, qmlName.c_str());
    }
};

} // namespace crudpp

#include <wobjectimpl.h>
W_OBJECT_IMPL(crudpp::bridge);
