#ifndef UDP_WORKER_H
#define UDP_WORKER_H

#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QString>
#include <QMap>

/**
 * @brief UDP Worker Thread for handling UDP communication
 *
 * This worker thread handles receiving and sending UDP packets on a specific port.
 * It emits a signal when data is received, allowing the main thread to process it.
 */
class UDPWorker : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param ip IP address to bind to
     * @param port Port to bind to
     * @param parent Parent object
     */
    explicit UDPWorker(const QString& ip, quint16 port, QObject* parent = nullptr);
    ~UDPWorker() override;

    /**
     * @brief Check if the socket is bound
     */
    bool isBound() const;

    /**
     * @brief Get the bound port
     */
    quint16 getBoundPort() const;

    /**
     * @brief Get the bound IP address
     */
    QString getBoundIP() const;

public slots:
    /**
     * @brief Send data to a remote address (thread-safe)
     * @param address Target IP address
     * @param port Target port
     * @param data Data to send
     */
    void sendData(const QHostAddress& address, quint16 port, const QByteArray& data);

    /**
     * @brief Stop the worker thread gracefully
     */
    void stop();

signals:
    /**
     * @brief Signal to request data sending (for thread-safe invocation)
     */
    void requestSendData(const QHostAddress& address, quint16 port, const QByteArray& data);

signals:
    /**
     * @brief Emitted when data is received
     * @param senderAddress IP address of the sender
     * @param senderPort Port of the sender
     * @param data Received data
     */
    void dataReceived(const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data);

    /**
     * @brief Emitted when an error occurs
     */
    void errorOccurred(const QString& errorString);

protected:
    /**
     * @brief Thread run loop
     */
    void run() override;

private slots:
    /**
     * @brief Handle readyRead signal from socket
     */
    void onReadyRead();

private:
    QString m_bindIP;
    quint16 m_bindPort;
    QUdpSocket* m_socket = nullptr;
    bool m_running = true;
};

#endif // UDP_WORKER_H
