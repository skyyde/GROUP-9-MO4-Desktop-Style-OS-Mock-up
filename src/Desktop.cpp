#include "Desktop.h"
#include "AppState.h"
#include "imgui.h"
#include <cfloat>
#include <cmath>
#include <ctime>
#include <iostream>

namespace
{
DesktopSettings g_settings;

// shutdown sequence 
bool g_dialogRequested = false; // RequestShutdown() called
bool g_shuttingDown = false;    // shutdown confirmed followed by the shutdown screen
double g_shutdownStartTime = 0.0;

const char* const kShutdownDialogTitle = "shutdown the OS?";

struct Area
{
    ImVec2 min;
    ImVec2 max;
};

Area GetDesktopArea(const ImGuiViewport* vp)
{
    Area a;
    a.min = ImVec2(vp->Pos.x, vp->Pos.y + g_settings.topReserved);
    a.max = ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y - g_settings.bottomReserved);
    if (a.max.y < a.min.y) { // Reserved space bigger than the window: fall back to the whole window.
        a.min.y = vp->Pos.y;
        a.max.y = vp->Pos.y + vp->Size.y;
    }
    return a;
}

ImVec2 PlaceInCorner(const Area& area, DesktopCorner corner, ImVec2 itemSize)
{
    const float m = g_settings.cornerMargin;
    const bool right = corner == DesktopCorner::TopRight || corner == DesktopCorner::BottomRight;
    const bool top = corner == DesktopCorner::TopRight || corner == DesktopCorner::TopLeft;
    return ImVec2(right ? area.max.x - m - itemSize.x : area.min.x + m,
                  top ? area.min.y + m : area.max.y - m - itemSize.y);
}

void StartShutdown()
{
    if (g_shuttingDown) {
        return;
    }
    g_shuttingDown = true;
    g_shutdownStartTime = ImGui::GetTime();
    std::cout << "[PWR] Shutdown requested.\n";
}


// wallpaper (icl we can prob change later)
template <typename CurveFn>
void FillUnderCurve(ImDrawList* dl, float left, float right, float bottom, int segments,
                    ImU32 colTop, ImU32 colBottom, CurveFn curveY)
{
    const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
    const float step = (right - left) / static_cast<float>(segments);
    dl->PrimReserve(segments * 6, segments * 4);
    for (int i = 0; i < segments; ++i) {
        const float x0 = left + step * static_cast<float>(i);
        const float x1 = x0 + step;
        const auto base = static_cast<ImDrawIdx>(dl->_VtxCurrentIdx);
        dl->PrimWriteVtx(ImVec2(x0, curveY(x0)), uv, colTop);
        dl->PrimWriteVtx(ImVec2(x1, curveY(x1)), uv, colTop);
        dl->PrimWriteVtx(ImVec2(x1, bottom), uv, colBottom);
        dl->PrimWriteVtx(ImVec2(x0, bottom), uv, colBottom);
        dl->PrimWriteIdx(base);
        dl->PrimWriteIdx(static_cast<ImDrawIdx>(base + 1));
        dl->PrimWriteIdx(static_cast<ImDrawIdx>(base + 2));
        dl->PrimWriteIdx(base);
        dl->PrimWriteIdx(static_cast<ImDrawIdx>(base + 2));
        dl->PrimWriteIdx(static_cast<ImDrawIdx>(base + 3));
    }
}

// clouds
void DrawCloud(ImDrawList* dl, ImVec2 c, float s, int shade)
{
    const int g = shade + (255 - shade) / 3;
    const ImU32 col = IM_COL32(shade, g, 255, 255);
    dl->AddCircleFilled(ImVec2(c.x - 34 * s, c.y + 6 * s), 22 * s, col, 32);
    dl->AddCircleFilled(ImVec2(c.x - 8 * s, c.y - 10 * s), 30 * s, col, 32);
    dl->AddCircleFilled(ImVec2(c.x + 24 * s, c.y - 2 * s), 26 * s, col, 32);
    dl->AddCircleFilled(ImVec2(c.x + 50 * s, c.y + 10 * s), 18 * s, col, 32);
    dl->AddRectFilled(ImVec2(c.x - 34 * s, c.y + 6 * s), ImVec2(c.x + 50 * s, c.y + 28 * s), col, 14 * s);
}

