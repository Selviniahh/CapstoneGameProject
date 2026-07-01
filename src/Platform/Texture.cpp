#include "Texture.h"
#include <cstring>
#include <SDL3/SDL.h>
#include <stb_image.h>
#include "RenderWindow.h"

namespace ETG
{
    //------------------------------------ Image ------------------------------------
    void Image::create(const unsigned width, const unsigned height, const Color& color)
    {
        m_size = {width, height};
        m_pixels.assign(static_cast<size_t>(width) * height * 4, 0);
        for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i)
        {
            m_pixels[i * 4 + 0] = color.r;
            m_pixels[i * 4 + 1] = color.g;
            m_pixels[i * 4 + 2] = color.b;
            m_pixels[i * 4 + 3] = color.a;
        }
    }

    bool Image::loadFromFile(const std::string& path)
    {
        int w = 0, h = 0, channels = 0;
        stbi_uc* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
        if (!data) return false;

        m_size = {static_cast<unsigned>(w), static_cast<unsigned>(h)};
        m_pixels.assign(data, data + static_cast<size_t>(w) * h * 4);
        stbi_image_free(data);
        return true;
    }

    void Image::copy(const Image& source, const unsigned destX, const unsigned destY, const IntRect& sourceRect)
    {
        IntRect srcRect = sourceRect;
        if (srcRect.width == 0 || srcRect.height == 0)
            srcRect = IntRect(0, 0, static_cast<int>(source.m_size.x), static_cast<int>(source.m_size.y));

        for (int y = 0; y < srcRect.height; ++y)
        {
            const unsigned dy = destY + y;
            const unsigned sy = srcRect.top + y;
            if (dy >= m_size.y || sy >= source.m_size.y) continue;

            for (int x = 0; x < srcRect.width; ++x)
            {
                const unsigned dx = destX + x;
                const unsigned sx = srcRect.left + x;
                if (dx >= m_size.x || sx >= source.m_size.x) continue;

                std::memcpy(&m_pixels[(static_cast<size_t>(dy) * m_size.x + dx) * 4],
                            &source.m_pixels[(static_cast<size_t>(sy) * source.m_size.x + sx) * 4], 4);
            }
        }
    }

    //------------------------------------ Texture ------------------------------------
    Texture::~Texture()
    {
        destroy();
    }

    Texture::Texture(const Texture& other)
    {
        if (other.m_handle) loadFromImage(other.m_image);
    }

    Texture& Texture::operator=(const Texture& other)
    {
        if (this == &other) return *this;
        destroy();
        m_size = {0, 0};
        m_image = Image{};
        if (other.m_handle) loadFromImage(other.m_image);
        return *this;
    }

    Texture::Texture(Texture&& other) noexcept
        : m_handle(other.m_handle), m_size(other.m_size), m_image(std::move(other.m_image))
    {
        other.m_handle = nullptr;
        other.m_size = {0, 0};
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this == &other) return *this;
        destroy();
        m_handle = other.m_handle;
        m_size = other.m_size;
        m_image = std::move(other.m_image);
        other.m_handle = nullptr;
        other.m_size = {0, 0};
        return *this;
    }

    void Texture::destroy()
    {
        if (m_handle && RenderWindow::GetRenderer())
        {
            SDL_DestroyTexture(m_handle);
        }
        m_handle = nullptr;
    }

    bool Texture::loadFromFile(const std::string& path)
    {
        Image image;
        if (!image.loadFromFile(path)) return false;
        return loadFromImage(image);
    }

    bool Texture::loadFromImage(const Image& image)
    {
        SDL_Renderer* renderer = RenderWindow::GetRenderer();
        if (!renderer) return false;

        const Vector2u size = image.getSize();
        if (size.x == 0 || size.y == 0) return false;

        SDL_Texture* newTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
                                                    static_cast<int>(size.x), static_cast<int>(size.y));
        if (!newTexture) return false;

        if (!SDL_UpdateTexture(newTexture, nullptr, image.getPixelsPtr(), static_cast<int>(size.x) * 4))
        {
            SDL_DestroyTexture(newTexture);
            return false;
        }

        //Pixel art: nearest sampling keeps sprites crisp when the view is zoomed
        SDL_SetTextureScaleMode(newTexture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

        destroy();
        m_handle = newTexture;
        m_size = size;
        m_image = image;
        return true;
    }
}
