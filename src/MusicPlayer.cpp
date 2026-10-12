#include "MusicPlayer.h"
#include "Apps.h"
#include "AppState.h"
#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <iterator>

namespace
{
    // Edit these defaults to change the window and initial playback controls.
    struct MusicPlayerSettings
    {
        const char* title = "Music Player";
        ImVec2 defaultPosition = ImVec2(500.0f, 70.0f);
        ImVec2 defaultSize = ImVec2(460.0f, 450.0f);
        int defaultVolume = 70;
        bool autoplay = false;
        ImU32 accentColor = IM_COL32(120, 255, 140, 255);
    };

    struct Track
    {
        const char* title;
        const char* artist;
        int seconds;
    };

    // Mock playlist; durations are in seconds. Playback does not output audio.
    constexpr Track kTracks[] = {
        { "Midnight Scheduler", "The Round Robins", 214 },
        { "Context Switch", "Kernel Panic", 187 },
        { "Race Condition", "Mutex & The Locks", 246 },
        { "Heap Dreams", "Segfault Sisters", 203 },
        { "Idle Loop", "Null Pointers", 172 },
    };
    constexpr int kTrackCount = static_cast<int>(std::size(kTracks));

    struct PlaybackState
    {
        AppWindowState window;
        int currentTrack = 0;
        float elapsed = 0.0f;
        bool playing = false;
        int volume = 0;
        bool wasOpen = false;
    };

    const MusicPlayerSettings kSettings;
    PlaybackState g_player;

    void FormatTime(char* output, size_t size, int seconds)
    {
        std::snprintf(output, size, "%d:%02d", seconds / 60, seconds % 60);
    }

    void SelectTrack(int index)
    {
        // Previous and Next wrap at either end of the playlist.
        g_player.currentTrack = (index % kTrackCount + kTrackCount) % kTrackCount;
        g_player.elapsed = 0.0f;
    }

    void UpdatePlayback(float deltaTime)
    {
        if (!g_player.playing)
        {
            return;
        }

        g_player.elapsed += deltaTime;
        if (g_player.elapsed >= static_cast<float>(kTracks[g_player.currentTrack].seconds))
        {
            SelectTrack(g_player.currentTrack + 1);
        }
    }

    void DrawVinyl(ImDrawList* drawList, ImVec2 center, float radius, float angle)
    {
        const ImU32 recordColor = IM_COL32(24, 26, 34, 255);
        drawList->AddCircleFilled(center, radius, recordColor, 48);
        for (int ring = 1; ring <= 3; ++ring)
        {
            const float ringRadius = radius * (0.45f + 0.15f * static_cast<float>(ring));
            drawList->AddCircle(center, ringRadius, IM_COL32(60, 64, 78, 255), 48, 1.0f);
        }

        const ImU32 labelColor = g_player.playing
            ? kSettings.accentColor : IM_COL32(120, 128, 145, 255);
        drawList->AddCircleFilled(center, radius * 0.32f, labelColor, 32);
        drawList->AddCircleFilled(center, radius * 0.06f, recordColor, 16);

        const ImVec2 inner(
            center.x + std::cos(angle) * radius * 0.40f,
            center.y + std::sin(angle) * radius * 0.40f);
        const ImVec2 outer(
            center.x + std::cos(angle) * radius * 0.92f,
            center.y + std::sin(angle) * radius * 0.92f);
        drawList->AddLine(inner, outer, IM_COL32(255, 255, 255, 70), 2.0f);
    }

    void RenderNowPlaying(const Track& track)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImFont* font = ImGui::GetFont();
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float artworkSize = 92.0f;
        const float radius = artworkSize * 0.5f;
        const float textX = origin.x + artworkSize + 16.0f;
        const float angle = static_cast<float>(ImGui::GetTime())
            * (g_player.playing ? 3.0f : 0.0f);

