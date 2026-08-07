#pragma once
#include <cstdint>
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"

struct SDL_Window;

namespace ETG
{
    class Texture;

    //Which fragment program a draw goes through. Everything the game draws shares one vertex
    //shader; only the fragment stage differs.
    enum class ShaderEffect : std::uint8_t
    {
        None = 0, //fs_sprite: texel * vertex colour
        Grayscale //fs_sprite_grayscale: desaturated by GraphicsDevice::SetGrayscaleAmount
    };

    //One vertex of everything the game draws. Laid out exactly like ImGui's ImDrawVert
    //(pos, uv, packed RGBA8) on purpose, so a single bgfx vertex layout serves the game and
    //the editor UI alike.
    struct GfxVertex
    {
        float x{0.f}, y{0.f}; //Logical canvas pixels (see RenderWindow::LogicalSize)
        float u{0.f}, v{0.f}; //Normalized texture coordinates
        std::uint32_t rgba{0xffffffffu}; //Bytes in memory order r, g, b, a
    };

    //Pack a Color the way GfxVertex::rgba expects it.
    constexpr std::uint32_t PackColor(const Color& c)
    {
        return static_cast<std::uint32_t>(c.r)
            | static_cast<std::uint32_t>(c.g) << 8
            | static_cast<std::uint32_t>(c.b) << 16
            | static_cast<std::uint32_t>(c.a) << 24;
    }

    //bgfx's kInvalidHandle. GPU handles are passed around as plain uint16 so that bgfx.h stays
    //out of the engine's headers.
    inline constexpr std::uint16_t InvalidGpuHandle = 0xffffu;

    //The bgfx device: one backbuffer, one orthographic view over the fixed logical canvas, and
    //the two sprite programs. Every pixel the game puts on screen goes through here.
    //
    //Coordinate spaces, from the outside in:
    //  world      -> what game objects live in; RenderWindow::worldToScreen flattens it using the active View
    //  logical    -> the fixed 1920x1080 design canvas everything is submitted in
    //  backbuffer -> real pixels; the logical canvas is letterboxed into it (GetViewportRect)
    class GraphicsDevice
    {
    public:
        //Creates the bgfx device against an existing SDL window. Safe to call once; returns false
        //if bgfx could not pick a renderer for this platform.
        static bool Init(SDL_Window* window);
        static void Shutdown();
        static bool IsInitialized();

        //Human readable name of the backend bgfx actually chose (Vulkan, OpenGL, Direct3D 11, Metal, ...)
        static const char* GetRendererName();

        //---------------- Backbuffer ----------------
        //Called on window resize with the new size in *pixels* (not points).
        static void Resize(unsigned pixelWidth, unsigned pixelHeight);
        static void SetVSyncEnabled(bool enabled);
        [[nodiscard]] static Vector2u GetBackbufferSize();

        //The letterboxed rectangle (in backbuffer pixels) the logical canvas is mapped onto.
        [[nodiscard]] static FloatRect GetViewportRect();
        //Window pixel -> logical canvas coordinate, i.e. the inverse of the letterbox transform.
        [[nodiscard]] static Vector2f WindowPixelToLogical(const Vector2f& pixel);
        //Logical canvas coordinate -> backbuffer pixel.
        [[nodiscard]] static Vector2f LogicalToWindowPixel(const Vector2f& logical);

        //---------------- Frame ----------------
        //Clears the backbuffer (black bars) and the game viewport (clearColor), then sets up the view.
        static void BeginFrame(const Color& clearColor);
        static void EndFrame();

        //---------------- Draw ----------------
        //Indexed triangle list. A null texture draws against the built-in 1x1 white texture, which
        //is how untextured geometry (shapes, bounds, lines) reuses the sprite program.
        static void DrawIndexed(const GfxVertex* vertices, std::uint32_t vertexCount,
                                const std::uint16_t* indices, std::uint32_t indexCount,
                                const Texture* texture, ShaderEffect effect = ShaderEffect::None);

        //One piece of a batched draw: an index range into a shared vertex buffer, with its own
        //raw GPU texture handle and its own scissor rectangle in backbuffer pixels. Sampling comes
        //from the flags the texture was created with.
        struct RawDrawRange
        {
            const std::uint16_t* indices{nullptr};
            std::uint32_t indexCount{0};
            std::uint16_t textureHandle{InvalidGpuHandle};
            IntRect scissorPixels{};
        };

        //Several index ranges submitted against ONE shared vertex buffer, which is uploaded once.
        //This is the entry point the ImGui backend uses: every command in an ImGui draw list
        //indexes the same vertex buffer, so calling DrawIndexedRaw per command would re-upload
        //those vertices once per command. bgfx's transient buffers are a fixed per-frame budget
        //(6 MB of vertices by default), and geometry that no longer fits is dropped silently, so
        //that quadratic cost turns into missing UI. A property-heavy editor panel reaches it
        //easily: ImGui::Columns starts a new draw command per cell.
        static void DrawIndexedRawBatched(const GfxVertex* vertices, std::uint32_t vertexCount,
                                          const RawDrawRange* ranges, std::uint32_t rangeCount);

        //Line list: vertices are consumed in pairs.
        static void DrawLines(const GfxVertex* vertices, std::uint32_t vertexCount);

        //---------------- Effects ----------------
        //0 leaves ShaderEffect::Grayscale draws untouched, 1 fully desaturates them. Default 1.
        static void SetGrayscaleAmount(float amount);
        [[nodiscard]] static float GetGrayscaleAmount();

        //---------------- Textures ----------------
        //RGBA8 upload. `pitch` is the source stride in bytes (0 = tightly packed).
        static std::uint16_t CreateTexture2D(unsigned width, unsigned height, const void* rgba, unsigned pitch, bool linearSampling);
        static void UpdateTexture2D(std::uint16_t handle, unsigned x, unsigned y, unsigned width, unsigned height, const void* rgba, unsigned pitch);
        static void DestroyTexture(std::uint16_t handle);
    };
}
