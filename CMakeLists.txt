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
set(CMAKE_PROJECT_NAME teste)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
if(NOT DEFINED ROOM_ID)
    message(FATAL_ERROR
        "ROOM_ID não definido.\n"
        "Use: cmake -DROOM_ID=0 (broadcast/coordenador), -DROOM_ID=1 ou -DROOM_ID=2 ..")
endif()
if(NOT ROOM_ID EQUAL 0 AND NOT ROOM_ID EQUAL 1 AND NOT ROOM_ID EQUAL 2)
    message(FATAL_ERROR "ROOM_ID deve ser 0, 1 ou 2, recebido: ${ROOM_ID}")
endif()

message("Room ID: " ${ROOM_ID})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/Modules/dht11.c
    Core/Src/Modules/press.c
    Core/Src/Modules/Communication/cc1101.c
    Core/Src/Modules/Communication/com.c
    Core/Src/Modules/Communication/events.c
    Core/Src/Modules/Communication/dw_stm32_delay.c
    Core/Src/Modules/Communication/uart_protocol.c
    Core/Src/Modules/RTC/rtc_api.c
    Core/Src/Modules/RTC/rtc_sync.c
    Core/Src/Modules/flash_db.c
    Core/Src/Modules/NFC/pn532.c
    Core/Src/Modules/NFC/pn532_stm32f4.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Inc
    Core/Inc/Modules
    Core/Inc/Modules/Communication
    Core/Inc/Modules/RTC
    Core/Inc/Modules/NFC
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    ROOM_ID=${ROOM_ID}
    $<$<BOOL:${COM_DIAG_DISABLE_ADDR_CHK}>:COM_DIAG_DISABLE_ADDR_CHK>
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
