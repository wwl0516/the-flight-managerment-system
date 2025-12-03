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

    // ... 其余代码保持不变

// 数据库管理单例类
class DBManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DBManager)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStateChanged)
    Q_PROPERTY(bool isAdminLoggedIn READ isAdminLoggedIn NOTIFY adminLoginStateChanged)

public:
    // 全局获取单例
    static DBManager* getInstance(QObject *parent = nullptr);

    Q_INVOKABLE bool connectDB();          // 连接数据库
    Q_INVOKABLE void disconnectDB();       // 断开连接
    Q_INVOKABLE bool isConnected() const;  // 检查连接状态

    Q_INVOKABLE bool verifyAdminLogin(const QString& adminName, const QString& password);//管理员登录验证
    Q_INVOKABLE bool isAdminLoggedIn() const;      // 检查登录状态
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

signals:
    void connectionStateChanged(bool isConnected);
    void operateResult(bool success, const QString &msg);
    void adminLoginStateChanged(bool isLoggedIn);
    void adminLoginSuccess(const QString& adminName);
    void adminLoginFailed(const QString& errorMessage);
    void adminLogoutSuccess();

private:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager() override;

    // 初始化数据库配置
    void initDBConfig();

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

    bool isValidDateTimeFormat(const QString &dateStr);

};


#endif // DBMANAGER_H
