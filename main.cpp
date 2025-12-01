#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "HuskarUI/husapp.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    HusApp::initialize(&engine);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("the_flight_managerment_system", "Main");

    return app.exec();
}
