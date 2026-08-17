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
        void drawRectOutline(const FloatRect& rect, const Color& color, float thickness, float depth);

        void end(ETG::RenderWindow& window);

        static void SimpleDraw(const std::shared_ptr<ETG::Texture>& tex, const ETG::Vector2f& pos, float Rotation = 0, ETG::Vector2f origin = {1, 1}, float Scale = 1, float depth = 1);
        static void Draw(const GameObjectBase::DrawProperties& DrawProperties);

        static void AddDebugCircle(const ETG::Vector2f& pos, float radius = 10.f, const ETG::Color& color = ETG::Color::Red, float thickness = 1.0f);

        //Queue a single 1x1 (green) pixel into the global batch; used for origin/point visualization
        static bool DrawSinglePixelAtLoc(const ETG::Vector2f& Loc, ETG::Vector2i scale = {1, 1}, float rotation = 0);

        //Queue a small colored cross centered on Loc. The default depth sorts in front of everything, because a
        //marker hidden behind the sprite it is annotating tells you nothing
        static void DrawDebugCross(const ETG::Vector2f& Loc, const ETG::Color& color, float armLength = 2.f, float depth = -1000.f);

        //Batched counterpart to RenderWindow::drawLine, and it exists for the two things that one cannot do.
        //An immediate draw lands on the screen the instant it is called, which for anything inside an object's
        //Draw() is before the batch flushes - so it ends up underneath every sprite in the frame regardless of
        //what it was annotating. And its width is a GPU line strip, always one screen pixel no matter the zoom.
        //This one is a rotated quad instead: it sorts by depth like everything else, and thickness is in world
        //pixels, so the view's 5x magnification makes it 5x thicker on screen the same way it does for sprites
        static void DrawDebugLine(const ETG::Vector2f& from, const ETG::Vector2f& to, const ETG::Color& color, float thickness = 1.f, float depth = -1000.f);

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
            ETG::ShaderEffect effect; //Which fragment program this quad is submitted with
            ETG::ShaderEffectParams effectParams; //The uniform that program is given, so it can differ per quad
        };

        std::vector<SpriteQuad> sprites;
        int drawCounter = 0;
    };

    extern SpriteBatch GlobSpriteBatch;
}
