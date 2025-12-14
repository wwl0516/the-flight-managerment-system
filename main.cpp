#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>    // 新增：用于QML单例注册
#include <QJSEngine>     // 新增：用于QML单例注册
#include <QQmlContext>
#include "HuskarUI/husapp.h"
#include "DBManager.h"

void test(DBManager* dbManager){
    qInfo() << "=== 开始测试 DBManager ===";

    qInfo() << "\n1. 测试连接数据库...";
    // 测试数据（可根据你的数据库调整，确保航班号唯一）
    const QString testFlightId = "TEST1234";  // 测试航班号（避免与已有航班冲突）
    const QString testDeparture = "广州";
    const QString testDestination = "深圳";
    const QString testDepartTime = "2025-12-02 09:00:00";
    const QString testArriveTime = "2025-12-02 10:30:00";
    const double testPrice = 380.50;
    const int testTotalSeats = 100;
    const int testRemainSeats = 100;

    qInfo() << "\n2. 测试添加航班（航班号：" << testFlightId << "）...";
    bool addSuccess = dbManager->addFlight(
        testFlightId, testDeparture, testDestination,
        testDepartTime, testArriveTime, testPrice,
        testTotalSeats, testRemainSeats
        );
    if (addSuccess) {
        qInfo() << "添加航班成功！";
    } else {
        qWarning() << "添加航班失败（可能航班号已存在，换一个测试航班号试试）";
    }

    qInfo() << "\n3. 测试查询所有航班...";
    QVariantList allFlights = dbManager->queryAllFlights();
    dbManager->printFlightList(allFlights);

    qInfo() << "\n4. 测试按航班号查询（" << testFlightId << "）...";
    QVariantMap targetFlight = dbManager->queryFlightByNum(testFlightId);
    dbManager->printFlight(targetFlight);

    qInfo() << "\n5. 测试更新票价（改为 420.00 元）...";
    bool updatePriceSuccess = dbManager->updateFlightPrice(testFlightId, 420.00);
    if (updatePriceSuccess) {
        // 更新后重新查询，验证结果
        QVariantMap updatedFlight = dbManager->queryFlightByNum(testFlightId);
        qInfo() << "更新后票价：" << updatedFlight["price"].toDouble() << "元";
    }

    qInfo() << "\n6. 测试更新剩余座位（改为 88 个）...";
    bool updateSeatsSuccess = dbManager->updateFlightSeats(testFlightId, 88);
    if (updateSeatsSuccess) {
        QVariantMap updatedFlight = dbManager->queryFlightByNum(testFlightId);
        qInfo() << "更新后剩余座位：" << updatedFlight["remainSeats"].toInt() << "个";
    }

    qInfo() << "\n7. 测试删除航班（" << testFlightId << "）...";
    bool deleteSuccess = dbManager->deleteFlight(testFlightId);
    if (deleteSuccess) {
        qInfo() << "删除航班成功！";
        // 删除后查询所有航班，验证是否已删除
        QVariantList flightsAfterDelete = dbManager->queryAllFlights();
        dbManager->printFlightList(flightsAfterDelete);
    }

    qInfo() << "\n8. 测试断开数据库...";
    dbManager->disconnectDB();
    qInfo() << "断开连接成功！";

    qInfo() << "\n=== DBManager 测试完成 ===";
    return;
}

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

    // 原有逻辑保持不变
    HusApp::initialize(&engine);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("the_flight_managerment_system", "Login");

    //test(dbManager);

    return app.exec();
}
