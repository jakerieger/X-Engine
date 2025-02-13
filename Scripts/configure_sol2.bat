@echo off
setlocal EnableDelayedExpansion

set "ORIGINAL_DIR=%CD%"
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"

if not exist "%VS_PATH%" (
    echo Visual Studio 2022 not found at %VS_PATH%
    echo Please adjust the VS_PATH variable to match your installation
    exit /b 1
)

REM Set up the Visual Studio environment with explicit x64 architecture
echo Setting up Visual Studio x64 environment...
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=x64
if errorlevel 1 (
    echo Failed to set up Visual Studio environment
    exit /b 1
)

REM Verify we're actually in x64 mode by checking environment variables
if not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo Failed to properly set x64 architecture
    exit /b 1
)

cd /d "Vendor\luajit\src"
if errorlevel 1 (
    echo Failed to change to LuaJIT directory
    cd /d "%ORIGINAL_DIR%"
    exit /b 1
)

REM Build LuaJIT
echo Building LuaJIT (64-bit)...
call msvcbuild.bat
if errorlevel 1 (
    echo Failed to build LuaJIT
    cd /d "%ORIGINAL_DIR%"
    exit /b 1
)

cd /d "%ORIGINAL_DIR%"

echo.
echo *************************************
echo  LuaJIT build completed successfully
echo *************************************
echo.