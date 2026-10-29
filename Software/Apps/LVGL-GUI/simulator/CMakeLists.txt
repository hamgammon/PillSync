cmake_minimum_required(VERSION 3.21)

if("$ENV{UNA_SDK}" STREQUAL "")
    message(FATAL_ERROR "UNA_SDK environment variable is not set. Point it at the SDK root.")
endif()

project(PillSyncLVGLSimulator C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include($ENV{UNA_SDK}/cmake/una-simulator.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../Libs/libs.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../lvgl-gui.cmake)

set(SIM_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SIM_BIN_DIR}")
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${cfg} CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} "${SIM_BIN_DIR}")
endforeach()

add_executable(PillSyncLVGLSimulator
    main.cpp
    ${GUI_APP_SOURCES}
    ${LIBS_SOURCES}
    ${UNA_SDK_SOURCES_SIMULATOR}
    ${UNA_SDK_SOURCES_SIMULATOR_SERVICE}
    ${UNA_SDK_SOURCES_SIMULATOR_LVGL}
)

target_include_directories(PillSyncLVGLSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GUI_APP_INCLUDE_DIRS}
    ${LIBS_INCLUDE_DIRS}
    ${UNA_SDK_INCLUDE_DIRS_SIMULATOR}
    ${UNA_SDK_INCLUDE_DIRS_GUI_LVGL}
)

target_compile_definitions(PillSyncLVGLSimulator PRIVATE
    ${UNA_SDK_DEFINES_SIMULATOR}
    ${UNA_SDK_DEFINES_GUI_LVGL}
    APP_ID="4EB8121CEAB34CD5"
    DEV_ID="UNA Simulator"
    BUILD_VERSION="1.3.0-sim"
)

find_package(Threads REQUIRED)
target_link_libraries(PillSyncLVGLSimulator PRIVATE Threads::Threads)
una_simulator_link_sdl2(PillSyncLVGLSimulator)

if(MSVC)
    target_compile_options(PillSyncLVGLSimulator PRIVATE /W3 /MP)
else()
    target_compile_options(PillSyncLVGLSimulator PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

