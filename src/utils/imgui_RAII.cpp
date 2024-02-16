#include "imgui_RAII.h"

ImGuiStyleRAII::ImGuiStyleRAII(float newAlpha) : originalAlpha(ImGui::GetStyle().Alpha)
{
    ImGui::GetStyle().Alpha = newAlpha;
}

ImGuiStyleRAII::~ImGuiStyleRAII()
{
    ImGui::GetStyle().Alpha = originalAlpha;
}
