#include "DBUser.h"

// 初始化静态成员
DBUser* DBUser::m_instance = nullptr;
QMutex DBUser::m_mutex;

DBUser::DBUser(QObject *parent)
    : QObject(parent)
    , m_isUserLoggedIn(false)
    , m_currentUserId(-1)
{
    // 构造函数
}

DBUser::~DBUser()
{
    // 析构函数
}

DBUser* DBUser::getInstance(QObject *parent)
{
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);
        if (m_instance == nullptr) {
            m_instance = new DBUser(parent);
        }
    }
    return m_instance;
}

void DBUser::setDatabase(const QSqlDatabase &database)
{
    QMutexLocker locker(&m_mutex);
    m_db = database;

    // 检查并创建用户表
    if (m_db.isOpen()) {
        createUserTable();
    }
}

bool DBUser::isDatabaseConnected() const
{
    return m_db.isOpen();
}

// 用户名验证（使用QRegularExpression）
bool DBUser::isValidUsername(const QString& username)
{
    // 用户名规则：字母开头，3-20位，允许字母、数字、下划线
    QRegularExpression usernameRegex("^[a-zA-Z][a-zA-Z0-9_]{2,19}$");
    return usernameRegex.match(username).hasMatch();
}

// 密码强度验证
bool DBUser::isPasswordStrong(const QString& password)
{
    // 密码规则：至少8位，包含字母和数字
    if (password.length() < 8) {
        return false;
    }

    bool hasLetter = false;
    bool hasDigit = false;

    for (const QChar& ch : password) {
        if (ch.isLetter()) hasLetter = true;
        if (ch.isDigit()) hasDigit = true;
        if (hasLetter && hasDigit) break;
    }

    return hasLetter && hasDigit;
}

// 邮箱验证（使用QRegularExpression）
bool DBUser::isValidEmail(const QString& email)
{
    if (email.isEmpty()) {
        return false; // 邮箱是必填项
    }

    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return emailRegex.match(email).hasMatch();
}

// 密码加密
QString DBUser::encryptPassword(const QString& password)
{
    // 使用SHA-256哈希加密密码
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    return QString(hash.result().toHex());
}

// 创建用户表
bool DBUser::createUserTable()
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit operateResult(false, "数据库未连接");
        return false;
    }

    // 创建用户表（只包含必需字段）
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS user_info (
            Uid INT AUTO_INCREMENT PRIMARY KEY,
            User_name VARCHAR(50) UNIQUE NOT NULL,
            Password VARCHAR(100) NOT NULL,
            Email VARCHAR(100) NOT NULL,
            Create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            Last_login TIMESTAMP NULL,
            INDEX idx_username (User_name),
            INDEX idx_email (Email)
        )
    )";

    QSqlQuery query(m_db);
    if (!query.exec(createTableSQL)) {
        QString errorMsg = "创建用户表失败: " + query.lastError().text();
        qCritical() << "[DBUser]" << errorMsg;
        emit operateResult(false, errorMsg);
        return false;
    }

    qDebug() << "[DBUser] 用户表创建成功或已存在";
    emit operateResult(true, "用户表初始化成功");
    return true;
}

