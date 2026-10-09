#pragma once

struct AppState;

// Currently renders only the ImGui demo.
// TODO: Two distinct app screens with their own content and shared visibility.
void RenderApps(AppState& state);
