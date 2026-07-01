#pragma once
#include <string>
#include <vector>
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"

struct SDL_Texture;

namespace ETG
{
    //CPU side RGBA8 pixel buffer, replacement for sf::Image
    class Image
    {
    public:
        Image() = default;

        void create(unsigned width, unsigned height, const Color& color = Color::Black);
        bool loadFromFile(const std::string& path); //Decoded with stb_image

        //Copy the source image onto this image at (destX, destY). An empty sourceRect means "whole source image".
        void copy(const Image& source, unsigned destX, unsigned destY, const IntRect& sourceRect = IntRect());

        [[nodiscard]] Vector2u getSize() const { return m_size; }
        [[nodiscard]] const std::uint8_t* getPixelsPtr() const { return m_pixels.data(); }

    private:
        Vector2u m_size{0, 0};
        std::vector<std::uint8_t> m_pixels; //RGBA, m_size.x * m_size.y * 4
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
