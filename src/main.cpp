#include <QCoreApplication>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include "logging/logger.h"
#include "core/startup.h"
#include "core/global_data.h"

// Constants
const char* SEMAPHORE_NAME = "OnlyOneNENet_B93EAD1B0CFFE537FBC2779";
const char* SHARED_MEMORY_KEY = "OnlyOneNENet_SharedMemory_B93EAD1B0CFFE537FBC2779";

/**
 * @brief Check if another instance is already running
 * @return true if another instance exists, false if this is the only instance
 */
bool isAnotherInstanceRunning()
{
    // Try to create a system-wide semaphore
    // If it already exists, another instance is running
    QSystemSemaphore semaphore(SEMAPHORE_NAME, 1);

    // In Qt, QSystemSemaphore auto-creates if it doesn't exist
    // For Windows, we use native API to check properly
#ifdef Q_OS_WIN
    HANDLE hMutex = CreateMutexA(nullptr, FALSE, SEMAPHORE_NAME);
    if (hMutex == nullptr) {
        return true;  // Failed to create, another instance likely exists
    }

    // Try to acquire the mutex with timeout
    DWORD result = WaitForSingleObject(hMutex, 100);

    if (result == WAIT_OBJECT_0) {
        // Acquired successfully - this is first instance
        // Keep mutex locked
        return false;
    } else if (result == WAIT_TIMEOUT) {
        // Another instance has the lock
        CloseHandle(hMutex);
        return true;
    }

    CloseHandle(hMutex);
    return true;
#else
    // For non-Windows, use QSystemSemaphore
    if (!semaphore.acquire()) {
        return true;
    }
    return false;
#endif
}

/**
 * @brief Detect if running in virtual machine
 * @return true if VM detected, false otherwise
 */
bool detectVirtualMachine()
{
    // Check for common VM processes
    // This is a simplified version - can be expanded as needed
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    const char* vm_processes[] = {
        "vmsrvc", "tcpview", "wireshark", "visual basic", "fiddler", "vmware",
        "vbox", "process explorer", "autoit", "vboxtray", "vmtools", "vmrawdsk",
        "vmusbmouse", "vmvss", "vmnet", "vmx_svga", "vmmemct1", "df5serv",
        "vboxservice", "vmhgfs", "vmtoolsd"
    };

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return false;
    }

    bool vm_detected = false;
    do {
        QString processName = QString::fromLatin1(pe32.szExeFile).toLower();
        for (const auto& vm_proc : vm_processes) {
            if (processName.contains(vm_proc, Qt::CaseInsensitive)) {
                vm_detected = true;
                break;
            }
        }
        if (vm_detected) break;
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return vm_detected;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Initialize logger - console output disabled for thread safety
    Logger::instance().initialize("nenet.log", true);
    Logger::instance().info("NENet Application Starting...");

    // Check for single instance
    if (isAnotherInstanceRunning()) {
        std::cout << "\033[31m";  // Red color
        std::cout << "Program can only be started once. Please press any key to exit.\n";
        std::cout << "\033[0m";   // Reset color
        Logger::instance().error("Another instance is already running. Exiting.");

        // Wait for user input
        std::cin.get();
        return 1;
    }

    Logger::instance().info("Single instance check passed.");

    // VM detection (optional - can be disabled)
    // if (detectVirtualMachine()) {
    //     Logger::instance().warning("Virtual machine detected. Some features may be restricted.");
    // }

    // Global initialization
    try {
        if (!Startup::dataInit()) {
            Logger::instance().error("Data initialization failed.");
            std::cout << "Initialization failed. Please press any key to exit.\n";
            std::cin.get();
            return 1;
        }
    } catch (const std::exception& e) {
        Logger::instance().error(QString("Exception during initialization: %1").arg(e.what()));
        std::cout << "Initialization error. Please press any key to exit.\n";
        std::cin.get();
        return 1;
    }

    Logger::instance().info("NENet Application initialized successfully.");
    Logger::instance().info("Version: V20230918.02");
    Logger::instance().info("Type 'quit' or 'exit' to stop, press Ctrl+C to terminate.");

    // Main event loop (or custom processing loop)
    // For now, we'll keep the application running
    // TODO: Implement main processing loops for:
    // - Hardware communication (CAN, JFPlate)
    // - Network communication (UDP, NEC)
    // - Metadata updates

    // Simple command-line interface for now
    std::string inputLine;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, inputLine);
        QString command = QString::fromStdString(inputLine);

        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "status") {
            GlobalData::instance().logState("CLI Status Check");
        } else if (command == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  quit/exit - Exit application\n";
            std::cout << "  status    - Show application status\n";
            std::cout << "  help      - Show this help message\n";
        }
    }

    // Cleanup
    Logger::instance().info("NENet Application shutting down...");
    Startup::cleanup();
    Logger::instance().info("Goodbye!");
    Logger::instance().cleanup();

    return 0;
}
