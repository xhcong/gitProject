#include "jf_plate.h"

JFPlate::JFPlate(QObject* parent)
    : QObject(parent)
{
}

JFPlate::~JFPlate()
{
    cleanup();
}

bool JFPlate::initialize()
{
    m_initialized = true;
    return true;
}

void JFPlate::cleanup()
{
    m_initialized = false;
}
