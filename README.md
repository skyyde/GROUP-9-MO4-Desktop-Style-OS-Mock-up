# GROUP-9-MO4-Desktop-Style-OS-Mock-up

## Project structure

A desktop mock-up built with Dear ImGui, GLFW, and OpenGL. The taskbar opens
a To-Do List and a simulated Music Player. Task Manager is still a stub.

| File | Role |
| --- | --- |
| `src/main.cpp` | GLFW/OpenGL/ImGui setup, frame loop, resize redraws, shutdown, and cleanup. Calls components in desktop, taskbar, apps, Task Manager order. |
| `src/AppState.h` | Shared visibility flags for two apps and Task Manager, plus a shutdown request. Owned by `main` and passed by reference. |
| `src/Desktop.h` / `Desktop.cpp` | Desktop wallpaper, clock, and power controls. |
| `src/Taskbar.h` / `Taskbar.cpp` | Panel layout, app buttons, tooltips, and running indicators. |
| `src/Apps.h` / `Apps.cpp` | Shared app window layout and the `RenderApps` entry point. |
| `src/TodoList.h` / `TodoList.cpp` | App 1: task entry, completion checkboxes, deletion, and progress. Tasks remain in memory during the current program session. |
| `src/MusicPlayer.h` / `MusicPlayer.cpp` | App 2: mock playlist, playback timer, vinyl artwork, and player controls. No audio is played. |
| `src/TaskManager.h` / `TaskManager.cpp` | Placeholder for a future Task Manager window; currently renders nothing. |

To customize the To-Do List, edit `TodoListSettings` at the top of
`src/TodoList.cpp`. Its private helpers handle task entry, the task list, and
completion progress; `RenderTodoList` handles opening and drawing the window.
Tasks and unfinished input stay in memory until the program exits.

To customize the Music Player, edit `MusicPlayerSettings` and `kTracks` at the
top of `src/MusicPlayer.cpp`. Its helpers handle the playback timer, artwork,
progress, controls, and playlist. `RenderMusicPlayer` resets playback when
the window reopens. Playback and volume are simulated; no audio is played.

Taskbar defaults live in `TaskbarSettings` in `src/Taskbar.h`. The button list
in `RenderTaskbar` links each label and icon to its visibility flag in
`AppState`. `ApplyTaskbarLayout` reserves panel space once per frame, before
the desktop is drawn. Both apps use `ConfigureAppWindow` in `src/Apps.cpp`
for their initial position and size limits.

Add each component's UI in its corresponding `.cpp` render function. Headers
declare those functions. Keep platform setup and cleanup in `main.cpp`; the
PWR controls set `state.shutdownRequested`, allowing the normal
cleanup path to run.

## Build and run

### Frame lifecycle

The loop in `src/main.cpp` processes GLFW events, then calls `drawFrame`:

1. Start the OpenGL3 and GLFW backend frames, then call `ImGui::NewFrame()`.
2. Reserve space with `ApplyTaskbarLayout`, then call
   `RenderDesktop`, `RenderTaskbar`, `RenderApps`, and `RenderTaskManager`
   in that order, passing the same `AppState` to each.
3. Call `ImGui::Render()`, size the OpenGL viewport from the framebuffer's pixel
   dimensions, clear the background, render the ImGui draw data, and swap buffers.

Resize and refresh callbacks reuse `drawFrame` so the UI can redraw during a
Windows resize operation. Minimized windows wait briefly for events instead of
drawing. A close event or `state.shutdownRequested` ends the loop; cleanup shuts
down both ImGui backends and the ImGui context before destroying the GLFW window
and terminating GLFW.

### Run from VS Code

The existing `build.cmd` uses MSVC 2026 x64, `glfw/lib-vc2026`, and the local
`imgui-master` sources, including its GLFW/OpenGL3 backends. It now also compiles
the component `.cpp` files, including both app implementations.

In VS Code, choose **Terminal > Run Task > Run GLFW window** to build and run.
**Ctrl+Shift+B** builds only. Open the To-Do List or Music Player from the taskbar.
Click an app's taskbar button again or its title-bar **X** to close it.
Both apps have a square **Maximize** icon beside the title-bar **X** to fill the
desktop area while keeping the taskbar visible. Click the overlapping-squares
**Restore** icon to return to the previous position and size.
Use the desktop power controls or the outer window's **X** to exit.
