#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <crudpp/bindigs/qt/interface/net_manager.hpp>
#include <crudpp/bindigs/qt/interface/bridge.hpp>
#include <crudpp/bindigs/qt/wrappers/controller.hpp>
#include <crudpp/macros.hpp>
#include <singleton.hpp>

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

    bridge& b{bridge::instance()};
    b.init();
    b.registerQml<CLASSES_STRING>();

    USER_CLASS u{};
    auto User = property_holder{boost::pfr::structure_tie(u)};

    const auto uri{make_uri<USER_CLASS>()};
    qmlRegisterUncreatableType<decltype(User)>(uri.c_str(), 1, 0, uri.c_str(), "");

    b.context()->setContextProperty(USER_CLASS::table(), &User);

    make_ctls<CLASSES_STRING>();

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
        }, Qt::QueuedConnection);

    b.engine->load(url);

    return app.exec();
}
