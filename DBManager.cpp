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

// 邮箱格式验证
bool DBManager::isValidEmailFormat(const QString& email)
{
    // 标准邮箱正则表达式
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    QRegularExpressionMatch match = emailRegex.match(email);
    return match.hasMatch();
}

// 密码强度验证
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

// 用户名唯一性检查
bool DBManager::isUsernameExists(const QString& User_name)
{
    if (!m_db.isOpen()) {
        qCritical() << "[DB] 检查用户名失败：数据库未连接！";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT User_name FROM user_info WHERE User_name = :User_name");
    query.bindValue(":User_name", User_name);

    if (!query.exec()) {
        qCritical() << "[DB] 检查用户名失败：" << query.lastError().text();
        return false;
    }

    return query.next(); // 有结果则用户名已存在
}

// 邮箱唯一性检查
bool DBManager::isEmailExists(const QString& Email)
{
    if (!m_db.isOpen()) {
        qCritical() << "[DB] 检查邮箱失败：数据库未连接！";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT Email FROM user_info WHERE Email = :Email");
    query.bindValue(":Email", Email);

    if (!query.exec()) {
        qCritical() << "[DB] 检查邮箱失败：" << query.lastError().text();
        return false;
    }

    return query.next(); // 有结果则邮箱已存在
}

// 密码加密（SHA256）
QString DBManager::encryptPassword(const QString& password)
{
    QByteArray byteArray = password.toUtf8();
    // SHA256 加密
    byteArray = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    // 转换为十六进制字符串
    return byteArray.toHex();
}

// 用户注册
int DBManager::userRegister(const QString& Email, const QString& User_name, const QString& Password)
{
    QMutexLocker locker(&m_mutex); // 线程安全

    // 1. 检查数据库连接
    if (!m_db.isOpen()) {
        QString errMsg = "注册失败：数据库未连接！";
        emit userRegisterFailed(errMsg);
        return 0;
    }

    // 2. 空值检查
    if (Email.isEmpty() || User_name.isEmpty() || Password.isEmpty()) {
        QString errMsg = "注册失败：邮箱、用户名、密码不能为空！";
        emit userRegisterFailed(errMsg);
        return 5;
    }

    // 3. 邮箱格式验证
    if (!isValidEmailFormat(Email)) {
        QString errMsg = "注册失败：邮箱格式错误！";
        emit userRegisterFailed(errMsg);
        return 2;
    }

    // 4. 密码强度验证
    if (!isValidPasswordStrength(Password)) {
        QString errMsg = "注册失败：密码至少8位，且包含字母和数字！";
        emit userRegisterFailed(errMsg);
        return 3;
    }

    // 5. 用户名唯一性检查
    if (isUsernameExists(User_name)) {
        QString errMsg = "注册失败：用户名 " + User_name + " 已存在！";
        emit userRegisterFailed(errMsg);
        return 1;
    }

    // 6. 邮箱唯一性检查
    if (isEmailExists(Email)) {
        QString errMsg = "注册失败：邮箱 " + Email + " 已被注册！";
        emit userRegisterFailed(errMsg);
        return 4;
    }

    // 7. 加密密码
    QString encryptedPwd = encryptPassword(Password);

    // 8. 插入用户数据
    QSqlQuery query(m_db);
    QString insertSql = R"(
        INSERT INTO user_info (Email, User_name, Password)
        VALUES (:Email, :User_name, :Password)
    )";

    if (!query.prepare(insertSql)) {
        QString errMsg = "[DB] 注册预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit userRegisterFailed("注册失败：数据库操作错误！");
        return 0;
    }

    query.bindValue(":Email", Email);
    query.bindValue(":User_name", User_name);
    query.bindValue(":Password", encryptedPwd);

    bool success = query.exec();
    if (success) {
        qInfo() << "[DB] 用户 " << User_name << " 注册成功！";
        emit userRegisterSuccess(User_name);
        emit operateResult(true, "注册成功！");
        return 5;
    } else {
        QString errMsg = "[DB] 注册插入失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit userRegisterFailed("注册失败：" + query.lastError().text());
        return 6;
    }
}

// 用户登录
int DBManager::userLogin(const QString& User_name, const QString& Password)
{
    QMutexLocker locker(&m_mutex); // 线程安全

    // 1. 检查数据库连接
    if (!m_db.isOpen()) {
        QString errMsg = "登录失败：数据库未连接！";
        emit userLoginFailed(errMsg);
        return 0;
    }

    // 2. 空值检查
    if (User_name.isEmpty() || Password.isEmpty()) {
        QString errMsg = "登录失败：用户名或密码不能为空！";
        emit userLoginFailed(errMsg);
        return 3;
    }

    // 3. 查询用户信息
    QSqlQuery query(m_db);
    query.prepare("SELECT Uid, Email, Password FROM user_info WHERE User_name = :User_name");
    query.bindValue(":User_name", User_name);

    if (!query.exec()) {
        QString errMsg = "[DB] 登录查询失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit userLoginFailed("登录失败：数据库操作错误！");
        return 0;
    }

    // 4. 检查用户是否存在
    if (!query.next()) {
        emit userLoginFailed("登录失败：用户名不存在！");
        return 1;
    }

    // 5. 验证密码
    QString storedPwd = query.value("Password").toString();
    QString inputPwd = encryptPassword(Password);
    if (storedPwd != inputPwd) {
        emit userLoginFailed("登录失败：密码错误！");
        return 2;
    }

    // 6. 更新用户登录状态
    m_isUserLoggedIn = true;
    m_currentUserId = query.value("Uid").toInt();
    m_currentUserName = User_name;
    m_currentUserEmail = query.value("Email").toString();

    qInfo() << "[DB] 用户 " << User_name << " 登录成功！";
    emit userLoginStateChanged(true);
    emit userLoginSuccess(User_name);
    emit operateResult(true, "登录成功！");

    return 4;
}

// 用户登出
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

// 忘记密码
int DBManager::forgetPassword(const QString& Email, const QString& verifyCode, const QString& newPassword)
{
    QMutexLocker locker(&m_mutex); // 线程安全

    // 1. 检查数据库连接状态
    if (!isConnected()) {
        emit passwordResetFailed("数据库未连接，请先连接数据库");
        return 0;
    }

    // 2. 验证邮箱格式是否合法
    if (!isValidEmailFormat(Email)) {
        emit passwordResetFailed("邮箱格式不合法，请输入正确的邮箱");
        return 1;
    }

    // 3. 检查邮箱是否已注册（不存在则无法重置）
    if (!isEmailExists(Email)) {
        emit passwordResetFailed("该邮箱未注册，无法重置密码");
        return 2;
    }

    // 4. 验证验证码是否为固定的0000
    if (verifyCode != "0000") {
        emit passwordResetFailed("验证码错误，正确验证码为0000");
        return 3;
    }

    // 5. 验证新密码强度（沿用原有密码强度规则）
    if (!isValidPasswordStrength(newPassword)) {
        emit passwordResetFailed("密码强度不足（至少8位，包含字母和数字）");
        return 4;
    }

    // 6. 从数据库中获取用户名（用于后续成功信号）
    QSqlQuery query(m_db);
    query.prepare("SELECT User_name FROM user_info WHERE Email = :Email");
    query.bindValue(":Email", Email);
    if (!query.exec()) {
        qDebug() << "查询用户信息失败：" << query.lastError().text();
        emit passwordResetFailed("查询用户信息失败，请稍后重试");
        return 5;
    }
    if (!query.next()) {
        emit passwordResetFailed("该邮箱未注册，无法重置密码");
        return 2;
    }
    QString username = query.value("User_name").toString();

    // 7. 加密新密码（沿用原有SHA256加密逻辑）
    QString encryptedPwd = encryptPassword(newPassword);

    // 8. 更新数据库中的密码
    query.prepare("UPDATE user_info SET Password = :newPassword WHERE Email = :Email");
    query.bindValue(":newPassword", encryptedPwd);
    query.bindValue(":Email", Email);
    if (!query.exec()) {
        qDebug() << "更新密码失败：" << query.lastError().text();
        emit passwordResetFailed("密码重置失败，请稍后重试");
        return 5;
    }

    // 9. 密码重置成功，发送信号
    emit passwordResetSuccess(username);
    return 7;
}

// 用户状态查询
bool DBManager::isUserLoggedIn() const
{
    return m_isUserLoggedIn;
}

// 获取Uid
int DBManager::getCurrentUserId() const
{
    return m_currentUserId;
}

// 获取用户名
QString DBManager::getCurrentUserName() const
{
    return m_currentUserName;
}

// 获取邮箱
QString DBManager::getCurrentUserEmail() const
{
    return m_currentUserEmail;
}

// 查询所有航班
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
               status, price, total_seats, remain_seats
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
            flight["Flight_id"] = query.value("Flight_id").toString();
            flight["Departure"] = query.value("Departure").toString();
            flight["Destination"] = query.value("Destination").toString();
            flight["depart_time"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            flight["arrive_time"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            flight["status"] = query.value("status").toString();
            flight["price"] = query.value("price").toDouble();
            flight["total_seats"] = query.value("total_seats").toInt();
            flight["remain_seats"] = query.value("remain_seats").toInt();
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

QVariantList DBManager::queryFlightsByCondition(const QString& departure, const QString& destination, const QString& departDate)
{
    QMutexLocker locker(&m_mutex);
    QVariantList result;

    if (!m_db.isOpen()) {
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    QString sql = "SELECT * FROM flight";
    QList<QString> conditions;
    QVariantMap params; // 存储参数绑定（键：参数名，值：参数值）

    // 处理目的地条件（非空则添加）
    if (!destination.isEmpty()) {
        conditions.append("Destination = :destination");
        params[":destination"] = destination;
    }

    // 处理出发日期条件（非空则添加，匹配日期部分，忽略时间）
    if (!departDate.isEmpty()) {
        // MySQL：DATE(depart_time) 提取日期部分
        conditions.append("DATE(depart_time) = :departDate");
        params[":departDate"] = departDate;
    }

    // 拼接WHERE条件（多个条件用AND连接）
    if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
    }

    sql += " ORDER BY depart_time ASC";

    // 执行查询
    QSqlQuery query(m_db);
    query.prepare(sql);

    // 绑定参数（遍历params，给SQL语句的参数赋值）
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        qDebug() << "查询航班失败：" << query.lastError().text();
        qDebug() << "执行的SQL：" << sql;
        return result;
    }

    while (query.next()) {
        QVariantMap flightMap;
        // 封装航班字段
        flightMap["Flight_id"] = query.value("Flight_id").toString();
        flightMap["Departure"] = query.value("Departure").toString();
        flightMap["Destination"] = query.value("Destination").toString();
        flightMap["depart_time"] = query.value("depart_time").toString();
        flightMap["arrive_time"] = query.value("arrive_time").toString();
        flightMap["price"] = query.value("price").toDouble();
        flightMap["total_seats"] = query.value("total_seats").toInt();
        flightMap["remain_seats"] = query.value("remain_seats").toInt();
        flightMap["status"] = query.value("status").toString();

        result.append(flightMap);
    }

    return result;
}

// 按航班号查询航班
QVariantList DBManager::queryFlightByNum(const QString& flightId)
{
    QMutexLocker locker(&m_mutex);
    QVariantList result;

    if (!m_db.isOpen()) {
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    // 用 bindValue 绑定参数，避免 SQL 注入
    QSqlQuery query(m_db);
    QString sql = R"(
        SELECT Flight_id, Departure, Destination, depart_time, arrive_time,
               status, price, total_seats, remain_seats
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
        QVariantMap flightMap;
        flightMap["Flight_id"] = query.value("Flight_id").toString();
        flightMap["Departure"] = query.value("Departure").toString();
        flightMap["Destination"] = query.value("Destination").toString();
        flightMap["depart_time"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        flightMap["arrive_time"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        flightMap["status"] = query.value("status").toString();
        flightMap["price"] = query.value("price").toDouble();
        flightMap["total_seats"] = query.value("total_seats").toInt();
        flightMap["remain_seats"] = query.value("remain_seats").toInt();
        emit operateResult(true, "查询成功！");

        result.append(flightMap);
    } else {
        emit operateResult(false, "查询失败：未找到该航班或查询出错！");
    }
    return result;
}

// 添加航班
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

// 更新剩余座位数
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

// 更新航班状态
bool DBManager::updateFlightStatus(const QString& Flight_id, const QString& newststus)
{
    QMutexLocker locker(&m_mutex);

    if(!m_db.isOpen()){
        emit operateResult(false, "更新失败：数据库未连接！");
        return false;
    }

    QSqlQuery query(m_db);
    QString sql = "UPDATE flight SET status = :newstatus WHERE Flight_id = :Flight_id";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 更新预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit operateResult(false, errMsg);
        return false;
    }

    query.bindValue(":newststus", newststus);
    query.bindValue(":Flight_id", Flight_id);
    bool success = query.exec();

    if (success && query.numRowsAffected() > 0) {
        emit operateResult(true, "航班 " + Flight_id + " 状态更新为 " + newststus + "！ ");
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

// 收藏航班
bool DBManager::collectFlight(int userId, const QString& flightId, const QString& createTime)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "删除失败：数据库未连接！");
        return false;
    }

    if (isFlightCollected(userId, flightId)) {
        emit operateResult(false, "已收藏该航班");
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO user_collect_flights (user_id, flight_id, create_time)
        VALUES (:user_id, :flight_id, :create_time)
    )");
    query.bindValue(":user_id", userId);
    query.bindValue(":flight_id", flightId);
    query.bindValue(":create_time", createTime);

    if (!query.exec()) {
        qDebug() << "收藏航班失败：" << query.lastError().text();
        emit operateResult(false, "收藏航班失败：" + query.lastError().text());
        return false;
    }

    emit operateResult(true, "收藏航班成功");
    return true;
}

// 取消收藏航班
bool DBManager::cancelCollectFlight(int userId, const QString& flightId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "取消收藏失败：数据库未连接！");
        return false;
    }

    if (!isFlightCollected(userId, flightId)) {
        emit operateResult(false, "未收藏该航班，无需取消");
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        DELETE FROM user_collect_flights
        WHERE user_id = :user_id AND flight_id = :flight_id
    )");
    query.bindValue(":user_id", userId);
    query.bindValue(":flight_id", flightId);

    if (!query.exec()) {
        qDebug() << "取消收藏失败：" << query.lastError().text();
        emit operateResult(false, "取消收藏失败：" + query.lastError().text());
        return false;
    }

    emit operateResult(true, "取消收藏成功");
    return true;
}

