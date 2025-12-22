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

    Q_INVOKABLE bool connectDB();          // 连接数据库
    Q_INVOKABLE void disconnectDB();       // 断开连接
    Q_INVOKABLE bool isConnected() const;  // 检查连接状态

    Q_INVOKABLE bool verifyAdminLogin(const QString& adminName, const QString& password);//管理员登录验证
    Q_INVOKABLE bool isAdminLoggedIn() const;      // 检查管理员登录状态
    Q_INVOKABLE void adminLogout();               // 管理员登出
    Q_INVOKABLE QString getCurrentAdminName() const; // 获取当前管理员名
    Q_INVOKABLE int getCurrentAdminId() const;     // 获取当前管理员ID

    Q_INVOKABLE QVariantList queryAllFlights();                // 查询所有航班
    Q_INVOKABLE QVariantList queryFlightsByCondition(const QString& departure, const QString& destination, const QString& departDate);  // 按地点，日期查询
    Q_INVOKABLE QVariantList queryFlightByNum(const QString& Flight_id);  // 按航班号查询
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
    Q_INVOKABLE bool updateFlightStatus(const QString& Flight_id, const QString& newststus);  // 更新航班状态
    Q_INVOKABLE bool deleteFlight(const QString& Flight_id);    // 删除航班

    Q_INVOKABLE bool collectFlight(int userId, const QString& flightId, const QString& createTime);  // 收藏航班
    Q_INVOKABLE bool cancelCollectFlight(int userId, const QString& flightId);  // 取消收藏航班
    Q_INVOKABLE QVariantList queryCollectedFlights(int userId);  // 查询用户收藏的所有航班
    Q_INVOKABLE bool isFlightCollected(int userId, const QString& flightId);  // 判断用户是否已收藏某航班

    Q_INVOKABLE void printFlight(const QVariantMap &flight);  // 打印航班
    Q_INVOKABLE void printFlightList(const QVariantList &flightList);  // 打印所有航班

    Q_INVOKABLE int userRegister(const QString& email, const QString& username, const QString& password);  // 普通用户注册
    Q_INVOKABLE int userLogin(const QString& username, const QString& password);  // 普通用户登录
    Q_INVOKABLE void userLogout();  // 普通用户登出

    Q_INVOKABLE bool isUserLoggedIn() const;  // 检查普通用户登录状态
    Q_INVOKABLE int getCurrentUserId() const;  // 获取当前登录用户的ID
    Q_INVOKABLE QString getCurrentUserName() const;  // 获取当前登录用户的用户名
    Q_INVOKABLE QString getCurrentUserEmail() const;  // 获取当前登录用户的邮箱

    Q_INVOKABLE int forgetPassword(const QString& email, const QString& verifyCode, const QString& newPassword);  // 忘记密码（验证码默认为0000）
    // 查看我的订单
    QVariantList queryMyOrders(int userId);
    // 按条件查询我的订单
    QVariantList queryMyOrdersByCondition(int userId,
                                          const QString& flightId = QString(),
                                          const QString& passengerName = QString(),
                                          const QString& status = QString(),
                                          const QString& startDate = QString(),
                                          const QString& endDate = QString());
    // 获取订单详情
    QVariantMap queryOrderDetail(int userId, int orderId);
    // 检查订单是否存在
    bool isOrderExists(int userId, int orderId);
    // 获取订单状态列表
    QStringList getOrderStatusList();
signals:
    void connectionStateChanged(bool isConnected);  // 数据库连接信号
    void operateResult(bool success, const QString &msg);  // 操作结果

    void adminLoginStateChanged(bool isLoggedIn);  // 管理员登录状态改变
    void adminLoginSuccess(const QString& adminName);  // 管理员登录成功
    void adminLoginFailed(const QString& errorMessage);  // 管理员登录失败
    void adminLogoutSuccess();  // 管理员登出

    void userRegisterSuccess(const QString& username);  // 注册成功信号
    void userRegisterFailed(const QString& errorMessage);  // 注册失败信号

    void userLoginStateChanged(bool isLoggedIn);  // 用户登录状态变化信号
    void userLoginSuccess(const QString& username);  // 用户登录成功信号
    void userLoginFailed(const QString& errorMessage);  // 用户登录失败信号
    void userLogoutSuccess();  //用户登出成功信号

    void passwordResetSuccess(const QString& username);  // 密码重置成功
    void passwordResetFailed(const QString& errorMessage);  // 密码重置失败

    // 在信号区域添加订单相关信号
    void queryMyOrdersSuccess(const QVariantList& orders);
    void queryMyOrdersFailed(const QString& errorMsg);
    void orderCanceledSuccess(int orderId);
    void orderCanceledFailed(const QString& errorMsg);
    void orderCreatedSuccess(int orderId);
    void orderCreatedFailed(const QString& errorMsg);
    void orderDetailQuerySuccess(const QVariantMap& orderDetail);
    void orderDetailQueryFailed(const QString& errorMsg);

private:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager() override;

    void initDBConfig();  // 初始化数据库配置

    bool isValidDateTimeFormat(const QString &dateStr);  // 验证日期时间格式

    bool isValidEmailFormat(const QString& email);  // 验证邮箱格式是否合法
    bool isValidPasswordStrength(const QString& password);  // 验证密码强度（至少8位，包含字母和数字）
    bool isUsernameExists(const QString& username);  // 检查用户名是否已存在
    bool isEmailExists(const QString& email);  // 检查邮箱是否已存在
    QString encryptPassword(const QString& password);  // 密码加密（SHA256）

    QSqlDatabase m_db;          // 数据库连接对象
    static DBManager *m_instance;
    static QMutex m_mutex;      // 线程安全锁（避免多线程冲突）
    QString m_dsn;              // ODBC DSN 名称
    QString m_user;             // 数据库用户名
    QString m_password;         // 数据库密码
    QString m_databaseName;     // 目标数据库名
    bool m_isAdminLoggedIn;     // 管理员登录状态
    int m_currentAdminId;       // 当前管理员ID
    QString m_currentAdminName; // 当前管理员名
    bool m_isUserLoggedIn;      // 普通用户登录状态
    int m_currentUserId;        // 当前登录用户ID
    QString m_currentUserName;  // 当前登录用户名
    QString m_currentUserEmail; // 当前登录用户邮箱
};

#endif // DBMANAGER_H
