@echo off
setlocal
rem Locate MSVC without requiring VS Code to run from a Developer Command Prompt.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: Visual Studio Installer's vswhere.exe was not found.
    exit /b 1
)
set "VSINSTALL="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%I"
if not defined VSINSTALL (
    echo ERROR: Install Visual Studio Build Tools with Desktop development with C++.
    exit /b 1
)
rem VsDevCmd also invokes vswhere by name; support shells that disable cwd lookup.
set "PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%"
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" -no_logo -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 1

pushd "%~dp0"
if errorlevel 1 exit /b 1
if not exist "build\x64" mkdir "build\x64"
if not exist "build\x64" (
    popd
    exit /b 1
)
rem Compile in the output directory so all object and debug files stay there.
pushd "build\x64"
cl /nologo /Zi /Od /EHsc /MD /utf-8 /std:c++17 ^
    /I"..\..\..\.." /I"..\..\..\..\backends" /I"..\..\..\libs\glfw\include" ^
    "..\..\main.cpp" ^
    "..\..\..\..\imgui.cpp" ^
    "..\..\..\..\imgui_demo.cpp" ^
    "..\..\..\..\imgui_draw.cpp" ^
    "..\..\..\..\imgui_tables.cpp" ^
    "..\..\..\..\imgui_widgets.cpp" ^
    "..\..\..\..\backends\imgui_impl_glfw.cpp" ^
    "..\..\..\..\backends\imgui_impl_opengl3.cpp" ^
    /Fe:example_glfw_opengl3.exe /Fd:compiler.pdb ^
    /link /DEBUG /INCREMENTAL:NO ^
    /LIBPATH:"..\..\..\libs\glfw\lib-vc2010-64" ^
    glfw3.lib opengl32.lib gdi32.lib shell32.lib
set "BUILD_RESULT=%ERRORLEVEL%"
popd
popd
exit /b %BUILD_RESULT%
