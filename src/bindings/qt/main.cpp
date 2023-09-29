#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <crudpp/bindigs/qt/interface/net_manager.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include <crudpp/bindigs/qt/wrappers/controller.hpp>
#include <crudpp/bindigs/qt/wrappers/property_holder.hpp>
#include <crudpp/macros.hpp>

#include STRINGIFY_MACRO(INCLUDE)

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    using namespace crudpp;

    qDebug() << "Device supports OpenSSL: " << QSslSocket::supportsSsl();

    QString host{"http://127.0.0.0:8080"};

    for (int i = 0; i < argc; i++)
        if (QString::compare(argv[i], "--host") == 0)
        {
            host = argv[i + 1];
            break;
        }

    qDebug() << "Host :" << host;
    net_manager::instance().init(host);

    bridge::instance().init();
    bridge::instance().registerQml<CLASSES_STRING>();
//    client::instance().init();

    make_ctls<CLASSES_STRING>();

    property_holder<USER_CLASS> p{};

    // qml engine
    const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
    QObject::connect(bridge::instance().engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url]
        (QObject* obj, const QUrl &objUrl)
        {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
            else
                bridge::instance().setQmlObject(obj);
        }, Qt::QueuedConnection);

    bridge::instance().engine->load(url);

    return app.exec();
}
