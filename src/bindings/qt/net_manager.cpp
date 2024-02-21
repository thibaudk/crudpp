#include <QSslConfiguration>
#include <QSaveFile>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include "wobjectimpl.h"

#include <crudpp/bindigs/qt/interface/net_manager.hpp>
#include <crudpp/bindigs/qt/wrappers/property_holder.hpp>

namespace qt
{
W_OBJECT_IMPL(net_manager)

void net_manager::init(const QString &url)
{
    rqst = {};
    set_prefix(url);

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

void net_manager::set_prefix(const QString &url)
{
    const auto new_prefix = url + '/';
    if (prefix != new_prefix) prefix = new_prefix;
}

void net_manager::authenticate(const QString &identifier, const QString &secret)
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
            // singleton<property_holder<USER_CLASS>>::instance().read(obj);
            emit loggedIn(true);
        },
        "Authentication",
        [this]() { emit loggedIn(false); });
}

void net_manager::downloadFile(const char *key,
                               const QString &path,
                               const std::function<void (bool, const QString &)> &&callback,
                               const std::function<void (qint64, qint64)> &&onProgress)

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

void net_manager::getFromKey(const char *key,
                             const std::function<void (const QByteArray &)> &&callback,
                             const char *params)
{
    setRequest(key);
    auto* reply = get(rqst);
    setCallback(reply,
                std::forward<const std::function<void (const QByteArray &)>&&>(callback));
}

void net_manager::putToKey(const char *key,
                           const QByteArray &&data,
                           const std::function<void (const QJsonObject &)> &&callback,
                           const QString &&errorPrefix, const std::function<void ()> &&errorCallback,
                           const std::function<void (qint64, qint64)> &&onProgress)
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

void net_manager::postToKey(const char *key,
                            const QByteArray &&data,
                            const std::function<void (const QJsonObject &)> &&callback,
                            const QString &&errorPrefix, const std::function<void ()> &&errorCallback,
                            const std::function<void (qint64, qint64)> &&onProgress)
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

void net_manager::deleteToKey(const char *key,
                              const QByteArray &&data,
                              const std::function<void (const QJsonObject &)> &&callback,
                              const QString &&errorPrefix)
{
    setRequest(key);
    auto* reply = sendCustomRequest(rqst, "DELETE", data);
    setCallback(reply,
                std::forward<const std::function<void (const QJsonObject &)>&&>(callback),
                std::forward<const QString&&>(errorPrefix));
}

void net_manager::deleteToKey(const char *key,
                              const std::function<void (const QJsonObject &)> &&callback,
                              const QString &&errorPrefix,
                              const std::function<void ()> &&errorCallback)
{
    setRequest(key);
    auto* reply = deleteResource(rqst);
    setCallback(reply,
                std::forward<const std::function<void (const QJsonObject &)>&&>(callback),
                std::forward<const QString&&>(errorPrefix),
                std::forward<const std::function<void ()>&&>(errorCallback));
}

void net_manager::setCallback(QNetworkReply *reply,
                              const std::function<void (const QByteArray &)> &&callback)
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

void net_manager::setCallback(QNetworkReply *reply,
                              const std::function<void (const QJsonObject &)> &&callback,
                              const QString &&errorPrefix,
                              const std::function<void ()> &&errorCallback)
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

void net_manager::setRequest(const char *key, const char *params)
{
    QUrl url{prefix + key + '?' + params};
    rqst.setUrl(url);
}

} // qt
