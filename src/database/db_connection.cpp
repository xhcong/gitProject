#include "db_connection.h"
#include "../logging/logger.h"
#include <QSqlDriver>
#include <QSqlError>

DBConnection& DBConnection::instance()
{
    static DBConnection s_instance;
    return s_instance;
}

DBConnection::DBConnection()
{
}

DBConnection::~DBConnection()
{
    close();
}

bool DBConnection::initialize(const QString& dbType, const QString& host,
                              const QString& user, const QString& password,
                              const QString& database)
{
    const QString typeLower = dbType.toLower();

    if (typeLower == "sqlite" || typeLower == "qsqlite") {
        // SQLite: 'database' is the file path
        m_database = QSqlDatabase::addDatabase("QSQLITE");
        m_database.setDatabaseName(database);
    } else if (typeLower == "mysql" || typeLower == "qmysql") {
        m_database = QSqlDatabase::addDatabase("QMYSQL");
        m_database.setHostName(host);
        m_database.setUserName(user);
        m_database.setPassword(password);
        m_database.setDatabaseName(database);
    } else {
        Logger::instance().error(QString("Unsupported database type: %1").arg(dbType));
        return false;
    }

    if (!m_database.open()) {
        Logger::instance().error(QString("Database connection failed: %1")
                                  .arg(m_database.lastError().text()));
        m_connected = false;
        return false;
    }

    Logger::instance().info(QString("Database connected: %1").arg(dbType));
    m_connected = true;
    return true;
}

bool DBConnection::isConnected() const
{
    return m_connected && m_database.isOpen();
}

QSqlDatabase DBConnection::getConnection() const
{
    return m_database;
}

void DBConnection::close()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
    m_connected = false;
}
