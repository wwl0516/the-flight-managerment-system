#ifndef DBUSER_H
#define DBUSER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QDateTime>
#include <QMutex>
#include <QCryptographicHash>
#include <QRegularExpression>  // 使用QRegularExpression替代QRegExp

// 用户管理类
class DBUser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DBUser)

    Q_PROPERTY(bool isUserLoggedIn READ isUserLoggedIn NOTIFY userLoginStateChanged)
    Q_PROPERTY(QString currentUsername READ getCurrentUsername NOTIFY currentUserChanged)
    Q_PROPERTY(int currentUserId READ getCurrentUserId NOTIFY currentUserChanged)

public:
    // 获取单例实例
    static DBUser* getInstance(QObject *parent = nullptr);

    // 数据库连接
    Q_INVOKABLE void setDatabase(const QSqlDatabase &database);
    Q_INVOKABLE bool isDatabaseConnected() const;

    // 用户注册（只需用户名、密码、邮箱）
    Q_INVOKABLE bool registerUser(const QString& username, const QString& password,
                                  const QString& confirmPassword, const QString& email);

    // 用户登录验证（只需用户名和密码）
    Q_INVOKABLE bool verifyUserLogin(const QString& username, const QString& password);
    Q_INVOKABLE bool isUserLoggedIn() const;
    Q_INVOKABLE void userLogout();
    Q_INVOKABLE QString getCurrentUsername() const;
    Q_INVOKABLE int getCurrentUserId() const;

    // 用户表管理（基础功能）
    Q_INVOKABLE bool createUserTable();

signals:
    // 用户状态信号
    void userLoginStateChanged(bool isLoggedIn);
    void userLoginSuccess(const QString& username);
    void userLoginFailed(const QString& errorMessage);
    void userLogoutSuccess();
    void currentUserChanged();

    // 注册相关信号
    void registerSuccess(const QString& username);
    void registerFailed(const QString& errorMessage);

    // 操作结果信号
    void operateResult(bool success, const QString& message);

private:
    explicit DBUser(QObject *parent = nullptr);
    ~DBUser() override;

    // 输入验证
    bool isValidUsername(const QString& username);
    bool isPasswordStrong(const QString& password);
    bool isValidEmail(const QString& email);

    // 密码加密（SHA-256哈希）
    QString encryptPassword(const QString& password);

    QSqlDatabase m_db;
    static DBUser *m_instance;
    static QMutex m_mutex;

    // 当前登录用户信息
    bool m_isUserLoggedIn;
    int m_currentUserId;
    QString m_currentUsername;
};

#endif // DBUSER_H
