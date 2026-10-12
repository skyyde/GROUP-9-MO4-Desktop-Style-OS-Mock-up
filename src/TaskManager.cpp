#include "TaskManager.h"
#include "Apps.h"
#include "AppState.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
    struct ProcessItem
    {
        int pid;
        const char* name;
        float baseCpu;
        float cpu;
        float memoryMB;
        bool isSystem;
        bool* linkedAppStateFlag; // Allows ending actual running app windows
    };

    struct TaskManagerWindowState
    {
        AppWindowState window;
        bool initialized = false;
        std::vector<ProcessItem> processes;
        int selectedPid = -1;
    };

    TaskManagerWindowState g_taskManager;

    void InitProcesses(AppState& state)
    {
        g_taskManager.processes = {
            { 4,    "System Interrupts", 0.4f,  0.4f,  4.2f,   true,  nullptr },
            { 104,  "CSOPESY Kernel",    1.2f,  1.2f,  32.5f,  true,  nullptr },
            { 412,  "Desktop Shell",     2.1f,  2.1f,  64.0f,  true,  nullptr },
            { 520,  "Taskbar Host",      0.5f,  0.5f,  18.4f,  true,  nullptr },
            { 880,  "To-Do List",        1.0f,  1.0f,  42.1f,  false, &state.showFirstApp },
            { 914,  "Music Player",      3.4f,  3.4f,  88.6f,  false, &state.showSecondApp },
            { 1102, "Task Manager",      1.8f,  1.8f,  28.0f,  false, &state.showTaskManager },
            { 1450, "Web Browser",       7.5f,  7.5f,  340.2f, false, nullptr },
            { 1820, "Discord Client",    3.8f,  3.8f,  195.8f, false, nullptr },
            { 2100, "Code Editor",       2.6f,  2.6f,  210.5f, false, nullptr }
        };
        g_taskManager.initialized = true;
    }

    void UpdateProcessMetrics(float time)
    {
        for (auto& proc : g_taskManager.processes)
        {
            float cpuNoise = std::sin(time * 2.0f + static_cast<float>(proc.pid)) * 0.8f;
            proc.cpu = std::clamp(proc.baseCpu + cpuNoise, 0.1f, 99.9f);

            float memNoise = std::cos(time * 0.5f + static_cast<float>(proc.pid)) * 1.5f;
            proc.memoryMB = std::max(1.0f, proc.memoryMB + memNoise * 0.05f);
        }
    }
} 

void RenderTaskManager(AppState& state)
{
    if (!state.showTaskManager)
    {
        return;
    }

    if (!g_taskManager.initialized)
    {
        InitProcesses(state);
        ImGui::SetNextWindowFocus();
    }

    UpdateProcessMetrics(static_cast<float>(ImGui::GetTime()));

    const ImVec2 defaultPos(220.0f, 100.0f);
    const ImVec2 defaultSize(580.0f, 420.0f);
    const ImGuiWindowFlags flags = ConfigureAppWindow(g_taskManager.window, defaultPos, defaultSize);

    if (ImGui::Begin("Task Manager", &state.showTaskManager, flags))
    {
        RenderAppTitleBarControls(g_taskManager.window);

        // system summary headers
        float totalCpu = 0.0f;
        float totalMemMB = 0.0f;
        int activeProcs = 0;
        const ProcessItem* selectedProcess = nullptr;

        for (const auto& proc : g_taskManager.processes)
        {
            if (proc.linkedAppStateFlag && !(*proc.linkedAppStateFlag))
            {
                continue;
            }

            if (proc.pid == g_taskManager.selectedPid)
            {
                selectedProcess = &proc;
            }

            totalCpu += proc.cpu;
            totalMemMB += proc.memoryMB;
            activeProcs++;
        }

        // automatic clear selection if selected process was closed externally
        if (!selectedProcess)
        {
            g_taskManager.selectedPid = -1;
        }

        ImGui::Text("Processes: %d", activeProcs);
        ImGui::SameLine(160.0f);
        ImGui::Text("CPU Usage: %.1f%%", std::min(totalCpu, 100.0f));
        ImGui::SameLine(320.0f);
        ImGui::Text("Memory: %.1f MB", totalMemMB);

        ImGui::Separator();
        ImGui::Spacing();

        // Process Table
        constexpr ImGuiTableFlags tableFlags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Hideable | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
            ImGuiTableFlags_ScrollY;

        const float tableHeight = ImGui::GetContentRegionAvail().y - 36.0f;

        if (ImGui::BeginTable("ProcessTable", 4, tableFlags, ImVec2(0.0f, tableHeight)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("CPU Usage", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Memory Usage", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableHeadersRow();

            for (const auto& proc : g_taskManager.processes)
            {
                if (proc.linkedAppStateFlag && !(*proc.linkedAppStateFlag))
                {
                    continue;
                }

                ImGui::TableNextRow();

                // PID
                ImGui::TableSetColumnIndex(0);
                char pidStr[16];
                std::snprintf(pidStr, sizeof(pidStr), "%d", proc.pid);
                const bool isSelected = (g_taskManager.selectedPid == proc.pid);
                if (ImGui::Selectable(pidStr, isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
                {
                    g_taskManager.selectedPid = proc.pid;
                }

                // process name
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s %s", proc.name, proc.isSystem ? "(System)" : "");

                // cpu usage
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.1f%%", proc.cpu);

                // memory usage
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f MB", proc.memoryMB);
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();

        // end task button
        const bool disableEndTask = (selectedProcess == nullptr || selectedProcess->isSystem);
        if (disableEndTask)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("End Task", ImVec2(100.0f, 24.0f)))
        {
            auto it = std::find_if(g_taskManager.processes.begin(), g_taskManager.processes.end(),
                [](const ProcessItem& p) { return p.pid == g_taskManager.selectedPid; });

            if (it != g_taskManager.processes.end())
            {
                if (it->linkedAppStateFlag)
                {
                    *it->linkedAppStateFlag = false;
                }
                else if (!it->isSystem)
                {
                    g_taskManager.processes.erase(it);
                }
                g_taskManager.selectedPid = -1;
            }
        }

        if (disableEndTask)
        {
            ImGui::EndDisabled();

            if (selectedProcess && selectedProcess->isSystem)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(System process cannot be terminated)");
            }
        }
    }
    ImGui::End();
}