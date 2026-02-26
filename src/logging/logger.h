#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QMutex>
#include <QQueue>

/**
 * @brief Simple logging system for the application
 */
class Logger
{
public:
    enum LogLevel {
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR
    };

    static Logger& instance();

    /**
     * @brief Log a message with specified level
     */
    void log(LogLevel level, const QString& message);

    /**
     * @brief Log info message
     */
    void info(const QString& message);

    /**
     * @brief Log warning message
     */
    void warning(const QString& message);

    /**
     * @brief Log error message
     */
    void error(const QString& message);

    /**
     * @brief Log debug message
     */
    void debug(const QString& message);

    /**
     * @brief Initialize logger with optional file output
     * @param filename if empty, only console output
     * @param console_output enable console output (disabled for thread safety)
     */
    void initialize(const QString& filename = "", bool console_output = true);

    /**
     * @brief Cleanup logger resources
     */
    void cleanup();

private:
    Logger();
    ~Logger();

    // Deleted copy/move constructors
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QString levelToString(LogLevel level) const;
    QString getCurrentTime() const;

    QFile m_logFile;
    bool m_consoleOutput = false;  // Disabled by default for thread safety
    bool m_initialized = false;
    mutable QMutex m_mutex;
};

#endif // LOGGER_H
