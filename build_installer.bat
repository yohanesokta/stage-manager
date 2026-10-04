@echo off

echo ==============================================
echo Building Stage Manager (Release Configuration)
echo ==============================================

cmake -B build
if %errorlevel% neq 0 (
    echo [ERROR] CMake configure failed.
    exit /b %errorlevel%
)

cmake --build build --config Release
if %errorlevel% neq 0 (
    echo [ERROR] CMake Release build failed.
    exit /b %errorlevel%
)

echo.
echo ==============================================
echo Compiling Inno Setup Installer
echo ==============================================

set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"

%ISCC% installer.iss
if %errorlevel% neq 0 (
    echo [ERROR] Installer generation failed.
    exit /b %errorlevel%
)

echo.
echo [SUCCESS] Installer successfully created in installer_output\StageManager_Setup.exe
echo ==============================================
