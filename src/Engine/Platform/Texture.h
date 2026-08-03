#pragma once
#include <cstdint>
#include <string>
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"

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

    //GPU texture backed by a bgfx texture, replacement for sf::Texture.
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

        //Uploads the image to the GPU. Sprites are point sampled by default so pixel art stays
        //crisp when the view is zoomed; pass true for text/UI atlases that want smoothing.
        bool loadFromImage(const Image& image, bool linearSampling = false);

        [[nodiscard]] Vector2u getSize() const { return m_size; }
        //bgfx texture handle index, or ETG::InvalidGpuHandle when nothing is uploaded.
        [[nodiscard]] std::uint16_t getNativeHandle() const { return m_handle; }
        [[nodiscard]] const Image& copyToImage() const { return m_image; }

    private:
        void destroy();

        std::uint16_t m_handle = 0xffffu; //ETG::InvalidGpuHandle, spelled out to keep GraphicsDevice.h out of this header
        Vector2u m_size{0, 0};
        Image m_image;
        bool m_linearSampling = false;
    };
}
