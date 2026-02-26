#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QMap>

/**
 * @brief Network protocol definitions and utilities
 * Compatible with C# NENet version message formats
 */
namespace Protocol
{
    // Message types (compatible with C# version)
    enum MessageType {
        MSG_HEARTBEAT = 0,      // Heart beat message
        MSG_DATA = 1,           // Data message
        MSG_COMMAND = 2,        // Command message
        MSG_REGISTER = 3,       // Register/subscribe message
        MSG_UNREGISTER = 4,     // Unregister/unsubscribe message

        // C# version message types
        MSG_MD_IN = 10,         // Metadata input (md_in)
        MSG_MD_OUT = 11,        // Metadata output (md_out)
        MSG_MD_CHANGE = 12,     // Metadata change (md_change)
        MSG_SET_VALUE = 13,     // Set value (setValue)
        MSG_ADD_REG_LISTEN = 14, // Add register listen (addRegListen)
        MSG_IMITATE_DATE = 15,  // Imitate data (imitateDate)
        MSG_BUTTON_GRADE = 16,  // Button grade (buttonGrade)
        MSG_END_GRADE = 17,     // End grade (endGrade)

        MSG_UNKNOWN = 99
    };

    /**
     * @brief Metadata information structure
     * Corresponds to C# net_metaInfo
     */
    struct MetaInfo {
        int d = 0;      // Element pk_id
        QString v;      // Element value
        int n = 0;      // Press count
        int model = 0;  // Model
    };

    /**
     * @brief Base message structure
     * Corresponds to C# net_msg
     */
    struct Message {
        QString t;                      // Message type (e.g., "md_in", "md_out")
        QList<MetaInfo> i;             // Metadata information list
    };

    /**
     * @brief Message with timestamp
     * Corresponds to C# net_msg1
     */
    struct MessageWithTime : public Message {
        QString time;                   // Timestamp
    };

    /**
     * @brief Grade message structure
     * Corresponds to C# net_msg_grade
     */
    struct GradeMessage : public Message {
        int taskID = 0;                 // Task ID
        double score = 0.0;            // Score
    };

    /**
     * @brief Parse JSON message string to QJsonObject
     */
    QJsonObject parseJsonMessage(const QString& data);

    /**
     * @brief Create JSON message string from QJsonObject
     */
    QString createJsonMessage(const QJsonObject& obj);

    /**
     * @brief Validate if message format is correct
     */
    bool isValidMessage(const QJsonObject& msg);

    /**
     * @brief Parse meta info from JSON object
     */
    MetaInfo parseMetaInfo(const QJsonObject& obj);

    /**
     * @brief Convert meta info to JSON object
     */
    QJsonObject metaInfoToJson(const MetaInfo& info);

    /**
     * @brief Parse message from JSON object
     */
    Message parseMessage(const QJsonObject& obj);

    /**
     * @brief Convert message to JSON object
     */
    QJsonObject messageToJson(const Message& msg);

    /**
     * @brief Get message type string from message
     */
    QString getMessageType(const QString& msgStr);

    /**
     * @brief Get message type enum from type string
     */
    MessageType getMessageTypeEnum(const QString& typeStr);

    /**
     * @brief Get message type string from enum
     */
    QString getMessageTypeString(MessageType type);
}

#endif // PROTOCOL_H

