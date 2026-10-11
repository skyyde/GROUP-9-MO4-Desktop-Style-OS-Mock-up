#pragma once

#include "imgui.h"

struct AppState;

// Each app keeps its own maximize state and previous window bounds.
struct AppWindowState
{
    bool maximized = false;
    bool restorePending = false;
    ImVec2 restorePosition;
    ImVec2 restoreSize;
};

// Apply layout before ImGui::Begin and return the appropriate window flags.
ImGuiWindowFlags ConfigureAppWindow(AppWindowState& window,
    const ImVec2& defaultPosition, const ImVec2& defaultSize);

// Draw Maximize/Restore beside the title-bar X after a successful ImGui::Begin.
void RenderAppTitleBarControls(AppWindowState& window);

// Called by main each frame, including when the app windows are closed.
void RenderApps(AppState& state);
