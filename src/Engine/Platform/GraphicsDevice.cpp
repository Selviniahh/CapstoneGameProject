#include "GraphicsDevice.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>
#include <bx/math.h>

#include "RenderWindow.h"
#include "Texture.h"
#include "../Managers/AssetManager.h"

#if defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
#include <SDL3/SDL_metal.h>
#endif

namespace ETG
{
    namespace
    {
        //---------------- Views ----------------
        //0 covers the whole backbuffer and only paints the letterbox bars black.
        //1 is the game: the letterboxed viewport with an orthographic projection over the logical
        //canvas. Sprites, shapes, text and the editor UI all submit into it, in submission order.
        constexpr bgfx::ViewId BackdropView = 0;
        constexpr bgfx::ViewId SceneView = 1;

        //Alpha blended 2D. No depth test: draw order is resolved on the CPU (SpriteBatch sorts by
        //depth, the view is Sequential) exactly as it was under SDL's 2D renderer.
        constexpr std::uint64_t State2D = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA;

        struct DeviceState
        {
            bool initialized = false;
            SDL_Window* window = nullptr;

            unsigned backbufferWidth = 0;
            unsigned backbufferHeight = 0;
            std::uint32_t resetFlags = BGFX_RESET_NONE;

            bgfx::VertexLayout layout;
            bgfx::ProgramHandle spriteProgram = BGFX_INVALID_HANDLE;
            bgfx::ProgramHandle grayscaleProgram = BGFX_INVALID_HANDLE;
            bgfx::UniformHandle texColorUniform = BGFX_INVALID_HANDLE;
            bgfx::UniformHandle effectParamsUniform = BGFX_INVALID_HANDLE;
            bgfx::TextureHandle whiteTexture = BGFX_INVALID_HANDLE;

            float grayscaleAmount = 1.0f;

#if defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
            SDL_MetalView metalView = nullptr;
#endif
        };

        DeviceState& Device()
        {
            static DeviceState state;
            return state;
        }

        //---------------- Platform data ----------------
        //Hand bgfx the native surface SDL created for us. Every platform SDL3 supports is wired
        //here; which branch compiles is decided by SDL's own platform macros.
        bgfx::PlatformData BuildPlatformData(SDL_Window* window)
        {
            bgfx::PlatformData pd{};
            const SDL_PropertiesID props = SDL_GetWindowProperties(window);

#if defined(SDL_PLATFORM_WIN32)
            pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
            //bgfx renders through Metal on Apple platforms, so it wants the CAMetalLayer rather
            //than the NSWindow/UIWindow. SDL builds one for us and keeps owning it.
            Device().metalView = SDL_Metal_CreateView(window);
            pd.nwh = Device().metalView ? SDL_Metal_GetLayer(Device().metalView) : nullptr;
            if (!pd.nwh)
            {
#if defined(SDL_PLATFORM_MACOS)
                pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#else
                pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr);
#endif
            }
#elif defined(SDL_PLATFORM_ANDROID)
            pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
#elif defined(SDL_PLATFORM_EMSCRIPTEN)
            //On the web the "native window handle" is the CSS selector of the canvas SDL owns.
            //The string has to outlive init, hence the static.
            static std::string canvasSelector;
            const char* canvasId = SDL_GetStringProperty(props, SDL_PROP_WINDOW_EMSCRIPTEN_CANVAS_ID_STRING, "#canvas");
            canvasSelector = canvasId;
            if (!canvasSelector.empty() && canvasSelector[0] != '#')
                canvasSelector.insert(canvasSelector.begin(), '#');
            pd.nwh = const_cast<char*>(canvasSelector.c_str());
#else
            //Linux/BSD: SDL may be running on either display server, and bgfx needs to be told which.
            const char* videoDriver = SDL_GetCurrentVideoDriver();
            if (videoDriver && SDL_strcmp(videoDriver, "wayland") == 0)
            {
                pd.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
                pd.nwh = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
                pd.type = bgfx::NativeWindowHandleType::Wayland;
            }
            else
            {
                pd.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
                pd.nwh = reinterpret_cast<void*>(SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
                pd.type = bgfx::NativeWindowHandleType::Default;
            }
#endif
            return pd;
        }

