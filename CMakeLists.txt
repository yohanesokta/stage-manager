cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "stagemanager")
project(${PROJECT_NAME} VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER cl)
set(CMAKE_C_COMPILER cl)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/main.h
    ui/dock.cpp
)

target_link_libraries(${PROJECT_NAME}  PRIVATE Qt6::Widgets)