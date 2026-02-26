#ifndef UDP_INTERFACE_H
#define UDP_INTERFACE_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QHostAddress>

class UDPWorker;

/**
 * @brief UDP Interface for managing UDP communication
 *
 * This class manages UDP communication by creating and maintaining UDPWorker threads.
 * It provides a unified interface for binding ports and sending/receiving data.
 */
class UDPInterface : public QObject
{
    Q_OBJECT

public:
    static UDPInterface& instance();

    /**
     * @brief Initialize UDP interface
     */
    bool initialize();

    /**
     * @brief Clean up UDP interface
     */
    void cleanup();

    /**
     * @brief Bind a UDP socket to a specific port
     * @param ip IP address to bind to
     * @param port Port to bind to
     * @return true if successfully bound
     */
    bool bindToPort(const QString& ip, quint16 port);

    /**
     * @brief Unbind a UDP socket from a specific port
     * @param port Port to unbind
     */
    void unbindFromPort(quint16 port);

    /**
     * @brief Send data to a remote address via UDP
     * @param address Target IP address
     * @param port Target port
     * @param data Data to send
     */
    void sendBytes(const QHostAddress& address, quint16 port, const QByteArray& data);

    /**
     * @brief Send data by port (route to the appropriate worker)
     * @param sourcePort Source port (which worker to use)
     * @param address Target IP address
     * @param port Target port
     * @param data Data to send
     */
    void sendBytesByPort(quint16 sourcePort, const QHostAddress& address, quint16 port, const QByteArray& data);

signals:
    /**
     * @brief Emitted when data is received on any port
     * @param senderAddress IP address of the sender
     * @param senderPort Port of the sender
     * @param data Received data
     */
    void dataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);

    /**
     * @brief Emitted when data is received on a specific port
     * @param localPort Local port that received the data
     * @param senderAddress IP address of the sender
     * @param senderPort Port of the sender
     * @param data Received data
     */
    void dataReceivedOnPort(quint16 localPort, const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);

    /**
     * @brief Emitted when an error occurs
     */
    void errorOccurred(const QString& errorString);

private slots:
    /**
     * @brief Handle data received from UDP worker
     */
    void onWorkerDataReceivedWithPort(UDPWorker* worker, const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);

    /**
     * @brief Handle error from UDP worker
     */
    void onWorkerError(const QString& errorString);

private:
    UDPInterface(QObject* parent = nullptr);
    ~UDPInterface() override;

    UDPInterface(const UDPInterface&) = delete;
    UDPInterface& operator=(const UDPInterface&) = delete;

    // Map of port to UDPWorker
    QMap<quint16, UDPWorker*> m_workers;
};

#endif // UDP_INTERFACE_H
