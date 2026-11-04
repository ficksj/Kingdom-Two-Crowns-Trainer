@echo off
set "GAME_DIR=C:\Program Files (x86)\Steam\steamapps\common\Kingdom Two Crowns"

echo ===================================================
echo Installing Cheat Mod for Kingdom Two Crowns
echo Destination: %GAME_DIR%
echo ===================================================

if not exist "%GAME_DIR%" (
    echo [ERROR] Game directory not found at:
    echo %GAME_DIR%
    pause
    exit /b 1
)

if not exist "version.dll" (
    echo [ERROR] version.dll not found in current folder! Run build.bat first.
    pause
    exit /b 1
)

copy /Y "version.dll" "%GAME_DIR%\version.dll"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================================
    echo [SUCCESS] Mod installed successfully!
    echo Just launch Kingdom Two Crowns from Steam or EXE.
    echo ===================================================
) else (
    echo.
    echo [ERROR] Failed to copy version.dll. Check permissions or close the game if running.
)
pause

