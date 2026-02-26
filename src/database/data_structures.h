#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <QString>
#include <QList>
#include <QMap>
#include <vector>

/**
 * @brief Represents a hardware plate configuration
 */
struct ne_plate
{
    int pk_id = 0;
    QString sn;
    QString plate_type;
    QString ip;
    int port = 0;
    int timeout = 5000;
    int retry = 3;
    // Add other fields as needed
};

/**
 * @brief Represents metadata information
 */
struct ne_md_info
{
    int pk_id = 0;
    int plate_id = 0;
    QString md_name;
    QString md_type;
    QString md_unit;
    int min_value = 0;
    int max_value = 0;
    int current_value = 0;
    int status = 0;  // 0: normal, 1: alarm, etc.
    // Add other fields as needed
};

/**
 * @brief Represents flow information
 */
struct ne_flow_info
{
    int pk_id = 0;
    QString flow_name;
    QString flow_type;
    int plate_id = 0;
    // Add other fields as needed
};

/**
 * @brief JF hardware control structure
 */
struct JFHardControl
{
    int board_id = 0;
    QString board_sn;
    QList<ne_plate> plates;
    QMap<int, ne_md_info> metadata_map;
    // Add other fields as needed
};

/**
 * @brief Hardware event message
 */
struct HardwareEvent
{
    enum EventType {
        UNKNOWN = 0,
        PLATE_CONNECTED = 1,
        PLATE_DISCONNECTED = 2,
        METADATA_CHANGED = 3,
        DATA_RECEIVED = 4,
        ERROR_OCCURRED = 5
    };

    EventType type = UNKNOWN;
    int source_id = 0;
    QString data;
    long timestamp = 0;
};

/**
 * @brief NEC message structure
 */
struct NECMessage
{
    int msg_id = 0;
    QString msg_type;
    QMap<QString, QString> params;
    QString json_data;
};

#endif // DATA_STRUCTURES_H
