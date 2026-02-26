#include "udp_interface.h"
#include "udp_worker.h"
#include "logging/logger.h"

UDPInterface& UDPInterface::instance()
{
    static UDPInterface s_instance;
    return s_instance;
}

UDPInterface::UDPInterface(QObject* parent) : QObject(parent)
{
}

UDPInterface::~UDPInterface()
{
    cleanup();
}

bool UDPInterface::initialize()
{
    Logger::instance().info("UDP Interface initialized");
    return true;
}

void UDPInterface::cleanup()
{
    qDebug() << "=== UDPInterface cleanup() called ===";
    qDebug() << "Number of workers to clean:" << m_workers.size();

    for (auto port : m_workers.keys()) {
        UDPWorker* worker = m_workers[port];
        if (worker) {
            qDebug() << "Stopping worker on port" << port;
            worker->stop();
            worker->wait();  // Wait for thread to finish
            delete worker;
            qDebug() << "Worker on port" << port << "deleted";
        }
    }
    m_workers.clear();
    qDebug() << "All workers cleaned up";
}

bool UDPInterface::bindToPort(const QString& ip, quint16 port)
{
    qDebug() << "=== bindToPort() called ===";
    qDebug() << "IP:" << ip << "Port:" << port;

    if (m_workers.contains(port)) {
        Logger::instance().warning(QString("UDP port %1 already bound").arg(port));
        return false;
    }

    // Create a new UDP worker
    UDPWorker* worker = new UDPWorker(ip, port, this);
    qDebug() << "Created UDPWorker, pointer:" << worker;

    // Connect signals from worker to this interface
    // Use lambda to capture worker pointer for reliable sender identification
    bool connected1 = connect(worker, &UDPWorker::dataReceived, this,
            [this, worker](const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data) {
                this->onWorkerDataReceivedWithPort(worker, senderAddress, senderPort, data);
            }, Qt::DirectConnection);

    bool connected2 = connect(worker, &UDPWorker::errorOccurred,
            this, &UDPInterface::onWorkerError,
            Qt::DirectConnection);

    qDebug() << "Signal connections - dataReceived:" << (connected1 ? "SUCCESS" : "FAILED")
             << "errorOccurred:" << (connected2 ? "SUCCESS" : "FAILED");

    if (!connected1 || !connected2) {
        qWarning() << "CRITICAL: Signal slot connection failed!";
    }

    // Start the worker thread
    worker->start();
    qDebug() << "Started UDPWorker thread";

    // Store worker in map
    m_workers[port] = worker;
    qDebug() << "Stored worker in map. Total workers:" << m_workers.size();

    Logger::instance().info(QString("UDP socket bound to %1:%2").arg(ip).arg(port));
    return true;
}

void UDPInterface::unbindFromPort(quint16 port)
{
    if (!m_workers.contains(port)) {
        return;
    }

    UDPWorker* worker = m_workers[port];
    worker->stop();
    worker->wait();
    delete worker;

    m_workers.remove(port);

    Logger::instance().info(QString("UDP socket unbound from port %1").arg(port));
}

void UDPInterface::sendBytes(const QHostAddress& address, quint16 port, const QByteArray& data)
{
    qDebug() << "=== sendBytes() called ===";
    qDebug() << "Target:" << address.toString() << ":" << port << "Data size:" << data.size();
    qDebug() << "Number of workers:" << m_workers.size();

    // Try to send from all available workers
    for (auto worker : m_workers) {
        if (worker) {
            qDebug() << "Found worker, emitting requestSendData signal...";
            // Emit signal to request data sending (thread-safe)
            emit worker->requestSendData(address, port, data);
            Logger::instance().debug(QString("Emitted signal to send data to %1:%2 (%3 bytes)")
                                    .arg(address.toString()).arg(port).arg(data.size()));
            return;
        }
    }

    Logger::instance().warning(QString("No UDP worker available to send data to %1:%2")
                               .arg(address.toString()).arg(port));
}

void UDPInterface::sendBytesByPort(quint16 sourcePort, const QHostAddress& address, quint16 port, const QByteArray& data)
{
    qDebug() << "=== sendBytesByPort() called ===";
    qDebug() << "Source port:" << sourcePort << "Target:" << address.toString() << ":" << port;

    if (!m_workers.contains(sourcePort)) {
        Logger::instance().warning(QString("No UDP worker bound to port %1").arg(sourcePort));
        qDebug() << "Available ports:" << m_workers.keys();
        return;
    }

    UDPWorker* worker = m_workers[sourcePort];
    qDebug() << "Worker pointer:" << worker;

    if (worker) {
        qDebug() << "Worker valid, emitting requestSendData signal...";
        // Emit signal to request data sending (thread-safe)
        // 直接发送，不检查isBound()
        emit worker->requestSendData(address, port, data);
        Logger::instance().debug(QString("Emitted signal on port %1 to %2:%3 (%4 bytes)")
                                .arg(sourcePort).arg(address.toString()).arg(port).arg(data.size()));
    } else {
        Logger::instance().warning(QString("UDP worker on port %1 is null").arg(sourcePort));
    }
}

void UDPInterface::onWorkerDataReceivedWithPort(UDPWorker* worker, const QHostAddress& senderAddress, quint16 senderPort, const QByteArray& data)
{
    qDebug() << "=== onWorkerDataReceivedWithPort() called ===";
    qDebug() << "From:" << senderAddress.toString() << ":" << senderPort;
    qDebug() << "Data size:" << data.size();

    // Emit the general signal
    emit dataReceived(senderAddress, senderPort, data);
    qDebug() << "Emitted general dataReceived signal";

    // Get the local port from the worker directly
    if (worker) {
        quint16 localPort = worker->getBoundPort();
        qDebug() << "Received on local port:" << localPort;
        emit dataReceivedOnPort(localPort, senderAddress, senderPort, data);
        qDebug() << "Emitted port-specific signal";
    } else {
        qWarning() << "WARNING: worker is null!";
    }
}

void UDPInterface::onWorkerError(const QString& errorString)
{
    Logger::instance().error(QString("UDP Error: %1").arg(errorString));
    emit errorOccurred(errorString);
}
