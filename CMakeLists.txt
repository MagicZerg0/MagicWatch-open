cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME u585_newscreen_test)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add touchgfx generated sources and libraries
add_subdirectory(cmake/touchgfx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Lib/ST7789/Src/ST7789.c
    Lib/CST816/Src/bsp_touch_cst816.c
    Lib/PowerManager/Src/PowerManager.c
    Lib/max30102/Src/max30102.c
    Lib/BackLight/Src/BackLight.c
    Lib/badminton_detect/Src/badminton_detect.c
    Lib/mpu6050/Src/mpu6050.c
    Lib/training_session/Src/training_session.c
    Core/Src/swing_task.c
    Lib/madgwick/Src/madgwick.c
    Lib/dtw_matcher/Src/dtw_matcher.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Lib/ST7789/Inc
    Lib/CST816/Inc
    Lib/PowerManager/Inc
    Lib/max30102/Inc
    Lib/BackLight/Inc
    Lib/badminton_detect/Inc
    Lib/mpu6050/Inc
    Lib/training_session/Inc
    Lib/madgwick/Inc
    Lib/dtw_matcher/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
