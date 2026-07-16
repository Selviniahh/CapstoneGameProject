#include "Texture.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "RenderWindow.h"

namespace ETG
{
    //------------------------------------ Image ------------------------------------
    namespace
    {
        //Blits must copy raw RGBA (like sf::Image::copy) instead of alpha-blending,
        //otherwise stitching sprites onto a transparent atlas would darken AA edges.
        void PrepareSurface(SDL_Surface* surface)
        {
            SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
        }
    }

    Image::~Image()
    {
        destroy();
    }

    void Image::destroy()
    {
        if (m_surface) SDL_DestroySurface(m_surface);
        m_surface = nullptr;
    }

    Image::Image(const Image& other)
    {
        if (other.m_surface)
        {
            m_surface = SDL_DuplicateSurface(other.m_surface);
            if (m_surface) PrepareSurface(m_surface);
        }
    }

    Image& Image::operator=(const Image& other)
    {
        if (this == &other) return *this;
        destroy();
        if (other.m_surface)
        {
            m_surface = SDL_DuplicateSurface(other.m_surface);
            if (m_surface) PrepareSurface(m_surface);
        }
        return *this;
    }

    Image::Image(Image&& other) noexcept : m_surface(other.m_surface)
    {
        other.m_surface = nullptr;
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        if (this == &other) return *this;
        destroy();
        m_surface = other.m_surface;
        other.m_surface = nullptr;
        return *this;
    }

    void Image::create(const unsigned width, const unsigned height, const Color& color)
    {
        destroy();
        m_surface = SDL_CreateSurface(static_cast<int>(width), static_cast<int>(height), SDL_PIXELFORMAT_RGBA32);
        if (!m_surface) return;

        PrepareSurface(m_surface);
        SDL_FillSurfaceRect(m_surface, nullptr, SDL_MapSurfaceRGBA(m_surface, color.r, color.g, color.b, color.a));
    }

    bool Image::loadFromFile(const std::string& path)
    {
        SDL_Surface* loaded = IMG_Load(path.c_str());
        if (!loaded) return false;

        destroy();
        if (loaded->format == SDL_PIXELFORMAT_RGBA32)
        {
            m_surface = loaded;
        }
        else
        {
            m_surface = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(loaded);
        }

        if (!m_surface) return false;
        PrepareSurface(m_surface);
        return true;
    }

    void Image::copy(const Image& source, const unsigned destX, const unsigned destY, const IntRect& sourceRect)
    {
        if (!m_surface || !source.m_surface) return;

        IntRect srcRect = sourceRect;
        if (srcRect.width == 0 || srcRect.height == 0)
            srcRect = IntRect(0, 0, source.m_surface->w, source.m_surface->h);

        const SDL_Rect src{srcRect.left, srcRect.top, srcRect.width, srcRect.height};
        SDL_Rect dst{static_cast<int>(destX), static_cast<int>(destY), srcRect.width, srcRect.height};
        SDL_BlitSurface(source.m_surface, &src, m_surface, &dst);
    }

    Vector2u Image::getSize() const
    {
        if (!m_surface) return {0, 0};
        return {static_cast<unsigned>(m_surface->w), static_cast<unsigned>(m_surface->h)};
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

        SDL_Surface* surface = image.getNativeSurface();
        if (!surface || surface->w == 0 || surface->h == 0) return false;

        SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!newTexture) return false;

        //Pixel art: nearest sampling keeps sprites crisp when the view is zoomed
        SDL_SetTextureScaleMode(newTexture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_BLEND);

        destroy();
        m_handle = newTexture;
        m_size = image.getSize();
        m_image = image;
        return true;
    }
}
