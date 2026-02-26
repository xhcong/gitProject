#include "startup.h"
#include "global_data.h"
#include "logging/logger.h"
#include "config/ini_config.h"
#include "database/db_connection.h"
#include "database/db_queries.h"
#include "hardware/can_interface.h"
#include "meta_manage.h"
#include <QCoreApplication>
#include <QDir>
#include <QVector>

bool Startup::dataInit()
{
    Logger::instance().info("=== Starting Data Initialization ===");

    try {
        // Step 1: Initialize CAN interface
        Logger::instance().info("Initializing CAN interface...");
        if (!CANInterface::instance().initialize()) {
            Logger::instance().warning("CAN interface initialization failed or not available");
        }

        // Step 2: Load configuration from INI file
        Logger::instance().info("Loading INI configuration...");
        const QString configPath = QCoreApplication::applicationDirPath() + "/Config/NEngineConfig.ini";
        Logger::instance().info(QString("INI config path: %1").arg(configPath));

        IniConfigInfo config;
        if (!IniConfig::loadConfig(configPath, config)) {
            Logger::instance().error("Failed to load INI configuration from " + configPath);
            return false;
        }

        Logger::instance().info(QString("Loaded Interface_Port=%1").arg(config.network.interface_port));
        GlobalData::instance().setConfig(config);

        // Step 3: Initialize database connection
        Logger::instance().info("Initializing database connection...");
        DBConnection& dbConn = DBConnection::instance();

        if (config.database.type == "sqlite" || config.database.type == "1") {
            QDir appDir(QCoreApplication::applicationDirPath());
            const QString dbPath = appDir.filePath(config.database.path);

            Logger::instance().info(QString("Using SQLite database file: %1").arg(dbPath));

            if (!dbConn.initialize("sqlite", "", "", "", dbPath)) {
                Logger::instance().error("Failed to initialize SQLite database");
                return false;
            }
        }else if (config.database.type == "mysql" || config.database.type == "2") {
            if (!dbConn.initialize("QMYSQL", config.mysql.host, config.mysql.user,
                                   config.mysql.password, config.mysql.database)) {
                Logger::instance().error("Failed to initialize MySQL database");
                return false;
            }
        }else {
            Logger::instance().error("Unknown database type: " + config.database.type);
            return false;
        }

        // Step 4: Load plate data from database
        Logger::instance().info("Loading plate data...");
        QList<ne_plate>& plateList = GlobalData::instance().getPlatelist();
        QMap<int, ne_plate>& plateDict = GlobalData::instance().getPlateDict();

        if (!DBQueries::selectAllPlates(plateList)) {
            Logger::instance().warning("Failed to load plates from database");
        }else {
            plateDict.clear();
            for (const auto& plate : plateList) {
                plateDict[plate.pk_id] = plate;
            }
            Logger::instance().info(QString("Loaded %1 plates").arg(plateList.size()));
        }

        // Step 5: Load metadata information
        Logger::instance().info("Loading metadata information...");
        QList<ne_md_info>& mdInfoList = GlobalData::instance().getMetaInfoList();
        if (!DBQueries::selectAllMetaInfo(mdInfoList)) {
            Logger::instance().warning("Failed to load metadata info from database");
        }else {
            Logger::instance().info(QString("Loaded %1 metadata items").arg(mdInfoList.size()));
        }

        // Step 5.1: Rebuild legacy hardware maps (aligned with C# selectSqlData)
        Logger::instance().info("Rebuilding legacy hardware maps...");
        QMap<int, JFHardControl>& jfHardDict = GlobalData::instance().getJFHardDict();
        jfHardDict.clear();

        int plateType2Count = 0;
        int plateType3Count = 0;
        int plateType4Count = 0;
        int plateType5Count = 0;
        int orphanChildPlateCount = 0;

        for (const auto& plate : plateList) {
            const int plateType = plate.plate_type_id;
            if (plateType == 2 || plateType == 5) {
                JFHardControl control;
                control.pk_id = plate.pk_id;
                control.station_name = plate.station_name;
                control.ip_addr = plate.ip_addr.isEmpty() ? plate.ip : plate.ip_addr;
                control.ip_port = (plate.ip_port == 0) ? plate.port : plate.ip_port;
                control.plantType = plateType;
                control.login_name = plate.login_name;
                control.login_password = plate.login_password;
                jfHardDict[plate.pk_id] = control;

                if (plateType == 2) {
                    ++plateType2Count;
                }else {
                    ++plateType5Count;
                    if (plate.hard_addr > 0) {
                        jfHardDict[plate.pk_id].allMNdMap[plate.hard_addr] = QVector<int>(16, 0);
                    }
                }
            }else if (plateType == 3 || plateType == 4) {
                const int parentId = plate.plate_parent_id;
                if (!jfHardDict.contains(parentId)) {
                    ++orphanChildPlateCount;
                    continue;
                }

                const int hardAddr = plate.hard_addr;
                if (hardAddr <= 0) {
                    continue;
                }

                if (plateType == 3) {
                    jfHardDict[parentId].allDOIdMap[hardAddr] = QVector<int>(16, 0);
                    jfHardDict[parentId].allDOValue[hardAddr] = QVector<int>(16, 0);
                    ++plateType3Count;
                }else {
                    jfHardDict[parentId].allDIIdMap[hardAddr] = QVector<int>(16, 0);
                    jfHardDict[parentId].allDIValue[hardAddr] = QVector<int>(16, 0);
                    ++plateType4Count;
                }
            }
        }

        int mappedDOCount = 0;
        int mappedDICount = 0;
        int mappedMNCount = 0;

        for (const auto& md : mdInfoList) {
            if (md.plate_type_id == 3) {
                if (jfHardDict.contains(md.plate_control_id) &&
                    jfHardDict[md.plate_control_id].allDOIdMap.contains(md.plate_hard_addr) &&
                    md.tport >= 0 && md.tport < 16) {
                    jfHardDict[md.plate_control_id].allDOIdMap[md.plate_hard_addr][md.tport] = md.pk_id;
                    jfHardDict[md.plate_control_id].allDOValue[md.plate_hard_addr][md.tport] = md.init_value;
                    ++mappedDOCount;
                }
            }else if (md.plate_type_id == 4) {
                if (jfHardDict.contains(md.plate_control_id) &&
                    jfHardDict[md.plate_control_id].allDIIdMap.contains(md.plate_hard_addr) &&
                    md.tport >= 0 && md.tport < 16) {
                    jfHardDict[md.plate_control_id].allDIIdMap[md.plate_hard_addr][md.tport] = md.pk_id;
                    jfHardDict[md.plate_control_id].allDIValue[md.plate_hard_addr][md.tport] = md.init_value;
                    ++mappedDICount;
                }
            }else if (md.plate_type_id == 5) {
                if (jfHardDict.contains(md.plate_id) &&
                    jfHardDict[md.plate_id].allMNdMap.contains(md.plate_hard_addr) &&
                    md.tport >= 0 && md.tport < 16) {
                    jfHardDict[md.plate_id].allMNdMap[md.plate_hard_addr][md.tport] = md.pk_id;
                    ++mappedMNCount;
                }
            }
        }

        Logger::instance().info(
            QString("Legacy map stats: type2=%1, type3=%2, type4=%3, type5=%4, orphanChildren=%5")
                .arg(plateType2Count)
                .arg(plateType3Count)
                .arg(plateType4Count)
                .arg(plateType5Count)
                .arg(orphanChildPlateCount));
        Logger::instance().info(
            QString("Legacy md map stats: DO=%1, DI=%2, MN=%3")
                .arg(mappedDOCount)
                .arg(mappedDICount)
                .arg(mappedMNCount));

        // Step 6: Load flow information
        Logger::instance().info("Loading flow information...");
        QList<ne_flow_info>& flowInfoList = GlobalData::instance().getFlowInfoList();
        if (!DBQueries::selectAllFlowInfo(flowInfoList)) {
            Logger::instance().warning("Failed to load flow info from database");
        }else {
            Logger::instance().info(QString("Loaded %1 flow items").arg(flowInfoList.size()));
        }

        // Step 7: Initialize metadata management
        Logger::instance().info("Initializing metadata management...");
        if (!MetaManage::instance().initialize()) {
            Logger::instance().error("Failed to initialize metadata management");
            return false;
        }

        GlobalData::instance().logState("After DataInit");

        Logger::instance().info("Welcome to NEngine, Current NENet Version: " +
                                GlobalData::instance().getConfig().version);
        Logger::instance().info("=== Data Initialization Complete ===");
        return true;

    }catch (const std::exception& e) {
        Logger::instance().error(QString("Exception during initialization: %1").arg(e.what()));
        return false;
    }
}

void Startup::cleanup()
{
    Logger::instance().info("=== Starting Cleanup ===");

    MetaManage::instance().cleanup();
    CANInterface::instance().cleanup();
    DBConnection::instance().close();
    GlobalData::instance().clearAllData();

    Logger::instance().info("=== Cleanup Complete ===");
}
