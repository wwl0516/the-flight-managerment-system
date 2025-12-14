#include "DBManager.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>

// 初始化静态成员
DBManager* DBManager::m_instance = nullptr;
QMutex DBManager::m_mutex;

DBManager::DBManager(QObject *parent) : QObject(parent)
    , m_isAdminLoggedIn(false)
    , m_currentAdminId(-1)
    // 新增：用户相关成员变量初始化
    , m_isUserLoggedIn(false)
    , m_currentUserId(-1)
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
        // 连接已存在时，确保用户表已初始化
        initUserTable();
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
        // 初始化用户表（确保表存在）
        initUserTable();
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
    // 重置用户登录状态
    m_isUserLoggedIn = false;
    m_currentUserId = -1;
    m_currentUserName.clear();
    m_currentUserEmail.clear();
    emit userLoginStateChanged(false);
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

// ========== 新增：用户表初始化 ==========
void DBManager::initUserTable()
{
    if (!m_db.isOpen()) {
        qCritical() << "[DB] 初始化用户表失败：数据库未连接！";
        return;
    }

    QSqlQuery query(m_db);
    // 创建用户表（t_users），确保表结构存在
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS t_users (
            id INT AUTO_INCREMENT PRIMARY KEY,
            email VARCHAR(255) NOT NULL UNIQUE,
            username VARCHAR(50) NOT NULL UNIQUE,
            password VARCHAR(64) NOT NULL,
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
    )";

    if (!query.exec(createTableSql)) {
        qCritical() << "[DB] 初始化用户表失败：" << query.lastError().text();
    } else {
        qInfo() << "[DB] 用户表初始化成功（或已存在）";
    }
}

// ========== 新增：邮箱格式验证 ==========
bool DBManager::isValidEmailFormat(const QString& email)
{
    // 标准邮箱正则表达式
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QRegularExpressionMatch match = emailRegex.match(email);
    return match.hasMatch();
}

// ========== 新增：密码强度验证 ==========
bool DBManager::isValidPasswordStrength(const QString& password)
{
    // 至少8位，包含字母和数字（可根据需求调整）
    if (password.length() < 8) {
        return false;
    }
    bool hasLetter = false;
    bool hasNumber = false;
    for (const QChar &c : password) {
        if (c.isLetter()) {
            hasLetter = true;
        } else if (c.isNumber()) {
            hasNumber = true;
        }
        if (hasLetter && hasNumber) {
            break;
        }
    }
    return hasLetter && hasNumber;
}

// ========== 新增：用户名唯一性检查 ==========
bool DBManager::isUsernameExists(const QString& username)
{
    if (!m_db.isOpen()) {
        qCritical() << "[DB] 检查用户名失败：数据库未连接！";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT username FROM t_users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qCritical() << "[DB] 检查用户名失败：" << query.lastError().text();
        return false;
    }

    return query.next(); // 有结果则用户名已存在
}

// ========== 新增：邮箱唯一性检查 ==========
bool DBManager::isEmailExists(const QString& email)
{
    if (!m_db.isOpen()) {
        qCritical() << "[DB] 检查邮箱失败：数据库未连接！";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT email FROM t_users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qCritical() << "[DB] 检查邮箱失败：" << query.lastError().text();
        return false;
    }

    return query.next(); // 有结果则邮箱已存在
}

// ========== 新增：密码加密（SHA256） ==========
QString DBManager::encryptPassword(const QString& password)
{
    QByteArray byteArray = password.toUtf8();
    // SHA256 加密
    byteArray = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    // 转换为十六进制字符串
    return byteArray.toHex();
}

// ========== 新增：用户注册 ==========
bool DBManager::userRegister(const QString& email, const QString& username, const QString& password)
{
    QMutexLocker locker(&m_mutex); // 线程安全

    // 1. 检查数据库连接
    if (!m_db.isOpen()) {
        QString errMsg = "注册失败：数据库未连接！";
        emit userRegisterFailed(errMsg);
        return false;
    }

    // 2. 空值检查
    if (email.isEmpty() || username.isEmpty() || password.isEmpty()) {
        QString errMsg = "注册失败：邮箱、用户名、密码不能为空！";
        emit userRegisterFailed(errMsg);
        return false;
    }

    // 3. 邮箱格式验证
    if (!isValidEmailFormat(email)) {
        QString errMsg = "注册失败：邮箱格式错误！";
        emit userRegisterFailed(errMsg);
        return false;
    }

    // 4. 密码强度验证
    if (!isValidPasswordStrength(password)) {
        QString errMsg = "注册失败：密码至少8位，且包含字母和数字！";
        emit userRegisterFailed(errMsg);
        return false;
    }

    // 5. 用户名唯一性检查
    if (isUsernameExists(username)) {
        QString errMsg = "注册失败：用户名 " + username + " 已存在！";
        emit userRegisterFailed(errMsg);
        return false;
    }

    // 6. 邮箱唯一性检查
    if (isEmailExists(email)) {
        QString errMsg = "注册失败：邮箱 " + email + " 已被注册！";
        emit userRegisterFailed(errMsg);
        return false;
    }

    // 7. 加密密码
    QString encryptedPwd = encryptPassword(password);

    // 8. 插入用户数据
    QSqlQuery query(m_db);
    QString insertSql = R"(
        INSERT INTO t_users (email, username, password)
        VALUES (:email, :username, :password)
    )";

    if (!query.prepare(insertSql)) {
        QString errMsg = "[DB] 注册预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit userRegisterFailed("注册失败：数据库操作错误！");
        return false;
    }

    query.bindValue(":email", email);
    query.bindValue(":username", username);
    query.bindValue(":password", encryptedPwd);

    bool success = query.exec();
    if (success) {
        qInfo() << "[DB] 用户 " << username << " 注册成功！";
        emit userRegisterSuccess(username);
        emit operateResult(true, "注册成功！");
    } else {
        QString errMsg = "[DB] 注册插入失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit userRegisterFailed("注册失败：" + query.lastError().text());
    }

    return success;
}

