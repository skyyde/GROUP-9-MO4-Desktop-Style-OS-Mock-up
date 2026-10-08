#include "Taskbar.h"
#include "AppState.h"
#include "Desktop.h"
#include "imgui.h"
#include <algorithm>
#include <cfloat>
#include <cstdio>

namespace
{
TaskbarSettings g_settings;

const char* const kTaskbarWindowName = "##Taskbar";

// Taskbar thickness actually used this frame: never taller than the window itself.
float EffectiveHeight(const ImGuiViewport* vp)
{
    return std::max(0.0f, std::min(g_settings.height, vp->Size.y));
}

// icons: drawn with ImGui draw commands so no image assets are needed.
// c = icon center, s = icon box size in pixels.

// App 1: a window with a title bar and content lines.
void DrawIconWindow(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    const float h = s * 0.5f;
    const ImVec2 a(c.x - h, c.y - h * 0.85f), b(c.x + h, c.y + h * 0.85f);
    dl->AddRect(a, b, col, 3.0f, 0, 1.6f);
    dl->AddLine(ImVec2(a.x, a.y + s * 0.28f), ImVec2(b.x, a.y + s * 0.28f), col, 1.6f);
    dl->AddLine(ImVec2(a.x + s * 0.18f, c.y + s * 0.08f), ImVec2(b.x - s * 0.18f, c.y + s * 0.08f), col, 1.4f);
    dl->AddLine(ImVec2(a.x + s * 0.18f, c.y + s * 0.26f), ImVec2(c.x + s * 0.08f, c.y + s * 0.26f), col, 1.4f);
}

// App 2: a 2x2 grid of tiles.
void DrawIconGrid(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    const float gap = s * 0.12f;
    const float tile = (s - gap) * 0.5f - s * 0.04f;
    for (int row = 0; row < 2; ++row) {
        for (int colIdx = 0; colIdx < 2; ++colIdx) {
            const float x = c.x - gap * 0.5f - tile + (tile + gap) * static_cast<float>(colIdx);
            const float y = c.y - gap * 0.5f - tile + (tile + gap) * static_cast<float>(row);
            dl->AddRectFilled(ImVec2(x, y), ImVec2(x + tile, y + tile), col, 2.5f);
        }
    }
}

// Task Manager: activity bars.
void DrawIconActivity(ImDrawList* dl, ImVec2 c, float s, ImU32 col)
{
    const float barW = s * 0.18f;
    const float gap = s * 0.10f;
    const float heights[] = { 0.45f, 0.85f, 0.60f, 1.00f };
    const float total = 4 * barW + 3 * gap;
    const float bottom = c.y + s * 0.5f;
    for (int i = 0; i < 4; ++i) {
        const float x = c.x - total * 0.5f + (barW + gap) * static_cast<float>(i);
        dl->AddRectFilled(ImVec2(x, bottom - s * heights[i]), ImVec2(x + barW, bottom), col, 1.5f);
    }
}

using IconFn = void (*)(ImDrawList*, ImVec2, float, ImU32);

// One taskbar entry. `flag` points at the shared visibility flag in AppState,
// so the button, the app window, and the running indicator can never disagree.
struct TaskbarButton
{
    const char* label;
    IconFn drawIcon;
    bool AppState::* flag;
};

// Pill-shaped, non-clickable brand label on the left of the panel.
float DrawBrand(ImDrawList* dl, ImVec2 origin, float panelHeight)
{
    const char* text = g_settings.brandText;
    if (text == nullptr || text[0] == '\0') {
        return 0.0f;
    }
    const float fontSize = 15.0f;
    ImFont* font = ImGui::GetFont();
    const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    const ImVec2 pill(textSize.x + 28.0f, 28.0f);
    const ImVec2 p0(origin.x, origin.y + (panelHeight - pill.y) * 0.5f);
    const ImVec2 p1(p0.x + pill.x, p0.y + pill.y);
    dl->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 18), pill.y * 0.5f);
    dl->AddCircleFilled(ImVec2(p0.x + 12.0f, p0.y + pill.y * 0.5f), 4.0f, g_settings.accentColor);
    dl->AddText(font, fontSize, ImVec2(p0.x + 22.0f, p0.y + (pill.y - textSize.y) * 0.5f),
                IM_COL32(235, 240, 250, 255), text);
    return pill.x;
}

