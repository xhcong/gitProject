#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <QString>
#include <QTimer>

/**
 * @brief CAN hardware interface
 *
 * Manages CAN communication using USBCAN2 driver
 */
class CANInterface
{
public:
    static CANInterface& instance();

    /**
     * @brief Initialize CAN device
     */
    bool initialize();

    /**
     * @brief Cleanup CAN device
     */
    void cleanup();

    /**
     * @brief Start receiving data
     */
    void startReceiving();

    /**
     * @brief Stop receiving data
     */
    void stopReceiving();

private:
    CANInterface();
    ~CANInterface();

    CANInterface(const CANInterface&) = delete;
    CANInterface& operator=(const CANInterface&) = delete;

    QTimer m_receiveTimer;
    bool m_initialized = false;
};

#endif // CAN_INTERFACE_H
