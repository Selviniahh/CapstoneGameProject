#include "Text.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
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
        if (m_texture && RenderWindow::GetRenderer()) SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    bool Text::ensureTexture() const
    {
        if (!m_font || !m_font->isLoaded() || m_string.empty()) return false;
        if (m_texture && m_string == m_cachedString && m_characterSize == m_cachedSize) return true;

        SDL_Renderer* renderer = RenderWindow::GetRenderer();
        if (!renderer) return false;

        TTF_Font* font = m_font->getNativeHandle();
        TTF_SetFontSize(font, static_cast<float>(m_characterSize));

        SDL_Surface* surface = TTF_RenderText_Blended(font, m_string.c_str(), 0, SDL_Color{255, 255, 255, 255});
        if (!surface) return false;

        destroyTexture();
        m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        m_textureSize = {static_cast<float>(surface->w), static_cast<float>(surface->h)};
        SDL_DestroySurface(surface);
        if (!m_texture) return false;

        //Text looks better with linear filtering when scaled by the view
        SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
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
        SDL_Renderer* renderer = window.getNativeRenderer();
        if (!renderer) return;

        const Vector2f screen = window.worldToScreen(m_position - m_origin);
        const Vector2f scale = window.worldToScreenScale();
        const SDL_FRect dst{screen.x, screen.y, m_textureSize.x * scale.x, m_textureSize.y * scale.y};

        SDL_SetTextureColorMod(m_texture, m_fillColor.r, m_fillColor.g, m_fillColor.b);
        SDL_SetTextureAlphaMod(m_texture, m_fillColor.a);
        SDL_RenderTexture(renderer, m_texture, nullptr, &dst);
    }
}
