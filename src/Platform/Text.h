#pragma once
#include <string>
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"

struct TTF_Font;
struct SDL_Texture;

namespace ETG
{
    class RenderWindow;

    //TTF font handle, replacement for sf::Font. Rasterization is done by SDL3_ttf (freetype).
    class Font
    {
    public:
        Font() = default;
        ~Font();

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        Font(Font&& other) noexcept;
        Font& operator=(Font&& other) noexcept;

        bool loadFromFile(const std::string& path);

        [[nodiscard]] TTF_Font* getNativeHandle() const { return m_font; }
        [[nodiscard]] bool isLoaded() const { return m_font != nullptr; }

    private:
        TTF_Font* m_font = nullptr;
    };

    //Drawable string, replacement for sf::Text. Rendered via RenderWindow::draw(text).
    //The string is rasterized once by SDL3_ttf into a cached texture (re-rendered only when
    //the string or character size changes) and drawn as a quad through the window's view.
    class Text
    {
    public:
        Text() = default;
        ~Text();

        Text(const Text&) = delete;
        Text& operator=(const Text&) = delete;
        Text(Text&&) = delete;
        Text& operator=(Text&&) = delete;

        void setFont(const Font& font) { m_font = &font; }
        void setString(const std::string& str) { m_string = str; }
        void setCharacterSize(unsigned size) { m_characterSize = size; }
        void setFillColor(const Color& color) { m_fillColor = color; }
        void setPosition(const Vector2f& pos) { m_position = pos; }
        void setPosition(float x, float y) { m_position = {x, y}; }
        void setOrigin(const Vector2f& origin) { m_origin = origin; }
        void setOrigin(float x, float y) { m_origin = {x, y}; }

        [[nodiscard]] const std::string& getString() const { return m_string; }
        [[nodiscard]] const Vector2f& getPosition() const { return m_position; }
        [[nodiscard]] FloatRect getLocalBounds() const;

        //Draws with the window's current view (called by RenderWindow::draw)
        void drawTo(RenderWindow& window) const;

    private:
        bool ensureTexture() const;
        void destroyTexture() const;

        const Font* m_font = nullptr;
        std::string m_string;
        unsigned m_characterSize = 30;
        Color m_fillColor = Color::White;
        Vector2f m_position{0.f, 0.f};
        Vector2f m_origin{0.f, 0.f};

        //Cached rasterization. Rendered white and tinted at draw time, so color changes
        //don't force a re-render.
        mutable SDL_Texture* m_texture = nullptr;
        mutable Vector2f m_textureSize{0.f, 0.f};
        mutable std::string m_cachedString;
        mutable unsigned m_cachedSize = 0;
    };
}