void DrawLandscape(ImDrawList* dl, ImVec2 pos, ImVec2 size, float time)
{
    const float w = size.x;
    const float h = size.y;
    const float left = pos.x;
    const float right = pos.x + w;
    const float bottom = pos.y + h;

    // sky 
    dl->AddRectFilledMultiColor(pos, ImVec2(right, bottom),
                                g_settings.wallpaperTop, g_settings.wallpaperTop,
                                g_settings.wallpaperBottom, g_settings.wallpaperBottom);

    // sun 
    const ImVec2 sun(left + w * 0.80f, pos.y + h * 0.18f);
    for (int i = 14; i >= 1; --i) {
        dl->AddCircleFilled(sun, h * 0.016f * static_cast<float>(i) + h * 0.03f, IM_COL32(255, 244, 214, 6), 64);
    }
    dl->AddCircleFilled(sun, h * 0.045f, IM_COL32(255, 250, 235, 255), 48);

    // drifting clouds
    struct CloudSpec { float x, y, scale, speed; int shade; };
    const CloudSpec clouds[] = {
        { 0.10f, 0.16f, 1.30f, 9.0f, 250 },
        { 0.45f, 0.10f, 0.90f, 6.0f, 238 },
        { 0.62f, 0.30f, 1.10f, 11.0f, 246 },
        { 0.28f, 0.36f, 0.70f, 7.5f, 222 },
        { 0.88f, 0.42f, 0.80f, 8.0f, 230 },
    };
    const float drift = g_settings.animateClouds ? time : 0.0f;
    const float span = w + 320.0f;
    for (const CloudSpec& c : clouds) {
        float x = std::fmod(c.x * w + drift * c.speed, span);
        x = left - 160.0f + x;
        DrawCloud(dl, ImVec2(x, pos.y + c.y * h), c.scale * (h / 800.0f + 0.4f), c.shade);
    }

    // backhill
    FillUnderCurve(dl, left, right, bottom, 96,
                   IM_COL32(86, 150, 60, 255), IM_COL32(52, 110, 38, 255),
                   [&](float x) {
                       const float t = (x - left) / w;
                       return pos.y + h * (0.60f + 0.05f * std::sin(t * 5.0f + 1.2f) - 0.04f * t);
                   });

    // fronthill
    FillUnderCurve(dl, left, right, bottom, 96,
                   IM_COL32(126, 196, 70, 255), IM_COL32(58, 128, 30, 255),
                   [&](float x) {
                       const float t = (x - left) / w;
                       const float bump = std::exp(-((t - 0.32f) * (t - 0.32f)) / 0.09f);
                       return pos.y + h * (0.80f - 0.16f * bump + 0.03f * std::sin(t * 9.0f));
                   });
}

void DrawWallpaper(ImDrawList* dl, ImVec2 pos, ImVec2 size, float time)
{
    const ImVec2 end(pos.x + size.x, pos.y + size.y);
    switch (g_settings.wallpaper) {
    case DesktopWallpaper::Solid:
        dl->AddRectFilled(pos, end, g_settings.wallpaperTop);
        break;
    case DesktopWallpaper::Gradient:
        dl->AddRectFilledMultiColor(pos, end, g_settings.wallpaperTop, g_settings.wallpaperTop,
                                    g_settings.wallpaperBottom, g_settings.wallpaperBottom);
        break;
    case DesktopWallpaper::Landscape:
    default:
        DrawLandscape(dl, pos, size, time);
        break;
    }
}

