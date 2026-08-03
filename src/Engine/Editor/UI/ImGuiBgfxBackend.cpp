#include "ImGuiBgfxBackend.h"

#include <algorithm>
#include <cstdint>
#include <imgui.h>

#include "../../Platform/GraphicsDevice.h"

//ImGui's vertex is our vertex. If this ever stops holding, the reinterpret_cast below has to
//become a per-vertex copy.
static_assert(sizeof(ImDrawVert) == sizeof(ETG::GfxVertex), "ImDrawVert no longer matches ETG::GfxVertex");
static_assert(offsetof(ImDrawVert, pos) == offsetof(ETG::GfxVertex, x), "ImDrawVert position offset changed");
static_assert(offsetof(ImDrawVert, uv) == offsetof(ETG::GfxVertex, u), "ImDrawVert uv offset changed");
static_assert(offsetof(ImDrawVert, col) == offsetof(ETG::GfxVertex, rgba), "ImDrawVert colour offset changed");
static_assert(sizeof(ImDrawIdx) == sizeof(std::uint16_t), "This backend only handles 16 bit ImGui indices");

namespace
{
    //Create/update/destroy on demand, the ImGui 1.92+ way. Nothing is uploaded up front; ImGui
    //asks for exactly the regions it changed.
    void UpdateTexture(ImTextureData* tex)
    {
        if (tex->Status == ImTextureStatus_WantCreate)
        {
            IM_ASSERT(tex->Format == ImTextureFormat_RGBA32);

            //Linear sampling: ImGui bakes anti-aliased line textures that need it.
            const std::uint16_t handle = ETG::GraphicsDevice::CreateTexture2D(
                static_cast<unsigned>(tex->Width), static_cast<unsigned>(tex->Height),
                tex->GetPixels(), static_cast<unsigned>(tex->GetPitch()), true);

            if (handle == ETG::InvalidGpuHandle) return; //Retry next frame rather than draw with a bad handle

            tex->SetTexID(static_cast<ImTextureID>(handle));
            tex->SetStatus(ImTextureStatus_OK);
        }
        else if (tex->Status == ImTextureStatus_WantUpdates)
        {
            const auto handle = static_cast<std::uint16_t>(tex->TexID);
            for (const ImTextureRect& r : tex->Updates)
            {
                ETG::GraphicsDevice::UpdateTexture2D(handle,
                                                     static_cast<unsigned>(r.x), static_cast<unsigned>(r.y),
                                                     static_cast<unsigned>(r.w), static_cast<unsigned>(r.h),
                                                     tex->GetPixelsAt(r.x, r.y), static_cast<unsigned>(tex->GetPitch()));
            }
            tex->SetStatus(ImTextureStatus_OK);
        }
        else if (tex->Status == ImTextureStatus_WantDestroy)
        {
            if (tex->TexID != ImTextureID_Invalid)
                ETG::GraphicsDevice::DestroyTexture(static_cast<std::uint16_t>(tex->TexID));

            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
        }
    }
}

bool ImGui_ImplBgfx_Init()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    io.BackendRendererName = "imgui_impl_bgfx";
    //No ImGuiBackendFlags_RendererHasVtxOffset on purpose: without it ImGui keeps every draw list
    //under 64k vertices, which is exactly what the 16 bit index buffers here can address.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    return true;
}

void ImGui_ImplBgfx_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();

    //Hand every texture back to the device before the device goes away
    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
    {
        if (tex->RefCount == 1 && tex->TexID != ImTextureID_Invalid)
        {
            ETG::GraphicsDevice::DestroyTexture(static_cast<std::uint16_t>(tex->TexID));
            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
        }
    }

    io.BackendRendererName = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
}

void ImGui_ImplBgfx_NewFrame()
{
}

void ImGui_ImplBgfx_RenderDrawData(ImDrawData* drawData)
{
    if (!drawData || drawData->CmdListsCount == 0) return;
    if (!ETG::GraphicsDevice::IsInitialized()) return;

    //Catch up with whatever ImGui wants uploaded this frame (font atlas growth, mostly)
    if (drawData->Textures != nullptr)
        for (ImTextureData* tex : *drawData->Textures)
            if (tex->Status != ImTextureStatus_OK)
                UpdateTexture(tex);

    //ImGui draws in the same logical canvas the game does (Engine::Update pins io.DisplaySize to
    //RenderWindow::LogicalSize), so its vertices need no transform at all - only its clip
    //rectangles have to be converted into the backbuffer pixels bgfx scissors in.
    const ImVec2 clipOffset = drawData->DisplayPos;
    const ETG::FloatRect viewport = ETG::GraphicsDevice::GetViewportRect();

    for (const ImDrawList* drawList : drawData->CmdLists)
    {
        const auto* vertices = reinterpret_cast<const ETG::GfxVertex*>(drawList->VtxBuffer.Data);
        const auto vertexCount = static_cast<std::uint32_t>(drawList->VtxBuffer.Size);

        for (const ImDrawCmd& cmd : drawList->CmdBuffer)
        {
            if (cmd.UserCallback)
            {
                if (cmd.UserCallback != ImDrawCallback_ResetRenderState)
                    cmd.UserCallback(drawList, &cmd);
                continue;
            }

            const ETG::Vector2f clipMin = ETG::GraphicsDevice::LogicalToWindowPixel({cmd.ClipRect.x - clipOffset.x, cmd.ClipRect.y - clipOffset.y});
            const ETG::Vector2f clipMax = ETG::GraphicsDevice::LogicalToWindowPixel({cmd.ClipRect.z - clipOffset.x, cmd.ClipRect.w - clipOffset.y});

            //Clamp to the letterboxed viewport: anything outside it is not ours to draw over
            const float left = std::max(clipMin.x, viewport.left);
            const float top = std::max(clipMin.y, viewport.top);
            const float right = std::min(clipMax.x, viewport.left + viewport.width);
            const float bottom = std::min(clipMax.y, viewport.top + viewport.height);
            if (right <= left || bottom <= top) continue;

            const ETG::IntRect scissor{
                static_cast<int>(left), static_cast<int>(top),
                static_cast<int>(right - left), static_cast<int>(bottom - top)
            };

            ETG::GraphicsDevice::DrawIndexedRaw(vertices, vertexCount,
                                                drawList->IdxBuffer.Data + cmd.IdxOffset,
                                                static_cast<std::uint32_t>(cmd.ElemCount),
                                                static_cast<std::uint16_t>(cmd.GetTexID()),
                                                &scissor);
        }
    }
}
