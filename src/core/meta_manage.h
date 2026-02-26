#ifndef META_MANAGE_H
#define META_MANAGE_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QHostAddress>
#include <memory>

class UDPInterface;
class QThread;

/**
 * @brief Metadata management and core processing
 * Handles UDP communication with NEC, Interface, and QI services
 */
class MetaManage : public QObject
{
    Q_OBJECT

public:
    static MetaManage& instance();

    bool initialize();
    void cleanup();

    /**
     * @brief Process hardware events
     */
    void processHardwareEvents();

    /**
     * @brief Send messages to NEC clients
     */
    void sendToNECClients();

    /**
     * @brief Send message to NEC
     */
    void sendMessageToNEC(const QString& message);

    /**
     * @brief Send message to interface clients
     */
    void sendMessageToInterface(const QHostAddress& address, quint16 port, const QString& message);

    /**
     * @brief Send message to QI
     */
    void sendMessageToQI(const QString& message);

    /**
     * @brief Get UDP interface instance
     */
    UDPInterface* getUDPInterface() const { return m_udpInterface; }

private slots:
    /**
     * @brief Handle UDP data received on NEC port
     */
    void onNECDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);

    /**
     * @brief Handle UDP data received on interface port
     */
    void onInterfaceDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);

    /**
     * @brief Handle UDP error
     */
    void onUDPError(const QString& errorString);

    /**
     * @brief Process message sending queue (async)
     */
    void processSendQueue();

private:
    MetaManage(QObject* parent = nullptr);
    ~MetaManage() override;

    MetaManage(const MetaManage&) = delete;
    MetaManage& operator=(const MetaManage&) = delete;

    /**
     * @brief Process NEC message (md_in, md_out, etc.)
     */
    void processNECMessage(const QString& messageStr);

    /**
     * @brief Process interface message (setValue, addRegListen, etc.)
     */
    void processInterfaceMessage(const QHostAddress& senderAddress, quint16 senderPort, const QString& messageStr);

    // UDP Interface instance
    UDPInterface* m_udpInterface = nullptr;

    // Background thread for message sending
    QThread* m_sendThread = nullptr;

    // NEC status
    bool m_necConnected = false;

    // Port numbers for routing
    quint16 m_necPort = 6001;
    quint16 m_interfacePort = 7000;

    // Store registered interface clients for monitoring
    QMap<QString, QPair<QHostAddress, quint16>> m_registeredClients;
};

#endif // META_MANAGE_H
