#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <crudpp/bindings/qt/interface/net_manager.hpp>
#include <crudpp/bindings/qt/interface/bridge.hpp>
#include <crudpp/bindings/qt/wrappers/property_holder.hpp>
#include <crudpp/bindings/qt/wrappers/sort_filter.hpp>
#include <crudpp/macros.hpp>
#include <singleton.hpp>

#include STRINGIFY_MACRO(INCLUDE)

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    using namespace qt;

    bridge& b{bridge::instance()};

#ifndef EMSCRIPTEN
    qDebug() << "Device supports OpenSSL: " << QSslSocket::supportsSsl();
    QString host{"http://localhost:8080"};

    for (int i = 0; i < argc; i++)
        if (QString::compare(argv[i], "--host") == 0)
        {
            host = argv[i + 1];
            break;
        }

    qDebug() << "Host :" << host;
    net_manager::instance().init(host);
#else
    b.context()->setContextProperty("EMSCRIPTEN", QVariant{true});
    net_manager::instance().init();
#endif

    b.init();
    b.registerQml<CLASSES_STRING>();

#ifdef USER_CLASS
    b.context()->setContextProperty("appUser",
                                    &singleton<property_holder<user>>::instance());
#endif

    qmlRegisterType<sort_filter>("QSortFilter", 1, 0, "QSortFilter");

    // qml engine
    const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
    QObject::connect(b.engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [&b, url]
        (QObject* obj, const QUrl &objUrl)
        {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
            else
                b.setQmlObject(obj);
        },
        Qt::QueuedConnection);

    b.engine->load(url);

    return app.exec();
}
