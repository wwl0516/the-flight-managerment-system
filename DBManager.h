#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QDateTime>
#include <QMutex>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QString>

// 数据库管理单例类
class DBManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DBManager)
    // 管理员相关属性
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStateChanged)
    Q_PROPERTY(bool isAdminLoggedIn READ isAdminLoggedIn NOTIFY adminLoginStateChanged)
    // 普通用户相关属性（新增）
    Q_PROPERTY(bool isUserLoggedIn READ isUserLoggedIn NOTIFY userLoginStateChanged)
    Q_PROPERTY(QString currentUserName READ getCurrentUserName NOTIFY userLoginStateChanged)
    Q_PROPERTY(QString currentUserEmail READ getCurrentUserEmail NOTIFY userLoginStateChanged)

public:
    // 全局获取单例
    static DBManager* getInstance(QObject *parent = nullptr);

    // ========== 原有核心方法 ==========
    Q_INVOKABLE bool connectDB();          // 连接数据库
    Q_INVOKABLE void disconnectDB();       // 断开连接
    Q_INVOKABLE bool isConnected() const;  // 检查连接状态

    Q_INVOKABLE bool verifyAdminLogin(const QString& adminName, const QString& password);//管理员登录验证
    Q_INVOKABLE bool isAdminLoggedIn() const;      // 检查管理员登录状态
    Q_INVOKABLE void adminLogout();               // 管理员登出
    Q_INVOKABLE QString getCurrentAdminName() const; // 获取当前管理员名
    Q_INVOKABLE int getCurrentAdminId() const;     // 获取当前管理员ID

    Q_INVOKABLE QVariantList queryAllFlights();                // 查询所有航班
    Q_INVOKABLE QVariantMap queryFlightByNum(const QString& Flight_id);  // 按航班号查询
    Q_INVOKABLE bool addFlight(
        const QString& Flight_id,
        const QString& Departure,
        const QString& Destination,
        const QString& depart_time,
        const QString& arrive_time,
        double price,
        int total_seats,
        int remain_seats
        );  // 添加航班
    Q_INVOKABLE bool updateFlightPrice(const QString& Flight_id, double newPrice);  // 更新价格
    Q_INVOKABLE bool updateFlightSeats(const QString& Flight_id, int newRemainSeats);  // 更新剩余座位
    Q_INVOKABLE bool deleteFlight(const QString& Flight_id);    // 删除航班

    Q_INVOKABLE void printFlight(const QVariantMap &flight);
    Q_INVOKABLE void printFlightList(const QVariantList &flightList);

    // ========== 新增：普通用户登录/注册功能 ==========
    /**
     * @brief 用户注册
     * @param email 邮箱（唯一）
     * @param username 用户名（唯一）
     * @param password 密码（建议至少8位，包含字母和数字）
     * @return 注册成功返回true，失败返回false
     */
    Q_INVOKABLE bool userRegister(const QString& email, const QString& username, const QString& password);

    /**
     * @brief 用户登录
     * @param username 用户名
     * @param password 密码
     * @return 登录成功返回true，失败返回false
     */
    Q_INVOKABLE bool userLogin(const QString& username, const QString& password);

    /**
     * @brief 用户登出
     */
    Q_INVOKABLE void userLogout();

    /**
     * @brief 检查普通用户登录状态
     * @return 已登录返回true，否则返回false
     */
    Q_INVOKABLE bool isUserLoggedIn() const;

    /**
     * @brief 获取当前登录用户的ID
     * @return 用户ID
     */
    Q_INVOKABLE int getCurrentUserId() const;

    /**
     * @brief 获取当前登录用户的用户名
     * @return 用户名
     */
    Q_INVOKABLE QString getCurrentUserName() const;

    /**
     * @brief 获取当前登录用户的邮箱
     * @return 邮箱
     */
    Q_INVOKABLE QString getCurrentUserEmail() const;

signals:
    // ========== 原有信号 ==========
    void connectionStateChanged(bool isConnected);
    void operateResult(bool success, const QString &msg);
    void adminLoginStateChanged(bool isLoggedIn);
    void adminLoginSuccess(const QString& adminName);
    void adminLoginFailed(const QString& errorMessage);
    void adminLogoutSuccess();

    // ========== 新增：用户登录/注册相关信号 ==========
    /**
     * @brief 注册成功信号
     * @param username 注册的用户名
     */
    void userRegisterSuccess(const QString& username);

    /**
     * @brief 注册失败信号
     * @param errorMessage 失败原因（如邮箱格式错误、用户名已存在等）
     */
    void userRegisterFailed(const QString& errorMessage);

    /**
     * @brief 用户登录状态变化信号
     * @param isLoggedIn 登录状态
     */
    void userLoginStateChanged(bool isLoggedIn);

    /**
     * @brief 用户登录成功信号
     * @param username 登录的用户名
     */
    void userLoginSuccess(const QString& username);

    /**
     * @brief 用户登录失败信号
     * @param errorMessage 失败原因（如用户名不存在、密码错误等）
     */
    void userLoginFailed(const QString& errorMessage);

    /**
     * @brief 用户登出成功信号
     */
    void userLogoutSuccess();

private:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager() override;

    // ========== 原有私有方法 ==========
    // 初始化数据库配置
    void initDBConfig();
    // 验证日期时间格式
    bool isValidDateTimeFormat(const QString &dateStr);

    // ========== 新增：私有工具方法（数据验证、加密等） ==========
    /**
     * @brief 验证邮箱格式是否合法
     * @param email 待验证的邮箱
     * @return 合法返回true，否则返回false
     */
    bool isValidEmailFormat(const QString& email);

    /**
     * @brief 验证密码强度（至少8位，包含字母和数字）
     * @param password 待验证的密码
     * @return 符合强度返回true，否则返回false
     */
    bool isValidPasswordStrength(const QString& password);

    /**
     * @brief 检查用户名是否已存在
     * @param username 用户名
     * @return 存在返回true，否则返回false
     */
    bool isUsernameExists(const QString& username);

    /**
     * @brief 检查邮箱是否已存在
     * @param email 邮箱
     * @return 存在返回true，否则返回false
     */
    bool isEmailExists(const QString& email);

    /**
     * @brief 密码加密（SHA256）
     * @param password 原始密码
     * @return 加密后的密码字符串
     */
    QString encryptPassword(const QString& password);

    /**
     * @brief 初始化用户表（t_users），确保表存在
     */
    void initUserTable();

    // ========== 成员变量 ==========
    // 原有变量
    QSqlDatabase m_db;          // 数据库连接对象
    static DBManager *m_instance;
    static QMutex m_mutex;      // 线程安全锁（避免多线程冲突）
    QString m_dsn;              // ODBC DSN 名称
    QString m_user;             // 数据库用户名
    QString m_password;         // 数据库密码
    QString m_databaseName;     // 目标数据库名
    bool m_isAdminLoggedIn;
    int m_currentAdminId;
    QString m_currentAdminName;

    // 新增：普通用户相关变量
    bool m_isUserLoggedIn;      // 普通用户登录状态
    int m_currentUserId;        // 当前登录用户ID
    QString m_currentUserName;  // 当前登录用户名
    QString m_currentUserEmail; // 当前登录用户邮箱
};

#endif // DBMANAGER_H
