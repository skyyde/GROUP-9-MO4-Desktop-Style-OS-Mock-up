#include "Taskbar.h"
#include "AppState.h"
#include "Desktop.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>

namespace
{
    const TaskbarSettings kSettings;

    const char* const kTaskbarWindowName = "##Taskbar";

    // Taskbar thickness actually used this frame: never taller than the window itself.
    float EffectiveHeight(const ImGuiViewport* viewport)
    {
        return std::max(0.0f, std::min(kSettings.height, viewport->Size.y));
    }

    // To-Do List icon: a window with a title bar and content lines.
    void DrawTodoListIcon(ImDrawList* drawList, ImVec2 center, float iconSize, ImU32 color)
    {
        const float halfSize = iconSize * 0.5f;
        const ImVec2 a(center.x - halfSize, center.y - halfSize * 0.85f);
        const ImVec2 b(center.x + halfSize, center.y + halfSize * 0.85f);
        drawList->AddRect(a, b, color, 3.0f, 0, 1.6f);
        drawList->AddLine(ImVec2(a.x, a.y + iconSize * 0.28f), ImVec2(b.x, a.y + iconSize * 0.28f), color, 1.6f);
        drawList->AddLine(ImVec2(a.x + iconSize * 0.18f, center.y + iconSize * 0.08f),
            ImVec2(b.x - iconSize * 0.18f, center.y + iconSize * 0.08f), color, 1.4f);
        drawList->AddLine(ImVec2(a.x + iconSize * 0.18f, center.y + iconSize * 0.26f),
            ImVec2(center.x + iconSize * 0.08f, center.y + iconSize * 0.26f), color, 1.4f);
    }

    // Music Player icon: a 2x2 grid of tiles.
    void DrawMusicPlayerIcon(ImDrawList* drawList, ImVec2 center, float iconSize, ImU32 color)
    {
        const float gap = iconSize * 0.12f;
        const float tile = (iconSize - gap) * 0.5f - iconSize * 0.04f;
        for (int row = 0; row < 2; ++row)
        {
            for (int colIdx = 0; colIdx < 2; ++colIdx)
            {
                const float x = center.x - gap * 0.5f - tile + (tile + gap) * static_cast<float>(colIdx);
                const float y = center.y - gap * 0.5f - tile + (tile + gap) * static_cast<float>(row);
                drawList->AddRectFilled(ImVec2(x, y), ImVec2(x + tile, y + tile), color, 2.5f);
            }
        }
    }

    // Task Manager: activity bars.
    void DrawTaskManagerIcon(ImDrawList* drawList, ImVec2 center, float iconSize, ImU32 color)
    {
        const float barWidth = iconSize * 0.18f;
        const float gap = iconSize * 0.10f;
        const float heights[] = { 0.45f, 0.85f, 0.60f, 1.00f };
        const float total = 4 * barWidth + 3 * gap;
        const float bottom = center.y + iconSize * 0.5f;
        for (int i = 0; i < 4; ++i)
        {
            const float x = center.x - total * 0.5f + (barWidth + gap) * static_cast<float>(i);
            drawList->AddRectFilled(ImVec2(x, bottom - iconSize * heights[i]), ImVec2(x + barWidth, bottom), color, 1.5f);
        }
    }

    using DrawIcon = void (*)(ImDrawList*, ImVec2, float, ImU32);

    // Each button uses the same visibility flag as its app window.
    struct TaskbarButton
    {
        const char* label;
        DrawIcon drawIcon;
        bool AppState::* visible;
    };