void DrawStatusText(ImDrawList* dl, const Area& area)
{
    const char* text = g_settings.statusText;
    if (text == nullptr || text[0] == '\0') {
        return;
    }
    const float fontSize = 15.0f;
    ImFont* font = ImGui::GetFont();
    const ImVec2 at = PlaceInCorner(area, DesktopCorner::BottomLeft,
                                    font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text));
    dl->AddText(font, fontSize, ImVec2(at.x + 1, at.y + 1), IM_COL32(0, 0, 0, 140), text);
    dl->AddText(font, fontSize, at, IM_COL32(120, 255, 140, 255), text);
}

// clock fucntion
std::tm LocalTimeNow()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    return local;
}

void DrawClock(ImDrawList* dl, const Area& area)
{
    const std::tm local = LocalTimeNow();
    char text[128];
    const char* format = g_settings.clockFormat;
    if (format == nullptr || format[0] == '\0' || std::strftime(text, sizeof(text), format, &local) == 0) {
        std::strftime(text, sizeof(text), "%I:%M:%S %p", &local); // Fallback for a missing or too-long format.
    }

    // if at size 0 or below this falls back to the default size
    const float fontSize = g_settings.clockFontSize > 0.0f ? g_settings.clockFontSize : 18.0f;
    ImFont* font = ImGui::GetFont();
    const ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    const ImVec2 padding(12.0f, 7.0f);
    const ImVec2 boxSize(textSize.x + padding.x * 2, textSize.y + padding.y * 2);
    const ImVec2 boxMin = PlaceInCorner(area, g_settings.clockCorner, boxSize);
    const ImVec2 boxMax(boxMin.x + boxSize.x, boxMin.y + boxSize.y);

    dl->AddRectFilled(boxMin, boxMax, IM_COL32(15, 18, 26, 190), 6.0f);
    dl->AddRect(boxMin, boxMax, IM_COL32(255, 255, 255, 40), 6.0f);
    dl->AddText(font, fontSize, ImVec2(boxMin.x + padding.x, boxMin.y + padding.y),
                IM_COL32(235, 240, 250, 255), text);
}

// below is power button, confirmation dialog and shutdown screen
// standard power symbol
void DrawPowerIcon(ImDrawList* dl, ImVec2 c, float r, ImU32 col)
{
    const float pi = 3.14159265f;
    dl->PathArcTo(c, r, -pi * 0.5f + 0.75f, pi * 1.5f - 0.75f, 32);
    dl->PathStroke(col, ImDrawFlags_None, 2.2f);
    dl->AddLine(ImVec2(c.x, c.y - r - 2.0f), ImVec2(c.x, c.y + 1.0f), col, 2.2f);
}

