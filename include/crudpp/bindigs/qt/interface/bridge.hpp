#pragma once

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <wobjectdefs.h>

#include <crudpp/bindigs/qt/wrappers/list_model.hpp>

class QQmlContext;

namespace qt
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

    void init();

    template <typename ...Types>
    void registerQml() { (registerSingleQml<Types>(), ...); }

    bridge(bridge const&) = delete;
    void operator = (bridge const&) = delete;

    QQmlContext* context() { return engine->rootContext(); }

    void onLogin(bool success, const QString& errorString) const;

    void onException(const QString& prefix,
                     const QString& errorString) const;

    void setHost(const QString& newHost) const;
    W_INVOKABLE(setHost)

    void authenticate(const QString& username, const QString& password) const;
    W_INVOKABLE(authenticate, (const QString&, const QString&))

    void updatePwd(const QString& newPwd) const;
    W_INVOKABLE(updatePwd)

    void resetPwd(int id) const;
    W_INVOKABLE(resetPwd)

    void setQmlObject(QObject* obj) noexcept { qmlObject = obj; }

    bool hasFlag(int value, int flag) const noexcept { return value & flag; }
    W_INVOKABLE(hasFlag, (int, int))

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
        const auto qmlName{uri + "ListModel"};

        qmlRegisterType<list_model<T>>(uri.c_str(), 1, 0, qmlName.c_str());
    }
};

} // namespace qt
