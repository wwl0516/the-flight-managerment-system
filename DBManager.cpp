#include "DBManager.h"

// 初始化静态成员
DBManager* DBManager::m_instance = nullptr;
QMutex DBManager::m_mutex;

DBManager::DBManager(QObject *parent) : QObject(parent)
{
    initDBConfig();
    // 加载 ODBC 驱动（仅初始化一次）
    if (QSqlDatabase::contains("QT_ODBC_CONN")) {
        m_db = QSqlDatabase::database("QT_ODBC_CONN");
    } else {
        m_db = QSqlDatabase::addDatabase("QODBC", "QT_ODBC_CONN");  // 自定义连接名，避免与其他连接冲突
    }
}

DBManager::~DBManager()
{
    disconnectDB();
}

// 全局获取单例（线程安全）
DBManager* DBManager::getInstance(QObject *parent)
{
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);  // 加锁，避免多线程同时创建
        if (m_instance == nullptr) {
            m_instance = new DBManager(parent);
        }
    }
    return m_instance;
}

// 初始化 ODBC 连接参数
void DBManager::initDBConfig()
{
    m_dsn = "QtODBC_MySQL";     // ODBC DSN 名称
    m_user = "GYT";            // 数据库用户名
    m_password = "123456";      // 数据库密码
    m_databaseName = "flight_manage_system_db";  // 你要操作的数据库名
}

// 连接数据库
bool DBManager::connectDB()
{
    QMutexLocker locker(&m_mutex);  // 线程安全

    if (m_db.isOpen()) {
        emit connectionStateChanged(true);
        emit operateResult(true, "数据库已连接！");
        return true;
    }

    // 配置连接参数（ODBC）
    m_db.setDatabaseName(m_dsn);
    m_db.setUserName(m_user);
    m_db.setPassword(m_password);

    // 打开连接
    bool success = m_db.open();
    if (success) {
        qInfo() << "[DB] 连接成功！DSN:" << m_dsn;
        emit connectionStateChanged(true);
        emit operateResult(true, "数据库连接成功！");
    } else {
        QString errMsg = "[DB] 连接失败：" + m_db.lastError().text();
        qCritical() << errMsg;
        emit connectionStateChanged(false);
        emit operateResult(false, errMsg);
    }
    return success;
}

// 断开连接
void DBManager::disconnectDB()
{
    QMutexLocker locker(&m_mutex);

    if (m_db.isOpen()) {
        m_db.close();
        qInfo() << "[DB] 连接已断开";
        emit connectionStateChanged(false);
        emit operateResult(true, "数据库已断开连接！");
    }
}

// 检查连接状态
bool DBManager::isConnected() const
{
    return m_db.isOpen();
}

// 验证日期格式
bool DBManager::isValidDateTimeFormat(const QString &dateStr)
{
    QDateTime dt = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
    return dt.isValid();
}

