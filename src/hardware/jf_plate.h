#pragma once

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTcpSocket>
#include <QHostAddress>

struct JFHardControl;

class JFPlate : public QObject
{
    Q_OBJECT

public:
    enum class JFPlateFlag : quint8 {
        getRandomCode      = 0x00,
        setVerifyPassword  = 0x01,
        getOutRegister     = 0x02,
        getVerifyReply     = 0x03,
        setGetDI           = 0x10,
        setGetDO           = 0x11,
        setOutRegister     = 0x12,
        getMoNiDI          = 0x13,
        setDO              = 0x21,
        setCom             = 0x28,
        getEventUp         = 0x30,
        getMoNiBH          = 0x31,
        getCom             = 0x38,
        getDI              = 0x80,
        getDO              = 0x81,
        getMoNiDIHF        = 0x83,
        getSetDO           = 0x91,
        getSetOutRegister  = 0x92,
        getSetCom          = 0x98
    };

    explicit JFPlate(const JFHardControl& control, QObject* parent = nullptr);
    explicit JFPlate(QObject* parent = nullptr);
    ~JFPlate() override;

    bool initialize();
    void cleanup();

    bool setEachDO(bool isSend, int high, int low);
    bool setSlaveEachDO(bool isSend, int high, int low);

    static QByteArray createSendMsg(JFPlateFlag cmd, int msgSerial, const QByteArray& data);
    static QByteArray createSlaveSendMsg(JFPlateFlag cmd, const QByteArray& data);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void handlePlatePacket(const QByteArray& msg);
    void handleCommand(JFPlateFlag cmd, const QByteArray& data);
    void plateLogin(const QByteArray& randomCode);
    bool sendPacketToHardware(const QByteArray& packet);

private:
    QTcpSocket* m_socket = nullptr;
    QHostAddress m_ip;
    quint16 m_port = 0;
    QString m_password = "1234567890abcdef";

    bool m_initialized = false;
    bool m_canSend = true;
    bool m_scanSend = true;
    int m_msgSerial = 1125;

    QList<quint8> m_waitSendList;
    QList<quint8> m_waitSendList1;
    QByteArray m_recvBuffer;
};
