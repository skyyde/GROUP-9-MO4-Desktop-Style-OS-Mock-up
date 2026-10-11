#include "Apps.h"
#include "Desktop.h"
#include "TodoList.h"
#include "MusicPlayer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>

ImGuiWindowFlags ConfigureAppWindow(AppWindowState& window,
    const ImVec2& defaultPosition, const ImVec2& defaultSize)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const DesktopSettings& desktop = GetDesktopSettings();
    const float usableWidth = std::max(1.0f, viewport->Size.x);
    const float usableHeight = std::max(
        1.0f, viewport->Size.y - desktop.topReserved - desktop.bottomReserved);
    const ImVec2 minimumSize(std::min(300.0f, usableWidth), std::min(260.0f, usableHeight));
    const ImVec2 desktopOrigin(viewport->Pos.x, viewport->Pos.y + desktop.topReserved);
    ImVec2 size(
        std::clamp(defaultSize.x, minimumSize.x, usableWidth),
        std::clamp(defaultSize.y, minimumSize.y, usableHeight));
    ImVec2 position(
        viewport->Pos.x + std::clamp(defaultPosition.x, 0.0f, usableWidth - size.x),
        viewport->Pos.y + desktop.topReserved
            + std::clamp(defaultPosition.y, 0.0f, usableHeight - size.y));

    ImGuiCond condition = ImGuiCond_Appearing;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    if (window.maximized)
    {
        position = desktopOrigin;
        size = ImVec2(usableWidth, usableHeight);
        condition = ImGuiCond_Always;
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }
    else if (window.restorePending)
    {
        // Clamp saved bounds if the desktop was resized while maximized.
        size = ImVec2(
            std::clamp(window.restoreSize.x, minimumSize.x, usableWidth),
            std::clamp(window.restoreSize.y, minimumSize.y, usableHeight));
        position = ImVec2(
            std::clamp(window.restorePosition.x, desktopOrigin.x,
                desktopOrigin.x + usableWidth - size.x),
            std::clamp(window.restorePosition.y, desktopOrigin.y,
                desktopOrigin.y + usableHeight - size.y));
        condition = ImGuiCond_Always;
        window.restorePending = false;
    }

    ImGui::SetNextWindowPos(position, condition);
    ImGui::SetNextWindowSize(size, condition);
    ImGui::SetNextWindowSizeConstraints(minimumSize, ImVec2(usableWidth, usableHeight));
    return flags;
}

void RenderAppTitleBarControls(AppWindowState& window)
{
    ImGuiWindow* currentWindow = ImGui::GetCurrentWindow();
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImRect titleBar = currentWindow->TitleBarRect();
    const float buttonSize = ImGui::GetFontSize();
    // Match ImGui's close-button spacing, one slot to its left.
    const ImVec2 position(
        titleBar.Max.x - currentWindow->WindowBorderSize - style.FramePadding.x
            - buttonSize * 2.0f - style.ItemInnerSpacing.x,
        titleBar.Min.y + style.FramePadding.y);
    const ImRect bounds(position, ImVec2(position.x + buttonSize, position.y + buttonSize));
    const ImGuiID id = currentWindow->GetID("##Maximize");

    // Title-bar widgets use the menu navigation layer and must bypass content clipping.
    // ItemAdd/ButtonBehavior avoid changing the app's content cursor or scroll extent.
    ImRect clip = titleBar;
    clip.ClipWith(currentWindow->OuterRectClipped);
    ImGui::PushClipRect(clip.Min, clip.Max, false);
    const ImGuiNavLayer previousLayer = currentWindow->DC.NavLayerCurrent;
    currentWindow->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
    ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
    bool hovered = false;
    bool held = false;
    bool pressed = false;
    if (ImGui::ItemAdd(bounds, id))
    {
        pressed = ImGui::ButtonBehavior(bounds, id, &hovered, &held);
        ImDrawList* drawList = currentWindow->DrawList;
        if (hovered || held)
        {
            drawList->AddRectFilled(bounds.Min, bounds.Max,
                ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered));
        }
        ImGui::RenderNavCursor(bounds, id, ImGuiNavRenderCursorFlags_Compact);

        const float inset = buttonSize * 0.2f;
        const ImVec2 iconMin(position.x + inset, position.y + inset);
        const ImVec2 iconMax(position.x + buttonSize - inset, position.y + buttonSize - inset);
        const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        if (window.maximized)
        {
            const float offset = buttonSize * 0.2f;
            drawList->AddLine(ImVec2(iconMin.x + offset, iconMin.y),
                ImVec2(iconMax.x, iconMin.y), color);
            drawList->AddLine(ImVec2(iconMax.x, iconMin.y),
                ImVec2(iconMax.x, iconMax.y - offset), color);
            drawList->AddRect(ImVec2(iconMin.x, iconMin.y + offset),
                ImVec2(iconMax.x - offset, iconMax.y), color);
        }
        else
        {
            drawList->AddRect(iconMin, iconMax, color);
        }
        ImGui::SetItemTooltip("%s", window.maximized ? "Restore" : "Maximize");
    }
    ImGui::PopItemFlag();
    currentWindow->DC.NavLayerCurrent = previousLayer;
    ImGui::PopClipRect();

    if (pressed)
    {
        if (window.maximized)
        {
            window.maximized = false;
            window.restorePending = true;
        }
        else
        {
            window.restorePosition = ImGui::GetWindowPos();
            window.restoreSize = ImGui::GetWindowSize();
            window.maximized = true;
        }
    }
}

void RenderApps(AppState& state)
{
    RenderTodoList(state);
    RenderMusicPlayer(state);
}
