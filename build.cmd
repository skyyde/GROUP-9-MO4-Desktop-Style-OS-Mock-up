@echo off
setlocal
cd /d "%~dp0"

rem Find the installed VS 2026 C++ tools that match glfw/lib-vc2026.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :missingCompiler
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
set "VSINSTALL="
for /f "usebackq delims=" %%I in (`vswhere.exe -latest -products * -version "[18.0,19.0)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%I"
if not defined VSINSTALL goto :missingCompiler

call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1

if not exist "build" mkdir "build"
if not exist "build" exit /b 1

rem /MT matches glfw3_mt.lib: GLFW and the C++ runtime are linked statically.
rem Compile ImGui core, the demo, and only the GLFW/OpenGL3 backends.
cl /nologo /std:c++17 /EHsc /W4 /Zi /Od /MT ^
    /I"glfw\include" /I"imgui-master" /I"imgui-master\backends" ^
    "src\main.cpp" "src\Desktop.cpp" "src\Taskbar.cpp" ^
    "src\Apps.cpp" "src\TaskManager.cpp" ^
    "imgui-master\imgui.cpp" "imgui-master\imgui_draw.cpp" ^
    "imgui-master\imgui_tables.cpp" "imgui-master\imgui_widgets.cpp" ^
    "imgui-master\imgui_demo.cpp" ^
    "imgui-master\backends\imgui_impl_glfw.cpp" ^
    "imgui-master\backends\imgui_impl_opengl3.cpp" ^
    /Fo"build\\" /Fd"build\compiler.pdb" /Fe"build\desktop-os.exe" ^
    /link /DEBUG /PDB:"build\desktop-os.pdb" /LIBPATH:"glfw\lib-vc2026" ^
    glfw3_mt.lib opengl32.lib user32.lib gdi32.lib shell32.lib
exit /b %errorlevel%

:missingCompiler
echo ERROR: Visual Studio Build Tools 2026 with x64 C++ tools was not found.
echo Install the Desktop development with C++ workload, including MSVC x64/x86 tools and a Windows SDK.
echo See README.md for setup instructions.
exit /b 1
