# Group 9 - Desktop-Style OS Mock-up
# Members:
Aguirre, Mikyla Kirsten, P.

Cruz, Enrique Mateo, A.

Pallarca, Cedric, S.

Reyes, Julian Nicos, S.

A C++17 desktop simulation for CSOPESY, built with Dear ImGui, GLFW, and OpenGL.

## Features

- **Desktop:** landscape wallpaper with animated clouds, a live date and clock, and a shutdown confirmation screen.
- **Taskbar:** open or close apps, see running indicators, and access the power button.
- **To-Do List:** add tasks, mark them complete, delete them, and track completion progress. Tasks stay in memory until the program exits.
- **Music Player:** five mock tracks, play/pause, previous/next, a playback timer, and a volume slider. No audio is played, and playback resets when reopened.
- **Task Manager:** view simulated processes, CPU usage, and memory usage. Select a process and click **End Task** to close its linked app or remove a mock process. System processes are protected.
- **App windows:** move, resize, maximize, restore, and close all three windows.

This is a UI mock-up. Task Manager does not monitor or end real Windows processes, and the power button only exits this program.

## Build and run

Requirements:

- Windows with OpenGL 3.0 support.
- Visual Studio 2026 or Build Tools 2026 with the **Desktop development with C++** workload, including MSVC x64/x86 tools and a Windows SDK.
- The included `glfw/` and `imgui-master/` folders.

The build script uses the VS 2026 x64 compiler and the bundled `glfw/lib-vc2026` library.

From PowerShell in the project folder:

```powershell
.\build.cmd
.\build\desktop-os.exe
```

In VS Code, choose **Terminal > Run Task > Run GLFW window** to build and run.
Use **Ctrl+Shift+B** to build only.

## Controls

- Click an app's taskbar button to open or close it.
- In the To-Do List, type a task and press **Enter** or click **Add**.
- Drag a window's title bar to move it or its edges to resize it. Use the icon beside **X** to maximize or restore it.
- Click the power button and confirm **Shut Down**, or close the outer window, to exit.

## Project files

| File | Purpose |
| --- | --- |
| `src/main.cpp` | Window setup, rendering loop, and cleanup. |
| `src/AppState.h` | Shared app visibility and shutdown state. |
| `src/Desktop.cpp` | Wallpaper, clock, theme, and shutdown screen. |
| `src/Taskbar.cpp` | App buttons, running indicators, and power button. |
| `src/Apps.cpp` | Shared window layout and maximize/restore controls. |
| `src/TodoList.cpp` | To-Do List app. |
| `src/MusicPlayer.cpp` | Simulated Music Player app. |
| `src/TaskManager.cpp` | Simulated process list and End Task controls. |
| `build.cmd` | Windows build script. |
