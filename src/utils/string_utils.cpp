#include "string_utils.h"
#include <QCryptographicHash>

QString StringUtils::toUpper(const QString& str)
{
    return str.toUpper();
}

QString StringUtils::toLower(const QString& str)
{
    return str.toLower();
}

QString StringUtils::md5(const QString& str)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(str.toLatin1(), QCryptographicHash::Md5).toHex());
}

bool StringUtils::stringCompare(const QString& str1, const QString& str2)
{
    return str1.compare(str2, Qt::CaseInsensitive) == 0;
}

QString StringUtils::trim(const QString& str)
{
    return str.trimmed();
}
