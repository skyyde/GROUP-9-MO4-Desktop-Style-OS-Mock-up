#pragma once

// Shared UI state owned by main and passed to the components each frame.
struct AppState
{
    // Shared by each app window and its taskbar button.
    bool showFirstApp = false;
    bool showSecondApp = false;
    bool showTaskManager = false;
    // Set by Desktop's power controls; main exits through its normal cleanup path.
    bool shutdownRequested = false;
};
