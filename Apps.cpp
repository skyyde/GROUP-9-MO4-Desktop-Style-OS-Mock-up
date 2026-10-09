
#include "Apps.h"
#include "AppState.h"
#include "Desktop.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>

namespace
{
    // ------------------------------------------------------------
    // App 2 settings
    // ------------------------------------------------------------

    struct App2Settings
    {
        const char* title = "Music Player";
        ImVec2 defaultPos = ImVec2(280.0f, 90.0f);
        ImVec2 defaultSize = ImVec2(420.0f, 400.0f);
        int defaultVolume = 70;
        bool autoplay = false;
        ImU32 accentColor = IM_COL32(120, 255, 140, 255);
    };

    App2Settings g_app2Settings;

    // ------------------------------------------------------------
    // App 2 playlist
    // ------------------------------------------------------------

    struct Track
    {
        const char* title;
        const char* artist;
        int seconds;
    };

    const Track kTracks[] =
    {
        { "Midnight Scheduler", "The Round Robins", 214 },
        { "Context Switch", "Kernel Panic", 187 },
        { "Race Condition", "Mutex & The Locks", 246 },
        { "Heap Dreams", "Segfault Sisters", 203 },
        { "Idle Loop", "Null Pointers", 172 },
    };

    const int kTrackCount =
        static_cast<int>(sizeof(kTracks) / sizeof(kTracks[0]));

    // ------------------------------------------------------------
    // App 2 runtime state
    // ------------------------------------------------------------

    int g_currentTrack = 0;
    float g_elapsed = 0.0f;
    bool g_playing = false;
    int g_volume = 70;
    bool g_wasOpen = false;

    void FormatTime(char* out, size_t size, int totalSeconds)
    {
        std::snprintf(
            out,
            size,
            "%d:%02d",
            totalSeconds / 60,
            totalSeconds % 60
        );
    }

    void SelectTrack(int index)
    {
        g_currentTrack =
            (index % kTrackCount + kTrackCount) % kTrackCount;

        g_elapsed = 0.0f;
    }

    // ------------------------------------------------------------
    // App 2 vinyl artwork
    // ------------------------------------------------------------

    void DrawVinyl(
        ImDrawList* drawList,
        ImVec2 center,
        float radius,
        float angle,
        bool playing
    )
    {
        drawList->AddCircleFilled(
            center,
            radius,
            IM_COL32(24, 26, 34, 255),
            48
        );

        for (int i = 1; i <= 3; ++i)
        {
            drawList->AddCircle(
                center,
                radius * (0.45f + 0.15f * static_cast<float>(i)),
                IM_COL32(60, 64, 78, 255),
                48,
                1.0f
            );
        }

        drawList->AddCircleFilled(
            center,
            radius * 0.32f,
            playing
                ? g_app2Settings.accentColor
                : IM_COL32(120, 128, 145, 255),
            32
        );

        drawList->AddCircleFilled(
            center,
            radius * 0.06f,
            IM_COL32(24, 26, 34, 255),
            16
        );

        const ImVec2 a(
            center.x + std::cos(angle) * radius * 0.40f,
            center.y + std::sin(angle) * radius * 0.40f
        );

        const ImVec2 b(
            center.x + std::cos(angle) * radius * 0.92f,
            center.y + std::sin(angle) * radius * 0.92f
        );

        drawList->AddLine(
            a,
            b,
            IM_COL32(255, 255, 255, 70),
            2.0f
        );
    }

    // ------------------------------------------------------------
    // App 1 placeholder
    // Replace this function when App 1 is implemented.
    // ------------------------------------------------------------

    void RenderApp1(AppState& state)
    {
        if (!state.showFirstApp)
        {
            return;
        }

        ImGui::SetNextWindowSize(
            ImVec2(420.0f, 300.0f),
            ImGuiCond_Appearing
        );

        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("App 1", &state.showFirstApp, flags))
        {
            ImGui::Text("App 1");
            ImGui::Separator();
            ImGui::TextWrapped(
                "App 1 content will be implemented separately."
            );
        }

