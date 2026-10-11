#include "TodoList.h"
#include "Apps.h"
#include "AppState.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <string>
#include <vector>

namespace
{
    // Edit these defaults to change the app's appearance.
    struct TodoListSettings
    {
        const char* title = "To-Do List";
        ImVec2 defaultPosition = ImVec2(100.0f, 70.0f);
        ImVec2 defaultSize = ImVec2(420.0f, 400.0f);
        ImU32 accentColor = IM_COL32(120, 255, 140, 255);
    };

    struct TodoTask
    {
        int id; // Stable across deletions, including tasks with identical text.
        std::string text;
        bool completed = false;
    };

    // Tasks and unfinished input survive closing the window for this program session.
    struct TodoListState
    {
        AppWindowState window;
        std::vector<TodoTask> tasks;
        char input[256] = {};
        int nextTaskId = 0;
        bool wasOpen = false;
        bool focusInput = false;
    };

    const TodoListSettings kSettings;
    TodoListState g_todo;

    void RenderTaskInput()
    {
        // Task entry: Enter and Add both trim whitespace and reject empty input.
        const float addWidth = 56.0f;
        ImGui::SetNextItemWidth(std::max(
            1.0f,
            ImGui::GetContentRegionAvail().x
                - addWidth - ImGui::GetStyle().ItemSpacing.x
        ));
        if (g_todo.focusInput)
        {
            ImGui::SetKeyboardFocusHere();
            g_todo.focusInput = false;
        }

        const bool submitted = ImGui::InputTextWithHint(
            "##newTask", "What needs to get done?", g_todo.input,
            sizeof(g_todo.input), ImGuiInputTextFlags_EnterReturnsTrue
        );
        const std::string draft(g_todo.input);
        const size_t first = draft.find_first_not_of(" \t\r\n");
        const bool hasText = first != std::string::npos;

        ImGui::SameLine();
        ImGui::BeginDisabled(!hasText);
        const bool addClicked = ImGui::Button("Add", ImVec2(addWidth, 0.0f));
        ImGui::EndDisabled();
        if (hasText && (submitted || addClicked))
        {
            const size_t last = draft.find_last_not_of(" \t\r\n");
            g_todo.tasks.push_back({ g_todo.nextTaskId++, draft.substr(first, last - first + 1), false });
            g_todo.input[0] = '\0';
            g_todo.focusInput = true;
        }
    }

    void RenderTaskList()
    {
        // Reserve space below the scrolling list for its summary and progress.
        const float footerHeight = ImGui::GetFrameHeightWithSpacing()
            + ImGui::GetStyle().ItemSpacing.y + 8.0f;
        int deleteId = -1;
        if (ImGui::BeginChild(
            "##tasks", ImVec2(0.0f, -footerHeight), ImGuiChildFlags_Borders))
        {
            if (g_todo.tasks.empty())
            {
                ImGui::TextDisabled("No tasks yet.");
                ImGui::TextWrapped("Type a task above and press Enter or Add.");
            }
            else if (ImGui::BeginTable("##taskRows", 3, ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Done", ImGuiTableColumnFlags_WidthFixed,
                    ImGui::GetFrameHeight());
                ImGui::TableSetupColumn("Task", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed,
                    ImGui::CalcTextSize("Delete").x + ImGui::GetStyle().FramePadding.x * 2.0f);

                for (TodoTask& task : g_todo.tasks)
                {
                    ImGui::PushID(task.id);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("##done", &task.completed);
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(
                        task.completed ? ImGuiCol_TextDisabled : ImGuiCol_Text));
                    ImGui::TextWrapped("%s", task.text.c_str());
                    ImGui::PopStyleColor();
                    ImGui::TableNextColumn();
                    if (ImGui::Button("Delete"))
                    {
                        deleteId = task.id;
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        // Erase after rendering so iteration and widget IDs stay valid.
        if (deleteId >= 0)
        {
            g_todo.tasks.erase(std::remove_if(g_todo.tasks.begin(), g_todo.tasks.end(),
                [deleteId](const TodoTask& task) { return task.id == deleteId; }),
                g_todo.tasks.end());
        }
    }

    void RenderProgress()
    {
        // Footer: recompute completion after checkbox changes and deletions.
        const int total = static_cast<int>(g_todo.tasks.size());
        const int completed = static_cast<int>(std::count_if(
            g_todo.tasks.begin(), g_todo.tasks.end(),
            [](const TodoTask& task) { return task.completed; }
        ));
        ImGui::Text("%d of %d completed", completed, total);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, kSettings.accentColor);
        ImGui::ProgressBar(
            total > 0 ? static_cast<float>(completed) / total : 0.0f,
            ImVec2(-FLT_MIN, 8.0f), ""
        );
        ImGui::PopStyleColor();
    }
} // namespace

void RenderTodoList(AppState& state)
{
    if (!state.showFirstApp)
    {
        g_todo.wasOpen = false;
        return;
    }

    if (!g_todo.wasOpen)
    {
        g_todo.focusInput = true;
        ImGui::SetNextWindowFocus();
    }

    const ImGuiWindowFlags flags = ConfigureAppWindow(
        g_todo.window, kSettings.defaultPosition, kSettings.defaultSize);

    // The title-bar X and the taskbar share this visibility flag.
    if (ImGui::Begin(kSettings.title, &state.showFirstApp, flags))
    {
        RenderAppTitleBarControls(g_todo.window);
        ImGui::TextUnformatted("MY TASKS");
        ImGui::TextDisabled("Add a task, then check it off when done.");
        ImGui::Spacing();

        RenderTaskInput();
        ImGui::Spacing();
        ImGui::Separator();
        RenderTaskList();
        RenderProgress();
    }
    ImGui::End();
    g_todo.wasOpen = state.showFirstApp;
}
