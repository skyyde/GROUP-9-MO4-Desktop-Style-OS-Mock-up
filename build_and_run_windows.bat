@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake was not found. Open a Visual Studio Developer Command Prompt.
    echo See README.txt for the required Visual Studio components.
    pause
    exit /b 1
)
cmake -S . -B build/windows -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed
cmake --build build/windows --config Debug
if errorlevel 1 goto failed
build\windows\Debug\mo4.exe
exit /b %errorlevel%
:failed
echo Build failed. Read the error above and see README.txt.
pause
exit /b 1
