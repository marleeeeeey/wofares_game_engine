#pragma once
#include <imgui.h>

class ImGuiStyleRAII
{
    float originalAlpha;
    // Add more fields if needed.
public:
    ImGuiStyleRAII(float newAlpha);
    ~ImGuiStyleRAII();
    ImGuiStyleRAII(const ImGuiStyleRAII&) = delete;
    ImGuiStyleRAII& operator=(const ImGuiStyleRAII&) = delete;
};
