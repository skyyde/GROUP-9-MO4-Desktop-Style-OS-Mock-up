#pragma once

#include <imgui.h>

namespace Theme
{
    inline constexpr float TaskbarHeight = 64.0f;
    inline constexpr float MinimumWidth = 800.0f;
    inline constexpr float MinimumHeight = 540.0f;
    inline const ImVec4 Accent{0.28f, 0.79f, 0.90f, 1.0f};
    inline const ImVec4 Muted{0.57f, 0.65f, 0.74f, 1.0f};
    void Apply();
}
