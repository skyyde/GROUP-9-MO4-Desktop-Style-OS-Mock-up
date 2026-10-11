#pragma once

#include "imgui.h"

struct AppState;

enum class TaskbarPosition
{
    Top,
    Bottom
};

// Edit these defaults to customize the panel, labels, and colors.
struct TaskbarSettings
{
    // Layout
    TaskbarPosition position = TaskbarPosition::Bottom;
    float height = 48.0f;        // Panel thickness; Desktop reserves this much space.
    float buttonHeight = 34.0f;
    float buttonSpacing = 6.0f;
    float padding = 10.0f;       // Left/right inner margin of the panel.

    // Labels
    const char* brandText = "CSOPESY OS";
    const char* firstAppLabel = "To-Do List";
    const char* secondAppLabel = "Music Player";
    const char* taskManagerLabel = "Task Manager";
    bool showLabels = true;      // false = icon-only buttons (name still shown as tooltip).
    bool showRunningCount = true; // "N apps running" on the right side.

    // Colors
    ImU32 panelColor = IM_COL32(15, 18, 26, 235);
    ImU32 borderColor = IM_COL32(255, 255, 255, 40);
    ImU32 accentColor = IM_COL32(120, 255, 140, 255); // Running indicator (matches desktop status text).
};

// Call once before RenderDesktop each frame to reserve space for the panel.
void ApplyTaskbarLayout();

// Draw the panel and toggle the app visibility flags in AppState.
void RenderTaskbar(AppState& state);
