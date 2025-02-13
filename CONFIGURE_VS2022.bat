@echo off
setlocal EnableDelayedExpansion

set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community"

if not exist "%VS_PATH%" (
    echo Visual Studio 2022 not found at %VS_PATH%
    echo Please adjust the VS_PATH variable to match your installation
    pause
    exit /b 1
)

REM Set up the Visual Studio environment with explicit x64 architecture
echo Setting up Visual Studio x64 environment...
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=x64
if errorlevel 1 (
    echo Failed to set up Visual Studio environment
    pause
    exit /b 1
)

REM Verify we're actually in x64 mode by checking environment variables
if not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo Failed to properly set x64 architecture
    pause
    exit /b 1
)

call Scripts/configure_assimp.bat
if errorlevel 1 (
    echo Assimp configuration failed
    pause
    exit /b 1
)

call Scripts/configure_sol2.bat
if errorlevel 1 (
    echo Sol2 configuration failed
    pause
    exit /b 1
)

echo ----------------------------------------------------------------------------------------------------
echo  Finished configuring third-party dependencies. You can now open the SpaceGame solution in VS 2022.
echo ----------------------------------------------------------------------------------------------------
pause
exit /b 0