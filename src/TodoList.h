#pragma once

struct AppState;

// Called by RenderApps each frame; AppState::showFirstApp controls visibility.
void RenderTodoList(AppState& state);
