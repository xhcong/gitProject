#include "ini_config.h"
#include <QSettings>
#include <QFile>

bool IniConfig::loadConfig(const QString& filePath, IniConfigInfo& config)
{
    QFile file(filePath);
    if (!file.exists()) {
        return false;
    }

    QSettings settings(filePath, QSettings::IniFormat);

    // Load DATABASE section
    settings.beginGroup("DATABASE");
    config.database.type = settings.value("Type", "sqlite").toString();
    config.database.path = settings.value("Path", "nenet.db").toString();
    settings.endGroup();

    // Load MYSQL section
    settings.beginGroup("MYSQL");
    config.mysql.host = settings.value("Host", "localhost").toString();
    config.mysql.port = settings.value("Port", 3306).toInt();
    config.mysql.user = settings.value("User", "root").toString();
    config.mysql.password = settings.value("Password", "").toString();
    config.mysql.database = settings.value("Database", "nenet").toString();
    settings.endGroup();

    // Load LOG section
    settings.beginGroup("LOG");
    config.log.level = settings.value("Level", "INFO").toString();
    config.log.path = settings.value("Path", "logs/nenet.log").toString();
    config.log.console_output = settings.value("ConsoleOutput", true).toBool();
    settings.endGroup();

    // Load IP section
    settings.beginGroup("IP");
    config.network.nec_ip = settings.value("NEC_IP", "").toString();
    config.network.nec_port = settings.value("NEC_Port", 0).toInt();
    config.network.nem_ip = settings.value("NEM_IP", "").toString();
    config.network.nem_port = settings.value("NEM_Port", 0).toInt();
    config.network.ned_ip = settings.value("NED_IP", "").toString();
    config.network.ned_port = settings.value("NED_Port", 0).toInt();
    config.network.qi_ip = settings.value("QI_IP", "127.0.0.1").toString();
    config.network.qi_port = settings.value("QI_Port", 0).toInt();

    // UDP communication settings (compatible with C# version)
    config.network.nenet_ip = settings.value("NENet_IP", "127.0.0.1").toString();
    config.network.nenet_ex_ip = settings.value("NENetEx_IP", "127.0.0.1").toString();
    config.network.nenet_nec_port = settings.value("NENet_NEC_Port", 6001).toInt();
    config.network.interface_port = settings.value("Interface_Port", 7000).toInt();
    settings.endGroup();

    // Load HardIO section
    settings.beginGroup("HardIO");
    config.hardio.can_type = settings.value("CAN_Type", "USBCAN").toString();
    config.hardio.can_channel = settings.value("CAN_Channel", 0).toInt();
    config.hardio.can_baudrate = settings.value("CAN_Baudrate", 500000).toInt();
    settings.endGroup();

    return true;
}

bool IniConfig::saveConfig(const QString& filePath, const IniConfigInfo& config)
{
    QSettings settings(filePath, QSettings::IniFormat);

    // Save DATABASE section
    settings.beginGroup("DATABASE");
    settings.setValue("Type", config.database.type);
    settings.setValue("Path", config.database.path);
    settings.endGroup();

    // Save MYSQL section
    settings.beginGroup("MYSQL");
    settings.setValue("Host", config.mysql.host);
    settings.setValue("Port", config.mysql.port);
    settings.setValue("User", config.mysql.user);
    settings.setValue("Password", config.mysql.password);
    settings.setValue("Database", config.mysql.database);
    settings.endGroup();

    // Save LOG section
    settings.beginGroup("LOG");
    settings.setValue("Level", config.log.level);
    settings.setValue("Path", config.log.path);
    settings.setValue("ConsoleOutput", config.log.console_output);
    settings.endGroup();

    // Save IP section
    settings.beginGroup("IP");
    settings.setValue("NEC_IP", config.network.nec_ip);
    settings.setValue("NEC_Port", config.network.nec_port);
    settings.setValue("NEM_IP", config.network.nem_ip);
    settings.setValue("NEM_Port", config.network.nem_port);
    settings.setValue("NED_IP", config.network.ned_ip);
    settings.setValue("NED_Port", config.network.ned_port);
    settings.setValue("QI_IP", config.network.qi_ip);
    settings.setValue("QI_Port", config.network.qi_port);

    // UDP communication settings
    settings.setValue("NENet_IP", config.network.nenet_ip);
    settings.setValue("NENetEx_IP", config.network.nenet_ex_ip);
    settings.setValue("NENet_NEC_Port", config.network.nenet_nec_port);
    settings.setValue("Interface_Port", config.network.interface_port);
    settings.endGroup();

    settings.sync();
    return settings.status() == QSettings::NoError;
}
