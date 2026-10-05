#include "Desktop.h"
#include "AppState.h"

void RenderDesktop(AppState& /*state*/)
{
    // Currently a stub; main.cpp clears the full framebuffer to a solid color.
    // TODO: Draw the desktop background as the base layer, filling the entire
    // application window on every frame, including after a resize.
    // TODO: Choose wallpaper: a solid color, gradient, ImGui-drawn pattern,
    // or loaded image texture. Only one of these options is required.
    // TODO: Read the current time each frame and show a clock in a fixed corner.
    // TODO: Add a PWR button that sets state.shutdownRequested = true.
    // Let main.cpp exit its loop and clean up normally; do not exit directly here.
}
