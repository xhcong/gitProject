#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <QString>
#include <QList>

/**
 * @brief String utility functions
 */
class StringUtils
{
public:
    /**
     * @brief Convert string to uppercase
     */
    static QString toUpper(const QString& str);

    /**
     * @brief Convert string to lowercase
     */
    static QString toLower(const QString& str);

    /**
     * @brief MD5 hash
     */
    static QString md5(const QString& str);

    /**
     * @brief String case-insensitive comparison
     */
    static bool stringCompare(const QString& str1, const QString& str2);

    /**
     * @brief Trim string
     */
    static QString trim(const QString& str);

private:
    StringUtils() = delete;
};

#endif // STRING_UTILS_H