// Draws one icon button; returns true when it was clicked this frame.
bool DrawTaskbarButton(ImDrawList* dl, const TaskbarButton& btn, bool running, ImVec2 pos, float panelHeight,
                       float* outWidth)
{
    const float iconSize = 18.0f;
    const float fontSize = 15.0f;
    const bool hasLabel = g_settings.showLabels && btn.label != nullptr && btn.label[0] != '\0';
    ImFont* font = ImGui::GetFont();
    const float labelW = hasLabel ? font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, btn.label).x : 0.0f;
    const float sidePad = 12.0f;
    const float width = sidePad * 2 + iconSize + (hasLabel ? 8.0f + labelW : 0.0f);
    const ImVec2 size(width, g_settings.buttonHeight);
    const ImVec2 p0(pos.x, pos.y + (panelHeight - size.y) * 0.5f);
    const ImVec2 p1(p0.x + size.x, p0.y + size.y);
    *outWidth = width;

    ImGui::SetCursorScreenPos(p0);
    const bool clicked = ImGui::InvisibleButton("##btn", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();

    // Tooltip tells the person what the click will do.
    const char* name = (btn.label != nullptr && btn.label[0] != '\0') ? btn.label : "App";
    if (running) {
        ImGui::SetItemTooltip("%s (running) - click to close", name);
    } else {
        ImGui::SetItemTooltip("Open %s", name);
    }

    ImU32 bg = IM_COL32(255, 255, 255, 0);
    if (held) {
        bg = IM_COL32(255, 255, 255, 40);
    } else if (hovered) {
        bg = IM_COL32(255, 255, 255, 28);
    } else if (running) {
        bg = IM_COL32(255, 255, 255, 14);
    }
    dl->AddRectFilled(p0, p1, bg, 7.0f);

    const ImU32 fg = running || hovered ? IM_COL32(245, 248, 255, 255) : IM_COL32(190, 198, 212, 255);
    btn.drawIcon(dl, ImVec2(p0.x + sidePad + iconSize * 0.5f, p0.y + size.y * 0.5f), iconSize, fg);
    if (hasLabel) {
        dl->AddText(font, fontSize, ImVec2(p0.x + sidePad + iconSize + 8.0f, p0.y + (size.y - fontSize) * 0.5f),
                    fg, btn.label);
    }

    // Running indicator: short accent bar along the bottom edge of the button.
    if (running) {
        const float barW = std::min(22.0f, size.x * 0.5f);
        const float cx = (p0.x + p1.x) * 0.5f;
        dl->AddRectFilled(ImVec2(cx - barW * 0.5f, p1.y - 3.0f), ImVec2(cx + barW * 0.5f, p1.y - 1.0f),
                          g_settings.accentColor, 2.0f);
    }
    return clicked;
}
} // namespace

TaskbarSettings& GetTaskbarSettings()
{
    return g_settings;
}

void ApplyTaskbarLayout()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float h = EffectiveHeight(vp);
    DesktopSettings& desktop = GetDesktopSettings();
    desktop.topReserved = g_settings.position == TaskbarPosition::Top ? h : 0.0f;
    desktop.bottomReserved = g_settings.position == TaskbarPosition::Bottom ? h : 0.0f;
}

void RenderTaskbar(AppState& state)
{
    // Keep the desktop's clock/PWR/status text clear of this panel. The desktop is drawn
    // earlier in the frame, so it picks this up from the next frame on (one frame at startup).
    ApplyTaskbarLayout();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float h = EffectiveHeight(vp);
    if (vp->Size.x <= 0.0f || h <= 0.0f) {
        return;
    }
    const bool atTop = g_settings.position == TaskbarPosition::Top;

    // Re-pinned every frame so the panel follows window resizes and can't be dragged away.
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, atTop ? vp->Pos.y : vp->Pos.y + vp->Size.y - h), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, h), ImGuiCond_Always);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoNav;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.0f, 1.0f)); // allow a thin taskbar in a tiny window
    ImGui::Begin(kTaskbarWindowName, nullptr, flags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p0 = ImGui::GetWindowPos();
    const ImVec2 p1(p0.x + vp->Size.x, p0.y + h);

    // panel + a thin border on the edge that faces the desktop
    dl->AddRectFilled(p0, p1, g_settings.panelColor);
    const float edgeY = atTop ? p1.y - 0.5f : p0.y + 0.5f;
    dl->AddLine(ImVec2(p0.x, edgeY), ImVec2(p1.x, edgeY), g_settings.borderColor, 1.0f);

    // left: brand, then the icon buttons
    ImVec2 cursor(p0.x + g_settings.padding, p0.y);
    const float brandW = DrawBrand(dl, cursor, h);
    if (brandW > 0.0f) {
        cursor.x += brandW + 14.0f;
    }

    // The three required buttons. Each toggles its own shared flag in AppState.
    const TaskbarButton buttons[] = {
        { g_settings.firstAppLabel, DrawIconWindow, &AppState::showFirstApp },
        { g_settings.secondAppLabel, DrawIconGrid, &AppState::showSecondApp },
        { g_settings.taskManagerLabel, DrawIconActivity, &AppState::showTaskManager },
    };
    int runningCount = 0;
    int index = 0;
    for (const TaskbarButton& btn : buttons) {
        bool& flag = state.*(btn.flag);
        float width = 0.0f;
        ImGui::PushID(index++);
        if (DrawTaskbarButton(dl, btn, flag, cursor, h, &width)) {
            flag = !flag; // closed -> open, open -> closed
        }
        ImGui::PopID();
        cursor.x += width + g_settings.buttonSpacing;
        if (flag) { // read after the click so the count never lags a frame
            ++runningCount;
        }
    }

    // right: running-app summary
    if (g_settings.showRunningCount) {
        char text[32];
        if (runningCount == 0) {
            std::snprintf(text, sizeof(text), "No apps running");
        } else {
            std::snprintf(text, sizeof(text), "%d app%s running", runningCount, runningCount == 1 ? "" : "s");
        }
        const float fontSize = 14.0f;
        ImFont* font = ImGui::GetFont();
        const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
        dl->AddText(font, fontSize, ImVec2(p1.x - g_settings.padding - textSize.x, p0.y + (h - textSize.y) * 0.5f),
                    runningCount > 0 ? g_settings.accentColor : IM_COL32(140, 148, 162, 255), text);
    }

    ImGui::End();
    ImGui::PopStyleVar(4);
}
