#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>
#include <queue>
#include "database/data_structures.h"
#include "config/config_info.h"
#include "hardware/jf_plate.h"

/**
 * @brief Thread-safe global data container
 *
 * Manages all global state of the application including:
 * - Configuration data from INI files
 * - Database connections
 * - Hardware state and mappings
 * - Message queues
 */
class GlobalData
{
public:
    static GlobalData& instance();

    // Configuration
    IniConfigInfo& getConfig();
    void setConfig(const IniConfigInfo& config);

    // Hardware data
    QMap<int, JFHardControl>& getJFHardDict();
    QMap<int, JFPlate>& getJFPlateDict();

    // Plate and metadata info
    QList<ne_plate>& getPlatelist();
    QMap<int, ne_plate>& getPlateDict();
    QList<ne_md_info>& getMetaInfoList();
    QList<ne_flow_info>& getFlowInfoList();

    // Message queues
    std::queue<HardwareEvent>& getHardwareEventQueue();
    std::queue<NECMessage>& getNECMessageQueue();

    // Mutex for thread-safe access
    QMutex& getMutex();

    // Utility methods
    void clearAllData();
    void logState(const QString& context = "");

private:
    GlobalData();
    ~GlobalData() = default;

    // Deleted copy/move constructors
    GlobalData(const GlobalData&) = delete;
    GlobalData& operator=(const GlobalData&) = delete;

    // Configuration data
    IniConfigInfo m_config;

    // Hardware data
    QMap<int, JFHardControl> m_jfHardAllDict;
    QMap<int, JFPlate> m_jfPlateDict;

    // Plate and metadata
    QList<ne_plate> m_listPlate;
    QMap<int, ne_plate> m_dictPlate;
    QList<ne_md_info> m_listMdInfo;
    QList<ne_flow_info> m_listFlowInfo;

    // Message queues for inter-thread communication
    std::queue<HardwareEvent> m_hardwareEventQueue;
    std::queue<NECMessage> m_necMessageQueue;

    // Thread synchronization
    mutable QMutex m_mutex;
};

#endif // GLOBAL_DATA_H
