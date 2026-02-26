#include "meta_manage.h"
#include "global_data.h"
#include "logging/logger.h"
#include "network/udp_interface.h"
#include "network/protocol.h"
#include <QThread>
#include <QDateTime>
#include <QJsonObject>

MetaManage& MetaManage::instance()
{
    static MetaManage s_instance;
    return s_instance;
}

MetaManage::MetaManage(QObject* parent) : QObject(parent), m_udpInterface(nullptr), m_sendThread(nullptr)
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
        // Get configuration
        const auto& config = GlobalData::instance().getConfig();

        // Store port numbers for later use in signal handlers
        m_necPort = config.network.nenet_nec_port;
        m_interfacePort = config.network.interface_port;

        // Initialize UDP interface
        m_udpInterface = &UDPInterface::instance();
        if (!m_udpInterface->initialize()) {
            Logger::instance().error("Failed to initialize UDP interface");
            return false;
        }

        // Bind NEC communication port
        Logger::instance().info(QString("Binding NEC UDP port %1 on %2")
                               .arg(m_necPort).arg(config.network.nenet_ip));
        if (!m_udpInterface->bindToPort(config.network.nenet_ip, m_necPort)) {
            Logger::instance().error(QString("Failed to bind NEC port %1").arg(m_necPort));
            return false;
        }

        // Bind interface port
        Logger::instance().info(QString("Binding interface UDP port %1 on %2")
                               .arg(m_interfacePort).arg(config.network.nenet_ex_ip));
        if (!m_udpInterface->bindToPort(config.network.nenet_ex_ip, m_interfacePort)) {
            Logger::instance().error(QString("Failed to bind interface port %1").arg(m_interfacePort));
            return false;
        }

        // Connect UDP dataReceivedOnPort signal with custom lambda to route to correct handler
        qDebug() << "Connecting dataReceivedOnPort signal...";
        bool connected = connect(m_udpInterface, QOverload<quint16, const QHostAddress&, quint16, const QByteArray&>::of(&UDPInterface::dataReceivedOnPort),
                this, [this](quint16 localPort, const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data) {
                    qDebug() << "MetaManage lambda: Received on port" << localPort << "from" << senderAddress.toString() << ":" << senderPort;
                    if (localPort == m_necPort) {
                        qDebug() << "Routing to onNECDataReceived";
                        onNECDataReceived(senderAddress, senderPort, data);
                    } else if (localPort == m_interfacePort) {
                        qDebug() << "Routing to onInterfaceDataReceived";
                        onInterfaceDataReceived(senderAddress, senderPort, data);
                    }
                }, Qt::DirectConnection);  // ← 改为DirectConnection

        qDebug() << "dataReceivedOnPort connection result:" << (connected ? "SUCCESS" : "FAILED");

        if (!connected) {
            Logger::instance().error("Failed to connect dataReceivedOnPort signal!");
            return false;
        }

        connect(m_udpInterface, &UDPInterface::errorOccurred,
                this, &MetaManage::onUDPError, Qt::DirectConnection);  // ← 改为DirectConnection

        // 等待线程启动 - 确保socket已经绑定
        Logger::instance().info("Waiting for UDP workers to start...");
        QThread::msleep(200);  // 给工作线程200ms来启动和绑定

        // Send initial heartbeat to NEC
        Logger::instance().info("Sending initial heartbeat to NEC");
        sendMessageToNEC("NENetRunSuccess");

        Logger::instance().info("MetaManage UDP initialization complete");
        return true;

    } catch (const std::exception& e) {
        Logger::instance().error(QString("Exception during MetaManage initialization: %1").arg(e.what()));
        return false;
    }
}

void MetaManage::cleanup()
{
    Logger::instance().info("Cleaning up MetaManage");

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
}

void MetaManage::processHardwareEvents()
{
    // TODO: Process hardware events from global queue
}

void MetaManage::sendToNECClients()
{
    // TODO: Send messages to registered NEC clients
}

void MetaManage::sendMessageToNEC(const QString& message)
{
    if (!m_udpInterface) {
        Logger::instance().warning("UDP Interface not initialized");
        return;
    }

    const auto& config = GlobalData::instance().getConfig();
    QHostAddress necAddress(config.network.nec_ip);

    // Send using the NEC port as source
    Logger::instance().debug(QString("Sending to NEC (%1:%2): %3")
                            .arg(necAddress.toString())
                            .arg(config.network.nec_port)
                            .arg(message));

    m_udpInterface->sendBytesByPort(config.network.nenet_nec_port,
                                    necAddress,
                                    config.network.nec_port,
                                    message.toUtf8());
}

void MetaManage::sendMessageToInterface(const QHostAddress& address, quint16 port, const QString& message)
{
    if (!m_udpInterface) {
        Logger::instance().warning("UDP Interface not initialized");
        return;
    }

    const auto& config = GlobalData::instance().getConfig();
    m_udpInterface->sendBytesByPort(config.network.interface_port, address, port, message.toUtf8());

    Logger::instance().debug(QString("Sent message to interface %1:%2").arg(address.toString()).arg(port));
}

void MetaManage::sendMessageToQI(const QString& message)
{
    if (!m_udpInterface) {
        Logger::instance().warning("UDP Interface not initialized");
        return;
    }

    const auto& config = GlobalData::instance().getConfig();
    QHostAddress qiAddress(config.network.qi_ip);

    // Send QI message using any available port
    m_udpInterface->sendBytes(qiAddress, config.network.qi_port, message.toUtf8());

    Logger::instance().debug(QString("Sent message to QI: %1").arg(message));
}

