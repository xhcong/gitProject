@echo off
REM 清理编译目录
echo Cleaning build directory...
if exist build (
    rmdir /s /q build
)

REM 创建新的编译目录
echo Creating new build directory...
mkdir build
cd build

REM 运行CMake配置
echo Configuring CMake...
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug

REM 编译项目
echo Building project...
cmake --build . --config Debug --parallel 4

REM 检查编译结果
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo Compilation successful!
    echo ====================================
) else (
    echo.
    echo ====================================
    echo Compilation failed! Check errors above.
    echo ====================================
)

pause
