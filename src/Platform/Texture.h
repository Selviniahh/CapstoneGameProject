#pragma once
#include <string>
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"

struct SDL_Texture;
struct SDL_Surface;

namespace ETG
{
    //CPU side RGBA8 pixel buffer backed by SDL_Surface, replacement for sf::Image.
    //Files are decoded by SDL3_image.
    class Image
    {
    public:
        Image() = default;
        ~Image();

        Image(const Image& other);
        Image& operator=(const Image& other);
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        void create(unsigned width, unsigned height, const Color& color = Color::Black);
        bool loadFromFile(const std::string& path);

        //Copy the source image onto this image at (destX, destY). An empty sourceRect means "whole source image".
        void copy(const Image& source, unsigned destX, unsigned destY, const IntRect& sourceRect = IntRect());

        [[nodiscard]] Vector2u getSize() const;
        [[nodiscard]] SDL_Surface* getNativeSurface() const { return m_surface; }

    private:
        void destroy();

        SDL_Surface* m_surface = nullptr; //Always SDL_PIXELFORMAT_RGBA32
    };

    //GPU texture backed by SDL_Texture, replacement for sf::Texture.
    //A CPU side copy of the pixels is retained so textures can be copied and read back cheaply.
    class Texture
    {
    public:
        Texture() = default;
        ~Texture();

        Texture(const Texture& other);
        Texture& operator=(const Texture& other);
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        bool loadFromFile(const std::string& path);
        bool loadFromImage(const Image& image);

        [[nodiscard]] Vector2u getSize() const { return m_size; }
        [[nodiscard]] SDL_Texture* getNativeHandle() const { return m_handle; }
        [[nodiscard]] const Image& copyToImage() const { return m_image; }

    private:
        void destroy();

        SDL_Texture* m_handle = nullptr;
        Vector2u m_size{0, 0};
        Image m_image;
    };
}