// 查询所有航班（返回 QVariantList）
QVariantList DBManager::queryAllFlights()
{
    QMutexLocker locker(&m_mutex);
    QVariantList result;

    if (!m_db.isOpen()) {
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    QSqlQuery query(m_db);

    QString sql = R"(
        SELECT Flight_id, Departure, Destination, depart_time, arrive_time,
               price, total_seats, remain_seats
        FROM flight
        ORDER BY depart_time DESC
    )";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 查询预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return result;
    }

    if (query.exec()) {
        while (query.next()) {
            QVariantMap flight;
            flight["flightId"] = query.value("Flight_id").toString();
            flight["departure"] = query.value("Departure").toString();
            flight["destination"] = query.value("Destination").toString();
            flight["departTime"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            flight["arriveTime"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            flight["price"] = query.value("price").toDouble();
            flight["totalSeats"] = query.value("total_seats").toInt();
            flight["remainSeats"] = query.value("remain_seats").toInt();
            result.append(flight);
        }
        emit operateResult(true, QString("查询成功，共 %1 条航班数据").arg(result.size()));
    } else {
        QString errMsg = "[DB] 查询失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
    }
    return result;
}

// 按航班号查询
QVariantMap DBManager::queryFlightByNum(const QString &flightNum)
{
    QMutexLocker locker(&m_mutex);
    QVariantMap result;

    if (!m_db.isOpen()) {
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    // 用 bindValue 绑定参数，避免 SQL 注入
    QSqlQuery query(m_db);
    QString sql = R"(
        SELECT Flight_id, Departure, Destination, depart_time, arrive_time,
               price, total_seats, remain_seats
        FROM flight
        WHERE Flight_id = :flightId
    )";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 查询预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return result;
    }

    query.bindValue(":flightNum", flightNum);
    if (query.exec() && query.next()) {
        result["flightId"] = query.value("Flight_id").toString();
        result["departure"] = query.value("Departure").toString();
        result["destination"] = query.value("Destination").toString();
        result["departTime"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        result["arriveTime"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        result["price"] = query.value("price").toDouble();
        result["totalSeats"] = query.value("total_seats").toInt();
        result["remainSeats"] = query.value("remain_seats").toInt();
        emit operateResult(true, "查询成功！");
    } else {
        emit operateResult(false, "查询失败：未找到该航班或查询出错！");
    }
    return result;
}

// 添加航班
bool DBManager::addFlight(
    const QString& Flight_id,
    const QString& Departure,
    const QString& Destination,
    const QString& depart_time,
    const QString& arrive_time,
    double price,
    int total_seats,
    int remain_seats
) {
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "添加失败：数据库未连接！");
        return false;
    }
    if (Flight_id.isEmpty() || Departure.isEmpty() || Destination.isEmpty()) {
        emit operateResult(false, "添加失败：航班号、出发地、目的地不能为空！");
        return false;
    }
    if (!isValidDateTimeFormat(depart_time) || !isValidDateTimeFormat(arrive_time)) {
        emit operateResult(false, "添加失败：时间格式错误！请输入 YYYY-MM-DD HH:MM:SS");
        return false;
    }
    if (QDateTime::fromString(depart_time, "yyyy-MM-dd HH:mm:ss") >= QDateTime::fromString(arrive_time, "yyyy-MM-dd HH:mm:ss")) {
        emit operateResult(false, "添加失败：起飞时间不能晚于降落时间！");
        return false;
    }
    if (price <= 0) {
        emit operateResult(false, "添加失败：票价必须大于 0！");
        return false;
    }
    if (total_seats <= 0 || remain_seats < 0 || remain_seats > total_seats) {
        emit operateResult(false, "添加失败：座位数无效（剩余座位不能大于总座位，且不能为负）！");
        return false;
    }

    // 检查航班号是否已存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT Flight_id FROM flight WHERE Flight_id = :Flight_id");
    checkQuery.bindValue(":Flight_id", Flight_id);
    if (checkQuery.exec() && checkQuery.next()) {
        emit operateResult(false, "添加失败：航班号 " + Flight_id + " 已存在！");
        return false;
    }

    // 插入数据
    QSqlQuery query(m_db);
    QString sql = R"(
        INSERT INTO flight (
            Flight_id, Departure, Destination, depart_time, arrive_time,
            price, total_seats, remain_seats
        ) VALUES (
            :Flight_id, :Departure, :Destination, :depart_time, :arrive_time,
            :price, :total_seats, :remain_seats
        )
    )";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 插入预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return false;
    }

    // 绑定所有参数
    query.bindValue(":Flight_id", Flight_id);
    query.bindValue(":Departure", Departure);
    query.bindValue(":Destination", Destination);
    query.bindValue(":depart_time", depart_time);  // 直接传字符串，SQL 自动解析为 datetime
    query.bindValue(":arrive_time", arrive_time);
    query.bindValue(":price", price);            // double 适配 decimal(10,2)
    query.bindValue(":total_seats", total_seats);
    query.bindValue(":remain_seats", remain_seats);

    bool success = query.exec();
    if (success) {
        emit operateResult(true, "航班 " + Flight_id + " 添加成功！");
    } else {
        QString errMsg = "[DB] 插入失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
    }
    return success;
}

