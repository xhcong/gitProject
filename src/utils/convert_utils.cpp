#include "convert_utils.h"

int ConvertUtils::toInt(const QString& str, bool* ok)
{
    return str.toInt(ok);
}

double ConvertUtils::toDouble(const QString& str, bool* ok)
{
    return str.toDouble(ok);
}

QString ConvertUtils::toString(int value)
{
    return QString::number(value);
}

QString ConvertUtils::toString(double value, int precision)
{
    return QString::number(value, 'f', precision);
}

QString ConvertUtils::toHexString(const QByteArray& data)
{
    return QString::fromLatin1(data.toHex());
}

QByteArray ConvertUtils::fromHexString(const QString& hexStr)
{
    return QByteArray::fromHex(hexStr.toLatin1());
}
