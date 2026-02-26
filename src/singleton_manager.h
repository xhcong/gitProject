#ifndef SINGLETON_MANAGER_H
#define SINGLETON_MANAGER_H

#include <QString>
#include <QSystemSemaphore>

/**
 * @brief Manages single instance of the application
 *
 * Ensures only one instance of NENET can run at a time
 * on the system using a system-wide semaphore/mutex.
 */
class SingletonManager
{
public:
    static SingletonManager& instance();

    /**
     * @brief Try to acquire single instance lock
     * @return true if lock acquired (first instance), false otherwise
     */
    bool tryLock();

    /**
     * @brief Release the instance lock
     */
    void unlock();

    /**
     * @brief Check if already locked
     * @return true if this process has the lock
     */
    bool isLocked() const;

private:
    SingletonManager();
    ~SingletonManager();

    // Deleted copy/move constructors
    SingletonManager(const SingletonManager&) = delete;
    SingletonManager& operator=(const SingletonManager&) = delete;

    QSystemSemaphore m_semaphore;
    bool m_locked = false;
};

#endif // SINGLETON_MANAGER_H
