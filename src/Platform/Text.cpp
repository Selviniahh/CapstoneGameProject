#include "Text.h"
#include <fstream>
#include <SDL3/SDL.h>
#include <stb_truetype.h>
#include "RenderWindow.h"

namespace ETG
{
    //------------------------------------ Font ------------------------------------
    bool Font::loadFromFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;
        std::vector<unsigned char> fontData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        m_atlasSize = 1024;
        std::vector<unsigned char> alphaBitmap(static_cast<size_t>(m_atlasSize) * m_atlasSize);
        m_bakedCharData.resize(sizeof(stbtt_bakedchar) * CharCount);

        const int result = stbtt_BakeFontBitmap(fontData.data(), 0, BakePixelHeight,
                                                alphaBitmap.data(), m_atlasSize, m_atlasSize,
                                                FirstChar, CharCount,
                                                reinterpret_cast<stbtt_bakedchar*>(m_bakedCharData.data()));
        if (result <= 0) return false;

        //Expand the alpha coverage bitmap into white RGBA so it can be tinted at draw time
        Image atlasImage;
        atlasImage.create(m_atlasSize, m_atlasSize, Color{255, 255, 255, 0});
        auto* pixels = const_cast<std::uint8_t*>(atlasImage.getPixelsPtr());
        for (size_t i = 0; i < alphaBitmap.size(); ++i)
        {
            pixels[i * 4 + 3] = alphaBitmap[i];
        }

        m_atlas = std::make_shared<Texture>();
        if (!m_atlas->loadFromImage(atlasImage))
        {
            m_atlas = nullptr;
            return false;
        }
        //Text looks better with linear filtering when scaled down
        SDL_SetTextureScaleMode(m_atlas->getNativeHandle(), SDL_SCALEMODE_LINEAR);
        return true;
    }

    bool Font::getGlyph(const char c, float& penX, float& penY, GlyphQuad& out) const
    {
        if (!m_atlas || c < FirstChar || c >= FirstChar + CharCount) return false;

        stbtt_aligned_quad quad{};
        stbtt_GetBakedQuad(reinterpret_cast<const stbtt_bakedchar*>(m_bakedCharData.data()),
                           m_atlasSize, m_atlasSize, c - FirstChar, &penX, &penY, &quad, 1);

        out.screen = FloatRect(quad.x0, quad.y0, quad.x1 - quad.x0, quad.y1 - quad.y0);
        out.uv = FloatRect(quad.s0, quad.t0, quad.s1 - quad.s0, quad.t1 - quad.t0);
        return true;
    }

    //------------------------------------ Text ------------------------------------
    FloatRect Text::getLocalBounds() const
    {
        if (!m_font || !m_font->isLoaded() || m_string.empty()) return {};

        const float scale = static_cast<float>(m_characterSize) / Font::BakePixelHeight;
        float penX = 0.f, penY = 0.f;
        float minX = 0.f, minY = 0.f, maxX = 0.f, maxY = 0.f;
        bool first = true;

        for (const char c : m_string)
        {
            Font::GlyphQuad quad{};
            if (!m_font->getGlyph(c, penX, penY, quad)) continue;

            if (first)
            {
                minX = quad.screen.left;
                minY = quad.screen.top;
                maxX = quad.screen.left + quad.screen.width;
                maxY = quad.screen.top + quad.screen.height;
                first = false;
            }
            else
            {
                minX = std::min(minX, quad.screen.left);
                minY = std::min(minY, quad.screen.top);
                maxX = std::max(maxX, quad.screen.left + quad.screen.width);
                maxY = std::max(maxY, quad.screen.top + quad.screen.height);
            }
        }

        return {minX * scale, minY * scale, (maxX - minX) * scale, (maxY - minY) * scale};
    }

    void Text::drawTo(RenderWindow& window) const
    {
        if (!m_font || !m_font->isLoaded() || m_string.empty()) return;
        SDL_Renderer* renderer = window.getNativeRenderer();
        if (!renderer) return;

        const float scale = static_cast<float>(m_characterSize) / Font::BakePixelHeight;
        const SDL_FColor color{m_fillColor.r / 255.f, m_fillColor.g / 255.f, m_fillColor.b / 255.f, m_fillColor.a / 255.f};

        //The baked pen sits on the baseline; shift down so m_position is the top-left like sf::Text
        const float ascentOffset = Font::BakePixelHeight * 0.8f;

        std::vector<SDL_Vertex> vertices;
        std::vector<int> indices;
        vertices.reserve(m_string.size() * 4);
        indices.reserve(m_string.size() * 6);

        float penX = 0.f, penY = 0.f;
        for (const char c : m_string)
        {
            Font::GlyphQuad quad{};
            if (!m_font->getGlyph(c, penX, penY, quad)) continue;

            const Vector2f local[4] = {
                {quad.screen.left, quad.screen.top + ascentOffset},
                {quad.screen.left + quad.screen.width, quad.screen.top + ascentOffset},
                {quad.screen.left + quad.screen.width, quad.screen.top + quad.screen.height + ascentOffset},
                {quad.screen.left, quad.screen.top + quad.screen.height + ascentOffset}
            };
            const Vector2f uv[4] = {
                {quad.uv.left, quad.uv.top},
                {quad.uv.left + quad.uv.width, quad.uv.top},
                {quad.uv.left + quad.uv.width, quad.uv.top + quad.uv.height},
                {quad.uv.left, quad.uv.top + quad.uv.height}
            };

            const int base = static_cast<int>(vertices.size());
            for (int i = 0; i < 4; ++i)
            {
                const Vector2f world = m_position - m_origin + local[i] * scale;
                const Vector2f screen = window.worldToScreen(world);
                vertices.push_back(SDL_Vertex{SDL_FPoint{screen.x, screen.y}, color, SDL_FPoint{uv[i].x, uv[i].y}});
            }

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        if (!vertices.empty())
        {
            SDL_RenderGeometry(renderer, m_font->getAtlas()->getNativeHandle(),
                               vertices.data(), static_cast<int>(vertices.size()),
                               indices.data(), static_cast<int>(indices.size()));
        }
    }
}
