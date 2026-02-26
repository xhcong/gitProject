#include "logger.h"
#include <QDateTime>
#include <QTextStream>
#include <QMutexLocker>

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
{
    initialize("", false);  // Default: no console output (not thread-safe)
}

Logger::~Logger()
{
    cleanup();
}

void Logger::initialize(const QString& filename, bool console_output)
{
    QMutexLocker locker(&m_mutex);

    m_consoleOutput = console_output;

    if (!filename.isEmpty()) {
        m_logFile.setFileName(filename);
        if (m_logFile.open(QIODevice::Append | QIODevice::Text)) {
            m_initialized = true;
        }
    }
}

void Logger::cleanup()
{
    QMutexLocker locker(&m_mutex);

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::log(LogLevel level, const QString& message)
{
    QMutexLocker locker(&m_mutex);

    QString timestamp = getCurrentTime();
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] %2: %3").arg(timestamp, levelStr, message);

    // File output only - std::cout is not thread-safe and can block
    if (m_logFile.isOpen()) {
        QTextStream out(&m_logFile);
        out << logMessage << "\n";
        out.flush();
    }
}

void Logger::info(const QString& message)
{
    log(LOG_INFO, message);
}

void Logger::warning(const QString& message)
{
    log(LOG_WARNING, message);
}

void Logger::error(const QString& message)
{
    log(LOG_ERROR, message);
}

void Logger::debug(const QString& message)
{
    log(LOG_DEBUG, message);
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
    case LOG_DEBUG:
        return "DEBUG";
    case LOG_INFO:
        return "INFO";
    case LOG_WARNING:
        return "WARNING";
    case LOG_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

QString Logger::getCurrentTime() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}
