// Managers/SpriteBatch.h
#pragma once
#include <memory>
#include <vector>
#include "../Platform/Platform.h"
#include "../Core/GameObjectBase.h"

namespace ETG
{
    class SpriteBatch
    {
    public:
        SpriteBatch() = default;

        void begin();

        //NOTE: Sprites added to a batch are transformed based on the active view
        void Draw(const Sprite& sprite, float depth);
        void drawRectOutline(const ETG::FloatRect& rect, const ETG::Color& color, float thickness, float depth);

        void end(ETG::RenderWindow& window);

        static void SimpleDraw(const std::shared_ptr<ETG::Texture>& tex, const ETG::Vector2f& pos, float Rotation = 0, ETG::Vector2f origin = {1, 1}, float Scale = 1, float depth = 1);
        static void Draw(const GameObjectBase::DrawProperties& DrawProperties);

        static void AddDebugCircle(const ETG::Vector2f& pos, float radius = 10.f, const ETG::Color& color = ETG::Color::Red, float thickness = 1.0f);

    private:
        struct Vertex
        {
            ETG::Vector2f position; //World space, transformed by the active view in end()
            ETG::Vector2f texCoords; //Pixel coordinates inside the texture
            ETG::Color color;
        };

        struct SpriteQuad
        {
            Vertex v0;
            Vertex v1;
            Vertex v2;
            Vertex v3;
            const ETG::Texture* texture;
            float depth;
            int drawOrder;
        };

        std::vector<SpriteQuad> sprites;
        int drawCounter = 0;
    };

    extern SpriteBatch GlobSpriteBatch;
}