        //---------------- Shaders ----------------
        //Which precompiled flavour of the shaders this backend needs. Mirrors the directory names
        //shaderc writes (and the ones bgfx's own runtime/shaders folder uses).
        const char* ShaderProfileDir()
        {
            switch (bgfx::getRendererType())
            {
            case bgfx::RendererType::Noop:
            case bgfx::RendererType::Direct3D11: return "dxbc";
            case bgfx::RendererType::Direct3D12: return "dxil";
            case bgfx::RendererType::Agc:
            case bgfx::RendererType::Gnm: return "pssl";
            case bgfx::RendererType::Metal: return "metal";
            case bgfx::RendererType::Nvn: return "nvn";
            case bgfx::RendererType::OpenGL: return "glsl";
            case bgfx::RendererType::OpenGLES: return "essl";
            case bgfx::RendererType::Vulkan: return "spirv";
            default: return "glsl";
            }
        }

        bgfx::ShaderHandle LoadShader(const std::string& name)
        {
            const std::string relative = std::string("Shaders/") + ShaderProfileDir() + "/" + name + ".sc.bin";
            const std::vector<unsigned char> bytes = AssetManager::LoadBytes(relative);
            if (bytes.empty())
                throw std::runtime_error("Failed to load shader: " + relative +
                    " (run the build with ETG_COMPILE_SHADERS=ON, or check in the compiled shaders for this backend)");

            const bgfx::Memory* mem = bgfx::copy(bytes.data(), static_cast<std::uint32_t>(bytes.size()));
            const bgfx::ShaderHandle handle = bgfx::createShader(mem);
            if (!bgfx::isValid(handle))
                throw std::runtime_error("bgfx rejected shader: " + relative);

            bgfx::setName(handle, name.c_str());
            return handle;
        }

        void CreateDeviceObjects()
        {
            DeviceState& d = Device();

            //Position, UV, packed RGBA8 - byte for byte an ImGui ImDrawVert, so the editor UI and
            //the game share one layout and one program.
            d.layout.begin()
             .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
             .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
             .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
             .end();

            const bgfx::ShaderHandle vs = LoadShader("vs_sprite");
            d.spriteProgram = bgfx::createProgram(vs, LoadShader("fs_sprite"), true);
            //The vertex shader is shared, so it must not be destroyed with the first program.
            d.grayscaleProgram = bgfx::createProgram(LoadShader("vs_sprite"), LoadShader("fs_sprite_grayscale"), true);

            d.texColorUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
            d.effectParamsUniform = bgfx::createUniform("u_effectParams", bgfx::UniformType::Vec4);

            //1x1 opaque white: lets untextured geometry (shapes, lines, bounds) reuse the sprite program.
            constexpr std::uint32_t whitePixel = 0xffffffffu;
            d.whiteTexture = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8,
                                                   BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP,
                                                   bgfx::copy(&whitePixel, sizeof(whitePixel)));
        }

        void DestroyDeviceObjects()
        {
            DeviceState& d = Device();
            if (bgfx::isValid(d.whiteTexture)) bgfx::destroy(d.whiteTexture);
            if (bgfx::isValid(d.effectParamsUniform)) bgfx::destroy(d.effectParamsUniform);
            if (bgfx::isValid(d.texColorUniform)) bgfx::destroy(d.texColorUniform);
            if (bgfx::isValid(d.grayscaleProgram)) bgfx::destroy(d.grayscaleProgram);
            if (bgfx::isValid(d.spriteProgram)) bgfx::destroy(d.spriteProgram);
            d.whiteTexture = BGFX_INVALID_HANDLE;
            d.effectParamsUniform = BGFX_INVALID_HANDLE;
            d.texColorUniform = BGFX_INVALID_HANDLE;
            d.grayscaleProgram = BGFX_INVALID_HANDLE;
            d.spriteProgram = BGFX_INVALID_HANDLE;
        }

        //Pack a Color into bgfx's 0xRRGGBBAA clear colour.
        std::uint32_t ToClearColor(const Color& c)
        {
            return static_cast<std::uint32_t>(c.r) << 24
                | static_cast<std::uint32_t>(c.g) << 16
                | static_cast<std::uint32_t>(c.b) << 8
                | static_cast<std::uint32_t>(c.a);
        }

