# GROUP-9-MO4-Desktop-Style-OS-Mock-up
te

## Starter structure

The current UI remains the Dear ImGui demo over the OpenGL background. The OS
components are only prepared as render functions; their interfaces come next.

| File | Role / next implementation |
| --- | --- |
| `src/main.cpp` | GLFW/OpenGL/ImGui setup, frame loop, resize redraws, shutdown, and cleanup. Calls components in desktop, taskbar, apps, Task Manager order. |
| `src/AppState.h` | Shared visibility flags for two apps and Task Manager, plus a shutdown request. Owned by `main` and passed by reference. |
| `src/Desktop.h` / `Desktop.cpp` | `RenderDesktop`: stub for wallpaper/background, a fixed-corner clock, and a PWR button that requests shutdown. |
| `src/Taskbar.h` / `Taskbar.cpp` | `RenderTaskbar`: stub for a fixed panel with three icon buttons that open the two apps and Task Manager. |
| `src/Apps.h` / `Apps.cpp` | `RenderApps`: preserves the demo for now; replace it with two distinct placeholder app windows controlled by the visibility flags. |
| `src/TaskManager.h` / `TaskManager.cpp` | `RenderTaskManager`: stub for a Task Manager-style window with dummy process, CPU, and memory data. |

Add each component's UI in its corresponding `.cpp` render function. Headers
declare those functions. Keep platform setup and cleanup in `main.cpp`; the
future PWR button should set `state.shutdownRequested`, allowing the normal
cleanup path to run.

## Build and run

### Frame lifecycle

The loop in `src/main.cpp` processes GLFW events, then calls `drawFrame`:

1. Start the OpenGL3 and GLFW backend frames, then call `ImGui::NewFrame()`.
2. Call `RenderDesktop`, `RenderTaskbar`, `RenderApps`, and `RenderTaskManager`
   in that order, passing the same `AppState` to each.
3. Call `ImGui::Render()`, size the OpenGL viewport from the framebuffer's pixel
   dimensions, clear the background, render the ImGui draw data, and swap buffers.

Resize and refresh callbacks reuse `drawFrame` so the UI can redraw during a
Windows resize operation. Minimized windows wait briefly for events instead of
drawing. A close event or `state.shutdownRequested` ends the loop; cleanup shuts
down both ImGui backends and the ImGui context before destroying the GLFW window
and terminating GLFW. The component stubs and existing demo are unchanged.

### Run from VS Code

The existing `build.cmd` uses MSVC 2026 x64, `glfw/lib-vc2026`, and the local
`imgui-master` sources, including its GLFW/OpenGL3 backends. It now also compiles
all four component `.cpp` files.

In VS Code, choose **Terminal > Run Task > Run GLFW window** to build and run.
**Ctrl+Shift+B** builds only. Expand the demo sections, resize or minimize/restore
the application window, and close it with **X**. No OS component UI is expected
in this structure-only step.
