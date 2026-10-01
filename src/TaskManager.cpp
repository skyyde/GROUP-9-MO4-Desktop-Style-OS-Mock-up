#include "TaskManager.h"
#include "AppState.h"

void TaskManager::Render(AppState& state)
{
    (void)state;
    // CED - October 8: add the Task Manager window and dummy process table.
    // Columns: Process Name, CPU Usage, Memory Usage.
    // Use state.showTaskManager to control whether its ImGui window is visible.
}
