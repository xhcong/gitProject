#include "can_interface.h"

CANInterface& CANInterface::instance()
{
    static CANInterface s_instance;
    return s_instance;
}

CANInterface::CANInterface()
{
}

CANInterface::~CANInterface()
{
    cleanup();
}

bool CANInterface::initialize()
{
    // TODO: Initialize CAN device
    m_initialized = true;
    return true;
}

void CANInterface::cleanup()
{
    stopReceiving();
    m_initialized = false;
}

void CANInterface::startReceiving()
{
    // TODO: Start receiving loop
}

void CANInterface::stopReceiving()
{
    // TODO: Stop receiving loop
}
