#ifndef META_MANAGE_H
#define META_MANAGE_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QHostAddress>
#include <QList>
#include "network/protocol.h"

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

    void processHardwareEvents();
    void sendToNECClients();
    void sendMessageToNEC(const QString& message);
    void sendMessageToInterface(const QHostAddress& address, quint16 port, const QString& message);
    void sendMessageToQI(const QString& message);

    UDPInterface* getUDPInterface() const { return m_udpInterface; }

private slots:
    void onNECDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);
    void onInterfaceDataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);
    void onUDPError(const QString& errorString);
    void processSendQueue();

private:
    struct MetaRoute {
        int mdId = 0;
        int plateType = 0;
        int controlId = 0;
        int hardAddr = 0;
        int tport = 0;
    };

    MetaManage(QObject* parent = nullptr);
    ~MetaManage() override;

    MetaManage(const MetaManage&) = delete;
    MetaManage& operator=(const MetaManage&) = delete;

    void processNECMessage(const QString& messageStr);
    void processInterfaceMessage(const QHostAddress& senderAddress, quint16 senderPort, const QString& messageStr);

    void rebuildMetaRouteCache();
    bool applySetValue(const QString& messageStr);
    void emitMdInSnapshotToNEC();
    void triggerLegacyNecHardwareDO();

    UDPInterface* m_udpInterface = nullptr;
    QThread* m_sendThread = nullptr;

    bool m_necConnected = false;
    quint16 m_necPort = 6001;
    quint16 m_interfacePort = 7000;

    QMap<QString, QPair<QHostAddress, quint16>> m_registeredClients;
    QMap<int, MetaRoute> m_metaRouteById;
};

#endif // META_MANAGE_H
