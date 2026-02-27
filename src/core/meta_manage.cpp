#include "meta_manage.h"
#include "global_data.h"
#include "logging/logger.h"
#include "network/udp_interface.h"
#include "network/protocol.h"
#include "database/db_queries.h"
#include <QThread>
#include <QJsonObject>
#include <QJsonArray>

MetaManage& MetaManage::instance()
{
    static MetaManage s_instance;
    return s_instance;
}

MetaManage::MetaManage(QObject* parent)
    : QObject(parent), m_udpInterface(nullptr), m_sendThread(nullptr)
{
}

MetaManage::~MetaManage()
{
    cleanup();
}

bool MetaManage::initialize()
{
    Logger::instance().info("Initializing MetaManage with UDP support");

    try {
        const auto& config = GlobalData::instance().getConfig();
        m_necPort = config.network.nenet_nec_port;
        m_interfacePort = config.network.interface_port;

        rebuildMetaRouteCache();

        m_udpInterface = &UDPInterface::instance();
        if (!m_udpInterface->initialize()) {
            Logger::instance().error("Failed to initialize UDP interface");
            return false;
        }

        if (!m_udpInterface->bindToPort(config.network.nenet_ip, m_necPort)) {
            Logger::instance().error(QString("Failed to bind NEC port %1").arg(m_necPort));
            return false;
        }

        if (!m_udpInterface->bindToPort(config.network.nenet_ex_ip, m_interfacePort)) {
            Logger::instance().error(QString("Failed to bind interface port %1").arg(m_interfacePort));
            return false;
        }

        const bool connected = connect(
            m_udpInterface,
            QOverload<quint16, const QHostAddress&, quint16, const QByteArray&>::of(&UDPInterface::dataReceivedOnPort),
            this,
            [this](quint16 localPort, const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data) {
                if (localPort == m_necPort) {
                    onNECDataReceived(senderAddress, senderPort, data);
                } else if (localPort == m_interfacePort) {
                    onInterfaceDataReceived(senderAddress, senderPort, data);
                }
            },
            Qt::DirectConnection);

        if (!connected) {
            Logger::instance().error("Failed to connect dataReceivedOnPort signal");
            return false;
        }

        connect(m_udpInterface, &UDPInterface::errorOccurred,
                this, &MetaManage::onUDPError, Qt::DirectConnection);

        QThread::msleep(200);

        sendMessageToNEC("NENetRunSuccess");
        emitMdInSnapshotToNEC();

        Logger::instance().info("MetaManage UDP initialization complete");
        return true;

    } catch (const std::exception& e) {
        Logger::instance().error(QString("Exception during MetaManage initialization: %1").arg(e.what()));
        return false;
    }
}

void MetaManage::cleanup()
{
    if (m_udpInterface) {
        m_udpInterface->cleanup();
    }

    if (m_sendThread) {
        m_sendThread->quit();
        m_sendThread->wait();
        delete m_sendThread;
        m_sendThread = nullptr;
    }

    m_registeredClients.clear();
    m_metaRouteById.clear();
}

void MetaManage::processHardwareEvents() {}
void MetaManage::sendToNECClients() {}
void MetaManage::processSendQueue() {}

void MetaManage::sendMessageToNEC(const QString& message)
{
    if (!m_udpInterface) {
        return;
    }

    const auto& config = GlobalData::instance().getConfig();
    const QHostAddress necAddress(config.network.nec_ip);
    m_udpInterface->sendBytesByPort(config.network.nenet_nec_port,
                                    necAddress,
                                    config.network.nec_port,
                                    message.toUtf8());
}

void MetaManage::sendMessageToInterface(const QHostAddress& address, quint16 port, const QString& message)
{
    if (!m_udpInterface) {
        return;
    }

    const auto& config = GlobalData::instance().getConfig();
    m_udpInterface->sendBytesByPort(config.network.interface_port, address, port, message.toUtf8());
}

void MetaManage::sendMessageToQI(const QString& message)
{
    if (!m_udpInterface) {
        return;
    }

    const auto& config = GlobalData::instance().getConfig();
    const QHostAddress qiAddress(config.network.qi_ip);
    m_udpInterface->sendBytes(qiAddress, config.network.qi_port, message.toUtf8());
}

void MetaManage::onNECDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data)
{
    Q_UNUSED(senderAddress)
    Q_UNUSED(senderPort)

    try {
        processNECMessage(QString::fromUtf8(data));
    } catch (const std::exception& e) {
        Logger::instance().error(QString("Error processing NEC data: %1").arg(e.what()));
    }
}

void MetaManage::onInterfaceDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data)
{
    try {
        processInterfaceMessage(senderAddress, senderPort, QString::fromUtf8(data));
    } catch (const std::exception& e) {
        Logger::instance().error(QString("Error processing interface data: %1").arg(e.what()));
    }
}

void MetaManage::onUDPError(const QString& errorString)
{
    Logger::instance().error(QString("UDP Error: %1").arg(errorString));
}

void MetaManage::processNECMessage(const QString& messageStr)
{
    if (messageStr == "NECRunSuccess") {
        if (!m_necConnected) {
            m_necConnected = true;
            sendMessageToNEC("NENetRunSuccess");
            emitMdInSnapshotToNEC();
        }
        return;
    }

    const QJsonObject msgObj = Protocol::parseJsonMessage(messageStr);
    if (!Protocol::isValidMessage(msgObj)) {
        return;
    }

    const auto type = Protocol::getMessageTypeEnum(msgObj.value("t").toString());

    // Legacy behavior from C# project:
    // when receiving NEC messages, trigger hardware DO commands.
    triggerLegacyNecHardwareDO();

    if (type == Protocol::MSG_MD_CHANGE || type == Protocol::MSG_MD_IN) {
        emitMdInSnapshotToNEC();
    }
}

