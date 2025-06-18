cmake_minimum_required(VERSION 3.14)
project(DeadlockServerPicker)

#-------------------------------------------------------------------------------

add_executable(${PROJECT_NAME} WIN32
    "DeadlockServerPicker.cpp"
    "Layout.gen.cpp"
    "MainFrame.cpp"
    "ServerData.cpp"
    "ServerDataModel.cpp")

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
endif()

#-------------------------------------------------------------------------------

include(FetchContent)

#-------------------------------------------------------------------------------

set(wxBUILD_SHARED OFF)
set(wxBUILD_SAMPLES OFF)
set(wxBUILD_INSTALL OFF)

FetchContent_Declare(wxWidgets URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.7/wxWidgets-3.2.7.tar.bz2)
FetchContent_MakeAvailable(wxWidgets)

target_link_libraries(${PROJECT_NAME} PRIVATE wxcore wxnet wxbase)

#-------------------------------------------------------------------------------

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
