@echo off

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