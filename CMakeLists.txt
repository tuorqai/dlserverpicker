cmake_minimum_required(VERSION 3.14)
project(DeadlockServerPicker VERSION 2026.07)

#-------------------------------------------------------------------------------

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always --match "v*"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(GIT_DESCRIBE)
    if(GIT_DESCRIBE MATCHES "^v?${PROJECT_VERSION}$")
        set(VERBOSE_VERSION "${PROJECT_VERSION}")
    else()
        string(REGEX REPLACE "^v${PROJECT_VERSION}-" "" GIT_SUFFIX "${GIT_DESCRIBE}")
        set(VERBOSE_VERSION "${PROJECT_VERSION}-git-${GIT_SUFFIX}")
    endif()
else()
    set(VERBOSE_VERSION "${PROJECT_VERSION}")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Version.h"
    @ONLY)

#-------------------------------------------------------------------------------

add_executable(${PROJECT_NAME} WIN32
    "DeadlockServerPicker.cpp"
    "Layout.gen.cpp"
    "MainFrame.cpp"
    "ServerData.cpp"
    "ServerDataModel.cpp")

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

if(WIN32)
    # Windows-specific sources
    target_sources(${PROJECT_NAME} PRIVATE
        "DeadlockServerPicker.rc"
        "WindowsFirewallManager.cpp"
        "WindowsPingTest.cpp")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        # Declare the app as the GUI app
        WIN32_EXECUTABLE ON

        # Modifying Firewall settings requires Administrator privileges
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    # Linux-specific sources
    target_sources(${PROJECT_NAME} PRIVATE
        "LinuxFirewallManager.cpp"
        "LinuxPingTest.cpp")
endif()

#-------------------------------------------------------------------------------

option(USE_SYSTEM_WXWIDGETS "Use system wxWidgets" OFF)

#-------------------------------------------------------------------------------

include(FetchContent)

#-------------------------------------------------------------------------------

if(USE_SYSTEM_WXWIDGETS)
    find_package(wxWidgets REQUIRED core base net)
    include(${wxWidgets_USE_FILE})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})
else()
    set(wxBUILD_SHARED OFF)
    set(wxBUILD_SAMPLES OFF)
    set(wxBUILD_INSTALL OFF)

    FetchContent_Declare(wxWidgets URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.7/wxWidgets-3.2.7.tar.bz2)
    FetchContent_MakeAvailable(wxWidgets)

    target_link_libraries(${PROJECT_NAME} PRIVATE wxcore wxnet wxbase)
endif()

#-------------------------------------------------------------------------------

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

#-------------------------------------------------------------------------------

if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBCAP REQUIRED libcap)
    pkg_check_modules(NFTABLES REQUIRED libnftables)
    target_include_directories(${PROJECT_NAME} PRIVATE ${LIBCAP_INCLUDE_DIRS} ${NFTABLES_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBCAP_LIBRARIES} ${NFTABLES_LIBRARIES})
endif()
