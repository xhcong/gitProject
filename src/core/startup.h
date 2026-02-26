#ifndef STARTUP_H
#define STARTUP_H

/**
 * @brief Startup and initialization routines
 */
class Startup
{
public:
    /**
     * @brief Main initialization sequence
     *
     * - Load configuration from INI files
     * - Initialize database connection
     * - Load hardware configuration
     * - Initialize all global data structures
     */
    static bool dataInit();

    /**
     * @brief Cleanup routine before exit
     */
    static void cleanup();

private:
    Startup() = delete;
};

#endif // STARTUP_H
