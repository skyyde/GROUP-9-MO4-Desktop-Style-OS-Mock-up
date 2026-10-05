# Build and run in VS Code on Windows

Open this `example_glfw_opengl3` folder in VS Code.

- **Ctrl+Shift+B** builds the example.
- **F5** builds and starts it with the C++ debugger. Choose **Dear ImGui (MSVC x64)** if prompted.
- **Ctrl+F5** builds and runs without debugging.
- Alternatively, use **Terminal > Run Task > Run Dear ImGui**.

Close the example window before rebuilding, because Windows locks running executables.

From VS Code's PowerShell terminal:

```powershell
.\build_vscode.bat
if ($LASTEXITCODE -eq 0) { .\build\x64\example_glfw_opengl3.exe }
```

To run the existing build again without compiling:

```powershell
.\build\x64\example_glfw_opengl3.exe
```

The build script detects the latest Visual Studio installation with the MSVC x64 tools and initializes its compiler environment automatically. A Developer Command Prompt is not required. This machine has Visual Studio Build Tools 2026, MSVC 14.51.36231, Windows SDK 10.0.26100.0, and the Microsoft C/C++ VS Code extension.

The build uses the ImGui sources two directories above this folder, both GLFW/OpenGL3 backends, and `../libs/glfw/include` with `../libs/glfw/lib-vc2010-64/glfw3.lib`. No dependency download is needed. Keep the downloaded repository's directory layout intact.

All compiler and linker output goes into `build/x64`. The example may create `imgui.ini` in this folder to save its window layout. The original example source and supplied build files are unchanged.

IntelliSense points to the compiler and SDK found on this machine. If those versions are removed after an upgrade, update `.vscode/c_cpp_properties.json`; the build script itself detects the compiler dynamically.
