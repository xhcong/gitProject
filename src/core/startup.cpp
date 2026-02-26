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

bool Startup::dataInit()
{
    Logger::instance().info("=== Starting Data Initialization ===");

    try {
        // Step 1: Initialize CAN interface
        Logger::instance().info("Initializing CAN interface...");
        if (!CANInterface::instance().initialize()) {
            Logger::instance().warning("CAN interface initialization failed or not available");
            // Don't return false - CAN may be optional
        }

        // Step 2: Load configuration from INI file
        Logger::instance().info("Loading INI configuration...");
        QString configPath = QCoreApplication::applicationDirPath() + "/Config/NEngineConfig.ini";
        IniConfigInfo config;
        if (!IniConfig::loadConfig(configPath, config)) {
            Logger::instance().error("Failed to load INI configuration from " + configPath);
            return false;
        }
        GlobalData::instance().setConfig(config);

        // Step 3: Initialize database connection
        Logger::instance().info("Initializing database connection...");
        DBConnection& dbConn = DBConnection::instance();

        if (config.database.type == "sqlite" || config.database.type == "1") {
            // Build absolute path to SQLite database: use applicationDirPath + configured file name
            QDir appDir(QCoreApplication::applicationDirPath());
            const QString dbPath = appDir.filePath(config.database.path);

            Logger::instance().info(QString("Using SQLite database file: %1").arg(dbPath));

            if (!dbConn.initialize("sqlite", "", "", "", dbPath)) {
                Logger::instance().error("Failed to initialize SQLite database");
                return false;
            }
        } else if (config.database.type == "mysql" || config.database.type == "2") {
            if (!dbConn.initialize("QMYSQL", config.mysql.host, config.mysql.user,
                                   config.mysql.password, config.mysql.database)) {
                Logger::instance().error("Failed to initialize MySQL database");
                return false;
            }
        } else {
            Logger::instance().error("Unknown database type: " + config.database.type);
            return false;
        }

        // Step 4: Load plate data from database
        Logger::instance().info("Loading plate data...");
        QList<ne_plate>& plateList = GlobalData::instance().getPlatelist();
        QMap<int, ne_plate>& plateDict = GlobalData::instance().getPlateDict();

        if (!DBQueries::selectAllPlates(plateList)) {
            Logger::instance().warning("Failed to load plates from database");
        } else {
            // Build plate dictionary for quick lookup
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
        } else {
            Logger::instance().info(QString("Loaded %1 metadata items").arg(mdInfoList.size()));
        }

        // Step 6: Load flow information
        Logger::instance().info("Loading flow information...");
        QList<ne_flow_info>& flowInfoList = GlobalData::instance().getFlowInfoList();
        if (!DBQueries::selectAllFlowInfo(flowInfoList)) {
            Logger::instance().warning("Failed to load flow info from database");
        } else {
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

    } catch (const std::exception& e) {
        Logger::instance().error(QString("Exception during initialization: %1").arg(e.what()));
        return false;
    }
}

void Startup::cleanup()
{
    Logger::instance().info("=== Starting Cleanup ===");

    // Cleanup metadata management
    MetaManage::instance().cleanup();

    // Cleanup CAN interface
    CANInterface::instance().cleanup();

    // Close database connection
    DBConnection::instance().close();

    // Clear all global data
    GlobalData::instance().clearAllData();

    Logger::instance().info("=== Cleanup Complete ===");
}
