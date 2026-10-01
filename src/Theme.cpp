#include "Theme.h"

void Theme::Apply()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 5.0f;
    style.ChildRounding = 7.0f;
    style.WindowPadding = ImVec2(18.0f, 18.0f);
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);
    style.WindowBorderSize = 1.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.065f, 0.09f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.045f, 0.065f, 0.10f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.17f, 0.23f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.16f, 0.23f, 0.30f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.17f, 0.23f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.28f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.36f, 0.44f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = Accent;
    style.Colors[ImGuiCol_SliderGrab] = Accent;
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.22f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.30f, 0.36f, 1.0f);
}