    // Pill-shaped, non-clickable brand label on the left of the panel.
    float DrawBrand(ImDrawList* drawList, ImVec2 origin, float panelHeight)
    {
        const char* text = kSettings.brandText;
        if (text == nullptr || text[0] == '\0')
        {
            return 0.0f;
        }
        const float fontSize = 15.0f;
        ImFont* font = ImGui::GetFont();
        const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        const ImVec2 pill(textSize.x + 28.0f, 28.0f);
        const ImVec2 p0(origin.x, origin.y + (panelHeight - pill.y) * 0.5f);
        const ImVec2 p1(p0.x + pill.x, p0.y + pill.y);
        drawList->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 18), pill.y * 0.5f);
        drawList->AddCircleFilled(ImVec2(p0.x + 12.0f, p0.y + pill.y * 0.5f), 4.0f, kSettings.accentColor);
        drawList->AddText(font, fontSize, ImVec2(p0.x + 22.0f, p0.y + (pill.y - textSize.y) * 0.5f),
                    IM_COL32(235, 240, 250, 255), text);
        return pill.x;
    }

    // Toggle the app before drawing its running indicator; return the button width.
    float DrawTaskbarButton(ImDrawList* drawList, const TaskbarButton& button, bool& running,
                           ImVec2 pos, float panelHeight)
    {
        const float iconSize = 18.0f;
        const float fontSize = 15.0f;
        const bool hasLabel = kSettings.showLabels && button.label != nullptr && button.label[0] != '\0';
        ImFont* font = ImGui::GetFont();
        const float labelWidth = hasLabel
            ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, button.label).x : 0.0f;
        const float sidePadding = 12.0f;
        const float width = sidePadding * 2 + iconSize + (hasLabel ? 8.0f + labelWidth : 0.0f);
        const ImVec2 size(width, kSettings.buttonHeight);
        const ImVec2 p0(pos.x, pos.y + (panelHeight - size.y) * 0.5f);
        const ImVec2 p1(p0.x + size.x, p0.y + size.y);

        ImGui::SetCursorScreenPos(p0);
        if (ImGui::InvisibleButton("##button", size))
        {
            running = !running;
        }
        const bool hovered = ImGui::IsItemHovered();
        const bool held = ImGui::IsItemActive();

        const char* name = (button.label != nullptr && button.label[0] != '\0') ? button.label : "App";
        if (running)
        {
            ImGui::SetItemTooltip("%s (running) - click to close", name);
        }
        else
        {
            ImGui::SetItemTooltip("Open %s", name);
        }

        ImU32 backgroundColor = IM_COL32(255, 255, 255, 0);
        if (held)
        {
            backgroundColor = IM_COL32(255, 255, 255, 40);
        }
        else if (hovered)
        {
            backgroundColor = IM_COL32(255, 255, 255, 28);
        }
        else if (running)
        {
            backgroundColor = IM_COL32(255, 255, 255, 14);
        }
        drawList->AddRectFilled(p0, p1, backgroundColor, 7.0f);

        const ImU32 foregroundColor = running || hovered
            ? IM_COL32(245, 248, 255, 255) : IM_COL32(190, 198, 212, 255);
        const ImVec2 iconCenter(p0.x + sidePadding + iconSize * 0.5f, p0.y + size.y * 0.5f);
        button.drawIcon(drawList, iconCenter, iconSize, foregroundColor);
        if (hasLabel)
        {
            const ImVec2 textPosition(
                p0.x + sidePadding + iconSize + 8.0f, p0.y + (size.y - fontSize) * 0.5f);
            drawList->AddText(font, fontSize, textPosition, foregroundColor, button.label);
        }

        // Running indicator: short accent bar along the bottom edge of the button.
        if (running)
        {
            const float barWidth = std::min(22.0f, size.x * 0.5f);
            const float centerX = (p0.x + p1.x) * 0.5f;
            drawList->AddRectFilled(
                ImVec2(centerX - barWidth * 0.5f, p1.y - 3.0f),
                ImVec2(centerX + barWidth * 0.5f, p1.y - 1.0f), kSettings.accentColor, 2.0f);
        }
        return width;
    }

    void DrawRunningCount(ImDrawList* drawList, ImVec2 panelMin, float rightX, float minX,
                          float panelHeight, int runningCount)
    {
        char text[32];
        if (runningCount == 0)
        {
            std::snprintf(text, sizeof(text), "No apps running");
        }
        else
        {
            std::snprintf(text, sizeof(text), "%d app%s running",
                runningCount, runningCount == 1 ? "" : "s");
        }
        const float fontSize = 14.0f;
        ImFont* font = ImGui::GetFont();
        const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        const ImVec2 textPosition(
            rightX - textSize.x,
            panelMin.y + (panelHeight - textSize.y) * 0.5f);
        if (textPosition.x < minX)
        {
            return; // no room left of PWR, so skip the text instead of overlapping buttons.
        }
        const ImU32 textColor = runningCount > 0
            ? kSettings.accentColor : IM_COL32(140, 148, 162, 255);
        drawList->AddText(font, fontSize, textPosition, textColor, text);
    }

    // PWR at the right end
    // also uses the desktop's shutdown sequence 
    float DrawPowerButton(ImDrawList* drawList, float rightX, float panelY, float panelHeight)
    {
        const float fontSize = 15.0f;
        const float iconRadius = 7.0f;
        const char* label = "PWR";
        ImFont* font = ImGui::GetFont();
        const float labelWidth = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label).x;
        const ImVec2 size(12.0f + iconRadius * 2.0f + 8.0f + labelWidth + 12.0f, kSettings.buttonHeight);
        const ImVec2 p0(rightX - size.x, panelY + (panelHeight - size.y) * 0.5f);
        const ImVec2 p1(p0.x + size.x, p0.y + size.y);

        ImGui::SetCursorScreenPos(p0);
        if (ImGui::InvisibleButton("##power", size))
        {
            RequestShutdown();
        }
        const bool hovered = ImGui::IsItemHovered();
        const bool held = ImGui::IsItemActive();
        ImGui::SetItemTooltip("Shut down CSOPESY OS");

        const ImU32 backgroundColor = held ? IM_COL32(255, 82, 82, 70)
            : hovered ? IM_COL32(255, 82, 82, 40) : IM_COL32(255, 82, 82, 16);
        drawList->AddRectFilled(p0, p1, backgroundColor, 7.0f);
        DrawPowerSymbol(drawList, ImVec2(p0.x + 12.0f + iconRadius, p0.y + size.y * 0.5f + 1.0f),
            iconRadius, kSettings.powerColor);
        drawList->AddText(font, fontSize,
            ImVec2(p0.x + 12.0f + iconRadius * 2.0f + 8.0f, p0.y + (size.y - fontSize) * 0.5f),
            kSettings.powerColor, label);
        return size.x;
    }
} // namespace

