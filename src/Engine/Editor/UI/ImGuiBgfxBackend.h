#pragma once

struct ImDrawData;

namespace ETG
{
    class RenderWindow;
}

//Dear ImGui renderer backend for bgfx, the replacement for imgui_impl_sdlrenderer3.
//Input still comes from imgui_impl_sdl3; only the drawing side lives here.
//
//It reuses the engine's own sprite program and vertex layout: ImDrawVert is byte for byte an
//ETG::GfxVertex, and ImGui already draws in the same fixed logical canvas the game does, so no
//separate view, projection or shader is needed.
bool ImGui_ImplBgfx_Init();
void ImGui_ImplBgfx_Shutdown();
void ImGui_ImplBgfx_NewFrame();
void ImGui_ImplBgfx_RenderDrawData(ImDrawData* drawData);
