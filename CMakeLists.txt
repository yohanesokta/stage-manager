cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "stagemanager")
project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED ENV{QT_PATH} AND NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "$ENV{QT_PATH}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/main.h
    src/WindowManager.h
    src/WindowManager.cpp
    src/StageManagerCore.h
    src/StageManagerCore.cpp
    ui/dock.h
    ui/dock.cpp
    ui/StagePanel.h
    ui/StagePanel.cpp
    ui/ThumbnailWidget.h
    ui/ThumbnailWidget.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Core
    dwmapi
    user32
    shell32
    gdi32
)

if(WIN32)
    if(TARGET Qt6::windeployqt)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND Qt6::windeployqt "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Running windeployqt to copy required Qt DLLs..."
        )
    else()
        include(GNUInstallDirs)
        if(COMMAND qt_generate_deploy_app_script)
            qt_generate_deploy_app_script(
                TARGET ${PROJECT_NAME}
                OUTPUT_SCRIPT deploy_script
            )
            install(SCRIPT ${deploy_script})
        endif()
    endif()
endif()