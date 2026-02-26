# NENET_Qt5 Quick Reference Guide

## Project Location
```
C:\Users\administer\Desktop\nenet\nem\NENET_Qt5
```

## Quick Start

### 1. View Documentation
- **Project Overview**: `README.md`
- **Build Instructions (Windows)**: `WINDOWS_BUILD_GUIDE.md`
- **Implementation Status**: `IMPLEMENTATION_PROGRESS.md`
- **Sample Configuration**: `res/Config/NEngineConfig.ini`

### 2. Build the Project
```bash
cd C:\Users\administer\Desktop\nenet\nem\NENET_Qt5
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\5.15.13\5.15.13\msvc2019_64"
cmake --build . --config Release
```

### 3. Run the Application
```bash
bin\Release\NENET_Qt5.exe
```

## Directory Structure at a Glance

```
NENET_Qt5/
├── src/
│   ├── main.cpp                    ← Program entry point
│   ├── singleton_manager.h         ← Single instance control
│   ├── core/                       ← Core functionality
│   │   ├── global_data.*           ← Global state manager
│   │   ├── startup.*               ← Initialization
│   │   └── meta_manage.*           ← Main processing loop
│   ├── config/                     ← Configuration handling
│   │   ├── config_info.h           ← Config structures
│   │   └── ini_config.*            ← INI parser
│   ├── database/                   ← Database operations
│   │   ├── data_structures.h       ← Data definitions
│   │   ├── db_connection.*         ← DB connection
│   │   └── db_queries.*            ← SQL operations
│   ├── hardware/                   ← Hardware interfaces (STUB)
│   │   ├── can_interface.*         ← CAN communication
│   │   ├── jf_plate.*              ← JFPlate board
│   │   ├── qj_custom.*             ← QJ protocol
│   │   └── hardware_control.h      ← Data structures
│   ├── network/                    ← Network interfaces (STUB)
│   │   ├── udp_interface.*         ← UDP communication
│   │   ├── nec_interface.*         ← NEC protocol
│   │   └── protocol.h              ← Protocol definitions
│   ├── logging/                    ← Logging system
│   │   └── logger.*                ← Log manager
│   └── utils/                      ← Utility functions
│       ├── string_utils.*          ← String operations
│       └── convert_utils.*         ← Type conversions
├── res/Config/
│   └── NEngineConfig.ini           ← Configuration file
├── CMakeLists.txt                  ← Build configuration
├── README.md                       ← Full documentation
├── WINDOWS_BUILD_GUIDE.md          ← Windows build guide
├── IMPLEMENTATION_PROGRESS.md      ← Status report
└── .gitignore                      ← Git configuration
```

## Key Classes and Their Purposes

### Singleton Managers
| Class | Purpose | Location |
|-------|---------|----------|
| `GlobalData` | Thread-safe global state | `core/global_data.*` |
| `Logger` | Logging system | `logging/logger.*` |
| `DBConnection` | Database connection | `database/db_connection.*` |
| `IniConfig` | Configuration parsing | `config/ini_config.*` |
| `CANInterface` | CAN communication | `hardware/can_interface.*` |
| `NECInterface` | NEC protocol | `network/nec_interface.*` |

### Data Structures
| Structure | Purpose | Location |
|-----------|---------|----------|
| `ne_plate` | Hardware plate config | `database/data_structures.h` |
| `ne_md_info` | Metadata information | `database/data_structures.h` |
| `HardwareEvent` | Event message | `database/data_structures.h` |
| `NECMessage` | NEC protocol message | `database/data_structures.h` |
| `IniConfigInfo` | Configuration container | `config/config_info.h` |

## Implementation Status

### ✅ COMPLETE (Phases 1-2)
- [x] CMake build system
- [x] Program entry point
- [x] Logging system
- [x] Global data manager
- [x] INI configuration parser
- [x] Database connection
- [x] SQL query operations
- [x] Data structure definitions
- [x] Documentation

### 🔄 READY FOR IMPLEMENTATION (Phases 3-5)

**Phase 3: Hardware Communication**
- [ ] Implement `CANInterface::initialize()` - Initialize CAN device
- [ ] Implement `CANInterface::startReceiving()` - Start CAN receive loop
- [ ] Implement `JFPlate::initialize()` - Connect to JFPlate board
- [ ] Implement `JFPlate` protocol encoding/decoding
- [ ] Implement `QJCustom` protocol handlers

