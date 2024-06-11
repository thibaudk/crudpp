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

QQmlContext* bridge::context() { return engine->rootContext(); }

void bridge::onLogin()
{
    QMetaObject::invokeMethod(qmlObject, "onLogin");
    decrement_load();
}

void bridge::onException(const QString& prefix, const QString& errorString)
{
    items_loading = 0;
    if (loading)
    {
        loading = false;
        emit loadingChanged();
    }

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

void bridge::authenticate(const QString& username, const QString& password)
{
    increment_load();
    net_manager::instance().authenticate(username, password);
}

void bridge::updatePwd(const QString& newPwd) const
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

void bridge::setQmlObject(QObject* obj) noexcept { qmlObject = obj; }

bool bridge::hasFlag(int value, int flag) const noexcept { return value & flag; }

void bridge::increment_load()
{
    items_loading++;
    if (loading) return;
    loading = true;
    emit loadingChanged();
}

void bridge::decrement_load()
{
    if (items_loading < 1) return;
    items_loading--;

    if (items_loading) return;
    loading = false;
    emit loadingChanged();
}

void bridge::dequeue() const
{
    QMetaObject::invokeMethod(qmlObject, "dequeue");
}

bool bridge::getLoading() const { return loading; }

float bridge::getDownloadProgress() const { return downloadProgress; }

void bridge::setDownloadProgress(float newDownloadProgress)
{
    if (newDownloadProgress == downloadProgress) return;
    downloadProgress = newDownloadProgress;
    emit downloadProgressChanged();
}

bridge::bridge()
    : engine {new QQmlApplicationEngine}
    , loading{false}
    , items_loading{0}
    , downloadProgress{-1.f}
{}

void bridge::changePwd(const char *key, const QJsonObject &json) const
{
    net_manager::instance().putToKey(key,
        QJsonDocument(json).toJson(),
        [this] (const QJsonObject& rep)
        {},
        "changePwd error");
}

} // qt
