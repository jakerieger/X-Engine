@echo off
setlocal EnableDelayedExpansion

set "ORIGINAL_DIR=%CD%"

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