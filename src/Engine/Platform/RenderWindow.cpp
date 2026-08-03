#include "RenderWindow.h"
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>
#include <SDL3/SDL.h>
#include "GraphicsDevice.h"
#include "Text.h"

namespace ETG
{
    RenderWindow::RenderWindow(const unsigned width, const unsigned height, const std::string& title, const bool fullscreen)
    {
        if (!SDL_WasInit(SDL_INIT_VIDEO))
        {
            if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
                throw std::runtime_error(std::string("SDL video init failed: ") + SDL_GetError());
        }

        //HIGH_PIXEL_DENSITY: bgfx renders into real pixels, so ask SDL for a real-pixel backbuffer
        //instead of an upscaled one. METAL is what lets SDL hand bgfx a CAMetalLayer on Apple platforms.
        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;
#if defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
        flags |= SDL_WINDOW_METAL;
#endif
        m_window = SDL_CreateWindow(title.c_str(), static_cast<int>(width), static_cast<int>(height), flags);
        if (!m_window)
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
        if (fullscreen)
            SDL_SyncWindow(m_window); //Wait for the fullscreen resize so the backbuffer below is sized correctly

        if (!GraphicsDevice::Init(m_window))
        {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
            throw std::runtime_error("bgfx could not create a renderer for this platform");
        }

        m_open = true;
        m_lastFrameTimeNs = SDL_GetTicksNS();
        m_view = getDefaultView();
    }

    RenderWindow::~RenderWindow()
    {
        close();
        SDL_Quit();
    }

