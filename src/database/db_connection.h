#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <QString>
#include <QSqlDatabase>

/**
 * @brief Database connection management
 */
class DBConnection
{
public:
    static DBConnection& instance();

    /**
     * @brief Initialize database connection
     */
    bool initialize(const QString& dbType, const QString& host = "",
                    const QString& user = "", const QString& password = "",
                    const QString& database = "");

    /**
     * @brief Check if connected
     */
    bool isConnected() const;

    /**
     * @brief Get database connection object
     */
    QSqlDatabase getConnection() const;

    /**
     * @brief Close connection
     */
    void close();

private:
    DBConnection();
    ~DBConnection();

    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;

    QSqlDatabase m_database;
    bool m_connected = false;
};

#endif // DB_CONNECTION_H
