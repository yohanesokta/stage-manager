cmake_minimum_required(VERSION 3.10)


set(PROJECT_NAME "stagemanager")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER cl)
set(CMAKE_C_COMPILER cl)

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/main.h
    src/ui/dock.cpp
)
