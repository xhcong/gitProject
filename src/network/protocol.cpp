#include "protocol.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

QJsonObject Protocol::parseJsonMessage(const QString& data)
{
    QJsonObject emptyObject;

    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        return emptyObject;
    }

    return doc.object();
}

QString Protocol::createJsonMessage(const QJsonObject& obj)
{
    QJsonDocument doc(obj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

bool Protocol::isValidMessage(const QJsonObject& msg)
{
    // A valid message should have at least a 't' (type) field
    return msg.contains("t") && msg["t"].isString();
}

Protocol::MetaInfo Protocol::parseMetaInfo(const QJsonObject& obj)
{
    MetaInfo info;
    info.d = obj.value("d").toInt(0);
    info.v = obj.value("v").toString("");
    info.n = obj.value("n").toInt(0);
    info.model = obj.value("model").toInt(0);
    return info;
}

QJsonObject Protocol::metaInfoToJson(const MetaInfo& info)
{
    QJsonObject obj;
    obj["d"] = info.d;
    obj["v"] = info.v;
    obj["n"] = info.n;
    obj["model"] = info.model;
    return obj;
}

Protocol::Message Protocol::parseMessage(const QJsonObject& obj)
{
    Message msg;
    msg.t = obj.value("t").toString("");

    // Parse metadata array
    if (obj.contains("i") && obj["i"].isArray()) {
        QJsonArray iArray = obj["i"].toArray();
        for (int idx = 0; idx < iArray.size(); ++idx) {
            if (iArray[idx].isObject()) {
                msg.i.append(parseMetaInfo(iArray[idx].toObject()));
            }
        }
    }

    return msg;
}

QJsonObject Protocol::messageToJson(const Message& msg)
{
    QJsonObject obj;
    obj["t"] = msg.t;

    // Convert metadata list to array
    QJsonArray iArray;
    for (const auto& meta : msg.i) {
        iArray.append(metaInfoToJson(meta));
    }
    obj["i"] = iArray;

    return obj;
}

QString Protocol::getMessageType(const QString& msgStr)
{
    QJsonObject obj = parseJsonMessage(msgStr);
    return obj.value("t").toString("");
}

Protocol::MessageType Protocol::getMessageTypeEnum(const QString& typeStr)
{
    if (typeStr == "md_in") return MSG_MD_IN;
    if (typeStr == "md_out") return MSG_MD_OUT;
    if (typeStr == "md_change") return MSG_MD_CHANGE;
    if (typeStr == "setValue") return MSG_SET_VALUE;
    if (typeStr == "addRegListen") return MSG_ADD_REG_LISTEN;
    if (typeStr == "imitateDate") return MSG_IMITATE_DATE;
    if (typeStr == "buttonGrade") return MSG_BUTTON_GRADE;
    if (typeStr == "endGrade") return MSG_END_GRADE;

    return MSG_UNKNOWN;
}

QString Protocol::getMessageTypeString(MessageType type)
{
    switch (type) {
        case MSG_MD_IN: return "md_in";
        case MSG_MD_OUT: return "md_out";
        case MSG_MD_CHANGE: return "md_change";
        case MSG_SET_VALUE: return "setValue";
        case MSG_ADD_REG_LISTEN: return "addRegListen";
        case MSG_IMITATE_DATE: return "imitateDate";
        case MSG_BUTTON_GRADE: return "buttonGrade";
        case MSG_END_GRADE: return "endGrade";
        default: return "unknown";
    }
}

