#include "Taskbar.h"
#include "AppState.h"

void Taskbar::Render(AppState& state)
{
    (void)state;
    // NICOS - October 6:
    // Add the fixed bottom taskbar and three clickable icon buttons.
    // Connect buttons to state.showApp1, state.showApp2, state.showTaskManager.
    // Show which applications are open. Keep desktop, clock, and PWR working.
}