    void RenderWindow::close()
    {
        if (m_window)
        {
            //Order matters: bgfx has to let go of the native surface before SDL destroys it
            GraphicsDevice::Shutdown();
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        m_open = false;
    }

    Vector2u RenderWindow::getSize() const
    {
        int w = 0, h = 0;
        if (m_window) SDL_GetWindowSize(m_window, &w, &h);
        return {static_cast<unsigned>(w), static_cast<unsigned>(h)};
    }

    bool RenderWindow::pollEvent(SDL_Event& event) const
    {
        return SDL_PollEvent(&event);
    }

    void RenderWindow::clear(const Color& color)
    {
        GraphicsDevice::BeginFrame(color);
    }

    void RenderWindow::display()
    {
        GraphicsDevice::EndFrame();

        //Manual framerate limiting (bgfx only offers vsync)
        if (m_framerateLimit > 0)
        {
            const std::uint64_t targetNs = 1'000'000'000ull / m_framerateLimit;
            const std::uint64_t now = SDL_GetTicksNS();
            const std::uint64_t elapsed = now - m_lastFrameTimeNs;
            if (elapsed < targetNs)
            {
                SDL_DelayNS(targetNs - elapsed);
            }
        }
        m_lastFrameTimeNs = SDL_GetTicksNS();
    }

    void RenderWindow::requestFocus() const
    {
        if (m_window)
            SDL_RaiseWindow(m_window);
    }

    bool RenderWindow::hasFocus() const
    {
        return m_window && (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS);
    }

    void RenderWindow::handleResize() const
    {
        if (!m_window) return;
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(m_window, &w, &h);
        GraphicsDevice::Resize(static_cast<unsigned>(w), static_cast<unsigned>(h));
    }

    //---------------- View handling ----------------
    View RenderWindow::getDefaultView() const
    {
        constexpr auto w = static_cast<float>(LogicalSize.x);
        constexpr auto h = static_cast<float>(LogicalSize.y);
        return View{{w / 2.f, h / 2.f}, {w, h}};
    }

    //Window point -> logical canvas pixel. SDL reports mouse positions in window points, which on a
    //HiDPI display are not backbuffer pixels, so scale by the density before undoing the letterbox.
    Vector2f RenderWindow::mapWindowPointToLogical(const Vector2f& point) const
    {
        const float density = m_window ? SDL_GetWindowPixelDensity(m_window) : 1.f;
        return GraphicsDevice::WindowPixelToLogical({point.x * density, point.y * density});
    }

    Vector2f RenderWindow::mapPixelToCoords(const Vector2i& pixel, const View& view) const
    {
        const Vector2f logicalPixel = mapWindowPointToLogical({static_cast<float>(pixel.x), static_cast<float>(pixel.y)});
        constexpr Vector2f canvas{static_cast<float>(LogicalSize.x), static_cast<float>(LogicalSize.y)};
        const Vector2f viewSize = view.getSize();
        const Vector2f center = view.getCenter();

        return {
            center.x + (logicalPixel.x - canvas.x / 2.f) * (viewSize.x / canvas.x),
            center.y + (logicalPixel.y - canvas.y / 2.f) * (viewSize.y / canvas.y)
        };
    }

    Vector2f RenderWindow::worldToScreen(const Vector2f& world) const
    {
        //Our own draw calls always target the fixed logical canvas — GraphicsDevice's orthographic
        //view then scales/letterboxes that onto the real backbuffer.
        constexpr Vector2f canvas{static_cast<float>(LogicalSize.x), static_cast<float>(LogicalSize.y)};
        const Vector2f viewSize = m_view.getSize();
        const Vector2f center = m_view.getCenter();

        return {
            (world.x - center.x) * (canvas.x / viewSize.x) + canvas.x / 2.f,
            (world.y - center.y) * (canvas.y / viewSize.y) + canvas.y / 2.f
        };
    }

    Vector2f RenderWindow::worldToScreenScale() const
    {
        const Vector2f viewSize = m_view.getSize();
        return {static_cast<float>(LogicalSize.x) / viewSize.x, static_cast<float>(LogicalSize.y) / viewSize.y};
    }

    bool RenderWindow::setVSyncEnabled(const bool enabled) const
    {
        GraphicsDevice::SetVSyncEnabled(enabled);
        return true;
    }

    //---------------- Immediate mode drawing ----------------
    namespace
    {
        //Every immediate draw is untextured, so they all go through the sprite program against the
        //device's built-in 1x1 white texture.
        void SubmitQuad(const Vector2f& topLeft, const Vector2f& size, const Color& color)
        {
            if (color.a == 0) return;

            const std::uint32_t rgba = PackColor(color);
            const GfxVertex vertices[4]{
                {topLeft.x, topLeft.y, 0.f, 0.f, rgba},
                {topLeft.x + size.x, topLeft.y, 1.f, 0.f, rgba},
                {topLeft.x + size.x, topLeft.y + size.y, 1.f, 1.f, rgba},
                {topLeft.x, topLeft.y + size.y, 0.f, 1.f, rgba},
            };
            constexpr std::uint16_t indices[6]{0, 1, 2, 0, 2, 3};
            GraphicsDevice::DrawIndexed(vertices, 4, indices, 6, nullptr);
        }

        //Consecutive points turned into a line list (bgfx has no line strip primitive)
        void SubmitLineStrip(const std::vector<Vector2f>& points, const Color& color)
        {
            if (points.size() < 2 || color.a == 0) return;

            const std::uint32_t rgba = PackColor(color);
            std::vector<GfxVertex> vertices;
            vertices.reserve((points.size() - 1) * 2);
            for (std::size_t i = 1; i < points.size(); ++i)
            {
                vertices.push_back(GfxVertex{points[i - 1].x, points[i - 1].y, 0.f, 0.f, rgba});
                vertices.push_back(GfxVertex{points[i].x, points[i].y, 0.f, 0.f, rgba});
            }
            GraphicsDevice::DrawLines(vertices.data(), static_cast<std::uint32_t>(vertices.size()));
        }
    }

    void RenderWindow::draw(const RectangleShape& rect)
    {
        //Normalize a possibly negative size (used by progress bars growing upwards)
        Vector2f pos = rect.getPosition() - rect.getOrigin();
        Vector2f size = rect.getSize();
        if (size.x < 0) { pos.x += size.x; size.x = -size.x; }
        if (size.y < 0) { pos.y += size.y; size.y = -size.y; }

        const Vector2f screenPos = worldToScreen(pos);
        const Vector2f scale = worldToScreenScale();
        const Vector2f screenSize{size.x * scale.x, size.y * scale.y};

        SubmitQuad(screenPos, screenSize, rect.getFillColor());

        if (rect.getOutlineThickness() > 0.f && rect.getOutlineColor().a > 0)
        {
            const std::vector<Vector2f> outline{
                {screenPos.x, screenPos.y},
                {screenPos.x + screenSize.x, screenPos.y},
                {screenPos.x + screenSize.x, screenPos.y + screenSize.y},
                {screenPos.x, screenPos.y + screenSize.y},
                {screenPos.x, screenPos.y},
            };
            SubmitLineStrip(outline, rect.getOutlineColor());
        }
    }

    void RenderWindow::draw(const CircleShape& circle)
    {
        //SFML circles are positioned by their top-left corner (minus origin); center accordingly
        const float radius = circle.getRadius();
        const Vector2f worldCenter = circle.getPosition() - circle.getOrigin() + Vector2f{radius, radius};
        const Vector2f screenCenter = worldToScreen(worldCenter);
        const Vector2f scale = worldToScreenScale();
        const float rx = radius * scale.x;
        const float ry = radius * scale.y;

        constexpr int segments = 32;
        std::vector<Vector2f> points;
        points.reserve(segments + 1);
        for (int i = 0; i <= segments; ++i)
        {
            const float angle = static_cast<float>(i) / segments * 2.f * std::numbers::pi_v<float>;
            points.push_back(Vector2f{screenCenter.x + std::cos(angle) * rx, screenCenter.y + std::sin(angle) * ry});
        }

        const Color& fill = circle.getFillColor();
        if (fill.a > 0)
        {
            //Triangle fan around the centre, expanded into the triangle list bgfx wants
            const std::uint32_t rgba = PackColor(fill);
            std::vector<GfxVertex> vertices;
            std::vector<std::uint16_t> indices;
            vertices.reserve(segments + 2);
            indices.reserve(segments * 3);

            vertices.push_back(GfxVertex{screenCenter.x, screenCenter.y, 0.f, 0.f, rgba});
            for (const Vector2f& p : points)
                vertices.push_back(GfxVertex{p.x, p.y, 0.f, 0.f, rgba});

            for (int i = 1; i <= segments; ++i)
            {
                indices.push_back(0);
                indices.push_back(static_cast<std::uint16_t>(i));
                indices.push_back(static_cast<std::uint16_t>(i + 1));
            }
            GraphicsDevice::DrawIndexed(vertices.data(), static_cast<std::uint32_t>(vertices.size()),
                                        indices.data(), static_cast<std::uint32_t>(indices.size()), nullptr);
        }

        if (circle.getOutlineThickness() > 0.f)
            SubmitLineStrip(points, circle.getOutlineColor());
    }

    void RenderWindow::drawLine(const Vector2f& from, const Vector2f& to, const Color& color)
    {
        const std::uint32_t rgba = PackColor(color);
        const Vector2f a = worldToScreen(from);
        const Vector2f b = worldToScreen(to);
        const GfxVertex vertices[2]{
            {a.x, a.y, 0.f, 0.f, rgba},
            {b.x, b.y, 0.f, 0.f, rgba},
        };
        GraphicsDevice::DrawLines(vertices, 2);
    }

    void RenderWindow::draw(const Text& text)
    {
        text.drawTo(*this);
    }
}
