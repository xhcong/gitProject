#ifndef CONFIG_INFO_H
#define CONFIG_INFO_H

#include <QString>
#include <QMap>

/**
 * @brief Stores all configuration from INI files
 */
struct IniConfigInfo
{
    // [DATABASE] section
    struct {
        QString type;  // sqlite or mysql
        QString path;  // for sqlite
    } database;

    // [MYSQL] section
    struct {
        QString host;
        int port = 3306;
        QString user;
        QString password;
        QString database;
    } mysql;

    // [LOG] section
    struct {
        QString level;
        QString path;
        bool console_output = true;
    } log;

    // [IP] section - Network configuration
    struct {
        QString nec_ip;
        int nec_port = 0;
        QString nem_ip;
        int nem_port = 0;
        QString ned_ip;
        int ned_port = 0;
        QString qi_ip;
        int qi_port = 0;

        // UDP通信配置（与C#版本一致）
        QString nenet_ip = "127.0.0.1";     // NENet内部通信IP
        QString nenet_ex_ip = "127.0.0.1";  // 外部接口IP
        int nenet_nec_port = 6001;          // NENet与NEC通信端口
        int interface_port = 7000;          // 外部接口通信端口
    } network;

    // [HardIO] section - Hardware configuration
    struct {
        QString can_type;          // CAN device type
        int can_channel = 0;       // CAN channel
        int can_baudrate = 500000; // CAN baudrate
        QMap<QString, QString> options;  // Other hardware options
    } hardio;

    // [JFPlate] section
    struct {
        QMap<int, QString> plate_configs;  // plate_id -> config string
    } jfplate;

    // [QJCustom] section
    struct {
        QMap<int, QString> port_mappings;  // port id -> mapping
    } qjcustom;

    // Version and metadata
    QString program_name = "NENet";
    QString version = "V20230918.02";
};

#endif // CONFIG_INFO_H
