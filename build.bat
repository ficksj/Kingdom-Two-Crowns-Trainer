@echo off
echo ===================================================
echo Compiling Kingdom Two Crowns Cheat DLL
echo ===================================================

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

cl /LD /O2 /EHsc /Fe:CheatKTC.dll CheatKTC_dll.cpp user32.lib

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] CheatKTC.dll compiled successfully!
    copy /Y CheatKTC.dll version.dll >nul
) else (
    echo [ERROR] Compilation failed!
)
