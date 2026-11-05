#include <QtCore>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationDomain("org.tal.cutedeck.imageview");
    QCoreApplication::setOrganizationName("tal.org");
    QCoreApplication::setApplicationName("CuteDeck Image View");
    QCoreApplication::setApplicationVersion("0.0.1");

    QQmlApplicationEngine engine;

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("CuteDeckImageView", "Main");

    return app.exec();
}
