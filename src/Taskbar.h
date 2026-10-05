#pragma once

struct AppState;

// Called after the desktop and before app windows; currently a stub.
// TODO: Fixed top/bottom panel with system access, running apps, and icon buttons.
void RenderTaskbar(AppState& state);
