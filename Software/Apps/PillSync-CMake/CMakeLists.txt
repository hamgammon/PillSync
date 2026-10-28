# Watch App CMakeLists.txt for PillSync (LVGL GUI + Background Service)
cmake_minimum_required(VERSION 3.21)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

# App configuration
set(APP_NAME "PillSync")
set(APP_USER_NAME "PillSync")
set(APP_TYPE "Utility")
set(APP_AUTOSTART On)
set(DEV_ID "UNA")
set(APP_ID "4EB8121CEAB34CD5")
set(RESOURCES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../Resources")
set(OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../Output")
set(LIBS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../Libs")
set(GUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../LVGL-GUI")

# Includes all build steps and required tools (sets toolchain before project)
include($ENV{UNA_SDK}/cmake/una-app.cmake)

# Includes SDK source and include lists
include($ENV{UNA_SDK}/cmake/una-sdk.cmake)

# Includes LIBS_SOURCES, LIBS_INCLUDE_DIRS
include(${LIBS_PATH}/libs.cmake)

# Includes GUI_APP_SOURCES, GUI_APP_INCLUDE_DIRS
include(${GUI_PATH}/lvgl-gui.cmake)

project(${APP_NAME})

# Set up version
set(BUILD_VERSION "1.2.0")
una_app_setup_version(BUILD_VERSION ${CMAKE_CURRENT_SOURCE_DIR})

# Build service
set(SERVICE_SOURCES
    ${LIBS_SOURCES}
    ${UNA_SDK_SOURCES_COMMON}
    ${UNA_SDK_SOURCES_APPSYSTEM}
    ${UNA_SDK_SOURCES_JSON}
    ${UNA_SDK_SOURCES_APPCONFIG}
)
set(SERVICE_INCLUDE_DIRS
    ${LIBS_INCLUDE_DIRS}
    ${UNA_SDK_INCLUDE_DIRS_COMMON}
    ${UNA_SDK_INCLUDE_DIRS_SERVICE}
)
una_app_build_service(${APP_NAME}Service.elf)

# Build GUI (LVGL)
set(GUI_SOURCES
    ${GUI_APP_SOURCES}
    ${UNA_SDK_SOURCES_COMMON}
    ${UNA_SDK_SOURCES_GUI_LVGL}
)
set(GUI_INCLUDE_DIRS
    ${LIBS_INCLUDE_DIRS}
    ${UNA_SDK_INCLUDE_DIRS_COMMON}
    ${UNA_SDK_INCLUDE_DIRS_GUI_LVGL}
    ${GUI_APP_INCLUDE_DIRS}
)
set(GUI_COMPILE_DEFINITIONS
    ${UNA_SDK_DEFINES_GUI_LVGL}
)
set(GUI_STACK_SIZE "24*1024")
set(GUI_RAM_LENGTH "900K")
una_app_build_gui(${APP_NAME}GUI.elf)

# Build the complete .uapp package
una_app_build_app()

