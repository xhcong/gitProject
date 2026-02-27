cmake_minimum_required(VERSION 3.16)

project(NENET_Qt5 VERSION 20230918.02 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5 configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 modules
find_package(Qt5 COMPONENTS
    Core
    Sql
    Network
    Concurrent
    REQUIRED
)

# Source files
set(PROJECT_SOURCES
    src/main.cpp
    src/logging/logger.cpp
    src/core/global_data.cpp
    src/core/startup.cpp
    src/core/meta_manage.cpp
    src/config/ini_config.cpp
    src/database/db_connection.cpp
    src/database/db_queries.cpp
    src/hardware/can_interface.cpp
    src/hardware/jf_plate.cpp
    src/hardware/qj_custom.cpp
    src/network/udp_interface.cpp
    src/network/udp_worker.cpp
    src/network/nec_interface.cpp
    src/network/protocol.cpp
    src/utils/string_utils.cpp
    src/utils/convert_utils.cpp
)

# Header files (added for IDE/project visibility and explicit moc tracking)
set(PROJECT_HEADERS
    src/singleton_manager.h
    src/logging/logger.h
    src/core/global_data.h
    src/core/startup.h
    src/core/meta_manage.h
    src/config/config_info.h
    src/config/ini_config.h
    src/database/data_structures.h
    src/database/db_connection.h
    src/database/db_queries.h
    src/hardware/hardware_control.h
    src/hardware/can_interface.h
    src/hardware/jf_plate.h
    src/hardware/jf_plate_fwd.h
    src/hardware/qj_custom.h
    src/network/udp_interface.h
    src/network/udp_worker.h
    src/network/nec_interface.h
    src/network/protocol.h
    src/utils/string_utils.h
    src/utils/convert_utils.h
)

# Create executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Link Qt5 libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    Qt5::Concurrent
)

# Windows-specific settings
if(WIN32)
    # Link Windows libraries for system calls
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ws2_32
        Kernel32
        User32
        Shell32
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# Compiler flags
if(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
        /W3
        /utf-8
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME}
        PRIVATE
        -Wall
        -Wextra
    )
endif()

# Output directory (default: build tree bin/)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable verbose linking
set(CMAKE_VERBOSE_MAKEFILE ON)
