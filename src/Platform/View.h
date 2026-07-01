#pragma once
#include "Vector2.h"

namespace ETG
{
    //2D camera replacement for sf::View. Defined by a world-space center and size.
    class View
    {
    public:
        View() = default;

        View(const Vector2f& center, const Vector2f& size) : m_center(center), m_size(size)
        {
        }

        void setCenter(float x, float y) { m_center = {x, y}; }
        void setCenter(const Vector2f& center) { m_center = center; }
        void setSize(float w, float h) { m_size = {w, h}; }
        void setSize(const Vector2f& size) { m_size = size; }

        [[nodiscard]] const Vector2f& getCenter() const { return m_center; }
        [[nodiscard]] const Vector2f& getSize() const { return m_size; }

        //Multiplies the visible world size. factor < 1 zooms in, factor > 1 zooms out (same as SFML).
        void zoom(float factor) { m_size = {m_size.x * factor, m_size.y * factor}; }

        void move(float offsetX, float offsetY) { m_center += {offsetX, offsetY}; }
        void move(const Vector2f& offset) { m_center += offset; }

    private:
        Vector2f m_center{0.f, 0.f};
        Vector2f m_size{1.f, 1.f};
    };
}
