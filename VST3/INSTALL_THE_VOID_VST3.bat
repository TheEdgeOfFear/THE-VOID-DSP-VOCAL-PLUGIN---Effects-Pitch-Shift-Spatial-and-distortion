@echo off
setlocal
echo =========================================================================
echo  THE VOID - VST3 System Installer / Updater
echo  THE EDGE OF FEAR
echo =========================================================================
echo.

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required to update C:\Program Files\Common Files\VST3.
    echo Requesting elevation...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process cmd -ArgumentList '/c \"\"%~f0\"\"' -Verb RunAs"
    exit /b
)

echo Cleaning old VST3 files from C:\Program Files\Common Files\VST3\THE VOID.vst3...
if exist "C:\Program Files\Common Files\VST3\THE VOID.vst3" (
    rd /s /q "C:\Program Files\Common Files\VST3\THE VOID.vst3"
)

echo Deploying latest THE VOID.vst3...
robocopy "%~dp0THE VOID.vst3" "C:\Program Files\Common Files\VST3\THE VOID.vst3" /E /IS /IT /NFL /NDL

if exist "C:\Program Files\Common Files\VST3\THE VOID.vst3\Contents\x86_64-win\THE VOID.vst3" (
    echo.
    echo [SUCCESS] THE VOID.vst3 successfully updated in C:\Program Files\Common Files\VST3!
    echo Rescan plugins in your DAW or restart your DAW now.
) else (
    echo.
    echo [WARNING] Please check that THE VOID.vst3 exists next to this script.
)

echo.
pause
