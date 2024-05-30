#include <QJsonDocument>
#include <QJsonObject>

#include <wobjectimpl.h>

#include <crudpp/bindings/qt/interface/bridge.hpp>
#include <crudpp/bindings/qt/interface/net_manager.hpp>

namespace qt
{
W_OBJECT_IMPL(bridge)

void bridge::init()
{
    qmlRegisterUncreatableType<bridge>("Interface", 1, 0, "Bridge", "");
    context()->setContextProperty("bridge", this);

    connect(&net_manager::instance(),
            &net_manager::loggedIn,
            this,
            &bridge::onLogin);

    connect(&net_manager::instance(),
            &net_manager::replyError,
            this,
            &bridge::onException);
}

void bridge::onLogin(bool success, const QString &errorString) const
{
    QMetaObject::invokeMethod(qmlObject,
                              "onLogin",
                              Q_ARG(bool, success),
                              Q_ARG(QString, errorString));
}

void bridge::onException(const QString &prefix, const QString &errorString) const
{
    QMetaObject::invokeMethod(qmlObject,
                              "onException",
                              Q_ARG(QString, prefix),
                              Q_ARG(QString, errorString));
}

#ifndef EMSCRIPTEN
void bridge::setHost(const QString &newHost) const
{
    net_manager::instance().set_prefix(newHost);
}
#endif

void bridge::authenticate(const QString &username, const QString &password) const
{
    net_manager::instance().authenticate(username, password);
}

void bridge::updatePwd(const QString &newPwd) const
{
    QJsonObject json;
    json["password"] = newPwd;

    changePwd("changePassword", json);
}

void bridge::resetPwd(int id) const
{
    QJsonObject json;
    json["id"] = id;

    changePwd("resetPassword", json);
}

void bridge::setDownloadProgress(float newDownloadProgress)
{
    downloadProgress = newDownloadProgress;
    emit downloadProgressChanged();
}

void bridge::changePwd(const char *key, const QJsonObject &json) const
{
    net_manager::instance().putToKey(key,
        QJsonDocument(json).toJson(),
        [this] (const QJsonObject& rep)
        { emit loaded(); },
        "changePwd error");
}

} // qt
