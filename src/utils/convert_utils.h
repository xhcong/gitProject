#ifndef CONVERT_UTILS_H
#define CONVERT_UTILS_H

#include <QString>

/**
 * @brief Type conversion utilities
 */
class ConvertUtils
{
public:
    /**
     * @brief Convert string to integer
     */
    static int toInt(const QString& str, bool* ok = nullptr);

    /**
     * @brief Convert string to double
     */
    static double toDouble(const QString& str, bool* ok = nullptr);

    /**
     * @brief Convert integer to string
     */
    static QString toString(int value);

    /**
     * @brief Convert double to string
     */
    static QString toString(double value, int precision = 2);

    /**
     * @brief Convert bytes to hex string
     */
    static QString toHexString(const QByteArray& data);

    /**
     * @brief Convert hex string to bytes
     */
    static QByteArray fromHexString(const QString& hexStr);

private:
    ConvertUtils() = delete;
};

#endif // CONVERT_UTILS_H
