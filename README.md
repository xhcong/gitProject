# NENET Qt5.15 - C# to C++ Conversion

A conversion of the NENET industrial hardware control system from C# + .NET Framework to C++ + Qt5.15.

## Project Structure

```
NENET_Qt5/
├── CMakeLists.txt                 # CMake build configuration
├── .gitignore                     # Git ignore rules
├── README.md                      # This file
├── src/
│   ├── main.cpp                   # Program entry point
│   ├── singleton_manager.h        # Single instance management
│   ├── core/
│   │   ├── global_data.h/cpp      # Global data manager
│   │   ├── startup.h/cpp          # Initialization
│   │   ├── meta_manage.h/cpp      # Metadata management
│   ├── config/
│   │   ├── config_info.h          # Configuration structures
│   │   ├── ini_config.h/cpp       # INI parser
│   ├── database/
│   │   ├── data_structures.h      # Data structure definitions
│   │   ├── db_connection.h/cpp    # Database connection
│   │   ├── db_queries.h/cpp       # SQL queries
│   ├── hardware/
│   │   ├── can_interface.h/cpp    # CAN communication
│   │   ├── jf_plate.h/cpp         # JFPlate acquisition board
│   │   ├── qj_custom.h/cpp        # QJ custom protocol
│   │   ├── hardware_control.h     # Hardware structures
│   ├── network/
│   │   ├── udp_interface.h/cpp    # UDP communication
│   │   ├── nec_interface.h/cpp    # NEC interface
│   │   ├── protocol.h             # Protocol definitions
│   ├── logging/
│   │   ├── logger.h/cpp           # Logging system
│   ├── utils/
│   │   ├── string_utils.h/cpp     # String utilities
│   │   ├── convert_utils.h/cpp    # Type conversion
├── res/
│   └── Config/
│       ├── NEngineConfig.ini      # Configuration file
└── build/                         # Build output directory (created by CMake)
```

## Prerequisites

### Windows
- **CMake** 3.16 or later
- **Qt5.15** (or compatible version)
  - Download from: https://www.qt.io/
  - Install with msvc2019 or later compiler
- **Visual Studio** with C++ development tools or **MinGW-w64**
- **Git** (optional, for version control)

### macOS
- **Xcode Command Line Tools**
- **CMake 3.16+** (via Homebrew: `brew install cmake`)
- **Qt5.15** (via Homebrew: `brew install qt5`)

### Linux
- **CMake 3.16+** (via apt: `sudo apt install cmake`)
- **Qt5 development files** (via apt: `sudo apt install qt5-qmake qtbase5-dev`)
- **GCC or Clang** with C++17 support

## Building the Project

### Windows (MSVC)

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt5_DIR path as needed)
cmake .. -G "Visual Studio 16 2019" -DCMAKE_PREFIX_PATH="C:/Qt/5.15.x/msvc2019_64"

# Build
cmake --build . --config Release

# Run
bin/NENET_Qt5.exe
```

### Windows (MinGW)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/5.15.x/mingw73_64"
cmake --build .
bin/NENET_Qt5.exe
```

### macOS / Linux

```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt5
cmake --build .
./bin/NENET_Qt5
```

## Configuration

The application reads configuration from `NEngineConfig.ini` in the working directory.

### Example Configuration

```ini
[DATABASE]
Type=sqlite
Path=nenet.db

[MYSQL]
Host=localhost
Port=3306
User=root
Password=
Database=nenet

[LOG]
Level=INFO
Path=logs/nenet.log
ConsoleOutput=true

[IP]
NEC_IP=127.0.0.1
NEC_Port=9000
NEM_IP=127.0.0.1
NEM_Port=9001

[HardIO]
CAN_Type=USBCAN
CAN_Channel=0
CAN_Baudrate=500000
```

## Features

- ✅ **Single Instance Protection**: Only one instance can run at a time
- ✅ **Logging System**: Console and file logging
- ✅ **Database Support**: SQLite and MySQL
- ✅ **Hardware Communication**:
  - CAN bus interface (USBCAN2 compatible)
  - JFPlate TCP acquisition board
  - QJ custom protocol support
- ✅ **Network Communication**:
  - UDP interface for multi-port listening
  - NEC interface with client registration
- ✅ **Metadata Management**: Real-time metadata tracking
- ✅ **INI Configuration**: Flexible configuration file support

## Implementation Status

