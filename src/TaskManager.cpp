#include "TaskManager.h"
#include "AppState.h"

void RenderTaskManager(AppState& /*state*/)
{
    // Currently a stub; no Task Manager window or process table is drawn yet.
    // TODO: Show this window when the taskbar sets state.showTaskManager to true.
    // Pass that flag to ImGui::Begin so the window's close button updates it.
    // TODO: Make the window closely resemble Windows Task Manager.
    // TODO: Add a placeholder table listing processes with CPU and memory usage
    // for each row. Use dummy values; real process monitoring is not required.
    // Use screenshots as visual references, not as additional feature requirements.
}
