#pragma once

#include "imgui.h"

struct AppState;

enum class TaskbarPosition
{
    Top,
    Bottom
};

// All taskbar tuning lives here (same pattern as DesktopSettings) so layout and
// labels can be changed from one place without touching the drawing code.
struct TaskbarSettings
{
    // layout
    TaskbarPosition position = TaskbarPosition::Bottom;
    float height = 48.0f;        // Panel thickness; Desktop reserves this much space.
    float buttonHeight = 34.0f;
    float buttonSpacing = 6.0f;
    float padding = 10.0f;       // Left/right inner margin of the panel.

    // text
    const char* brandText = "CSOPESY OS";
    const char* firstAppLabel = "App 1";
    const char* secondAppLabel = "App 2";
    const char* taskManagerLabel = "Task Manager";
    bool showLabels = true;      // false = icon-only buttons (name still shown as tooltip).
    bool showRunningCount = true; // "N apps running" on the right side.

    // colors
    ImU32 panelColor = IM_COL32(15, 18, 26, 235);
    ImU32 borderColor = IM_COL32(255, 255, 255, 40);
    ImU32 accentColor = IM_COL32(120, 255, 140, 255); // Running indicator (matches desktop status text).
};

// taskbar setting object
TaskbarSettings& GetTaskbarSettings();

// Tells the desktop to keep its clock/PWR/status text clear of the taskbar.
// RenderTaskbar() already calls this every frame, so nothing else needs to. Optionally
// call it before RenderDesktop to avoid a one-frame offset on the very first frame.
void ApplyTaskbarLayout();

// Called after the desktop and before app windows.
// Fixed top/bottom panel with three icon buttons (App 1, App 2, Task Manager)
// that toggle the shared state.showFirstApp / showSecondApp / showTaskManager flags.
void RenderTaskbar(AppState& state);