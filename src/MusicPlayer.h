#pragma once

struct AppState;

// Called by RenderApps each frame; AppState::showSecondApp controls visibility.
void RenderMusicPlayer(AppState& state);