void MetaManage::processInterfaceMessage(const QHostAddress& senderAddress,
                                         quint16 senderPort,
                                         const QString& messageStr)
{
    const QJsonObject msgObj = Protocol::parseJsonMessage(messageStr);
    if (!Protocol::isValidMessage(msgObj)) {
        return;
    }

    const QString messageType = msgObj.value("t").toString();
    const auto type = Protocol::getMessageTypeEnum(messageType);

    switch (type) {
    case Protocol::MSG_SET_VALUE:
        if (applySetValue(messageStr)) {
            sendMessageToInterface(senderAddress, senderPort, "{\"t\":\"setValueAck\",\"ok\":1}");
            emitMdInSnapshotToNEC();
        } else {
            sendMessageToInterface(senderAddress, senderPort, "{\"t\":\"setValueAck\",\"ok\":0}");
        }
        break;

    case Protocol::MSG_ADD_REG_LISTEN:
        m_registeredClients[QString("%1:%2").arg(senderAddress.toString()).arg(senderPort)] =
            QPair<QHostAddress, quint16>(senderAddress, senderPort);
        sendMessageToInterface(senderAddress, senderPort, "{\"t\":\"addRegListenAck\",\"ok\":1}");
        emitMdInSnapshotToNEC();
        break;

    case Protocol::MSG_BUTTON_GRADE:
    case Protocol::MSG_END_GRADE:
        Logger::instance().info(QString("Received interface command: %1").arg(messageType));
        break;

    default:
        break;
    }
}

void MetaManage::rebuildMetaRouteCache()
{
    m_metaRouteById.clear();
    const QList<ne_md_info>& mdList = GlobalData::instance().getMetaInfoList();

    for (const auto& md : mdList) {
        MetaRoute route;
        route.mdId = md.pk_id;
        route.plateType = md.plate_type_id;
        route.controlId = md.plate_control_id;
        route.hardAddr = md.plate_hard_addr;
        route.tport = md.tport;
        m_metaRouteById[md.pk_id] = route;
    }
}

bool MetaManage::applySetValue(const QString& messageStr)
{
    const QJsonObject msgObj = Protocol::parseJsonMessage(messageStr);
    const Protocol::Message msg = Protocol::parseMessage(msgObj);

    if (msg.i.isEmpty()) {
        return false;
    }

    QList<QPair<int, int>> updates;
    bool allKnown = true;

    for (const auto& meta : msg.i) {
        if (!m_metaRouteById.contains(meta.d)) {
            allKnown = false;
            continue;
        }
        updates.append(qMakePair(meta.d, meta.v.toInt()));
    }

    if (updates.isEmpty()) {
        return false;
    }

    if (!DBQueries::updateMetadataValues(updates)) {
        return false;
    }

    QMap<int, JFHardControl>& jfHardDict = GlobalData::instance().getJFHardDict();
    QList<ne_md_info>& mdList = GlobalData::instance().getMetaInfoList();

    for (const auto& item : updates) {
        const int mdId = item.first;
        const int v = item.second;

        for (auto& md : mdList) {
            if (md.pk_id == mdId) {
                md.current_value = v;
                break;
            }
        }

        const MetaRoute& route = m_metaRouteById[mdId];
        if (route.plateType == 3 && jfHardDict.contains(route.controlId) &&
            jfHardDict[route.controlId].allDOValue.contains(route.hardAddr) &&
            route.tport >= 0 && route.tport < 16) {
            jfHardDict[route.controlId].allDOValue[route.hardAddr][route.tport] = v;
        }

        if (route.plateType == 4 && jfHardDict.contains(route.controlId) &&
            jfHardDict[route.controlId].allDIValue.contains(route.hardAddr) &&
            route.tport >= 0 && route.tport < 16) {
            jfHardDict[route.controlId].allDIValue[route.hardAddr][route.tport] = v;
        }
    }

    return allKnown;
}

void MetaManage::emitMdInSnapshotToNEC()
{
    Protocol::Message msg;
    msg.t = "md_in";

    const QList<ne_md_info>& mdList = GlobalData::instance().getMetaInfoList();
    for (const auto& md : mdList) {
        Protocol::MetaInfo meta;
        meta.d = md.pk_id;
        meta.v = QString::number(md.current_value);
        meta.n = 0;
        meta.model = 0;
        msg.i.append(meta);
    }

    sendMessageToNEC(Protocol::createJsonMessage(Protocol::messageToJson(msg)));
}

void MetaManage::triggerLegacyNecHardwareDO()
{
    // 当前阶段先基于控制器数量即时触发，等后续接入真实 JFPlate 实例池。
    QMap<int, JFHardControl>& jfHardDict = GlobalData::instance().getJFHardDict();

    int okCount = 0;
    int failCount = 0;

    for (auto it = jfHardDict.begin(); it != jfHardDict.end(); ++it) {
        Q_UNUSED(it)

        JFPlate eachJfP;
        eachJfP.initialize();

        const bool ok1 = eachJfP.setEachDO(true, 0, 0);
        const bool ok2 = eachJfP.setSlaveEachDO(true, 0, 0);

        if (ok1 && ok2) {
            ++okCount;
        } else {
            ++failCount;
        }
    }

    Logger::instance().info(QString("triggerLegacyNecHardwareDO executed, controllers=%1, ok=%2, fail=%3")
                                .arg(jfHardDict.size())
                                .arg(okCount)
                                .arg(failCount));
}
