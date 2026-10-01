#pragma once

// Shared state for future modules. No later-deadline features implemented yet.
struct AppState
{
    bool showApp1 = false;
    bool showApp2 = false;
    bool showTaskManager = false;
    bool requestShutdown = false;
};
