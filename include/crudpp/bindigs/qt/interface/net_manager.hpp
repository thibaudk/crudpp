#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QSaveFile>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include <wobjectdefs.h>

#include <crudpp/macros.hpp>
#include <crudpp/bindigs/qt/wrappers/property_holder.hpp>
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

    void init(const QString& url)
    {
        rqst = {};
        set_pregix(url);

        auto conf = QSslConfiguration::defaultConfiguration();
        rqst.setSslConfiguration(conf);
        rqst.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        setTransferTimeout();

        connect(this, &QNetworkAccessManager::sslErrors,
                this, [] (QNetworkReply* reply, const QList<QSslError>& errors)
                { reply->ignoreSslErrors(errors); });

        connect(this, &QNetworkAccessManager::finished,
                this, [this] (QNetworkReply* reply)
                {
                    if (reply->error() != QNetworkReply::NoError)
                        emit replyError("net_manager reply error", reply->errorString());
                });
    }

    net_manager(net_manager const&) = delete;
    void operator = (net_manager const&) = delete;

    void set_pregix(const QString& url)
    {
        const auto new_prefix = url + '/';
        if (prefix != new_prefix) prefix = new_prefix;
    }

    void authenticate(const QString& identifier, const QString& secret)
    {
        USER_CLASS usr{};

        QJsonObject json;
        json[usr.username.c_name()] = identifier;
        json[usr.password.c_name()] = secret;

        std::string url{usr.table()};
        url += "/auth";

        authenticating = true;
        postToKey(url.c_str(),
                  QJsonDocument{json}.toJson(),
                  [this] (const QJsonObject& obj)
            {
                singleton<property_holder<USER_CLASS>>::instance().read(obj);
                emit loggedIn(true);
            },
            "Authentication",
            [this]() { emit loggedIn(false); });
    }

    void loggedIn(bool success, const QString& errorString = "")
    W_SIGNAL(loggedIn, success, errorString)

    void replyError(const QString& prefix = "", const QString& errorString = "")
    W_SIGNAL(replyError, prefix, errorString)

    void downloadFile(const char* key,
                      const QString& path,
                      const std::function<void (bool, const QString &)>&& callback,
                      const std::function<void (qint64, qint64)>&& onProgress = [](qint64 byteSent, qint64 totalBytes){})
    {
        setRequest(key);
        auto* reply = get(rqst);
        setCallback(reply,
                    [path, callback] (const QByteArray& bytes)
                    {
                        if (bytes.isValidUtf8())
                        {
                            qDebug() << QString(bytes);
                            callback(false, QString(bytes));
                            return;
                        }

                        QSaveFile file(path);
                        if (file.open(QIODevice::WriteOnly))
                        {
                            file.write(bytes);
                            if (file.commit())
                                callback(true, "");
                            else
                                callback(false, file.errorString());
                        }
                        else
                            callback(false, file.errorString());
                    });

        connect(reply,
                &QNetworkReply::uploadProgress,
                onProgress);
    }

    void getFromKey(const char* key,
                    const std::function<void (const QByteArray &)>&& callback,
                    const char* params = "")
    {
        setRequest(key);
        auto* reply = get(rqst);
        setCallback(reply,
                    std::forward<const std::function<void (const QByteArray &)>&&>(callback));
    }

    void putToKey(const char* key,
                  const QByteArray&& data,
                  const std::function<void (const QJsonObject &)>&& callback,
                  const QString&& errorPrefix = "",
                  const std::function<void ()>&& errorCallback = [](){},
                  const std::function<void (qint64, qint64)>&& onProgress = [](qint64 byteSent, qint64 totalBytes){})
    {
        setRequest(key);
        auto* reply = put(rqst, data);
        setCallback(reply,
                    std::forward<const std::function<void (const QJsonObject &)>&&>(callback),
                    std::forward<const QString&&>(errorPrefix),
                    std::forward<const std::function<void ()>&&>(errorCallback));

        connect(reply,
                &QNetworkReply::uploadProgress,
                onProgress);
    }

    void postToKey(const char* key,
                   const QByteArray&& data,
                   const std::function<void (const QJsonObject &)>&& callback,
                   const QString&& errorPrefix = "",
                   const std::function<void ()>&& errorCallback = [](){},
                   const std::function<void (qint64, qint64)>&& onProgress = [](qint64 byteSent, qint64 totalBytes){})
    {
        setRequest(key);
        auto* reply = post(rqst, data);
        setCallback(reply,
                    std::forward<const std::function<void (const QJsonObject &)>&&>(callback),
                    std::forward<const QString&&>(errorPrefix),
                    std::forward<const std::function<void ()>&&>(errorCallback));

        connect(reply,
                &QNetworkReply::uploadProgress,
                onProgress);
    }

    void deleteToKey(const char* key,
                     const QByteArray&& data,
                     const std::function<void (const QJsonObject &)>&& callback,
                     const QString&& errorPrefix = "")
    {
        setRequest(key);
        auto* reply = sendCustomRequest(rqst, "DELETE", data);
        setCallback(reply,
                    std::forward<const std::function<void (const QJsonObject &)>&&>(callback),
                    std::forward<const QString&&>(errorPrefix));
    }

    void deleteToKey(const char* key,
                     const std::function<void (const QJsonObject &)>&& callback,
                     const QString&& errorPrefix = "",
                     const std::function<void ()>&& errorCallback = [](){})
    {
        setRequest(key);
        auto* reply = deleteResource(rqst);
        setCallback(reply,
                    std::forward<const std::function<void (const QJsonObject &)>&&>(callback),
                    std::forward<const QString&&>(errorPrefix),
                    std::forward<const std::function<void ()>&&>(errorCallback));
    }

    void userChanged(int newId)
    W_SIGNAL(userChanged, newId)

    void clearanceChanged(int newClearance)
    W_SIGNAL(clearanceChanged, newClearance)

private:
    net_manager() {}

    QNetworkRequest rqst;
    QString prefix;

    void setCallback(QNetworkReply* reply,
                     const std::function<void (const QByteArray &)>&& callback)
    {
        connect(reply, &QNetworkReply::finished,
                [reply, callback, this]()
                {
                    if (reply->error() == QNetworkReply::NoError)
                        callback(reply->readAll());
                    else
                    {
                        const auto json{QJsonDocument::fromJson(reply->readAll()).object()};
                        QString error_str{};

                        if (json.contains("error") && json["error"].isString())
                            error_str = json["error"].toString();

                        emit replyError(reply->errorString(), error_str);
                    }

                    reply->deleteLater();
                });
    }

    void setCallback(QNetworkReply* reply,
                     const std::function<void (const QJsonObject &)>&& callback,
                     const QString&& errorPrefix,
                     const std::function<void ()>&& errorCallback = [](){})
    {
        connect(reply, &QNetworkReply::finished,
                this,
                [reply, callback, errorPrefix, errorCallback, this]()
                {
                    const auto json{QJsonDocument::fromJson(reply->readAll()).object()};
                    if (reply->error() == QNetworkReply::NoError)
                            callback(json);
                    else
                    {
                        QString error_str{};

                        if (json.contains("error") && json["error"].isString())
                            error_str = json["error"].toString();
                        else
                            error_str = reply->errorString();

                        emit replyError(errorPrefix, error_str);
                        errorCallback();
                    }

                    reply->deleteLater();
                });
    }

    void setRequest(const char* key, const char* params = "")
    {
        QUrl url{prefix + key + '?' + params};
        rqst.setUrl(url);
    }

    bool authenticating{false};
};

} // crudpp

#include "wobjectimpl.h"
W_OBJECT_IMPL(qt::net_manager)
