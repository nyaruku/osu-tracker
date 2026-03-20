#pragma once
#include <imgui/imgui.h>

static constexpr ImVec4 COL_BG       = { 0.051f, 0.067f, 0.090f, 1.0f };
static constexpr ImVec4 COL_SURFACE  = { 0.086f, 0.106f, 0.133f, 1.0f };
static constexpr ImVec4 COL_SURFACE2 = { 0.118f, 0.145f, 0.188f, 1.0f };
static constexpr ImVec4 COL_PRIMARY  = { 0.169f, 0.490f, 0.914f, 1.0f };
static constexpr ImVec4 COL_LINK     = { 0.302f, 0.624f, 1.000f, 1.0f };
static constexpr ImVec4 COL_TEXT     = { 0.902f, 0.929f, 0.953f, 1.0f };
static constexpr ImVec4 COL_MUTED    = { 0.545f, 0.580f, 0.620f, 1.0f };
static constexpr ImVec4 COL_BORDER   = { 1.0f,   1.0f,   1.0f,   0.07f };
static constexpr ImVec4 COL_GREEN    = { 0.20f,  0.80f,  0.40f,  1.0f };
static constexpr ImVec4 COL_RED      = { 1.0f,   0.35f,  0.35f,  1.0f };

static void applyTheme()
{
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowPadding        = ImVec2(12, 12);
    s.FramePadding         = ImVec2(8, 5);
    s.ItemSpacing          = ImVec2(6, 6);
    s.ItemInnerSpacing     = ImVec2(6, 4);
    s.ScrollbarSize        = 8.0f;
    s.WindowBorderSize     = 0.0f;
    s.FrameBorderSize      = 1.0f;
    s.FrameRounding        = 6.0f;
    s.ScrollbarRounding    = 4.0f;
    s.GrabRounding         = 4.0f;
    s.PopupRounding        = 6.0f;
    s.WindowRounding       = 0.0f;
    s.SeparatorTextPadding = ImVec2(0, 6);

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]             = COL_BG;
    c[ImGuiCol_ChildBg]              = COL_SURFACE;
    c[ImGuiCol_PopupBg]              = COL_SURFACE;
    c[ImGuiCol_Border]               = COL_BORDER;
    c[ImGuiCol_FrameBg]              = COL_SURFACE;
    c[ImGuiCol_FrameBgHovered]       = COL_SURFACE2;
    c[ImGuiCol_FrameBgActive]        = COL_SURFACE2;
    c[ImGuiCol_TitleBg]              = COL_PRIMARY;
    c[ImGuiCol_TitleBgActive]        = COL_PRIMARY;
    c[ImGuiCol_TitleBgCollapsed]     = COL_PRIMARY;
    c[ImGuiCol_ScrollbarBg]          = COL_BG;
    c[ImGuiCol_ScrollbarGrab]        = COL_SURFACE2;
    c[ImGuiCol_ScrollbarGrabHovered] = { 0.4f, 0.5f, 0.6f, 1.0f };
    c[ImGuiCol_ScrollbarGrabActive]  = COL_LINK;
    c[ImGuiCol_CheckMark]            = COL_LINK;
    c[ImGuiCol_SliderGrab]           = COL_PRIMARY;
    c[ImGuiCol_SliderGrabActive]     = COL_LINK;
    c[ImGuiCol_Button]               = COL_PRIMARY;
    c[ImGuiCol_ButtonHovered]        = COL_LINK;
    c[ImGuiCol_ButtonActive]         = COL_PRIMARY;
    c[ImGuiCol_Header]               = { COL_PRIMARY.x, COL_PRIMARY.y, COL_PRIMARY.z, 0.4f };
    c[ImGuiCol_HeaderHovered]        = { COL_PRIMARY.x, COL_PRIMARY.y, COL_PRIMARY.z, 0.6f };
    c[ImGuiCol_HeaderActive]         = COL_PRIMARY;
    c[ImGuiCol_Separator]            = COL_BORDER;
    c[ImGuiCol_SeparatorHovered]     = COL_LINK;
    c[ImGuiCol_SeparatorActive]      = COL_LINK;
    c[ImGuiCol_Text]                 = COL_TEXT;
    c[ImGuiCol_TextDisabled]         = COL_MUTED;
    c[ImGuiCol_NavHighlight]         = COL_LINK;
}