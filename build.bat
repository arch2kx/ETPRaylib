@echo off
setlocal enabledelayedexpansion

rem Build ETP (Eden Treaty Pandemonium) on Windows.
rem
rem Usage:
rem   build.bat            Configure (if needed) and build
rem   build.bat clean       Wipe build\ and do a full rebuild
rem                         (needed after CMakeLists.txt changes, e.g.
rem                         the app icon or the OUTPUT_NAME)
rem   build.bat run         Build, then launch the result
rem   build.bat clean run
rem
rem Arguments can be combined in any order.

set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build

set CLEAN=0
set RUNIT=0

:parse
if "%~1"=="" goto afterparse
if /I "%~1"=="clean" set CLEAN=1
if /I "%~1"=="run" set RUNIT=1
shift
goto parse
:afterparse

if %CLEAN%==1 if exist "%BUILD_DIR%" (
    echo Removing existing build directory...
    rmdir /s /q "%BUILD_DIR%"
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

echo Configuring...
cmake .. || exit /b 1

echo Building...
cmake --build . --config Release || exit /b 1

set RESULT=
if exist "Release\ETP.exe" set RESULT=%BUILD_DIR%\Release\ETP.exe
if exist "ETP.exe" set RESULT=%BUILD_DIR%\ETP.exe

if "%RESULT%"=="" (
    echo Build finished, but could not locate ETP.exe.
    exit /b 1
)

echo Build complete: %RESULT%

if %RUNIT%==1 (
    start "" "%RESULT%"
)

endlocal
