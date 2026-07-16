#pragma once
#include <string>
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"
#include "View.h"
#include "Shapes.h"

struct SDL_Window;
struct SDL_Renderer;
union SDL_Event;

namespace ETG
{
    class Text;

    //SDL3 window + 2D renderer, replacement for sf::RenderWindow.
    //Only one instance is expected to exist; the renderer is exposed statically so
    //that textures/fonts can be created from anywhere (mirroring SFML's behavior).
    class RenderWindow
    {
    public:
        RenderWindow(unsigned width, unsigned height, const std::string& title, bool fullscreen = false);
        ~RenderWindow();

        RenderWindow(const RenderWindow&) = delete;
        RenderWindow& operator=(const RenderWindow&) = delete;

        [[nodiscard]] bool isOpen() const { return m_open; }
        void close();

        [[nodiscard]] Vector2u getSize() const;

        bool pollEvent(SDL_Event& event) const;

        void clear(const Color& color);
        void display(); //Present + optional framerate limiting

        void setFramerateLimit(unsigned fps) { m_framerateLimit = fps; }
        void requestFocus() const;
        [[nodiscard]] bool hasFocus() const;

        //---------------- View handling ----------------
        void setView(const View& view) { m_view = view; }
        [[nodiscard]] const View& getView() const { return m_view; }
        [[nodiscard]] View getDefaultView() const;

        //Convert a pixel (window) coordinate to world coordinates for the given view.
        //Always accounts for the letterbox transform (see beginLetterboxedScene), since
        //the only caller maps mouse position onto the letterboxed game world view.
        [[nodiscard]] Vector2f mapPixelToCoords(const Vector2i& pixel, const View& view) const;

        //Transform a world coordinate to screen (pixel) coordinates using the current view
        [[nodiscard]] Vector2f worldToScreen(const Vector2f& world) const;
        //Scale factor from world units to pixels of the current view (x, y)
        [[nodiscard]] Vector2f worldToScreenScale() const;

        //---------------- Immediate mode drawing (uses the current view) ----------------
        void draw(const RectangleShape& rect);
        void draw(const CircleShape& circle);
        void draw(const Text& text);
        void drawLine(const Vector2f& from, const Vector2f& to, const Color& color);

        [[nodiscard]] SDL_Window* getNativeWindow() const { return m_window; }
        [[nodiscard]] SDL_Renderer* getNativeRenderer() const { return m_renderer; }

        //Global access for resource creation (textures, fonts). Null until a window exists.
        static SDL_Renderer* GetRenderer() { return s_renderer; }

        //Fixed design resolution everything (world, HUD, ImGui) is drawn against. SDL's
        //logical presentation (set once, permanently, in the constructor) letterboxes this
        //canvas onto the real window, regardless of how the window is resized/shaped.
        static constexpr Vector2u LogicalSize{1920, 1080};

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        bool m_open = false;
        View m_view;

        unsigned m_framerateLimit = 0;
        std::uint64_t m_lastFrameTimeNs = 0;

        Vector2u m_logicalSize;
        [[nodiscard]] Vector2f windowPixelToLogical(const Vector2f& physicalPixel) const;

        static SDL_Renderer* s_renderer;
    };
}
