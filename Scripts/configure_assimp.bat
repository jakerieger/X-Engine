@echo off
setlocal EnableDelayedExpansion

set "ORIGINAL_DIR=%CD%"
set "ASSIMP_PATH=%CD%\Vendor\assimp"

REM Verify that the Assimp directory exists
if not exist "%ASSIMP_PATH%" (
    echo Error: Assimp directory not found at %ASSIMP_PATH%
    echo Please ensure you're running this script from the project root
    exit /b 1
)

REM Verify that CMake is available in the system path
where cmake >nul 2>nul
if errorlevel 1 (
    echo Error: CMake not found in system PATH
    echo Please install CMake or ensure it's properly added to your PATH
    exit /b 1
)

echo Configuring Assimp with CMake...
echo Build directory: %CD%
echo Assimp source: %ASSIMP_PATH%

REM Execute CMake configuration with all required arguments
cmake ^
    "%ASSIMP_PATH%" ^
    -A x64 ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DASSIMP_BUILD_TESTS=OFF

if errorlevel 1 (
    echo Error: CMake configuration failed
    cd /d "%ORIGINAL_DIR%"
    exit /b 1
)

echo.
echo *********************************************
echo  Assimp configuration completed successfully
echo *********************************************
echo.