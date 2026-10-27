cmake_minimum_required(VERSION 3.21)

project(PillSyncHostTests LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UNA_SDK_ROOT "$ENV{UNA_SDK}")
if(UNA_SDK_ROOT STREQUAL "")
    message(FATAL_ERROR "UNA_SDK environment variable is not set.")
endif()

include(${UNA_SDK_ROOT}/cmake/una-sdk.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../Software/Apps/LVGL-GUI/lvgl-gui.cmake)

enable_testing()

add_executable(PillManager_test
    PillManager_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Software/Libs/Sources/AppConfigFields.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Software/Libs/Sources/PillManager.cpp
    ${UNA_SDK_ROOT}/Tests/Host/Support/KernelTestDoubles.cpp
    ${UNA_SDK_ROOT}/Libs/Source/AppConfig/AppConfig.cpp
    ${UNA_SDK_ROOT}/Libs/Source/JSON/JsonStreamReader.cpp
    ${UNA_SDK_ROOT}/Libs/Source/JSON/JsonStreamWriter.cpp
    ${UNA_SDK_ROOT}/Libs/Source/UnaLogger/Logger.cpp
    ${UNA_SDK_ROOT}/ThirdParty/coreJSON/source/core_json.c
)

target_include_directories(PillManager_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Software/Libs/Header
    ${UNA_SDK_ROOT}/Tests/Host/Support
    ${UNA_SDK_ROOT}/Libs/Header
    ${UNA_SDK_ROOT}/ThirdParty/coreJSON/source/include
)

add_executable(render_screens
    render_screens.cpp
    ${GUI_APP_SOURCES}
    ${UNA_SDK_ROOT}/Tests/Host/Support/KernelTestDoubles.cpp
    ${UNA_SDK_ROOT}/Libs/Source/UnaLogger/Logger.cpp
    ${UNA_SDK_ROOT}/Libs/Source/Port/GuiCommandProcessor.cpp
    ${UNA_SDK_ROOT}/Libs/Source/Port/LVGL/LvglPort.cpp
    ${UNA_SDK_SOURCES_GUI_LVGL_WIDGETS}
    ${UNA_SDK_LVGL_SOURCES}
)

target_include_directories(render_screens PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Software/Libs/Header
    ${GUI_APP_INCLUDE_DIRS}
    ${UNA_SDK_ROOT}/Tests/Host/Support
    ${UNA_SDK_ROOT}/Libs/Header
    ${UNA_SDK_INCLUDE_DIRS_GUI_LVGL}
)

target_compile_definitions(render_screens PRIVATE
    ${UNA_SDK_DEFINES_GUI_LVGL}
)

set(GTEST_SOURCE_DIR "${UNA_SDK_ROOT}/ThirdParty/googletest")
if(EXISTS "${GTEST_SOURCE_DIR}/CMakeLists.txt")
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    add_subdirectory("${GTEST_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/googletest" EXCLUDE_FROM_ALL)
    target_link_libraries(PillManager_test PRIVATE GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(PillManager_test)
else()
    target_include_directories(PillManager_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gtest_lite)
    target_compile_definitions(PillManager_test PRIVATE GTEST_LITE_MAIN=1)
    add_test(NAME PillManager_test COMMAND PillManager_test)
endif()