// red icon with PWR there
void DrawPowerButton(const Area& area)
{
    const ImVec2 buttonSize(92.0f, 38.0f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::SetNextWindowPos(PlaceInCorner(area, DesktopCorner::BottomRight, buttonSize), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##DesktopPower", nullptr, flags);

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    if (ImGui::InvisibleButton("PWR", buttonSize)) {
        RequestShutdown();
    }
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    if (hovered) {
        ImGui::SetTooltip("Shut down CSOPESY OS");
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p1(p0.x + buttonSize.x, p0.y + buttonSize.y);
    const ImU32 bg = held ? IM_COL32(120, 20, 28, 240) : hovered ? IM_COL32(70, 18, 24, 235) : IM_COL32(18, 20, 28, 220);
    const ImU32 red = IM_COL32(255, 82, 82, 255);
    dl->AddRectFilled(p0, p1, bg, buttonSize.y * 0.5f);
    dl->AddRect(p0, p1, hovered ? red : IM_COL32(255, 82, 82, 110), buttonSize.y * 0.5f, 0, 1.5f);
    DrawPowerIcon(dl, ImVec2(p0.x + 26.0f, p0.y + buttonSize.y * 0.5f + 1.0f), 8.0f, red);
    dl->AddText(ImGui::GetFont(), 16.0f, ImVec2(p0.x + 44.0f, p0.y + (buttonSize.y - 16.0f) * 0.5f), red, "PWR");

    ImGui::End();
    ImGui::PopStyleVar();
}

// "Are you sure?" dialog when RequestShutdown() is called 
void DrawShutdownDialog(const ImGuiViewport* vp)
{
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(ImVec2(1.0f, 1.0f));
    ImGui::Begin("##DesktopDialogHost", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);
    if (g_dialogRequested) {
        ImGui::OpenPopup(kShutdownDialogTitle);
        g_dialogRequested = false;
    }

    ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(kShutdownDialogTitle, nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("All open apps will be closed.");
        ImGui::Text("Are you sure you want to shut down?");
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.18f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.25f, 0.27f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.62f, 0.12f, 0.15f, 1.0f));
        if (ImGui::Button("Shut Down", ImVec2(120, 0))) {
            StartShutdown();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

// shutdown fade to black
bool DrawShutdownScreen(const ImGuiViewport* vp)
{
    const double elapsed = ImGui::GetTime() - g_shutdownStartTime;
    float fade = static_cast<float>(elapsed / 0.5);
    if (fade > 1.0f) {
        fade = 1.0f;
    }
    const auto alpha = static_cast<int>(fade * 255.0f);

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y), IM_COL32(0, 0, 0, alpha));

    const char* title = "CSOPESY OS is shutting down...";
    const float titleSize = 24.0f;
    ImFont* font = ImGui::GetFont();
    const ImVec2 titleDim = font->CalcTextSizeA(titleSize, FLT_MAX, 0.0f, title);
    const ImVec2 center = vp->GetCenter();
    dl->AddText(font, titleSize, ImVec2(center.x - titleDim.x * 0.5f, center.y - titleDim.y),
                IM_COL32(230, 235, 245, alpha), title);

    const int dots = static_cast<int>(elapsed * 6.0) % 4;
    for (int i = 0; i < 3; ++i) {
        const ImU32 col = i < dots ? IM_COL32(120, 255, 140, alpha) : IM_COL32(80, 90, 100, alpha);
        dl->AddCircleFilled(ImVec2(center.x - 16.0f + 16.0f * static_cast<float>(i), center.y + 22.0f), 4.0f, col);
    }
    return elapsed >= g_settings.shutdownScreenSeconds;
}
} 

DesktopSettings& GetDesktopSettings()
{
    return g_settings;
}

void RequestShutdown()
{
    if (g_shuttingDown) {
        return; // already shutting down
    }
    if (g_settings.confirmShutdown) {
        g_dialogRequested = true; // opened by function DrawShutdownDialog 
    } else {
        StartShutdown();
    }
}

bool IsShuttingDown()
{
    return g_shuttingDown;
}

void RenderDesktop(AppState& state)
{
    // Currently a stub; main.cpp clears the full framebuffer to a solid color.
    // TODO: Draw the desktop background as the base layer, filling the entire
    // application window on every frame, including after a resize.
      const ImGuiViewport* vp = ImGui::GetMainViewport();
    if (vp->Size.x <= 0.0f || vp->Size.y <= 0.0f) {
        return;
    }
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const Area area = GetDesktopArea(vp);
    // TODO: Choose wallpaper: a solid color, gradient, ImGui-drawn pattern,
    // or loaded image texture. Only one of these options is required.
    DrawWallpaper(dl, vp->Pos, vp->Size, static_cast<float>(ImGui::GetTime()));
    DrawStatusText(dl, area);
    // TODO: Read the current time each frame and show a clock in a fixed corner.
    DrawClock(dl, area);

    // TODO: Add a PWR button that sets state.shutdownRequested = true.
    // Let main.cpp exit its loop and clean up normally; do not exit directly here.
    if (g_settings.showPowerButton && !g_shuttingDown) {
        DrawPowerButton(area);
    }
    DrawShutdownDialog(vp);
    if (g_shuttingDown && DrawShutdownScreen(vp)) {
        state.shutdownRequested = true;
    }
}
