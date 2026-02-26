#include "qj_custom.h"

QJCustom& QJCustom::instance()
{
    static QJCustom s_instance;
    return s_instance;
}

QJCustom::QJCustom()
{
}

QJCustom::~QJCustom()
{
    cleanup();
}

bool QJCustom::initialize()
{
    m_initialized = true;
    return true;
}

void QJCustom::cleanup()
{
    m_initialized = false;
}