// 用户注册（只需用户名、密码、邮箱）
bool DBUser::registerUser(const QString& username, const QString& password,
                          const QString& confirmPassword, const QString& email)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit registerFailed("数据库未连接");
        return false;
    }

    // 输入验证
    if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty() || email.isEmpty()) {
        emit registerFailed("用户名、密码、确认密码和邮箱不能为空");
        return false;
    }

    if (!isValidUsername(username)) {
        emit registerFailed("用户名格式无效（字母开头，3-20位字母数字下划线）");
        return false;
    }

    if (!isPasswordStrong(password)) {
        emit registerFailed("密码强度不足（至少8位，包含字母和数字）");
        return false;
    }

    if (password != confirmPassword) {
        emit registerFailed("两次输入的密码不一致");
        return false;
    }

    if (!isValidEmail(email)) {
        emit registerFailed("邮箱格式无效");
        return false;
    }

    // 检查用户名是否已存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT User_name FROM user_info WHERE User_name = ?");
    checkQuery.addBindValue(username);

    if (!checkQuery.exec()) {
        emit registerFailed("查询失败: " + checkQuery.lastError().text());
        return false;
    }

    if (checkQuery.next()) {
        emit registerFailed("用户名已存在");
        return false;
    }

    // 检查邮箱是否已存在
    checkQuery.prepare("SELECT User_name FROM user_info WHERE Email = ?");
    checkQuery.addBindValue(email);

    if (checkQuery.exec() && checkQuery.next()) {
        emit registerFailed("邮箱已被注册");
        return false;
    }

    // 插入新用户
    QSqlQuery query(m_db);
    QString sql = "INSERT INTO user_info (User_name, Password, Email) VALUES (?, ?, ?)";

    if (!query.prepare(sql)) {
        emit registerFailed("插入预处理失败: " + query.lastError().text());
        return false;
    }

    query.addBindValue(username);
    query.addBindValue(encryptPassword(password)); // 存储加密后的密码
    query.addBindValue(email);

    if (!query.exec()) {
        emit registerFailed("注册失败: " + query.lastError().text());
        return false;
    }

    emit registerSuccess(username);
    emit operateResult(true, "用户注册成功");
    qDebug() << "[DBUser] 用户注册成功:" << username;
    return true;
}

// 用户登录验证（只需用户名和密码）
bool DBUser::verifyUserLogin(const QString& username, const QString& password)
{
    QMutexLocker locker(&m_mutex);

    if (!m_db.isOpen()) {
        emit userLoginFailed("数据库未连接");
        return false;
    }

    // 输入验证
    if (username.isEmpty()) {
        emit userLoginFailed("用户名不能为空");
        return false;
    }

    if (password.isEmpty()) {
        emit userLoginFailed("密码不能为空");
        return false;
    }

    // 查询用户信息
    QSqlQuery query(m_db);
    query.prepare("SELECT Uid, User_name, Password, Email FROM user_info WHERE User_name = ?");
    query.addBindValue(username);

    if (!query.exec()) {
        emit userLoginFailed("查询失败: " + query.lastError().text());
        return false;
    }

    if (query.next()) {
        QString storedPassword = query.value("Password").toString();
        QString inputPassword = encryptPassword(password);

        // 验证密码
        if (storedPassword == inputPassword) {
            // 登录成功
            m_isUserLoggedIn = true;
            m_currentUserId = query.value("Uid").toInt();
            m_currentUsername = query.value("User_name").toString();

            // 更新最后登录时间
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE user_info SET Last_login = CURRENT_TIMESTAMP WHERE Uid = ?");
            updateQuery.addBindValue(m_currentUserId);
            updateQuery.exec();

            emit userLoginStateChanged(true);
            emit userLoginSuccess(m_currentUsername);
            emit currentUserChanged();
            emit operateResult(true, "登录成功");

            qDebug() << "[DBUser] 用户登录成功:" << m_currentUsername;
            return true;
        } else {
            // 密码错误
            m_isUserLoggedIn = false;
            m_currentUserId = -1;
            m_currentUsername.clear();

            emit userLoginFailed("密码错误");
            qWarning() << "[DBUser] 用户登录失败: 无效密码";
            return false;
        }
    } else {
        // 用户不存在
        m_isUserLoggedIn = false;
        m_currentUserId = -1;
        m_currentUsername.clear();

        emit userLoginFailed("用户不存在");
        qWarning() << "[DBUser] 用户登录失败: 用户不存在";
        return false;
    }
}

bool DBUser::isUserLoggedIn() const
{
    return m_isUserLoggedIn;
}

void DBUser::userLogout()
{
    QMutexLocker locker(&m_mutex);

    m_isUserLoggedIn = false;
    m_currentUserId = -1;
    m_currentUsername.clear();

    emit userLoginStateChanged(false);
    emit userLogoutSuccess();
    emit currentUserChanged();
    emit operateResult(true, "已退出登录");

    qDebug() << "[DBUser] 用户已退出登录";
}

QString DBUser::getCurrentUsername() const
{
    return m_currentUsername;
}

int DBUser::getCurrentUserId() const
{
    return m_currentUserId;
}


