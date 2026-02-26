#include "nec_interface.h"

NECInterface& NECInterface::instance()
{
    static NECInterface s_instance;
    return s_instance;
}

NECInterface::NECInterface(QObject* parent) : QObject(parent)
{
}

NECInterface::~NECInterface()
{
    cleanup();
}

bool NECInterface::initialize()
{
    return true;
}

void NECInterface::cleanup()
{
    m_registeredClients.clear();
}

bool NECInterface::sendMessageToNEC(const QString& message)
{
    // TODO: Implement message sending to NEC
    return true;
}

bool NECInterface::registerClient(const QString& clientId, int port)
{
    m_registeredClients[clientId] = port;
    return true;
}

bool NECInterface::unregisterClient(const QString& clientId)
{
    m_registeredClients.remove(clientId);
    return true;
}