### Phase 1: Foundation Framework ✅ COMPLETE
- [x] CMakeLists.txt configuration
- [x] main.cpp entry point
- [x] Global data structures
- [x] Logger system
- [x] Directory structure setup

### Phase 2: Configuration & Database ✅ COMPLETE
- [x] INI configuration parser
- [x] Database connection management
- [x] SQL query classes
- [x] Startup initialization

### Phase 3: Hardware Communication 🔄 IN PROGRESS
- [ ] CAN interface implementation
- [ ] JFPlate acquisition board
- [ ] QJ custom protocol

### Phase 4: Network & Protocol 🔄 IN PROGRESS
- [ ] UDP interface implementation
- [ ] NEC interface protocol
- [ ] Protocol structures

### Phase 5: Core Processing Loop 🔄 TODO
- [ ] Metadata management
- [ ] Multi-threaded task dispatching
- [ ] Message routing

### Phase 6: Utilities & Testing 🔄 TODO
- [ ] Utility functions
- [ ] Python file generation
- [ ] Comprehensive testing

## Key Differences from C# Version

| Feature | C# Version | Qt5 Version |
|---------|-----------|------------|
| Language | C# | C++ |
| Framework | .NET Framework 4.8 | Qt5.15 |
| Threading | Task/Task.Factory | QThread/std::thread |
| Collections | List<T>, Dictionary<K,V> | QList<T>, QMap<K,V> |
| Networking | System.Net.Sockets | QTcpSocket/QUdpSocket |
| Database | Dapper ORM | Direct SQL (QSqlQuery) |
| JSON | Newtonsoft.Json | QJsonDocument |
| Mutex | System.Threading.Mutex | QSystemSemaphore |
| Timers | System.Timers.Timer | QTimer |

## Porting Notes

### Thread Safety
- All global data access is protected with QMutex
- Use QMutexLocker for RAII-style locking
- Hardware event queues use std::queue + QMutex for thread-safe operations

### Signal/Slot Usage
- Qt signals/slots are used for thread communication where appropriate
- Custom thread classes inherit from QThread or QObject for slot support

### Platform-Specific Code
- Windows API calls wrapped in `#ifdef Q_OS_WIN` blocks
- CAN driver integration requires Windows USBCAN2 SDK

### Building for Different Platforms
- **Windows**: Default, uses Windows API for VM detection and mutex
- **macOS/Linux**: Compatible but may need hardware driver adjustments

## Testing

To verify the build:

```bash
# Test single instance protection
./NENET_Qt5
# Try running another instance - should fail with message

# Test configuration loading
# Create NEngineConfig.ini and run

# Test database connection
# Point to valid SQLite/MySQL database
```

## Troubleshooting

### CMake Configuration Fails
- Ensure Qt5 is properly installed
- Set CMAKE_PREFIX_PATH to Qt installation directory
- On Windows, use correct Visual Studio version

### Build Errors
- Ensure C++17 support is enabled
- Check Qt5 version matches CMakeLists.txt expectations
- Verify all headers are in correct include paths

### Runtime Issues
- Check NEngineConfig.ini exists in working directory
- Verify database file/connection is accessible
- Check log file for detailed error messages

## Performance Comparison

Target: Maintain feature parity with C# version while achieving:
- Lower memory footprint (C++ vs C#/.NET)
- Faster startup time (no JIT compilation)
- Reduced CPU usage during idle periods
- Improved hardware communication responsiveness

## Future Enhancements

- [ ] Cross-platform support (Linux, macOS hardware drivers)
- [ ] REST API for remote monitoring
- [ ] Web dashboard for status monitoring
- [ ] Performance profiling and optimization
- [ ] Unit test suite
- [ ] Docker containerization

## Contributing

When adding new features:
1. Follow Qt/C++ naming conventions
2. Use RAII principles for resource management
3. Protect shared data with appropriate synchronization
4. Add logging for debugging
5. Update documentation

## License

[Specify license information]

## Version

- **Current**: V20230918.02 (matching original C# version)
- **Last Updated**: 2025-02-24

## Support

For issues or questions:
1. Check the troubleshooting section
2. Review log files for error details
3. Consult Qt documentation: https://doc.qt.io/
4. Check CMake documentation: https://cmake.org/documentation/

---

**Note**: This is an in-progress conversion. Full functionality parity with the C# version is being achieved incrementally through the implementation phases.
