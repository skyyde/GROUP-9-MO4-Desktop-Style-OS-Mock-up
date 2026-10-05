#include "Taskbar.h"
#include "AppState.h"

void RenderTaskbar(AppState& /*state*/)
{
    // Currently a stub; no taskbar controls are drawn yet.
    // TODO: Keep a fixed panel at the top or bottom of the application window.
    // TODO: Provide access to system functions and show which apps are running.
    // TODO: Add at least three clickable icon buttons:
    // - Two open distinct placeholder screens implemented in Apps.cpp by setting
    //   state.showFirstApp and state.showSecondApp to true, respectively.
    // - The third sets state.showTaskManager = true to open Task Manager.
    // TODO: Use the shared visibility flags to keep running-app indicators and
    // taskbar controls consistent when an app window is opened or closed.
}
