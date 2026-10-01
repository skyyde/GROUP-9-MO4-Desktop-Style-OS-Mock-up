#include "Desktop.h"
#include "AppState.h"

void Desktop::Render(AppState& state)
{
    (void)state;
    // ENRIQUE - October 3:
    // Draw the desktop background first and make it fill the application window.
    // Add wallpaper, a real-time clock, and PWR.
    // PWR should set state.requestShutdown = true; Application.cpp handles cleanup.
}
