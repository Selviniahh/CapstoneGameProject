#include "RenderWindow.h"
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>
#include <SDL3/SDL.h>
#include "Text.h"

namespace ETG
{
    SDL_Renderer* RenderWindow::s_renderer = nullptr;

    namespace
    {
        SDL_FColor ToFColor(const Color& c)
        {
            return SDL_FColor{c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f};
        }
    }

    RenderWindow::RenderWindow(const unsigned width, const unsigned height, const std::string& title)
    {
        if (!SDL_WasInit(SDL_INIT_VIDEO))
        {
            if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
                throw std::runtime_error(std::string("SDL video init failed: ") + SDL_GetError());
        }

        m_window = SDL_CreateWindow(title.c_str(), static_cast<int>(width), static_cast<int>(height), SDL_WINDOW_RESIZABLE);
        if (!m_window)
            throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer)
        {
            //Fall back to the software renderer (e.g. headless environments)
            m_renderer = SDL_CreateRenderer(m_window, SDL_SOFTWARE_RENDERER);
        }
        if (!m_renderer)
            throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());

        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

        s_renderer = m_renderer;
        m_open = true;
        m_view = getDefaultView();
        m_lastFrameTimeNs = SDL_GetTicksNS();
    }

    RenderWindow::~RenderWindow()
    {
        close();
        SDL_Quit();
    }

    void RenderWindow::close()
    {
        if (m_renderer)
        {
            SDL_DestroyRenderer(m_renderer);
            m_renderer = nullptr;
            s_renderer = nullptr;
        }
        if (m_window)
        {
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
        if (!m_renderer) return;
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
        SDL_RenderClear(m_renderer);
    }

    void RenderWindow::display()
    {
        if (!m_renderer) return;
        SDL_RenderPresent(m_renderer);

        //Manual framerate limiting (SDL has no built-in equivalent of SFML's setFramerateLimit)
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
        if (m_window) SDL_RaiseWindow(m_window);
    }

    bool RenderWindow::hasFocus() const
    {
        return m_window && (SDL_GetWindowFlags(m_window) & SDL_WINDOW_INPUT_FOCUS);
    }

    //---------------- View handling ----------------
    View RenderWindow::getDefaultView() const
    {
        const Vector2u size = getSize();
        const auto w = static_cast<float>(size.x);
        const auto h = static_cast<float>(size.y);
        return View{{w / 2.f, h / 2.f}, {w, h}};
    }

    Vector2f RenderWindow::mapPixelToCoords(const Vector2i& pixel, const View& view) const
    {
        const Vector2u winSize = getSize();
        const Vector2f win{static_cast<float>(winSize.x), static_cast<float>(winSize.y)};
        const Vector2f viewSize = view.getSize();
        const Vector2f center = view.getCenter();

        return {
            center.x + (static_cast<float>(pixel.x) - win.x / 2.f) * (viewSize.x / win.x),
            center.y + (static_cast<float>(pixel.y) - win.y / 2.f) * (viewSize.y / win.y)
        };
    }

    Vector2f RenderWindow::worldToScreen(const Vector2f& world) const
    {
        const Vector2u winSize = getSize();
        const Vector2f win{static_cast<float>(winSize.x), static_cast<float>(winSize.y)};
        const Vector2f viewSize = m_view.getSize();
        const Vector2f center = m_view.getCenter();

        return {
            (world.x - center.x) * (win.x / viewSize.x) + win.x / 2.f,
            (world.y - center.y) * (win.y / viewSize.y) + win.y / 2.f
        };
    }

    Vector2f RenderWindow::worldToScreenScale() const
    {
        const Vector2u winSize = getSize();
        const Vector2f viewSize = m_view.getSize();
        return {static_cast<float>(winSize.x) / viewSize.x, static_cast<float>(winSize.y) / viewSize.y};
    }

    //---------------- Immediate mode drawing ----------------
    void RenderWindow::draw(const RectangleShape& rect)
    {
        if (!m_renderer) return;

        //Normalize a possibly negative size (used by progress bars growing upwards)
        Vector2f pos = rect.getPosition() - rect.getOrigin();
        Vector2f size = rect.getSize();
        if (size.x < 0) { pos.x += size.x; size.x = -size.x; }
        if (size.y < 0) { pos.y += size.y; size.y = -size.y; }

        const Vector2f screenPos = worldToScreen(pos);
        const Vector2f scale = worldToScreenScale();
        const SDL_FRect frect{screenPos.x, screenPos.y, size.x * scale.x, size.y * scale.y};

        const Color& fill = rect.getFillColor();
        if (fill.a > 0)
        {
            SDL_SetRenderDrawColor(m_renderer, fill.r, fill.g, fill.b, fill.a);
            SDL_RenderFillRect(m_renderer, &frect);
        }

        const Color& outline = rect.getOutlineColor();
        if (rect.getOutlineThickness() > 0.f && outline.a > 0)
        {
            SDL_SetRenderDrawColor(m_renderer, outline.r, outline.g, outline.b, outline.a);
            SDL_RenderRect(m_renderer, &frect);
        }
    }

    void RenderWindow::draw(const CircleShape& circle)
    {
        if (!m_renderer) return;

        //SFML circles are positioned by their top-left corner (minus origin); center accordingly
        const float radius = circle.getRadius();
        const Vector2f worldCenter = circle.getPosition() - circle.getOrigin() + Vector2f{radius, radius};
        const Vector2f screenCenter = worldToScreen(worldCenter);
        const Vector2f scale = worldToScreenScale();
        const float rx = radius * scale.x;
        const float ry = radius * scale.y;

        constexpr int segments = 32;
        std::vector<SDL_FPoint> points;
        points.reserve(segments + 1);
        for (int i = 0; i <= segments; ++i)
        {
            const float angle = static_cast<float>(i) / segments * 2.f * std::numbers::pi_v<float>;
            points.push_back(SDL_FPoint{screenCenter.x + std::cos(angle) * rx, screenCenter.y + std::sin(angle) * ry});
        }

        const Color& fill = circle.getFillColor();
        if (fill.a > 0)
        {
            //Simple fan fill
            std::vector<SDL_Vertex> vertices;
            std::vector<int> indices;
            const SDL_FColor fcolor = ToFColor(fill);
            vertices.push_back(SDL_Vertex{SDL_FPoint{screenCenter.x, screenCenter.y}, fcolor, SDL_FPoint{0, 0}});
            for (int i = 0; i <= segments; ++i)
                vertices.push_back(SDL_Vertex{points[i], fcolor, SDL_FPoint{0, 0}});
            for (int i = 1; i <= segments; ++i)
            {
                indices.push_back(0);
                indices.push_back(i);
                indices.push_back(i + 1);
            }
            SDL_RenderGeometry(m_renderer, nullptr, vertices.data(), static_cast<int>(vertices.size()), indices.data(), static_cast<int>(indices.size()));
        }

        const Color& outline = circle.getOutlineColor();
        if (circle.getOutlineThickness() > 0.f && outline.a > 0)
        {
            SDL_SetRenderDrawColor(m_renderer, outline.r, outline.g, outline.b, outline.a);
            SDL_RenderLines(m_renderer, points.data(), static_cast<int>(points.size()));
        }
    }

    void RenderWindow::drawLine(const Vector2f& from, const Vector2f& to, const Color& color)
    {
        if (!m_renderer) return;
        const Vector2f a = worldToScreen(from);
        const Vector2f b = worldToScreen(to);
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
        SDL_RenderLine(m_renderer, a.x, a.y, b.x, b.y);
    }

    void RenderWindow::draw(const Text& text)
    {
        text.drawTo(*this);
    }
}