        //Copy possibly-padded RGBA8 rows into a tightly packed bgfx allocation.
        const bgfx::Memory* PackRows(const void* pixels, unsigned width, unsigned height, unsigned pitch)
        {
            const unsigned tight = width * 4;
            if (pitch == 0 || pitch == tight)
                return bgfx::copy(pixels, tight * height);

            const bgfx::Memory* mem = bgfx::alloc(tight * height);
            const auto* src = static_cast<const std::uint8_t*>(pixels);
            for (unsigned row = 0; row < height; ++row)
                std::memcpy(mem->data + row * tight, src + row * pitch, tight);
            return mem;
        }

        //Shared tail of every draw call: upload the geometry, bind, submit.
        void SubmitGeometry(const GfxVertex* vertices, std::uint32_t vertexCount,
                            const std::uint16_t* indices, std::uint32_t indexCount,
                            bgfx::TextureHandle texture, ShaderEffect effect,
                            const IntRect* scissorPixels, std::uint64_t extraState)
        {
            DeviceState& d = Device();
            if (!d.initialized || vertexCount == 0) return;

            //Silently dropping geometry beats asserting inside bgfx when a frame overflows the
            //transient buffers (which are sized by bgfx, not by us).
            if (bgfx::getAvailTransientVertexBuffer(vertexCount, d.layout) < vertexCount) return;
            if (indexCount > 0 && bgfx::getAvailTransientIndexBuffer(indexCount) < indexCount) return;

            bgfx::TransientVertexBuffer tvb{};
            bgfx::allocTransientVertexBuffer(&tvb, vertexCount, d.layout);
            std::memcpy(tvb.data, vertices, vertexCount * sizeof(GfxVertex));
            bgfx::setVertexBuffer(0, &tvb);

            if (indexCount > 0)
            {
                bgfx::TransientIndexBuffer tib{};
                bgfx::allocTransientIndexBuffer(&tib, indexCount);
                std::memcpy(tib.data, indices, indexCount * sizeof(std::uint16_t));
                bgfx::setIndexBuffer(&tib);
            }

            //UINT32_MAX = sample with the flags the texture itself was created with.
            bgfx::setTexture(0, d.texColorUniform, bgfx::isValid(texture) ? texture : d.whiteTexture, UINT32_MAX);

            if (scissorPixels)
            {
                bgfx::setScissor(static_cast<std::uint16_t>(std::max(0, scissorPixels->left)),
                                 static_cast<std::uint16_t>(std::max(0, scissorPixels->top)),
                                 static_cast<std::uint16_t>(std::max(0, scissorPixels->width)),
                                 static_cast<std::uint16_t>(std::max(0, scissorPixels->height)));
            }

            bgfx::setState(State2D | extraState);

            bgfx::ProgramHandle program = d.spriteProgram;
            if (effect == ShaderEffect::Grayscale && bgfx::isValid(d.grayscaleProgram))
            {
                program = d.grayscaleProgram;
                const float params[4]{d.grayscaleAmount, 0.f, 0.f, 0.f};
                bgfx::setUniform(d.effectParamsUniform, params);
            }

            bgfx::submit(SceneView, program);
        }
    }

    //------------------------------------ Lifetime ------------------------------------
    bool GraphicsDevice::Init(SDL_Window* window)
    {
        DeviceState& d = Device();
        if (d.initialized) return true;

        d.window = window;

        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        d.backbufferWidth = static_cast<unsigned>(std::max(w, 1));
        d.backbufferHeight = static_cast<unsigned>(std::max(h, 1));

        //Calling renderFrame() before init() puts bgfx in single threaded mode. The game drives
        //everything from the main thread, and the web build has no choice in the matter anyway.
        bgfx::renderFrame();

        bgfx::Init init{};
        init.type = bgfx::RendererType::Count; //Let bgfx pick the best backend for the platform
        init.resolution.width = d.backbufferWidth;
        init.resolution.height = d.backbufferHeight;
        init.resolution.reset = d.resetFlags;
        init.platformData = BuildPlatformData(window);

        if (!bgfx::init(init))
            return false;

        d.initialized = true;

        try
        {
            CreateDeviceObjects();
        }
        catch (...)
        {
            d.initialized = false;
            bgfx::shutdown();
            throw;
        }

        bgfx::setViewMode(SceneView, bgfx::ViewMode::Sequential);
        return true;
    }

    void GraphicsDevice::Shutdown()
    {
        DeviceState& d = Device();
        if (!d.initialized) return;

        DestroyDeviceObjects();
        bgfx::shutdown();
        d.initialized = false;

#if defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
        if (d.metalView) SDL_Metal_DestroyView(d.metalView);
        d.metalView = nullptr;
#endif
        d.window = nullptr;
    }

    bool GraphicsDevice::IsInitialized() { return Device().initialized; }

    const char* GraphicsDevice::GetRendererName()
    {
        if (!Device().initialized) return "none";
        return bgfx::getRendererName(bgfx::getRendererType());
    }

    //------------------------------------ Backbuffer ------------------------------------
    void GraphicsDevice::Resize(const unsigned pixelWidth, const unsigned pixelHeight)
    {
        DeviceState& d = Device();
        if (!d.initialized) return;
        if (pixelWidth == 0 || pixelHeight == 0) return;
        if (pixelWidth == d.backbufferWidth && pixelHeight == d.backbufferHeight) return;

        d.backbufferWidth = pixelWidth;
        d.backbufferHeight = pixelHeight;
        bgfx::reset(pixelWidth, pixelHeight, d.resetFlags);
    }

    void GraphicsDevice::SetVSyncEnabled(const bool enabled)
    {
        DeviceState& d = Device();
        const std::uint32_t flags = enabled ? (d.resetFlags | BGFX_RESET_VSYNC) : (d.resetFlags & ~BGFX_RESET_VSYNC);
        if (flags == d.resetFlags) return;

        d.resetFlags = flags;
        if (d.initialized)
            bgfx::reset(d.backbufferWidth, d.backbufferHeight, d.resetFlags);
    }

    Vector2u GraphicsDevice::GetBackbufferSize()
    {
        const DeviceState& d = Device();
        return {d.backbufferWidth, d.backbufferHeight};
    }

    //The logical canvas, scaled to fit and centred: what SDL_SetRenderLogicalPresentation used to do.
    FloatRect GraphicsDevice::GetViewportRect()
    {
        const DeviceState& d = Device();
        const auto logicalW = static_cast<float>(RenderWindow::LogicalSize.x);
        const auto logicalH = static_cast<float>(RenderWindow::LogicalSize.y);
        const auto bbW = static_cast<float>(std::max(d.backbufferWidth, 1u));
        const auto bbH = static_cast<float>(std::max(d.backbufferHeight, 1u));

        const float scale = std::min(bbW / logicalW, bbH / logicalH);
        const float width = logicalW * scale;
        const float height = logicalH * scale;
        return {(bbW - width) * 0.5f, (bbH - height) * 0.5f, width, height};
    }

    Vector2f GraphicsDevice::WindowPixelToLogical(const Vector2f& pixel)
    {
        const FloatRect vp = GetViewportRect();
        if (vp.width <= 0.f || vp.height <= 0.f) return pixel;

        const auto logicalW = static_cast<float>(RenderWindow::LogicalSize.x);
        const auto logicalH = static_cast<float>(RenderWindow::LogicalSize.y);
        return {(pixel.x - vp.left) * (logicalW / vp.width), (pixel.y - vp.top) * (logicalH / vp.height)};
    }

    Vector2f GraphicsDevice::LogicalToWindowPixel(const Vector2f& logical)
    {
        const FloatRect vp = GetViewportRect();
        const auto logicalW = static_cast<float>(RenderWindow::LogicalSize.x);
        const auto logicalH = static_cast<float>(RenderWindow::LogicalSize.y);
        return {vp.left + logical.x * (vp.width / logicalW), vp.top + logical.y * (vp.height / logicalH)};
    }

    //------------------------------------ Frame ------------------------------------
    void GraphicsDevice::BeginFrame(const Color& clearColor)
    {
        const DeviceState& d = Device();
        if (!d.initialized) return;

        //The bars around the letterboxed canvas are always black.
        bgfx::setViewRect(BackdropView, 0, 0, static_cast<std::uint16_t>(d.backbufferWidth), static_cast<std::uint16_t>(d.backbufferHeight));
        bgfx::setViewClear(BackdropView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff);
        bgfx::touch(BackdropView);

        const FloatRect vp = GetViewportRect();
        bgfx::setViewRect(SceneView,
                          static_cast<std::uint16_t>(vp.left), static_cast<std::uint16_t>(vp.top),
                          static_cast<std::uint16_t>(vp.width), static_cast<std::uint16_t>(vp.height));
        bgfx::setViewClear(SceneView, BGFX_CLEAR_COLOR, ToClearColor(clearColor));
        bgfx::setViewMode(SceneView, bgfx::ViewMode::Sequential);

        //Orthographic over the logical canvas with Y growing downwards, so every existing
        //screen-space coordinate in the game keeps its meaning.
        float ortho[16];
        bx::mtxOrtho(ortho, 0.f, static_cast<float>(RenderWindow::LogicalSize.x),
                     static_cast<float>(RenderWindow::LogicalSize.y), 0.f,
                     0.f, 1000.f, 0.f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(SceneView, nullptr, ortho);
        bgfx::touch(SceneView);
    }

    void GraphicsDevice::EndFrame()
    {
        if (!Device().initialized) return;
        bgfx::frame();
    }

    //------------------------------------ Draw ------------------------------------
    void GraphicsDevice::DrawIndexed(const GfxVertex* vertices, const std::uint32_t vertexCount,
                                     const std::uint16_t* indices, const std::uint32_t indexCount,
                                     const Texture* texture, const ShaderEffect effect)
    {
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
        if (texture && texture->getNativeHandle() != InvalidGpuHandle)
            handle = bgfx::TextureHandle{texture->getNativeHandle()};

        SubmitGeometry(vertices, vertexCount, indices, indexCount, handle, effect, nullptr, 0);
    }

    void GraphicsDevice::DrawIndexedRaw(const GfxVertex* vertices, const std::uint32_t vertexCount,
                                        const std::uint16_t* indices, const std::uint32_t indexCount,
                                        const std::uint16_t textureHandle, const IntRect* scissorPixels)
    {
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
        if (textureHandle != InvalidGpuHandle) handle = bgfx::TextureHandle{textureHandle};

        SubmitGeometry(vertices, vertexCount, indices, indexCount, handle, ShaderEffect::None, scissorPixels, 0);
    }

    void GraphicsDevice::DrawLines(const GfxVertex* vertices, const std::uint32_t vertexCount)
    {
        SubmitGeometry(vertices, vertexCount, nullptr, 0, BGFX_INVALID_HANDLE, ShaderEffect::None, nullptr, BGFX_STATE_PT_LINES);
    }

    //------------------------------------ Effects ------------------------------------
    void GraphicsDevice::SetGrayscaleAmount(const float amount) { Device().grayscaleAmount = std::clamp(amount, 0.f, 1.f); }
    float GraphicsDevice::GetGrayscaleAmount() { return Device().grayscaleAmount; }

    //------------------------------------ Textures ------------------------------------
    std::uint16_t GraphicsDevice::CreateTexture2D(const unsigned width, const unsigned height, const void* rgba, const unsigned pitch, const bool linearSampling)
    {
        if (!Device().initialized || width == 0 || height == 0 || !rgba) return InvalidGpuHandle;

        //Pixel art wants nearest sampling; text and the UI atlas want linear. Baked into the
        //texture so draw calls never have to think about it.
        const std::uint64_t flags = (linearSampling ? BGFX_SAMPLER_NONE : BGFX_SAMPLER_POINT) | BGFX_SAMPLER_UVW_CLAMP;

        const bgfx::TextureHandle handle = bgfx::createTexture2D(
            static_cast<std::uint16_t>(width), static_cast<std::uint16_t>(height),
            false, 1, bgfx::TextureFormat::RGBA8, flags, PackRows(rgba, width, height, pitch));

        return bgfx::isValid(handle) ? handle.idx : InvalidGpuHandle;
    }

    void GraphicsDevice::UpdateTexture2D(const std::uint16_t handle, const unsigned x, const unsigned y,
                                         const unsigned width, const unsigned height, const void* rgba, const unsigned pitch)
    {
        if (!Device().initialized || handle == InvalidGpuHandle || width == 0 || height == 0 || !rgba) return;

        bgfx::updateTexture2D(bgfx::TextureHandle{handle}, 0, 0,
                              static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y),
                              static_cast<std::uint16_t>(width), static_cast<std::uint16_t>(height),
                              PackRows(rgba, width, height, pitch));
    }

    void GraphicsDevice::DestroyTexture(const std::uint16_t handle)
    {
        if (!Device().initialized || handle == InvalidGpuHandle) return;
        bgfx::destroy(bgfx::TextureHandle{handle});
    }
}