// 查询用户收藏的所有航班
QVariantList DBManager::queryCollectedFlights(int userId)
{
    QMutexLocker locker(&m_mutex);
    QVariantList flightList;

    if (!m_db.isOpen()) {
        emit operateResult(false, "查询失败：数据库未连接！");
        return flightList;
    }
    if (userId <= 0) {
        emit operateResult(false, "查询失败：用户Id非法！");
        return flightList;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT f.* FROM flight f
        INNER JOIN user_collect_flights ucf ON f.Flight_id = ucf.flight_id
        WHERE ucf.user_id = :user_id
        ORDER BY ucf.create_time DESC
    )");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "查询收藏航班失败：" << query.lastError().text();
        return flightList;
    }

    while (query.next()) {
        QVariantMap flightMap;
        flightMap["Flight_id"] = query.value("Flight_id").toString();
        flightMap["Departure"] = query.value("Departure").toString();
        flightMap["Destination"] = query.value("Destination").toString();
        flightMap["depart_time"] = query.value("depart_time").toString();
        flightMap["arrive_time"] = query.value("arrive_time").toString();
        flightMap["price"] = query.value("price").toDouble();
        flightMap["total_seats"] = query.value("total_seats").toInt();
        flightMap["remain_seats"] = query.value("remain_seats").toInt();
        flightMap["status"] = query.value("status").toString();

        flightList.append(flightMap);
    }

    return flightList;
}