void ApplyTaskbarLayout()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = EffectiveHeight(viewport);
    DesktopSettings& desktop = GetDesktopSettings();
    desktop.topReserved = kSettings.position == TaskbarPosition::Top ? height : 0.0f;
    desktop.bottomReserved = kSettings.position == TaskbarPosition::Bottom ? height : 0.0f;
    desktop.showPowerButton = !kSettings.showPowerButton;
}

void RenderTaskbar(AppState& state)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = EffectiveHeight(viewport);
    if (viewport->Size.x <= 0.0f || height <= 0.0f)
    {
        return;
    }
    const bool atTop = kSettings.position == TaskbarPosition::Top;

    // Re-pinned every frame so the panel follows window resizes and can't be dragged away.
    const float panelY = atTop ? viewport->Pos.y : viewport->Pos.y + viewport->Size.y - height;
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, panelY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, height), ImGuiCond_Always);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoNav;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f));
    if (!ImGui::Begin(kTaskbarWindowName, nullptr, flags))
    {
        ImGui::End();
        ImGui::PopStyleVar(4);
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 p0 = ImGui::GetWindowPos();
    const ImVec2 p1(p0.x + viewport->Size.x, p0.y + height);

    // Draw the border on the edge facing the desktop.
    drawList->AddRectFilled(p0, p1, kSettings.panelColor);
    const float edgeY = atTop ? p1.y - 0.5f : p0.y + 0.5f;
    drawList->AddLine(ImVec2(p0.x, edgeY), ImVec2(p1.x, edgeY), kSettings.borderColor, 1.0f);

    // Place the brand and app buttons from left to right.
    ImVec2 cursor(p0.x + kSettings.padding, p0.y);
    const float brandWidth = DrawBrand(drawList, cursor, height);
    if (brandWidth > 0.0f)
    {
        cursor.x += brandWidth + 14.0f;
    }

    // Add taskbar entries here.
    const TaskbarButton buttons[] = {
        { kSettings.firstAppLabel, DrawTodoListIcon, &AppState::showFirstApp },
        { kSettings.secondAppLabel, DrawMusicPlayerIcon, &AppState::showSecondApp },
        { kSettings.taskManagerLabel, DrawTaskManagerIcon, &AppState::showTaskManager },
    };
    int runningCount = 0;
    int index = 0;
    for (const TaskbarButton& button : buttons)
    {
        bool& visible = state.*(button.visible);
        ImGui::PushID(index++);
        const float width = DrawTaskbarButton(drawList, button, visible, cursor, height);
        ImGui::PopID();
        cursor.x += width + kSettings.buttonSpacing;
        if (visible)
        {
            ++runningCount;
        }
    }

    float rightX = p1.x - kSettings.padding;
    if (kSettings.showPowerButton)
    {
        rightX -= DrawPowerButton(drawList, rightX, p0.y, height) + 14.0f;
    }
    if (kSettings.showRunningCount)
    {
        DrawRunningCount(drawList, p0, rightX, cursor.x + 8.0f, height, runningCount);
    }

    ImGui::End();
    ImGui::PopStyleVar(4);
}
