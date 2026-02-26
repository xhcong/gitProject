#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#include <QString>

/**
 * @brief Hardware control data structure
 */
struct HardwareControl
{
    int device_id = 0;
    QString device_type;
    QString device_name;
    QString device_ip;
    int device_port = 0;
    bool is_connected = false;
};

#endif // HARDWARE_CONTROL_H
