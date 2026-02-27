#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <QString>
#include <QList>
#include <QMap>
#include <QVector>
#include <vector>

/**
 * @brief Represents a hardware plate configuration
 */
struct ne_plate
{
    int pk_id = 0;
    QString sn;
    QString plate_type;

    // legacy-compatible fields
    int plate_type_id = 0;
    int plate_parent_id = 0;
    QString station_name;
    QString ip_addr;
    int ip_port = 0;
    QString login_name;
    QString login_password;
    int hard_addr = 0;

    QString ip;
    int port = 0;
    int timeout = 5000;
    int retry = 3;
};

/**
 * @brief Represents metadata information
 */
struct ne_md_info
{
    int pk_id = 0;
    int plate_id = 0;

    // legacy-compatible fields
    int plate_type_id = 0;
    int plate_control_id = 0;
    int plate_hard_addr = 0;
    int tport = 0;
    int init_value = 0;
    int kind_id = 0;                 // 1: DO, 2: analog/display, 3: string
    QString current_value_str;       // for kind_id == 3

    QString md_name;
    QString md_type;
    QString md_unit;
    int min_value = 0;
    int max_value = 0;
    int current_value = 0;
    int status = 0;  // 0: normal, 1: alarm, etc.
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
};

/**
 * @brief JF hardware control structure
 */
struct JFHardControl
{
    // legacy-compatible fields used by startup migration logic
    int pk_id = 0;
    QString station_name;
    QString ip_addr;
    int ip_port = 0;
    int plantType = 0;
    QString login_name;
    QString login_password;

    // key: hard_addr, value: 16 channel map/value
    QMap<int, QVector<int>> allDIIdMap;
    QMap<int, QVector<int>> allDOIdMap;
    QMap<int, QVector<int>> allDIValue;
    QMap<int, QVector<int>> allDOValue;
    QMap<int, QVector<int>> allMNdMap;

    // existing fields kept for compatibility
    int board_id = 0;
    QString board_sn;
    QList<ne_plate> plates;
    QMap<int, ne_md_info> metadata_map;
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