// 更新航班价格
bool DBManager::updateFlightPrice(const QString& Flight_id, double newPrice)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "更新失败：数据库未连接！");
        return false;
    }
    if (newPrice <= 0) {
        emit operateResult(false, "更新失败：票价必须大于 0！");
        return false;
    }

    QSqlQuery query(m_db);
    QString sql = "UPDATE flight SET price = :newPrice WHERE Flight_id = :Flight_id";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 更新预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return false;
    }

    query.bindValue(":newPrice", newPrice);
    query.bindValue(":Flight_id", Flight_id);
    bool success = query.exec();

    if (success && query.numRowsAffected() > 0) {
        emit operateResult(true, "航班 " + Flight_id + " 价格更新为 " + QString::number(newPrice, 'f', 2) + " 元！");
    } else if (success && query.numRowsAffected() == 0) {
        emit operateResult(false, "更新失败：未找到航班 " + Flight_id + "！");
        success = false;
    } else {
        QString errMsg = "[DB] 更新失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
    }
    return success;
}

bool DBManager::updateFlightSeats(const QString& Flight_id, int newRemainSeats)
{
    QMutexLocker locker(&m_mutex);

    if(!m_db.isOpen()){
        emit operateResult(false, "更新失败：数据库未连接！");
        return false;
    }

    QSqlQuery getTotalSeatsQuery(m_db);
    getTotalSeatsQuery.prepare("SELECT total_seats FROM flight WHERE Flight_id = :Flight_id");
    getTotalSeatsQuery.bindValue(":Flight_id",Flight_id);
    if(!getTotalSeatsQuery.exec() || !getTotalSeatsQuery.next()){
        emit operateResult(false, "更新失败：未找到航班 " + Flight_id + "! ");
        return false;
    }
    int total_seats = getTotalSeatsQuery.value("total_seats").toInt();
    if(newRemainSeats < 0 || newRemainSeats > total_seats){
        emit operateResult(false, "更新失败：剩余座位不能小于 0 或大于总座位（" + QString::number(total_seats) + "）！");
        return false;
    }

    QSqlQuery query(m_db);
    QString sql = "UPDATE flight SET remain_seats = :newRemainSeats WHERE Flight_id = :Flight_id";

    if(!query.prepare(sql)){
        QString errMsg = "[DB]  更新预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return false;
    }

    query.bindValue(":newRemainSeats", newRemainSeats);
    query.bindValue(":Flight_id", Flight_id);
    bool success = query.exec();

    if(success && query.numRowsAffected() > 0){
        emit operateResult(true, "航班 " + Flight_id + " 剩余座位更新为 " + QString::number(newRemainSeats) + "！");
    }
    else{
        QString errMsg = "[DB] 更新失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
    }
    return success;
}

// 删除航班
bool DBManager::deleteFlight(const QString& Flight_id)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "删除失败：数据库未连接！");
        return false;
    }

    QSqlQuery query(m_db);
    QString sql = "DELETE FROM flight WHERE Flight_id = :Flight_id";
    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 删除预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return false;
    }

    query.bindValue(":Flight_id", Flight_id);
    bool success = query.exec();

    if (success && query.numRowsAffected() > 0) {
        emit operateResult(true, "航班 " + Flight_id + " 删除成功！");
    } else if (success && query.numRowsAffected() == 0) {
        emit operateResult(false, "删除失败：未找到航班 " + Flight_id + "！");
        success = false;
    } else {
        QString errMsg = "[DB] 删除失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
    }
    return success;
}