        ImGui::End();
    }

    // ------------------------------------------------------------
    // App 2: Music Player
    // ------------------------------------------------------------

    void RenderApp2(AppState& state)
    {
        if (!state.showSecondApp)
        {
            g_wasOpen = false;
            g_playing = false;
            return;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const DesktopSettings& desktop = GetDesktopSettings();

        // Reset the player whenever the app is opened again.
        if (!g_wasOpen)
        {
            g_wasOpen = true;
            g_playing = g_app2Settings.autoplay;
            g_volume = std::clamp(
                g_app2Settings.defaultVolume,
                0,
                100
            );

            SelectTrack(0);
            ImGui::SetNextWindowFocus();
        }

        // Keep the window inside the usable desktop area.
        const float usableW =
            std::max(100.0f, viewport->Size.x);

        const float usableH =
            std::max(
                100.0f,
                viewport->Size.y
                    - desktop.topReserved
                    - desktop.bottomReserved
            );

        const ImVec2 size(
            std::min(g_app2Settings.defaultSize.x, usableW),
            std::min(g_app2Settings.defaultSize.y, usableH)
        );

        const ImVec2 pos(
            std::clamp(
                g_app2Settings.defaultPos.x,
                0.0f,
                usableW - size.x
            ),
            std::clamp(
                g_app2Settings.defaultPos.y,
                0.0f,
                usableH - size.y
            )
        );

        ImGui::SetNextWindowPos(
            ImVec2(
                viewport->Pos.x + pos.x,
                viewport->Pos.y + desktop.topReserved + pos.y
            ),
            ImGuiCond_Appearing
        );

        ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(300.0f, 260.0f),
            ImVec2(FLT_MAX, FLT_MAX)
        );

        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        // Closing the window updates the same flag used by the taskbar.
        if (ImGui::Begin(
            g_app2Settings.title,
            &state.showSecondApp,
            flags
        ))
        {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImFont* font = ImGui::GetFont();
            const float dt = ImGui::GetIO().DeltaTime;

            // Advance the simulated playback timer.
            if (g_playing)
            {
                g_elapsed += dt;

                if (g_elapsed >=
                    static_cast<float>(kTracks[g_currentTrack].seconds))
                {
                    SelectTrack(g_currentTrack + 1);
                }
            }

            const Track& track = kTracks[g_currentTrack];

            // Now-playing section.
            const ImVec2 origin = ImGui::GetCursorScreenPos();
            const float art = 92.0f;

            DrawVinyl(
                drawList,
                ImVec2(
                    origin.x + art * 0.5f,
                    origin.y + art * 0.5f
                ),
                art * 0.5f,
                static_cast<float>(ImGui::GetTime())
                    * (g_playing ? 3.0f : 0.0f),
                g_playing
            );

            drawList->AddText(
                font,
                12.0f,
                ImVec2(origin.x + art + 16.0f, origin.y + 4.0f),
                IM_COL32(150, 158, 175, 255),
                g_playing ? "NOW PLAYING" : "PAUSED"
            );

            drawList->AddText(
                font,
                24.0f,
                ImVec2(origin.x + art + 16.0f, origin.y + 22.0f),
                IM_COL32(240, 244, 252, 255),
                track.title
            );

            drawList->AddText(
                font,
                15.0f,
                ImVec2(origin.x + art + 16.0f, origin.y + 54.0f),
                IM_COL32(170, 178, 195, 255),
                track.artist
            );

            ImGui::Dummy(ImVec2(0.0f, art + 8.0f));

            // Progress bar and elapsed / total time.
            char elapsedText[16];
            char totalText[16];

            FormatTime(
                elapsedText,
                sizeof(elapsedText),
                static_cast<int>(g_elapsed)
            );

            FormatTime(
                totalText,
                sizeof(totalText),
                track.seconds
            );

            ImGui::PushStyleColor(
                ImGuiCol_PlotHistogram,
                ImGui::ColorConvertU32ToFloat4(
                    g_app2Settings.accentColor
                )
            );

            ImGui::ProgressBar(
                g_elapsed / static_cast<float>(track.seconds),
                ImVec2(-FLT_MIN, 8.0f),
                ""
            );

            ImGui::PopStyleColor();

            ImGui::TextDisabled("%s", elapsedText);

            ImGui::SameLine(
                ImGui::GetContentRegionMax().x
                    - ImGui::CalcTextSize(totalText).x
            );

            ImGui::TextDisabled("%s", totalText);

            // Previous, play / pause, and next controls.
            const ImVec2 buttonSize(76.0f, 30.0f);

            const float rowWidth =
                buttonSize.x * 3
                + ImGui::GetStyle().ItemSpacing.x * 2;

            ImGui::SetCursorPosX(
                (ImGui::GetWindowWidth() - rowWidth) * 0.5f
            );

            if (ImGui::Button("|<  Prev", buttonSize))
            {
                SelectTrack(g_currentTrack - 1);
            }

            ImGui::SameLine();

            if (ImGui::Button(
                g_playing ? "Pause" : "Play",
                buttonSize
            ))
            {
                g_playing = !g_playing;
            }

            ImGui::SameLine();

            if (ImGui::Button("Next  >|", buttonSize))
            {
                SelectTrack(g_currentTrack + 1);
            }

            ImGui::Spacing();

            ImGui::SetNextItemWidth(-60.0f);

            ImGui::SliderInt(
                "Volume",
                &g_volume,
                0,
                100,
                "%d%%"
            );

            // Playlist: selecting a track starts simulated playback.
            ImGui::Separator();

            ImGui::TextDisabled(
                "PLAYLIST  (%d tracks)",
                kTrackCount
            );

            if (ImGui::BeginChild(
                "##playlist",
                ImVec2(0.0f, 0.0f),
                ImGuiChildFlags_Borders
            ))
            {
                for (int i = 0; i < kTrackCount; ++i)
                {
                    char timeText[16];
                    char row[128];

                    FormatTime(
                        timeText,
                        sizeof(timeText),
                        kTracks[i].seconds
                    );

                    std::snprintf(
                        row,
                        sizeof(row),
                        "%02d   %s - %s",
                        i + 1,
                        kTracks[i].title,
                        kTracks[i].artist
                    );

                    ImGui::PushID(i);

                    if (ImGui::Selectable(
                        row,
                        i == g_currentTrack
                    ))
                    {
                        SelectTrack(i);
                        g_playing = true;
                    }

                    ImGui::SameLine(
                        ImGui::GetContentRegionMax().x
                            - ImGui::CalcTextSize(timeText).x
                    );

                    ImGui::TextDisabled("%s", timeText);

                    ImGui::PopID();
                }
            }

            ImGui::EndChild();
        }

        ImGui::End();

        // Stop playback immediately if the X button was clicked.
        if (!state.showSecondApp)
        {
            g_wasOpen = false;
            g_playing = false;
        }
    }
}

// ------------------------------------------------------------
// Shared application renderer
// ------------------------------------------------------------

void RenderApps(AppState& state)
{
    RenderApp1(state);
    RenderApp2(state);
}