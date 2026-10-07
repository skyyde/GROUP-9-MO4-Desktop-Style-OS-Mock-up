#pragma once

struct AppState;

// Called first each frame; currently a stub.
// TODO: Full-window wallpaper, a fixed-corner clock, and a PWR shutdown button.
void RenderDesktop(AppState& state);

#include "imgui.h"

enum class DesktopWallpaper
{
    Landscape, 
    Gradient,  
    Solid      
};

enum class DesktopCorner
{
    TopRight,
    TopLeft,
    BottomRight,
    BottomLeft
};

struct DesktopSettings
{
    // wallpaper
    DesktopWallpaper wallpaper = DesktopWallpaper::Landscape;
    ImU32 wallpaperTop = IM_COL32(28, 92, 196, 255);     // Gradient top / solid color.
    ImU32 wallpaperBottom = IM_COL32(150, 199, 245, 255); // Gradient bottom.
    bool animateClouds = true;

    // to make way for taskbar, should have space at the bottom 
    float topReserved = 0.0f;
    float bottomReserved = 0.0f;
    float cornerMargin = 14.0f;

    // real-time clock
    DesktopCorner clockCorner = DesktopCorner::TopRight;
    const char* clockFormat = "%A, %b %d, %Y | %I:%M:%S %p";
    float clockFontSize = 18.0f;

    // status
    const char* statusText = "CSOPESY OS - group 9 ";

    // power
    bool showPowerButton = true;
    bool confirmShutdown = true;        // Ask "Are you sure?" first.
    double shutdownScreenSeconds = 1.6; // How long "Shutting down..." shows.
};

// desktop setting object
DesktopSettings& GetDesktopSettings();

// power shutdown sequence
void RequestShutdown();

// confirm shutdown, followed by true shutdown (atleast it should pls lmk if it doesn't)
bool IsShuttingDown();