// ========== 新增：用户登录 ==========
bool DBManager::userLogin(const QString& username, const QString& password)
{
    QMutexLocker locker(&m_mutex); // 线程安全

    // 1. 检查数据库连接
    if (!m_db.isOpen()) {
        QString errMsg = "登录失败：数据库未连接！";
        emit userLoginFailed(errMsg);
        return false;
    }

    // 2. 空值检查
    if (username.isEmpty() || password.isEmpty()) {
        QString errMsg = "登录失败：用户名或密码不能为空！";
        emit userLoginFailed(errMsg);
        return false;
    }

    // 3. 查询用户信息
    QSqlQuery query(m_db);
    query.prepare("SELECT id, email, password FROM t_users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        QString errMsg = "[DB] 登录查询失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit userLoginFailed("登录失败：数据库操作错误！");
        return false;
    }

    // 4. 检查用户是否存在
    if (!query.next()) {
        emit userLoginFailed("登录失败：用户名不存在！");
        return false;
    }

    // 5. 验证密码
    QString storedPwd = query.value("password").toString();
    QString inputPwd = encryptPassword(password);
    if (storedPwd != inputPwd) {
        emit userLoginFailed("登录失败：密码错误！");
        return false;
    }

    // 6. 更新用户登录状态
    m_isUserLoggedIn = true;
    m_currentUserId = query.value("id").toInt();
    m_currentUserName = username;
    m_currentUserEmail = query.value("email").toString();

    qInfo() << "[DB] 用户 " << username << " 登录成功！";
    emit userLoginStateChanged(true);
    emit userLoginSuccess(username);
    emit operateResult(true, "登录成功！");

    return true;
}

// ========== 新增：用户登出 ==========
void DBManager::userLogout()
{
    QMutexLocker locker(&m_mutex); // 线程安全

    // 重置用户登录状态
    m_isUserLoggedIn = false;
    m_currentUserId = -1;
    m_currentUserName.clear();
    m_currentUserEmail.clear();

    qInfo() << "[DB] 用户已登出";
    emit userLoginStateChanged(false);
    emit userLogoutSuccess();
    emit operateResult(true, "登出成功！");
}

// ========== 新增：用户状态查询及getter方法 ==========
bool DBManager::isUserLoggedIn() const
{
    return m_isUserLoggedIn;
}

int DBManager::getCurrentUserId() const
{
    return m_currentUserId;
}

QString DBManager::getCurrentUserName() const
{
    return m_currentUserName;
}

QString DBManager::getCurrentUserEmail() const
{
    return m_currentUserEmail;
}

// ========== 原有航班相关方法（保持不变） ==========
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

QVariantMap DBManager::queryFlightByNum(const QString& flightId)
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

    query.bindValue(":flightId", flightId);
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

