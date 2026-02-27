#include "jf_plate.h"
#include "database/data_structures.h"
#include "logging/logger.h"
#include <QCryptographicHash>

JFPlate::JFPlate(const JFHardControl& control, QObject* parent)
    : QObject(parent)
{
    m_waitSendList.append(0x02);
    m_waitSendList1.append(0x02);

    m_ip = QHostAddress(control.ip_addr);
    m_port = static_cast<quint16>(control.ip_port);
    if (!control.login_password.isEmpty()) {
        m_password = control.login_password;
    }
}

JFPlate::JFPlate(QObject* parent)
    : QObject(parent)
{
    m_waitSendList.append(0x02);
    m_waitSendList1.append(0x02);
}

JFPlate::~JFPlate()
{
    cleanup();
}

bool JFPlate::initialize()
{
    if (m_initialized) {
        return true;
    }

    if (!m_socket) {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::connected, this, &JFPlate::onConnected);
        connect(m_socket, &QTcpSocket::disconnected, this, &JFPlate::onDisconnected);
        connect(m_socket, &QTcpSocket::readyRead, this, &JFPlate::onReadyRead);
        connect(m_socket, &QTcpSocket::errorOccurred, this, &JFPlate::onSocketError);
    }

    m_initialized = true;
    m_canSend = true;
    m_scanSend = true;

    if (!m_ip.isNull() && m_port > 0) {
        m_socket->connectToHost(m_ip, m_port);
    }

    return true;
}

void JFPlate::cleanup()
{
    m_initialized = false;

    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

QByteArray JFPlate::createSendMsg(JFPlateFlag cmd, int msgSerial, const QByteArray& data)
{
    QByteArray sendMsg;
    sendMsg.resize(data.size() + 7);
    sendMsg[0] = static_cast<char>(0xEA);
    sendMsg[1] = static_cast<char>(0xAE);
    sendMsg[2] = static_cast<char>(0x01);
    sendMsg[3] = static_cast<char>(cmd);
    sendMsg[4] = static_cast<char>(msgSerial & 0xFF);
    sendMsg[5] = static_cast<char>((msgSerial >> 8) & 0xFF);
    sendMsg[6] = static_cast<char>(data.size() & 0xFF);
    for (int i = 0; i < data.size(); ++i) {
        sendMsg[7 + i] = data[i];
    }
    return sendMsg;
}

QByteArray JFPlate::createSlaveSendMsg(JFPlateFlag cmd, const QByteArray& data)
{
    QByteArray sendMsg;
    sendMsg.resize(data.size() + 7);
    sendMsg[0] = static_cast<char>(0xEA);
    sendMsg[1] = static_cast<char>(0xAE);
    sendMsg[2] = static_cast<char>(0xBF);
    sendMsg[3] = static_cast<char>(0x01);
    sendMsg[4] = static_cast<char>(cmd);
    sendMsg[5] = static_cast<char>(data.size() & 0xFF);

    for (int i = 0; i < data.size(); ++i) {
        sendMsg[6 + i] = data[i];
    }

    int sum = 0;
    for (int i = 0; i < sendMsg.size(); ++i) {
        sum += static_cast<unsigned char>(sendMsg[i]);
    }
    sendMsg[sendMsg.size() - 1] = static_cast<char>(sum & 0xFF);
    return sendMsg;
}

bool JFPlate::setEachDO(bool isSend, int high, int low)
{
    if (!m_initialized) {
        return false;
    }

    if (!isSend) {
        m_waitSendList.append(static_cast<quint8>(high & 0xFF));
        m_waitSendList.append(static_cast<quint8>(low & 0xFF));
        return true;
    }

    QByteArray payload;
    for (quint8 b : m_waitSendList) {
        payload.append(static_cast<char>(b));
    }

    bool ok = true;
    if (payload.size() > 2 && m_canSend) {
        ++m_msgSerial;
        if (m_msgSerial >= 9999) {
            m_msgSerial = 1;
        }
        const QByteArray packet = createSendMsg(JFPlateFlag::setDO, m_msgSerial, payload);
        ok = sendPacketToHardware(packet);
        m_canSend = false;
    }

    m_waitSendList.clear();
    m_waitSendList.append(0x02);
    return ok;
}

bool JFPlate::setSlaveEachDO(bool isSend, int high, int low)
{
    if (!m_initialized) {
        return false;
    }

    if (!isSend) {
        m_waitSendList1.append(static_cast<quint8>(high & 0xFF));
        m_waitSendList1.append(static_cast<quint8>(low & 0xFF));
        return true;
    }

    QByteArray payload;
    for (quint8 b : m_waitSendList1) {
        payload.append(static_cast<char>(b));
    }

    bool ok = true;
    if (payload.size() > 2 && m_scanSend) {
        ++m_msgSerial;
        if (m_msgSerial >= 9999) {
            m_msgSerial = 1;
        }
        const QByteArray packet = createSlaveSendMsg(JFPlateFlag::setCom, payload);
        ok = sendPacketToHardware(packet);
        m_scanSend = false;
    }

    m_waitSendList1.clear();
    m_waitSendList1.append(0x02);
    return ok;
}

void JFPlate::onConnected()
{
    Logger::instance().info(QString("JFPlate connected: %1:%2").arg(m_ip.toString()).arg(m_port));
}

void JFPlate::onDisconnected()
{
    Logger::instance().warning("JFPlate disconnected, waiting reconnect by upper logic");
}

void JFPlate::onReadyRead()
{
    if (!m_socket) {
        return;
    }

    m_recvBuffer.append(m_socket->readAll());

    while (m_recvBuffer.size() >= 7) {
        if (static_cast<unsigned char>(m_recvBuffer[0]) != 0xEA ||
            static_cast<unsigned char>(m_recvBuffer[1]) != 0xAE) {
            m_recvBuffer.remove(0, 1);
            continue;
        }

        const int len = static_cast<unsigned char>(m_recvBuffer[6]);
        const int frameLen = 7 + len;
        if (m_recvBuffer.size() < frameLen) {
            return;
        }

        const QByteArray frame = m_recvBuffer.left(frameLen);
        m_recvBuffer.remove(0, frameLen);
        handlePlatePacket(frame);
    }
}

void JFPlate::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    if (m_socket) {
        Logger::instance().error(QString("JFPlate socket error: %1").arg(m_socket->errorString()));
    }
}

