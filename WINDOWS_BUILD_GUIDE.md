# Windows Build Guide - NENET Qt5

This guide provides step-by-step instructions for building NENET_Qt5 on Windows.

## Prerequisites

### 1. Install CMake
- Download from: https://cmake.org/download/
- **Recommended**: Version 3.20 or later
- During installation, choose "Add CMake to the system PATH"

### 2. Install Qt5.15
- Download from: https://www.qt.io/
- **Recommended installation options**:
  - Qt 5.15 (or 5.15.13 for LTS support)
  - MSVC 2019 64-bit (or your compiler version)
  - Include Qt SQL module (must have)
  - Include Qt Network module (must have)

**Installation Path Example**: `C:\Qt\5.15.13\5.15.13\msvc2019_64`

### 3. Install Visual Studio Build Tools
- **Option A**: Visual Studio 2019 Community Edition
  - Download from: https://visualstudio.microsoft.com/
  - Select "Desktop development with C++"
  - Ensure MSVC 2019 compiler is installed

- **Option B**: Just Visual Studio Build Tools
  - Download from: https://visualstudio.microsoft.com/downloads/
  - Install "Visual Studio Build Tools 2019"

### 4. Verify Git (Optional)
- Download from: https://git-scm.com/
- Useful for version control

## Building Steps

### Step 1: Open Command Prompt or PowerShell

Navigate to the NENET_Qt5 directory:

```powershell
cd C:\Users\administer\Desktop\nenet\nem\NENET_Qt5
```

### Step 2: Create Build Directory

```powershell
mkdir build
cd build
```

### Step 3: Configure with CMake

**For Visual Studio 2019 (64-bit)**:

```powershell
cmake .. -G "Visual Studio 16 2019" -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\5.15.13\5.15.13\msvc2019_64"
```

**Replace the Qt path if your installation is different!**

### Step 4: Build the Project

**Option A - Debug Build**:

```powershell
cmake --build . --config Debug
```

**Option B - Release Build** (Recommended for production):

```powershell
cmake --build . --config Release
```

### Step 5: Run the Application

```powershell
# For Debug build
.\bin\Debug\NENET_Qt5.exe

# For Release build
.\bin\Release\NENET_Qt5.exe
```

## Troubleshooting

### Problem: CMake command not found
**Solution**:
- Ensure CMake is in your PATH
- Restart command prompt after installing CMake
- Verify: `cmake --version`

### Problem: Qt5 not found
**Solution**:
- Verify Qt5 installation path
- Check CMAKE_PREFIX_PATH is correct
- Try finding Qt5Config.cmake:
  ```powershell
  ls "C:\Qt\5.15.13\5.15.13\msvc2019_64\lib\cmake\Qt5"
  ```

### Problem: Visual Studio compiler not found
**Solution**:
- Install Visual Studio Build Tools 2019
- Or install full Visual Studio Community Edition
- Verify MSVC is in PATH: `cl.exe` command should work

### Problem: Build fails with linking errors
**Solution**:
- Clean and rebuild:
  ```powershell
  cd build
  rm -r CMakeFiles CMakeCache.txt
  # Reconfigure with CMake (Step 3)
  # Rebuild (Step 4)
  ```

### Problem: Missing Qt5 modules
**Solution**:
- Ensure Qt5 installation includes:
  - Qt5::Core
  - Qt5::Sql
  - Qt5::Network
  - Qt5::Concurrent
- May need to reinstall Qt5 with these modules selected

## Running the Application

### First Time Setup

1. Copy configuration file to working directory:
   ```powershell
   copy res\Config\NEngineConfig.ini .
   ```

2. Edit `NEngineConfig.ini` as needed

3. Create logs directory:
   ```powershell
   mkdir logs
   ```

4. Run the application:
   ```powershell
   bin\Release\NENET_Qt5.exe
   ```

### Command Line Interface

The application supports these commands:
- `quit` or `exit` - Stop the application
- `status` - Show current status
- `help` - Show available commands

## Advanced: Using Qt Creator IDE

Instead of command line, you can use Qt Creator for a more visual experience:

### Step 1: Open Project
- Launch Qt Creator
- File → Open File or Project
- Select: `NENET_Qt5/CMakeLists.txt`

### Step 2: Configure Kit
- Select "Desktop (default)" or your MSVC kit
- Configure if needed

### Step 3: Build
- Build → Build Project NENET_Qt5
- Or press Ctrl+B

### Step 4: Run
- Build → Run
- Or press Ctrl+R

## Performance Testing

After building, you can test performance:

```powershell
# Measure startup time
Measure-Command { .\bin\Release\NENET_Qt5.exe }

# Monitor memory usage
# Open Task Manager → Performance tab → Open Resource Monitor
# Run the application and observe memory usage
```

## Creating Release Package

To create a standalone executable:

```powershell
# Copy dependencies
mkdir release_package
copy bin\Release\NENET_Qt5.exe release_package\

# Copy Qt runtime DLLs
copy "C:\Qt\5.15.13\5.15.13\msvc2019_64\bin\Qt5Core.dll" release_package\
copy "C:\Qt\5.15.13\5.15.13\msvc2019_64\bin\Qt5Sql.dll" release_package\
copy "C:\Qt\5.15.13\5.15.13\msvc2019_64\bin\Qt5Network.dll" release_package\
copy "C:\Qt\5.15.13\5.15.13\msvc2019_64\bin\Qt5Concurrent.dll" release_package\

# Copy configuration
copy res\Config\NEngineConfig.ini release_package\

# Copy SQL driver (for SQLite support)
mkdir release_package\sqldrivers
copy "C:\Qt\5.15.13\5.15.13\msvc2019_64\plugins\sqldrivers\qsqlite.dll" release_package\sqldrivers\
```

## Updating Configuration

The `NEngineConfig.ini` file supports:
- Database configuration (SQLite/MySQL)
- Network IP and ports
- Hardware parameters (CAN, JFPlate)
- Logging preferences

Edit this file before running the application to customize behavior.

## Next Steps

1. **Test Database Connection**:
   - Ensure SQLite file exists or MySQL server is running
   - Check logs for connection errors

2. **Configure Hardware**:
   - Set CAN parameters if using CAN interface
   - Configure JFPlate board IPs
   - Setup QJ custom mappings

3. **Monitor Logs**:
   - Check `logs/nenet.log` for any issues
   - Use `status` command to verify operation

4. **Integration Testing**:
   - Test with NEC/NEM/NED systems
   - Verify UDP communication
   - Monitor hardware events

## Support

For additional help:
- Qt Documentation: https://doc.qt.io/qt-5/
- CMake Help: https://cmake.org/cmake/help/v3.20/
- Windows Command Line: Built-in `help` command or PowerShell help

---

**Last Updated**: 2025-02-24
**Target Qt Version**: 5.15.x
**Target Compiler**: MSVC 2019 or later
