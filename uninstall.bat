@echo off
set "GAME_DIR=C:\Program Files (x86)\Steam\steamapps\common\Kingdom Two Crowns"

echo ===================================================
echo Uninstalling Cheat Mod for Kingdom Two Crowns
echo ===================================================

if exist "%GAME_DIR%\version.dll" (
    del /F /Q "%GAME_DIR%\version.dll"
    echo [OK] Removed version.dll
)

if exist "%GAME_DIR%\cheat_log.txt" (
    del /F /Q "%GAME_DIR%\cheat_log.txt"
    echo [OK] Removed cheat_log.txt
)

echo.
echo ===================================================
echo [SUCCESS] Mod uninstalled. The game is clean.
echo ===================================================
pause

