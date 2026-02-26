#include "global_data.h"
#include "logging/logger.h"

GlobalData& GlobalData::instance()
{
    static GlobalData s_instance;
    return s_instance;
}

GlobalData::GlobalData()
{
    // Initialize default values
    m_config.program_name = "NENet";
    m_config.version = "V20230918.02";
}

IniConfigInfo& GlobalData::getConfig()
{
    return m_config;
}

void GlobalData::setConfig(const IniConfigInfo& config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
}

QMap<int, JFHardControl>& GlobalData::getJFHardDict()
{
    return m_jfHardAllDict;
}

QMap<int, JFPlate>& GlobalData::getJFPlateDict()
{
    return m_jfPlateDict;
}

QList<ne_plate>& GlobalData::getPlatelist()
{
    return m_listPlate;
}

QMap<int, ne_plate>& GlobalData::getPlateDict()
{
    return m_dictPlate;
}

QList<ne_md_info>& GlobalData::getMetaInfoList()
{
    return m_listMdInfo;
}

QList<ne_flow_info>& GlobalData::getFlowInfoList()
{
    return m_listFlowInfo;
}

std::queue<HardwareEvent>& GlobalData::getHardwareEventQueue()
{
    return m_hardwareEventQueue;
}

std::queue<NECMessage>& GlobalData::getNECMessageQueue()
{
    return m_necMessageQueue;
}

QMutex& GlobalData::getMutex()
{
    return m_mutex;
}

void GlobalData::clearAllData()
{
    QMutexLocker locker(&m_mutex);

    m_jfHardAllDict.clear();
    m_jfPlateDict.clear();
    m_listPlate.clear();
    m_dictPlate.clear();
    m_listMdInfo.clear();
    m_listFlowInfo.clear();

    // Clear queues
    while (!m_hardwareEventQueue.empty()) {
        m_hardwareEventQueue.pop();
    }
    while (!m_necMessageQueue.empty()) {
        m_necMessageQueue.pop();
    }
}

void GlobalData::logState(const QString& context)
{
    QMutexLocker locker(&m_mutex);

    Logger::instance().info(QString("=== GlobalData State (%1) ===").arg(context));
    Logger::instance().info(QString("Program: %1 %2").arg(m_config.program_name, m_config.version));
    Logger::instance().info(QString("Plates: %1").arg(m_listPlate.size()));
    Logger::instance().info(QString("Metadata: %1").arg(m_listMdInfo.size()));
    Logger::instance().info(QString("Hardware Events: %1").arg(static_cast<int>(m_hardwareEventQueue.size())));
    Logger::instance().info(QString("NEC Messages: %1").arg(static_cast<int>(m_necMessageQueue.size())));
}
