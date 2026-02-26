#ifndef NEC_INTERFACE_H
#define NEC_INTERFACE_H

#include <QString>
#include <QObject>
#include <QMap>

class NECInterface : public QObject
{
    Q_OBJECT

public:
    static NECInterface& instance();

    bool initialize();
    void cleanup();

    bool sendMessageToNEC(const QString& message);
    bool registerClient(const QString& clientId, int port);
    bool unregisterClient(const QString& clientId);

private:
    NECInterface(QObject* parent = nullptr);
    ~NECInterface() override;

    NECInterface(const NECInterface&) = delete;
    NECInterface& operator=(const NECInterface&) = delete;

    QMap<QString, int> m_registeredClients;
};

#endif // NEC_INTERFACE_H
