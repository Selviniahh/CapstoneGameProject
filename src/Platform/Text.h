#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Texture.h"
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"

namespace ETG
{
    class RenderWindow;

    //TTF font baked into a texture atlas with stb_truetype, replacement for sf::Font
    class Font
    {
    public:
        static constexpr float BakePixelHeight = 48.f; //Glyphs are baked at this size and scaled at draw time
        static constexpr int FirstChar = 32; //ASCII range 32..126
        static constexpr int CharCount = 95;

        Font() = default;

        bool loadFromFile(const std::string& path);

        struct GlyphQuad
        {
            FloatRect screen; //Position relative to the pen, in baked pixels
            FloatRect uv; //Normalized texture coordinates
        };

        //Advances `penX` like stbtt_GetBakedQuad
        [[nodiscard]] bool getGlyph(char c, float& penX, float& penY, GlyphQuad& out) const;
        [[nodiscard]] const Texture* getAtlas() const { return m_atlas.get(); }
        [[nodiscard]] bool isLoaded() const { return m_atlas != nullptr; }

    private:
        std::shared_ptr<Texture> m_atlas;
        std::vector<std::uint8_t> m_bakedCharData; //stbtt_bakedchar array, kept opaque here
        int m_atlasSize = 0;
    };

    //Drawable string, replacement for sf::Text. Rendered via RenderWindow::draw(text).
    class Text
    {
    public:
        Text() = default;

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
        const Font* m_font = nullptr;
        std::string m_string;
        unsigned m_characterSize = 30;
        Color m_fillColor = Color::White;
        Vector2f m_position{0.f, 0.f};
        Vector2f m_origin{0.f, 0.f};
    };
}