void MetaManage::onNECDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data)
{
    try {
        qDebug() << "=== onNECDataReceived() called ===";
        qDebug() << "From:" << senderAddress.toString() << ":" << senderPort;
        qDebug() << "Data size:" << data.size();

        QString messageStr = QString::fromUtf8(data);
        qDebug() << "Message:" << messageStr;

        Logger::instance().debug(QString("Received NEC data from %1:%2: %3")
                                .arg(senderAddress.toString()).arg(senderPort).arg(messageStr));

        processNECMessage(messageStr);

    } catch (const std::exception& e) {
        Logger::instance().error(QString("Error processing NEC data: %1").arg(e.what()));
    }
}

void MetaManage::onInterfaceDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data)
{
    try {
        qDebug() << "=== onInterfaceDataReceived() called ===";
        qDebug() << "From:" << senderAddress.toString() << ":" << senderPort;
        qDebug() << "Data size:" << data.size();

        QString messageStr = QString::fromUtf8(data);
        qDebug() << "Message:" << messageStr;

        Logger::instance().debug(QString("Received interface data from %1:%2: %3")
                                .arg(senderAddress.toString()).arg(senderPort).arg(messageStr));

        processInterfaceMessage(senderAddress, senderPort, messageStr);

    } catch (const std::exception& e) {
        Logger::instance().error(QString("Error processing interface data: %1").arg(e.what()));
    }
}

void MetaManage::onUDPError(const QString& errorString)
{
    Logger::instance().error(QString("UDP Error: %1").arg(errorString));
}

void MetaManage::processSendQueue()
{
    // TODO: Implement message queue processing similar to C# version
}

void MetaManage::processNECMessage(const QString& messageStr)
{
    qDebug() << "=== processNECMessage() called ===";
    qDebug() << "Message string:" << messageStr;
    qDebug() << "Message length:" << messageStr.length();

    // Check for heartbeat response
    if (messageStr == "NECRunSuccess") {
        qDebug() << "Received heartbeat response";
        if (!m_necConnected) {
            m_necConnected = true;
            Logger::instance().info("NEC connection established");
            // Send acknowledgement back
            sendMessageToNEC("NENetRunSuccess");
        }
        return;
    }

    // Try to parse as JSON message
    qDebug() << "Attempting to parse as JSON...";
    QJsonObject msgObj = Protocol::parseJsonMessage(messageStr);
    qDebug() << "JSON parse result - keys:" << msgObj.keys();

    bool isValid = Protocol::isValidMessage(msgObj);
    qDebug() << "Message validation result:" << isValid;

    if (!isValid) {
        qWarning() << "WARNING: Invalid NEC message format:" << messageStr;
        Logger::instance().warning(QString("Invalid NEC message format: %1").arg(messageStr));
        return;
    }

    QString messageType = msgObj.value("t").toString("");
    qDebug() << "Message type:" << messageType;
    Logger::instance().debug(QString("Processing NEC message type: %1").arg(messageType));

    // Handle different message types
    Protocol::MessageType type = Protocol::getMessageTypeEnum(messageType);
    qDebug() << "Message type enum value:" << static_cast<int>(type);

    switch (type) {
        case Protocol::MSG_MD_OUT:
            // Handle metadata output (setValue from NEC)
            qDebug() << "Processing MSG_MD_OUT";
            Logger::instance().info("Received metadata output message");
            break;

        case Protocol::MSG_MD_IN:
            // Handle metadata input (sensor data to NEC)
            qDebug() << "Processing MSG_MD_IN";
            Logger::instance().info("Received metadata input message");
            break;

        case Protocol::MSG_MD_CHANGE:
            // Handle metadata change
            qDebug() << "Processing MSG_MD_CHANGE";
            Logger::instance().info("Received metadata change message");
            break;

        default:
            qWarning() << "WARNING: Unknown NEC message type:" << messageType;
            Logger::instance().warning(QString("Unknown NEC message type: %1").arg(messageType));
            break;
    }
    qDebug() << "=== processNECMessage() completed ===";
}

void MetaManage::processInterfaceMessage(const QHostAddress& senderAddress, quint16 senderPort, const QString& messageStr)
{
    // Try to parse as JSON message
    QJsonObject msgObj = Protocol::parseJsonMessage(messageStr);
    if (!Protocol::isValidMessage(msgObj)) {
        Logger::instance().warning(QString("Invalid interface message format: %1").arg(messageStr));
        return;
    }

    QString messageType = msgObj.value("t").toString("");
    Logger::instance().debug(QString("Processing interface message type: %1 from %2:%3")
                            .arg(messageType).arg(senderAddress.toString()).arg(senderPort));

    // Handle different message types
    Protocol::MessageType type = Protocol::getMessageTypeEnum(messageType);

    switch (type) {
        case Protocol::MSG_SET_VALUE:
            Logger::instance().info("Received setValue message");
            break;

        case Protocol::MSG_ADD_REG_LISTEN:
            Logger::instance().info("Received addRegListen message");
            // Store client registration
            m_registeredClients[QString("%1:%2").arg(senderAddress.toString()).arg(senderPort)] =
                QPair<QHostAddress, quint16>(senderAddress, senderPort);
            break;

        case Protocol::MSG_BUTTON_GRADE:
            Logger::instance().info("Received buttonGrade message");
            break;

        case Protocol::MSG_END_GRADE:
            Logger::instance().info("Received endGrade message");
            break;

        default:
            Logger::instance().warning(QString("Unknown interface message type: %1").arg(messageType));
            break;
    }
}