        DrawVinyl(drawList, ImVec2(origin.x + radius, origin.y + radius), radius, angle);
        drawList->AddText(font, 12.0f, ImVec2(textX, origin.y + 4.0f),
            IM_COL32(150, 158, 175, 255), g_player.playing ? "NOW PLAYING" : "PAUSED");
        drawList->AddText(font, 24.0f, ImVec2(textX, origin.y + 22.0f),
            IM_COL32(240, 244, 252, 255), track.title);
        drawList->AddText(font, 15.0f, ImVec2(textX, origin.y + 54.0f),
            IM_COL32(170, 178, 195, 255), track.artist);
        ImGui::Dummy(ImVec2(0.0f, artworkSize + 8.0f));
    }

    void RenderProgress(const Track& track)
    {
        char elapsedText[16];
        char totalText[16];
        FormatTime(elapsedText, sizeof(elapsedText), static_cast<int>(g_player.elapsed));
        FormatTime(totalText, sizeof(totalText), track.seconds);

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, kSettings.accentColor);
        ImGui::ProgressBar(
            g_player.elapsed / static_cast<float>(track.seconds), ImVec2(-FLT_MIN, 8.0f), "");
        ImGui::PopStyleColor();

        ImGui::TextDisabled("%s", elapsedText);
        ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(totalText).x);
        ImGui::TextDisabled("%s", totalText);
    }

    void RenderPlaybackControls()
    {
        const ImVec2 buttonSize(76.0f, 30.0f);
        const float rowWidth = buttonSize.x * 3 + ImGui::GetStyle().ItemSpacing.x * 2;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - rowWidth) * 0.5f);

        if (ImGui::Button("|<  Prev", buttonSize))
        {
            SelectTrack(g_player.currentTrack - 1);
        }
        ImGui::SameLine();
        if (ImGui::Button(g_player.playing ? "Pause" : "Play", buttonSize))
        {
            g_player.playing = !g_player.playing;
        }
        ImGui::SameLine();
        if (ImGui::Button("Next  >|", buttonSize))
        {
            SelectTrack(g_player.currentTrack + 1);
        }

        ImGui::Spacing();
        ImGui::SetNextItemWidth(-60.0f);
        ImGui::SliderInt("Volume", &g_player.volume, 0, 100, "%d%%");
    }

    void RenderPlaylist()
    {
        ImGui::Separator();
        ImGui::TextDisabled("PLAYLIST  (%d tracks)", kTrackCount);
        if (ImGui::BeginChild("##playlist", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders))
        {
            for (int index = 0; index < kTrackCount; ++index)
            {
                const Track& track = kTracks[index];
                char timeText[16];
                char row[128];
                FormatTime(timeText, sizeof(timeText), track.seconds);
                std::snprintf(row, sizeof(row), "%02d   %s - %s",
                    index + 1, track.title, track.artist);

                ImGui::PushID(index);
                if (ImGui::Selectable(row, index == g_player.currentTrack))
                {
                    SelectTrack(index);
                    g_player.playing = true;
                }
                ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(timeText).x);
                ImGui::TextDisabled("%s", timeText);
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }
} // namespace

void RenderMusicPlayer(AppState& state)
{
    if (!state.showSecondApp)
    {
        g_player.wasOpen = false;
        g_player.playing = false;
        return;
    }

    // Reopening starts at the first track with the configured volume and autoplay.
    if (!g_player.wasOpen)
    {
        SelectTrack(0);
        g_player.volume = std::clamp(kSettings.defaultVolume, 0, 100);
        g_player.playing = kSettings.autoplay;
        ImGui::SetNextWindowFocus();
    }

    const ImGuiWindowFlags flags = ConfigureAppWindow(
        g_player.window, kSettings.defaultPosition, kSettings.defaultSize);
    if (ImGui::Begin(kSettings.title, &state.showSecondApp, flags))
    {
        RenderAppTitleBarControls(g_player.window);
        UpdatePlayback(ImGui::GetIO().DeltaTime);
        const Track& track = kTracks[g_player.currentTrack];
        RenderNowPlaying(track);
        RenderProgress(track);
        RenderPlaybackControls();
        RenderPlaylist();
    }
    ImGui::End();

    g_player.wasOpen = state.showSecondApp;
    if (!state.showSecondApp)
    {
        g_player.playing = false;
    }
}