**Phase 4: Network Communication**
- [ ] Implement `UDPInterface::bindToPort()` - UDP socket creation
- [ ] Implement `NECInterface` message handling
- [ ] Implement protocol JSON parsing and validation
- [ ] Add inter-thread signal/slot communication

**Phase 5: Core Processing**
- [ ] Implement `MetaManage::processHardwareEvents()` - Event processing
- [ ] Implement `MetaManage::sendToNECClients()` - Message distribution
- [ ] Create multiple QThread workers
- [ ] Add main event loop integration

### ⏳ TODO (Phase 6)
- [ ] Implement Python file generation
- [ ] Add comprehensive unit tests
- [ ] Performance optimization
- [ ] Documentation updates

## Common Tasks

### Adding a New Module

1. Create header file in appropriate `src/` subdirectory
2. Create implementation file (.cpp)
3. Add to `CMakeLists.txt` in PROJECT_SOURCES
4. Update documentation if needed

### Running the Application

```bash
# With default configuration
NENET_Qt5.exe

# Check application status
> status

# View help
> help

# Exit
> quit
```

### Viewing Logs

```bash
# Real-time console output
# Application shows all INFO/WARNING/ERROR messages

# Log file (if configured)
logs/nenet.log
```

### Debugging

```cpp
// Add debug output
Logger::instance().debug("Debug message here");
Logger::instance().info("Info: " + QString::number(value));

// Check global state
GlobalData::instance().logState("Context name");
```

## Important Files to Modify

| File | Purpose | When to Modify |
|------|---------|---|
| `CMakeLists.txt` | Build config | Adding new source files |
| `config_info.h` | Configuration | Adding new config sections |
| `data_structures.h` | Data types | Adding new message types |
| `startup.cpp` | Init sequence | Changing startup order |
| `main.cpp` | Entry point | Changing main program flow |

## Building Variants

### Debug Build
```bash
cmake --build build --config Debug
bin\Debug\NENET_Qt5.exe
```

### Release Build
```bash
cmake --build build --config Release
bin\Release\NENET_Qt5.exe
```

### Clean Build
```bash
rm -r build
mkdir build
cd build
# Reconfigure and build
```

## Configuration Sections

### Example: Adding new configuration section

**In `config_info.h`:**
```cpp
struct IniConfigInfo {
    // ... existing sections ...
    struct {
        QString new_option;
    } new_section;
};
```

**In `ini_config.cpp` (loadConfig function):**
```cpp
settings.beginGroup("NEW_SECTION");
config.new_section.new_option = settings.value("Option", "default").toString();
settings.endGroup();
```

**In `NEngineConfig.ini`:**
```ini
[NEW_SECTION]
Option=value
```

## Useful Qt/C++ Resources

- Qt Documentation: https://doc.qt.io/qt-5/
- QSqlDatabase: https://doc.qt.io/qt-5/qsqldatabase.html
- QUdpSocket: https://doc.qt.io/qt-5/qudpsocket.html
- QThread: https://doc.qt.io/qt-5/qthread.html
- CMake: https://cmake.org/cmake/help/v3.20/

## Troubleshooting Quick Fixes

| Issue | Solution |
|-------|----------|
| CMake not found | Add CMake to PATH or use full path |
| Qt5 not found | Set `-DCMAKE_PREFIX_PATH` to Qt installation |
| Build fails | Run `cmake --build . --config Debug` for detailed errors |
| Only one instance check fails | Delete registry key or use different semaphore name |
| Database connection fails | Check NEngineConfig.ini path and database file |
| Logger not working | Ensure `Logger::instance().initialize()` is called |

## Next Steps

1. **Review Phase 3 Requirements**
   - Study original C# `CAN.cs` for CAN protocol
   - Study original C# `JFPlate.cs` for TCP protocol
   - Study original C# `QJCustom.cs` for custom protocol

2. **Implement Hardware Layer**
   - Replace stub implementations with actual logic
   - Add protocol encoding/decoding
   - Test with actual hardware

3. **Implement Network Layer**
   - Replace UDP/NEC stubs with working code
   - Add JSON message parsing
   - Test network communication

4. **Implement Core Processing**
   - Add event processing threads
   - Implement message routing
   - Add performance monitoring

## Questions?

Refer to:
- `README.md` for comprehensive project documentation
- `WINDOWS_BUILD_GUIDE.md` for build-specific help
- `IMPLEMENTATION_PROGRESS.md` for detailed status
- Source code comments for implementation details

---

**Last Updated**: 2025-02-24
**Version**: 1.0
**Status**: Ready for Phase 3 Implementation
