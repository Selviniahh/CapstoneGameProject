#pragma once
#include "Vector2.h"
#include "Color.h"

namespace ETG
{
    //Minimal replacement for sf::RectangleShape. Drawn immediately through RenderWindow::draw.
    class RectangleShape
    {
    public:
        void setPosition(const Vector2f& pos) { m_position = pos; }
        void setPosition(float x, float y) { m_position = {x, y}; }
        void setSize(const Vector2f& size) { m_size = size; }
        void setOrigin(const Vector2f& origin) { m_origin = origin; }
        void setOrigin(float x, float y) { m_origin = {x, y}; }
        void setFillColor(const Color& color) { m_fillColor = color; }
        void setOutlineColor(const Color& color) { m_outlineColor = color; }
        void setOutlineThickness(float thickness) { m_outlineThickness = thickness; }

        [[nodiscard]] const Vector2f& getPosition() const { return m_position; }
        [[nodiscard]] const Vector2f& getSize() const { return m_size; }
        [[nodiscard]] const Vector2f& getOrigin() const { return m_origin; }
        [[nodiscard]] const Color& getFillColor() const { return m_fillColor; }
        [[nodiscard]] const Color& getOutlineColor() const { return m_outlineColor; }
        [[nodiscard]] float getOutlineThickness() const { return m_outlineThickness; }

    private:
        Vector2f m_position{0.f, 0.f};
        Vector2f m_size{0.f, 0.f};
        Vector2f m_origin{0.f, 0.f};
        Color m_fillColor = Color::White;
        Color m_outlineColor = Color::Transparent;
        float m_outlineThickness = 0.f;
    };

    //Minimal replacement for sf::CircleShape. Drawn immediately through RenderWindow::draw.
    class CircleShape
    {
    public:
        CircleShape() = default;

        explicit CircleShape(const float radius) : m_radius(radius)
        {
        }

        void setRadius(float radius) { m_radius = radius; }
        void setPosition(const Vector2f& pos) { m_position = pos; }
        void setPosition(float x, float y) { m_position = {x, y}; }
        void setOrigin(float x, float y) { m_origin = {x, y}; }
        void setFillColor(const Color& color) { m_fillColor = color; }
        void setOutlineColor(const Color& color) { m_outlineColor = color; }
        void setOutlineThickness(float thickness) { m_outlineThickness = thickness; }

        [[nodiscard]] float getRadius() const { return m_radius; }
        [[nodiscard]] const Vector2f& getPosition() const { return m_position; }
        [[nodiscard]] const Vector2f& getOrigin() const { return m_origin; }
        [[nodiscard]] const Color& getFillColor() const { return m_fillColor; }
        [[nodiscard]] const Color& getOutlineColor() const { return m_outlineColor; }
        [[nodiscard]] float getOutlineThickness() const { return m_outlineThickness; }

    private:
        float m_radius = 0.f;
        Vector2f m_position{0.f, 0.f};
        Vector2f m_origin{0.f, 0.f};
        Color m_fillColor = Color::White;
        Color m_outlineColor = Color::Transparent;
        float m_outlineThickness = 0.f;
    };
}
