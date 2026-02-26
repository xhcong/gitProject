#include "udp_worker.h"
#include <QEventLoop>

UDPWorker::UDPWorker(const QString& ip, quint16 port, QObject* parent)
    : QThread(parent), m_bindIP(ip), m_bindPort(port), m_running(true)
{
}

UDPWorker::~UDPWorker()
{
    stop();
    wait();  // Wait for thread to finish

    if (m_socket) {
        m_socket->close();
        delete m_socket;
    }
}

bool UDPWorker::isBound() const
{
    return m_socket && m_socket->state() == QUdpSocket::BoundState;
}

quint16 UDPWorker::getBoundPort() const
{
    return m_bindPort;
}

QString UDPWorker::getBoundIP() const
{
    return m_bindIP;
}

void UDPWorker::sendData(const QHostAddress& address, quint16 port, const QByteArray& data)
{
    qDebug() << "=== sendData() called ===";
    qDebug() << "Port:" << m_bindPort << "Socket exists:" << (m_socket != nullptr);

    if (!m_socket) {
        qWarning() << "ERROR: UDP Socket not initialized on port" << m_bindPort;
        return;
    }

    qDebug() << "Sending" << data.size() << "bytes to" << address.toString() << ":" << port;

    // 发送数据
    qint64 sentBytes = m_socket->writeDatagram(data, address, port);

    if (sentBytes == -1) {
        qWarning() << "ERROR: Failed to send UDP data on port" << m_bindPort
                   << "Error:" << m_socket->errorString();
        emit errorOccurred(QString("Failed to send UDP data: %1").arg(m_socket->errorString()));
    } else if (sentBytes != data.size()) {
        qWarning() << "WARNING: Partial send on port" << m_bindPort
                   << "Sent:" << sentBytes << "bytes of" << data.size();
    } else {
        qDebug() << "SUCCESS: UDP data sent from port" << m_bindPort
                 << "to" << address.toString() << ":" << port
                 << "(" << sentBytes << "bytes)";
    }
}

void UDPWorker::stop()
{
    m_running = false;
    // quit()会让exec()退出
    quit();
}

void UDPWorker::run()
{
    // Create socket in this thread
    m_socket = new QUdpSocket();

    // Bind socket to the specified address and port
    if (!m_socket->bind(QHostAddress(m_bindIP), m_bindPort)) {
        qWarning() << "Failed to bind UDP socket to" << m_bindIP << ":" << m_bindPort;
        qWarning() << "Socket error:" << m_socket->errorString();
        delete m_socket;
        m_socket = nullptr;
        emit errorOccurred(QString("Failed to bind UDP socket to %1:%2").arg(m_bindIP).arg(m_bindPort));
        return;
    }

    qDebug() << "UDP socket successfully bound to" << m_bindIP << ":" << m_bindPort;

    // Connect readyRead signal to slot
    connect(m_socket, &QUdpSocket::readyRead, this, &UDPWorker::onReadyRead,
            Qt::DirectConnection);

    // Connect the requestSendData signal to sendData slot for thread-safe sending
    // 检查连接是否成功
    bool connected = connect(this, &UDPWorker::requestSendData, this, &UDPWorker::sendData,
                            Qt::DirectConnection);  // 改为DirectConnection，确保立即执行

    if (!connected) {
        qWarning() << "Failed to connect requestSendData signal to sendData slot!";
    } else {
        qDebug() << "Successfully connected requestSendData signal to sendData slot";
    }

    // 使用exec()替代手动事件循环 - 这是正确的做法
    // exec()会启动一个真正的事件循环，能够处理所有信号
    qDebug() << "UDP Worker thread" << m_bindPort << "started, entering event loop";
    this->exec();  // 这会一直运行直到quit()被调用

    // Clean up
    qDebug() << "Closing UDP socket on port" << m_bindPort;
    m_socket->close();
    delete m_socket;
    m_socket = nullptr;
}

void UDPWorker::onReadyRead()
{
    qDebug() << "=== onReadyRead() called on port" << m_bindPort;

    if (!m_socket) {
        qWarning() << "Socket is null!";
        return;
    }

    // Process all pending datagrams
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());

        QHostAddress senderAddress;
        quint16 senderPort;

        qint64 bytesRead = m_socket->readDatagram(datagram.data(), datagram.size(),
                                                   &senderAddress, &senderPort);
        if (bytesRead > 0) {
            qDebug() << "Received" << bytesRead << "bytes from" << senderAddress.toString() << ":" << senderPort;
            emit dataReceived(senderAddress, senderPort, datagram);
        }
    }
}
