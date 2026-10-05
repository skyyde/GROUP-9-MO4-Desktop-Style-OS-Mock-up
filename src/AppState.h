#pragma once

// Shared UI state owned by main and passed to the components each frame.
// main already handles shutdownRequested; component controls are still pending.
struct AppState
{
    // TODO: Share these flags between taskbar buttons and their app windows.
    bool showFirstApp = false;
    bool showSecondApp = false;
    bool showTaskManager = false;
    // TODO: Set this from Desktop's PWR button; main exits and cleans up normally.
    bool shutdownRequested = false;
};
