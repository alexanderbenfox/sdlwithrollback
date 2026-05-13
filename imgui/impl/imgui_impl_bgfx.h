#pragma once
#include "../imgui.h"
#include <cstdint>

// bgfx ImGui backend
bool ImGui_ImplBgfx_Init(uint16_t viewId);
void ImGui_ImplBgfx_Shutdown();
void ImGui_ImplBgfx_NewFrame();
void ImGui_ImplBgfx_RenderDrawData(ImDrawData* drawData);