void JFPlate::handlePlatePacket(const QByteArray& msg)
{
    if (msg.size() < 7) {
        return;
    }

    const auto cmd = static_cast<JFPlateFlag>(static_cast<unsigned char>(msg[3]));
    const int len = static_cast<unsigned char>(msg[6]);
    QByteArray data;
    if (len > 0 && msg.size() >= 7 + len) {
        data = msg.mid(7, len);
    }

    handleCommand(cmd, data);
}

void JFPlate::handleCommand(JFPlateFlag cmd, const QByteArray& data)
{
    switch (cmd) {
    case JFPlateFlag::getSetDO:
        m_canSend = true;
        break;
    case JFPlateFlag::getSetCom:
        m_scanSend = true;
        break;
    case JFPlateFlag::getRandomCode:
        plateLogin(data);
        break;
    case JFPlateFlag::getVerifyReply: {
        const QByteArray q1(1, static_cast<char>(0x00));
        sendPacketToHardware(createSendMsg(JFPlateFlag::setGetDI, 1123, q1));
        sendPacketToHardware(createSendMsg(JFPlateFlag::setGetDO, 1124, q1));
        break;
    }
    default:
        break;
    }
}

void JFPlate::plateLogin(const QByteArray& randomCode)
{
    if (randomCode.size() != 16) {
        Logger::instance().warning("JFPlate login failed: random code length invalid");
        return;
    }

    QByteArray pwd = m_password.toUtf8();
    if (pwd.size() < 16) {
        pwd.append(QByteArray(16 - pwd.size(), '\0'));
    }
    if (pwd.size() > 16) {
        pwd = pwd.left(16);
    }

    QByteArray comb;
    comb.reserve(32);
    comb.append(randomCode);
    comb.append(pwd);

    const QByteArray md5 = QCryptographicHash::hash(comb, QCryptographicHash::Md5);
    const QByteArray sendMsg = createSendMsg(JFPlateFlag::setVerifyPassword, 398, md5);
    sendPacketToHardware(sendMsg);
}

bool JFPlate::sendPacketToHardware(const QByteArray& packet)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        Logger::instance().warning("JFPlate send failed: socket not connected");
        return false;
    }

    const qint64 written = m_socket->write(packet);
    if (written != packet.size()) {
        Logger::instance().warning(QString("JFPlate send partial, expect=%1 actual=%2")
                                       .arg(packet.size())
                                       .arg(written));
        return false;
    }

    return true;
}