// 判断用户是否已收藏某航班
bool DBManager::isFlightCollected(int userId, const QString& flightId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "判断失败：数据库未连接！");
        return false;
    }

    if (userId <= 0 || flightId.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT 1 FROM user_collect_flights
        WHERE user_id = :user_id AND flight_id = :flight_id
        LIMIT 1
    )");
    query.bindValue(":user_id", userId);
    query.bindValue(":flight_id", flightId);

    if (query.exec() && query.next()) {
        return true; // 已收藏
    }
    return false; // 未收藏
}

// 打印航班（id）
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

// 打印航班列表
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

// 管理员登录验证
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

// 是否管理员登录
bool DBManager::isAdminLoggedIn() const
{
    return m_isAdminLoggedIn;
}

// 管理员登出
void DBManager::adminLogout()
{
    m_isAdminLoggedIn = false;
    m_currentAdminId = -1;
    m_currentAdminName.clear();

    emit adminLoginStateChanged(false);
    emit adminLogoutSuccess();
    qDebug() << "Admin logged out";
}

// 获取当前管理员名
QString DBManager::getCurrentAdminName() const
{
    return m_currentAdminName;
}

// 获取当前管理员Id
int DBManager::getCurrentAdminId() const
{
    return m_currentAdminId;
}
// 查看我的所有订单
QVariantList DBManager::queryMyOrders(int userId)
{
    QMutexLocker locker(&m_mutex);
    QVariantList result;

    if (!m_db.isOpen()) {
        emit queryMyOrdersFailed("查询失败：数据库未连接！");
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    if (userId <= 0) {
        emit queryMyOrdersFailed("查询失败：用户ID无效！");
        emit operateResult(false, "查询失败：用户ID无效！");
        return result;
    }

    QSqlQuery query(m_db);
    QString sql = R"(
        SELECT
            o.order_id,
            o.flight_id,
            o.passenger_name,
            o.passenger_idcard,
            o.order_time,
            o.status,
            f.Departure,
            f.Destination,
            f.depart_time,
            f.arrive_time,
            f.price,
            f.remain_seats
        FROM `order` o
        INNER JOIN flight f ON o.flight_id = f.Flight_id
        WHERE o.user_id = :userId
        ORDER BY o.order_time DESC
    )";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 查询订单预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit queryMyOrdersFailed("查询失败：数据库操作错误！");
        emit operateResult(false, errMsg);
        return result;
    }

    query.bindValue(":userId", userId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap order;
            order["order_id"] = query.value("order_id").toInt();
            order["flight_id"] = query.value("flight_id").toString();
            order["passenger_name"] = query.value("passenger_name").toString();
            order["passenger_idcard"] = query.value("passenger_idcard").toString();
            order["order_time"] = query.value("order_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            order["status"] = query.value("status").toString();
            order["departure"] = query.value("Departure").toString();
            order["destination"] = query.value("Destination").toString();
            order["depart_time"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            order["arrive_time"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            order["price"] = query.value("price").toDouble();
            order["remain_seats"] = query.value("remain_seats").toInt();

            result.append(order);
        }
        emit queryMyOrdersSuccess(result);
        emit operateResult(true, QString("查询成功，共 %1 个订单").arg(result.size()));
    } else {
        QString errMsg = "[DB] 查询订单失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit queryMyOrdersFailed("查询失败：" + query.lastError().text());
        emit operateResult(false, errMsg);
    }

    return result;
}

// 按条件查询我的订单
QVariantList DBManager::queryMyOrdersByCondition(int userId,
                                                 const QString& flightId,
                                                 const QString& passengerName,
                                                 const QString& status,
                                                 const QString& startDate,
                                                 const QString& endDate)
{
    QMutexLocker locker(&m_mutex);
    QVariantList result;

    if (!m_db.isOpen()) {
        emit queryMyOrdersFailed("查询失败：数据库未连接！");
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    if (userId <= 0) {
        emit queryMyOrdersFailed("查询失败：用户ID无效！");
        emit operateResult(false, "查询失败：用户ID无效！");
        return result;
    }

    QSqlQuery query(m_db);
    QString sql = R"(
        SELECT
            o.order_id,
            o.flight_id,
            o.passenger_name,
            o.passenger_idcard,
            o.order_time,
            o.status,
            f.Departure,
            f.Destination,
            f.depart_time,
            f.arrive_time,
            f.price,
            f.remain_seats
        FROM `order` o
        INNER JOIN flight f ON o.flight_id = f.Flight_id
        WHERE o.user_id = :userId
    )";

    QList<QString> conditions;
    QVariantMap params;
    params[":userId"] = userId;

    // 添加航班号条件
    if (!flightId.isEmpty()) {
        conditions.append("o.flight_id LIKE :flightId");
        params[":flightId"] = "%" + flightId + "%";
    }

    // 添加乘客姓名条件
    if (!passengerName.isEmpty()) {
        conditions.append("o.passenger_name LIKE :passengerName");
        params[":passengerName"] = "%" + passengerName + "%";
    }

    // 添加状态条件
    if (!status.isEmpty() && status != "全部") {
        conditions.append("o.status = :status");
        params[":status"] = status;
    }

    // 添加时间范围条件
    if (!startDate.isEmpty()) {
        conditions.append("DATE(o.order_time) >= :startDate");
        params[":startDate"] = startDate;
    }
    if (!endDate.isEmpty()) {
        conditions.append("DATE(o.order_time) <= :endDate");
        params[":endDate"] = endDate;
    }

    // 拼接条件
    if (!conditions.isEmpty()) {
        sql += " AND " + conditions.join(" AND ");
    }

    sql += " ORDER BY o.order_time DESC";

    if (!query.prepare(sql)) {
        QString errMsg = "[DB] 条件查询订单预处理失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit queryMyOrdersFailed("查询失败：数据库操作错误！");
        emit operateResult(false, errMsg);
        return result;
    }

    // 绑定参数
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (query.exec()) {
        while (query.next()) {
            QVariantMap order;
            order["order_id"] = query.value("order_id").toInt();
            order["flight_id"] = query.value("flight_id").toString();
            order["passenger_name"] = query.value("passenger_name").toString();
            order["passenger_idcard"] = query.value("passenger_idcard").toString();
            order["order_time"] = query.value("order_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            order["status"] = query.value("status").toString();
            order["departure"] = query.value("Departure").toString();
            order["destination"] = query.value("Destination").toString();
            order["depart_time"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            order["arrive_time"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            order["price"] = query.value("price").toDouble();
            order["remain_seats"] = query.value("remain_seats").toInt();

            result.append(order);
        }
        emit queryMyOrdersSuccess(result);
        emit operateResult(true, QString("条件查询成功，共 %1 个订单").arg(result.size()));
    } else {
        QString errMsg = "[DB] 条件查询订单失败：" + query.lastError().text();
        qCritical() << errMsg;
        emit queryMyOrdersFailed("查询失败：" + query.lastError().text());
        emit operateResult(false, errMsg);
    }

    return result;
}

// 获取订单详情
QVariantMap DBManager::queryOrderDetail(int userId, int orderId)
{
    QVariantMap result;
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit orderDetailQueryFailed("查询失败：数据库未连接！");
        emit operateResult(false, "查询失败：数据库未连接！");
        return result;
    }

    if (userId <= 0 || orderId <= 0) {
        emit orderDetailQueryFailed("查询失败：参数无效！");
        emit operateResult(false, "查询失败：参数无效！");
        return result;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT
            o.order_id,
            o.flight_id,
            o.passenger_name,
            o.passenger_idcard,
            o.order_time,
            o.status,
            f.Departure,
            f.Destination,
            f.depart_time,
            f.arrive_time,
            f.price,
            f.total_seats,
            f.remain_seats,
            f.status as flight_status,
            u.User_name,
            u.Email
        FROM `order` o
        INNER JOIN flight f ON o.flight_id = f.Flight_id
        INNER JOIN user_info u ON o.user_id = u.Uid
        WHERE o.order_id = :orderId AND o.user_id = :userId
    )");

    query.bindValue(":orderId", orderId);
    query.bindValue(":userId", userId);

    if (query.exec() && query.next()) {
        result["order_id"] = query.value("order_id").toInt();
        result["flight_id"] = query.value("flight_id").toString();
        result["passenger_name"] = query.value("passenger_name").toString();
        result["passenger_idcard"] = query.value("passenger_idcard").toString();
        result["order_time"] = query.value("order_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        result["status"] = query.value("status").toString();
        result["departure"] = query.value("Departure").toString();
        result["destination"] = query.value("Destination").toString();
        result["depart_time"] = query.value("depart_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        result["arrive_time"] = query.value("arrive_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        result["price"] = query.value("price").toDouble();
        result["total_seats"] = query.value("total_seats").toInt();
        result["remain_seats"] = query.value("remain_seats").toInt();
        result["flight_status"] = query.value("flight_status").toString();
        result["user_name"] = query.value("User_name").toString();
        result["email"] = query.value("Email").toString();

        emit orderDetailQuerySuccess(result);
        emit operateResult(true, "查询订单详情成功！");
    } else {
        emit orderDetailQueryFailed("查询失败：未找到该订单！");
        emit operateResult(false, "查询失败：未找到该订单！");
    }

    return result;
}


// 检查订单是否存在
bool DBManager::isOrderExists(int userId, int orderId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        return false;
    }

    if (userId <= 0 || orderId <= 0) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM `order` WHERE order_id = :orderId AND user_id = :userId");
    query.bindValue(":orderId", orderId);
    query.bindValue(":userId", userId);

    return query.exec() && query.next();
}

// 获取订单状态列表
QStringList DBManager::getOrderStatusList()
{
    QStringList statusList;
    statusList << "全部" << "已支付" << "已取消" << "已完成" << "已退款";
    return statusList;
}
