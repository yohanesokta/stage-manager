@echo off

rem Define build directory
set "BUILD_DIR=build"

rem Create build directory if it does not exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

rem Change to build directory
pushd "%BUILD_DIR%"

rem Generate build system (Debug configuration)
cmake .. -DCMAKE_BUILD_TYPE=Debug

rem Build the project
cmake --build . --config Debug

popd

rem Run the built executable if it exists
if exist "%BUILD_DIR%\\Debug\\stagemanager.exe" (
    "%BUILD_DIR%\\Debug\\stagemanager.exe"
) else if exist "%BUILD_DIR%\\stagemanager.exe" (
    "%BUILD_DIR%\\stagemanager.exe"
) else (
    echo Executable not found. Build may have failed.
)
