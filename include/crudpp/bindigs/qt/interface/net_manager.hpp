#pragma once

#include <QObject>
#include <QNetworkAccessManager>

#include <wobjectdefs.h>

#include <crudpp/macros.hpp>
#include <singleton.hpp>
#include STRINGIFY_MACRO(INCLUDE)

namespace qt
{
class smtp;

class net_manager final : public QNetworkAccessManager
{
    W_OBJECT(net_manager)

public:
    static net_manager& instance()
    {
        static net_manager instance;
        return instance;
    }

#ifndef EMSCRIPTEN
    void init(const QString& url);
    void set_prefix(const QString& url);
#endif
    void init();

    net_manager(net_manager const&) = delete;
    void operator = (net_manager const&) = delete;

    void authenticate(const QString& identifier, const QString& secret);

    void loggedIn(bool success, const QString& errorString = "")
    W_SIGNAL(loggedIn, success, errorString)

    void replyError(const QString& prefix = "", const QString& errorString = "")
    W_SIGNAL(replyError, prefix, errorString)

    void downloadFile(const char* key,
                      const QString& path,
                      const std::function<void (bool, const QString &)>&& callback,
                      const std::function<void (qint64, qint64)>&& onProgress = [](qint64 byteSent, qint64 totalBytes){});

    void getFromKey(const char* key,
                    const std::function<void (const QByteArray &)>&& callback,
                    const char* params = "");

    void searchAtKey(const char* key,
                     const std::function<void (const QByteArray &)>&& callback,
                     const char* params);

    void putToKey(const char* key,
                  const QByteArray&& data,
                  const std::function<void (const QJsonObject &)>&& callback,
                  const QString&& errorPrefix = "",
                  const std::function<void ()>&& errorCallback = [](){},
                  const std::function<void (qint64, qint64)>&& onProgress = [](qint64 byteSent, qint64 totalBytes){});

    void postToKey(const char* key,
                   const QByteArray&& data,
                   const std::function<void (const QJsonObject &)>&& callback,
                   const QString&& errorPrefix = "",
                   const std::function<void ()>&& errorCallback = [](){},
                   const std::function<void (qint64, qint64)>&& onProgress = [](qint64 byteSent, qint64 totalBytes){});

    void deleteToKey(const char* key,
                     const QByteArray&& data,
                     const std::function<void (const QJsonObject &)>&& callback,
                     const QString&& errorPrefix = "");

    void deleteToKey(const char* key,
                     const std::function<void (const QJsonObject &)>&& callback,
                     const QString&& errorPrefix = "",
                     const std::function<void ()>&& errorCallback = [](){});

    void userChanged(int newId)
    W_SIGNAL(userChanged, newId)

    void clearanceChanged(int newClearance)
    W_SIGNAL(clearanceChanged, newClearance)

private:
    net_manager() {}

    QNetworkRequest rqst{};
    QNetworkReply* search_rep;

#ifndef EMSCRIPTEN
    QString prefix;
#endif

    void setCallback(QNetworkReply* reply,
                     const std::function<void (const QByteArray &)>&& callback);

    void setCallback(QNetworkReply* reply,
                     const std::function<void (const QJsonObject &)>&& callback,
                     const QString&& errorPrefix,
                     const std::function<void ()>&& errorCallback = [](){});

    void setRequest(const char* key, const char* params = "");

    bool authenticating{false};
};

} // crudpp
