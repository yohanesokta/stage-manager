cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "stagemanager")
project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Build type and output directory
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Include source and UI directories
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(Qt6_DIR "D:/QT/6.11.2/msvc2022_64/lib/cmake/Qt6")

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
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Core
    dwmapi
    user32
    shell32
)