bool DBManager::addFlight(
    const QString& flightId,
    const QString& departure,
    const QString& destination,
    const QString& departTime,
    const QString& arriveTime,
    double price,
    int totalSeats,
    int remainSeats
    ) {
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "添加失败：数据库未连接！");
        return false;
    }
    if (flightId.isEmpty() || departure.isEmpty() || destination.isEmpty()) {
        emit operateResult(false, "添加失败：航班号、出发地、目的地不能为空！");
        return false;
    }
    if (!isValidDateTimeFormat(departTime) || !isValidDateTimeFormat(arriveTime)) {
        emit operateResult(false, "添加失败：时间格式错误！请输入 YYYY-MM-DD HH:MM:SS");
        return false;
    }
    if (QDateTime::fromString(departTime, "yyyy-MM-dd HH:mm:ss") >= QDateTime::fromString(arriveTime, "yyyy-MM-dd HH:mm:ss")) {
        emit operateResult(false, "添加失败：起飞时间不能晚于降落时间！");
        return false;
    }
    if (price <= 0) {
        emit operateResult(false, "添加失败：票价必须大于 0！");
        return false;
    }
    if (totalSeats <= 0 || remainSeats < 0 || remainSeats > totalSeats) {
        emit operateResult(false, "添加失败：座位数无效（剩余座位不能大于总座位，且不能为负）！");
        return false;
    }

    // 检查航班号是否已存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT Flight_id FROM flight WHERE Flight_id = :flightId");
    checkQuery.bindValue(":flightId", flightId);
    if (checkQuery.exec() && checkQuery.next()) {
        emit operateResult(false, "添加失败：航班号 " + flightId + " 已存在！");
        return false;
    }

    // 插入数据
    QSqlQuery query(m_db);
    QString sql = R"(
        INSERT INTO flight (
            Flight_id, Departure, Destination, depart_time, arrive_time,
            price, total_seats, remain_seats
        ) VALUES (
            :flightId, :departure, :destination, :departTime, :arriveTime,
            :price, :totalSeats, :remainSeats
        )
    )";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 插入预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return false;
    }

    // 绑定所有参数
    query.bindValue(":flightId", flightId);
    query.bindValue(":departure", departure);
    query.bindValue(":destination", destination);
    query.bindValue(":departTime", departTime);  // 直接传字符串，SQL 自动解析为 datetime
    query.bindValue(":arriveTime", arriveTime);
    query.bindValue(":price", price);            // double 适配 decimal(10,2)
    query.bindValue(":totalSeats", totalSeats);
    query.bindValue(":remainSeats", remainSeats);

    bool success = query.exec();
    if (success) {
        emit operateResult(true, "航班 " + flightId + " 添加成功！");
    } else {
        QString errMsg = "[DB] 插入失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
    }
    return success;
}

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

void DBManager::printFlight(const QVariantMap &flight) {
    if (flight.isEmpty()) {
        qInfo() << "查询结果：无此航班\n";
        return;
    }
    qInfo() << "\n===== 单个航班详情 =====";
    qInfo() << "航班号：" << flight["flightId"].toString();
    qInfo() << "出发地：" << flight["departure"].toString();
    qInfo() << "目的地：" << flight["destination"].toString();
    qInfo() << "起飞时间：" << flight["departTime"].toString();
    qInfo() << "降落时间：" << flight["arriveTime"].toString();
    qInfo() << "票价：" << flight["price"].toDouble() << "元";
    qInfo() << "总座位：" << flight["totalSeats"].toInt();
    qInfo() << "剩余座位：" << flight["remainSeats"].toInt();
    qInfo() << "======================\n";
}

void DBManager::printFlightList(const QVariantList &flightList) {
    qInfo() << "\n===== 航班列表（共" << flightList.size() << "条）=====";
    for (const auto &flightVar : flightList) {
        QVariantMap flight = flightVar.toMap();
        qInfo() << QString("航班号：%1 | 出发地：%2 | 目的地：%3 | 起飞时间：%4 | 票价：%5 元 | 剩余座位：%6")
                       .arg(flight["flightId"].toString())
                       .arg(flight["departure"].toString())
                       .arg(flight["destination"].toString())
                       .arg(flight["departTime"].toString())
                       .arg(flight["price"].toDouble(), 0, 'f', 2)
                       .arg(flight["remainSeats"].toInt());
    }
    qInfo() << "========================================\n";
}

//管理员登录
bool DBManager::verifyAdminLogin(const QString& adminName, const QString& password)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        qWarning() << "Database is not connected";
        emit adminLoginFailed("数据库未连接");
        return false;
    }

    if (adminName.isEmpty() || password.isEmpty()) {
        emit adminLoginFailed("用户名或密码不能为空");
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT Aid, Admin_name FROM admin_info WHERE Admin_name = ? AND Password = ?");
    query.addBindValue(adminName);
    query.addBindValue(password);

    if (!query.exec()) {
        qWarning() << "Login query failed:" << query.lastError();
        emit adminLoginFailed("查询失败: " + query.lastError().text());
        return false;
    }

    if (query.next()) {
        m_isAdminLoggedIn = true;
        m_currentAdminId = query.value("Aid").toInt();
        m_currentAdminName = query.value("Admin_name").toString();

        emit adminLoginStateChanged(true);
        emit adminLoginSuccess(m_currentAdminName);
        qDebug() << "Admin login successful:" << m_currentAdminName;
        return true;
    } else {
        m_isAdminLoggedIn = false;
        m_currentAdminId = -1;
        m_currentAdminName.clear();

        emit adminLoginFailed("用户名或密码错误");
        qWarning() << "Admin login failed: invalid credentials";
        return false;
    }
}

bool DBManager::isAdminLoggedIn() const
{
    return m_isAdminLoggedIn;
}

void DBManager::adminLogout()
{
    m_isAdminLoggedIn = false;
    m_currentAdminId = -1;
    m_currentAdminName.clear();

    emit adminLoginStateChanged(false);
    emit adminLogoutSuccess();
    qDebug() << "Admin logged out";
}

QString DBManager::getCurrentAdminName() const
{
    return m_currentAdminName;
}

int DBManager::getCurrentAdminId() const
{
    return m_currentAdminId;
}
