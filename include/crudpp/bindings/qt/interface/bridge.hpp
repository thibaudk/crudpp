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

    QQmlContext* context();

    void onLogin();

    void onException(const QString& prefix, const QString& errorString);

#ifndef EMSCRIPTEN
    void setHost(const QString& newHost) const;
    W_INVOKABLE(setHost)
#endif

    void authenticate(const QString& username, const QString& password);
    W_INVOKABLE(authenticate)

    void updatePwd(const QString& newPwd) const;
    W_INVOKABLE(updatePwd)

    void resetPwd(int id) const;
    W_INVOKABLE(resetPwd)

    void setQmlObject(QObject* obj) noexcept;

    bool hasFlag(int value, int flag) const noexcept;
    W_INVOKABLE(hasFlag)

    void logout() const
    W_SIGNAL(logout)

    void increment_load();
    void decrement_load();
    void dequeue() const;

    bool getLoading() const;
    void loadingChanged()
    W_SIGNAL(loadingChanged)

    W_PROPERTY(bool, loading
                          READ getLoading
                              NOTIFY loadingChanged)

    float getDownloadProgress() const;
    void setDownloadProgress(float newDownloadProgress);
    void downloadProgressChanged()
    W_SIGNAL(downloadProgressChanged)

    W_PROPERTY(float, downloadProgress
                          READ getDownloadProgress
                              NOTIFY downloadProgressChanged)

    QQmlApplicationEngine* engine;

private:
    bridge();

    QObject* qmlObject;
    bool loading;
    int items_loading;
    float downloadProgress;

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
