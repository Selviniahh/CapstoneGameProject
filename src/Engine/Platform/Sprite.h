#pragma once
#include "Texture.h"
#include "Vector2.h"
#include "Rect.h"
#include "Color.h"
#include "GraphicsDevice.h"

namespace ETG
{
    //Lightweight sprite description (texture + transform), replacement for sf::Sprite.
    //Drawing happens through the SpriteBatch which computes the transformed quad.
    class Sprite
    {
    public:
        Sprite() = default;

        void setTexture(const Texture& texture)
        {
            m_texture = &texture;
            if (m_textureRect == IntRect())
            {
                m_textureRect = IntRect(0, 0, static_cast<int>(texture.getSize().x), static_cast<int>(texture.getSize().y));
            }
        }

        void setTextureRect(const IntRect& rect) { m_textureRect = rect; }
        void setPosition(const Vector2f& pos) { m_position = pos; }
        void setPosition(float x, float y) { m_position = {x, y}; }
        void setScale(const Vector2f& scale) { m_scale = scale; }
        void setScale(float x, float y) { m_scale = {x, y}; }
        void setOrigin(const Vector2f& origin) { m_origin = origin; }
        void setOrigin(float x, float y) { m_origin = {x, y}; }
        void setRotation(float degrees) { m_rotation = degrees; }
        void rotate(float degrees) { m_rotation += degrees; }
        void setColor(const Color& color) { m_color = color; }
        //Which fragment program this sprite is drawn with (see ETG::ShaderEffect)
        void setEffect(ShaderEffect effect) { m_effect = effect; }
        //The vec4 that program is handed; what it means is the effect's business (see ShaderEffectParams)
        void setEffectParams(const ShaderEffectParams& params) { m_effectParams = params; }

        [[nodiscard]] const Texture* getTexture() const { return m_texture; }
        [[nodiscard]] const IntRect& getTextureRect() const { return m_textureRect; }
        [[nodiscard]] const Vector2f& getPosition() const { return m_position; }
        [[nodiscard]] const Vector2f& getScale() const { return m_scale; }
        [[nodiscard]] const Vector2f& getOrigin() const { return m_origin; }
        [[nodiscard]] float getRotation() const { return m_rotation; }
        [[nodiscard]] const Color& getColor() const { return m_color; }
        [[nodiscard]] ShaderEffect getEffect() const { return m_effect; }
        [[nodiscard]] const ShaderEffectParams& getEffectParams() const { return m_effectParams; }

    private:
        const Texture* m_texture = nullptr;
        IntRect m_textureRect{};
        Vector2f m_position{0.f, 0.f};
        Vector2f m_scale{1.f, 1.f};
        Vector2f m_origin{0.f, 0.f};
        float m_rotation = 0.f;
        Color m_color = Color::White;
        ShaderEffect m_effect = ShaderEffect::None;
        ShaderEffectParams m_effectParams{};
    };
}
