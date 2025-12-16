#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>    // 新增：用于QML单例注册
#include <QJSEngine>     // 新增：用于QML单例注册
#include <QQmlContext>
#include "HuskarUI/husapp.h"
#include "DBManager.h"
#include <QFile>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 获取DBManager单例
    DBManager *dbManager = DBManager::getInstance(&app);
    bool connectSuccess = dbManager->connectDB();
    if (!connectSuccess) {
        qCritical() << "数据库连接失败！=";
    }

    QQmlApplicationEngine engine;

    // ========== 核心：注册DBManager为QML单例（修复捕获问题） ==========
    qmlRegisterSingletonType<DBManager>(
        "com.flight.db",          // 自定义QML模块名
        1, 0,                     // 模块版本号
        "DBManager",              // QML中访问的别名
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            // 使用全局应用实例作为父对象，避免捕获局部变量
            return DBManager::getInstance(QGuiApplication::instance());
        }
        );
    qmlRegisterSingletonType(QUrl("qrc:/GlobalSettings.qml"),"com.flight.globalVars",1,0,"GlobalSettings");

    // 原有逻辑保持不变
    HusApp::initialize(&engine);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("the_flight_managerment_system", "RootContainer");

    return app.exec();
}
