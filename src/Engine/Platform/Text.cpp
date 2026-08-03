#include "Text.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "GraphicsDevice.h"
#include "RenderWindow.h"

namespace ETG
{
    namespace
    {
        bool EnsureTtfInit()
        {
            static const bool initialized = TTF_Init();
            return initialized;
        }
    }

    //------------------------------------ Font ------------------------------------
    Font::~Font()
    {
        if (m_font) TTF_CloseFont(m_font);
    }

    Font::Font(Font&& other) noexcept : m_font(other.m_font)
    {
        other.m_font = nullptr;
    }

    Font& Font::operator=(Font&& other) noexcept
    {
        if (this == &other) return *this;
        if (m_font) TTF_CloseFont(m_font);
        m_font = other.m_font;
        other.m_font = nullptr;
        return *this;
    }

    bool Font::loadFromFile(const std::string& path)
    {
        if (!EnsureTtfInit()) return false;
        if (m_font) TTF_CloseFont(m_font);

        //Opened at an arbitrary size; Text sets the real size before each rasterization
        m_font = TTF_OpenFont(path.c_str(), 32.f);
        return m_font != nullptr;
    }

    //------------------------------------ Text ------------------------------------
    Text::~Text()
    {
        destroyTexture();
    }

    void Text::destroyTexture() const
    {
        m_texture.reset();
    }

    bool Text::ensureTexture() const
    {
        if (!m_font || !m_font->isLoaded() || m_string.empty()) return false;
        if (m_texture && m_string == m_cachedString && m_characterSize == m_cachedSize) return true;
        if (!GraphicsDevice::IsInitialized()) return false;

        TTF_Font* font = m_font->getNativeHandle();
        TTF_SetFontSize(font, static_cast<float>(m_characterSize));

        SDL_Surface* surface = TTF_RenderText_Blended(font, m_string.c_str(), 0, SDL_Color{255, 255, 255, 255});
        if (!surface) return false;

        //SDL_ttf hands back whatever format suits it; Image (and therefore the GPU path) is RGBA32
        SDL_Surface* rgba = surface->format == SDL_PIXELFORMAT_RGBA32 ? surface : SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);

        auto texture = std::make_unique<Texture>();
        bool uploaded = false;
        if (rgba)
        {
            Image image;
            image.create(static_cast<unsigned>(rgba->w), static_cast<unsigned>(rgba->h), Color::Transparent);

            if (SDL_Surface* dst = image.getNativeSurface())
            {
                const int rowBytes = rgba->w * 4;
                for (int row = 0; row < rgba->h; ++row)
                {
                    SDL_memcpy(static_cast<std::uint8_t*>(dst->pixels) + static_cast<size_t>(row) * dst->pitch,
                               static_cast<const std::uint8_t*>(rgba->pixels) + static_cast<size_t>(row) * rgba->pitch,
                               rowBytes);
                }

                //Linear sampling: text is scaled by the view, so it should smooth rather than block up
                uploaded = texture->loadFromImage(image, true);
                if (uploaded) m_textureSize = {static_cast<float>(rgba->w), static_cast<float>(rgba->h)};
            }
        }

        if (rgba && rgba != surface) SDL_DestroySurface(rgba);
        SDL_DestroySurface(surface);
        if (!uploaded) return false;

        m_texture = std::move(texture);
        m_cachedString = m_string;
        m_cachedSize = m_characterSize;
        return true;
    }

    FloatRect Text::getLocalBounds() const
    {
        if (!m_font || !m_font->isLoaded() || m_string.empty()) return {};

        TTF_Font* font = m_font->getNativeHandle();
        TTF_SetFontSize(font, static_cast<float>(m_characterSize));

        int w = 0, h = 0;
        TTF_GetStringSize(font, m_string.c_str(), 0, &w, &h);
        return {0.f, 0.f, static_cast<float>(w), static_cast<float>(h)};
    }

    void Text::drawTo(RenderWindow& window) const
    {
        if (!ensureTexture()) return;

        const Vector2f screen = window.worldToScreen(m_position - m_origin);
        const Vector2f scale = window.worldToScreenScale();
        const float w = m_textureSize.x * scale.x;
        const float h = m_textureSize.y * scale.y;

        //The glyphs were rasterized white, so the fill colour is just the vertex tint
        const std::uint32_t rgba = PackColor(m_fillColor);
        const GfxVertex vertices[4]{
            {screen.x, screen.y, 0.f, 0.f, rgba},
            {screen.x + w, screen.y, 1.f, 0.f, rgba},
            {screen.x + w, screen.y + h, 1.f, 1.f, rgba},
            {screen.x, screen.y + h, 0.f, 1.f, rgba},
        };
        constexpr std::uint16_t indices[6]{0, 1, 2, 0, 2, 3};

        GraphicsDevice::DrawIndexed(vertices, 4, indices, 6, m_texture.get());
    }
}
