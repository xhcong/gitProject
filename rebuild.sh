#!/bin/bash
# Qt Creator build script for NENET_Qt5

echo "=========================================="
echo "Rebuilding NENET_Qt5 with UDP support"
echo "=========================================="

# Change to project directory
cd "$(dirname "$0")"

# Clean build directory
echo ""
echo "Step 1: Cleaning build directory..."
if [ -d "build" ]; then
    rm -rf build
fi

# Create new build directory
mkdir -p build
cd build

# Configure with CMake
echo ""
echo "Step 2: Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the project
echo ""
echo "Step 3: Building project..."
cmake --build . --config Debug --parallel 4

if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "Compilation successful!"
    echo "=========================================="
else
    echo ""
    echo "=========================================="
    echo "Compilation failed! Check errors above."
    echo "=========================================="
    exit 1
fi
