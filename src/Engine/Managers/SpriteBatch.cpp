#include "SpriteBatch.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <SDL3/SDL.h>
#include "RenderContext.h"
#include "../../Utils/TextureUtils.h"

//A forward declaration I've never seen before
namespace ETG
{
    SpriteBatch GlobSpriteBatch;
}


void ETG::SpriteBatch::begin()
{
    sprites.clear();
    drawCounter = 0;
}

void ETG::SpriteBatch::Draw(const Sprite& sprite, const float depth)
{
    const ETG::Texture* texture = sprite.getTexture();
    if (!texture) return;

    const ETG::IntRect& texRect = sprite.getTextureRect();
    const ETG::Vector2f& position = sprite.getPosition();
    const ETG::Vector2f& scale = sprite.getScale();
    const ETG::Vector2f& origin = sprite.getOrigin();
    const ETG::Color& color = sprite.getColor();

    //Same transform order as SFML: translate(position) * rotate * scale * translate(-origin)
    const float rad = sprite.getRotation() * (std::numbers::pi_v<float> / 180.f);
    const float cosR = std::cos(rad);
    const float sinR = std::sin(rad);

    //Yerel koordinatlardan dunya/ekran koordinatlarina donustur
    const auto transformPoint = [&](const float localX, const float localY) -> ETG::Vector2f
    {
        //Noktayı origin’e göre hizalayıp ölçeklendiriyor. Örneğin origin, sprite’ın merkeziyse dönüş merkez etrafında gerçekleşir.
        const float x = (localX - origin.x) * scale.x;
        const float y = (localY - origin.y) * scale.y;
        
        //Rotation matrix 
        float rotatedX = x * cosR - y * sinR;
        float rotatedY = x * sinR + y * cosR;
        
        return {
            position.x + rotatedX,
            position.y + rotatedY
        };
    };

    const auto w = static_cast<float>(texRect.width);
    const auto h = static_cast<float>(texRect.height);

    // Texture coordinates (pixels)
    const auto left = static_cast<float>(texRect.left);
    const float right = left + w;
    const auto top = static_cast<float>(texRect.top);
    const float bottom = top + h;

    SpriteQuad quad{
        Vertex{transformPoint(0.f, 0.f), {left, top}, color},
        Vertex{transformPoint(w, 0.f), {right, top}, color},
        Vertex{transformPoint(w, h), {right, bottom}, color},
        Vertex{transformPoint(0.f, h), {left, bottom}, color},
        texture, depth, drawCounter++
    };

    sprites.push_back(quad);
}

void ETG::SpriteBatch::end(ETG::RenderWindow& window)
{
    if (sprites.empty()) return;

    SDL_Renderer* renderer = window.getNativeRenderer();
    if (!renderer) return;

    // Sort sprites by draw order. If draw order same, draw the one has higher order.
    std::ranges::sort(sprites,
                      [](const SpriteQuad& a, const SpriteQuad& b)
                      {
                          if (a.depth == b.depth)
                              return a.drawOrder < b.drawOrder;
                          return a.depth > b.depth;
                      });

    std::vector<SDL_Vertex> vertices;
    std::vector<int> indices;
    vertices.reserve(sprites.size() * 4);
    indices.reserve(sprites.size() * 6);

    const auto flush = [&](const ETG::Texture* texture)
    {
        if (vertices.empty()) return;
        SDL_RenderGeometry(renderer, texture ? texture->getNativeHandle() : nullptr,
                           vertices.data(), static_cast<int>(vertices.size()),
                           indices.data(), static_cast<int>(indices.size()));
        vertices.clear();
        indices.clear();
    };

    const ETG::Texture* currentTexture = sprites[0].texture;

    for (const auto& quad : sprites)
    {
        if (quad.texture != currentTexture)
        {
            flush(currentTexture);
            currentTexture = quad.texture;
        }

        const ETG::Vector2u texSize = quad.texture->getSize();
        const float texW = texSize.x > 0 ? static_cast<float>(texSize.x) : 1.f;
        const float texH = texSize.y > 0 ? static_cast<float>(texSize.y) : 1.f;

        const Vertex* quadVertices[4] = {&quad.v0, &quad.v1, &quad.v2, &quad.v3};
        const int base = static_cast<int>(vertices.size());

        for (const Vertex* v : quadVertices)
        {
            const ETG::Vector2f screenPos = window.worldToScreen(v->position);
            SDL_Vertex sdlVertex;
            sdlVertex.position = SDL_FPoint{screenPos.x, screenPos.y};
            sdlVertex.color = SDL_FColor{v->color.r / 255.f, v->color.g / 255.f, v->color.b / 255.f, v->color.a / 255.f};
            sdlVertex.tex_coord = SDL_FPoint{v->texCoords.x / texW, v->texCoords.y / texH};
            vertices.push_back(sdlVertex);
        }

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    flush(currentTexture);
}

void ETG::SpriteBatch::SimpleDraw(const std::shared_ptr<ETG::Texture>& tex, const ETG::Vector2f& pos, float Rotation, ETG::Vector2f origin, float Scale, float depth)
{
    ETG::Sprite frame;
    frame.setTexture(*tex);
    frame.setScale(Scale, Scale);
    frame.setPosition(pos); // Position it at the specified location
    frame.setRotation(Rotation);
    frame.setOrigin(origin);
    frame.setColor(ETG::Color::White);

    GlobSpriteBatch.Draw(frame, depth);
}

void ETG::SpriteBatch::Draw(const GameObjectBase::DrawProperties& DrawProperties)
{
    ETG::Sprite frame;
    frame.setTexture(*DrawProperties.Texture);
    frame.setScale(DrawProperties.Scale);
    frame.setPosition(DrawProperties.Position); // Position it at the specified location
    frame.setRotation(DrawProperties.Rotation);
    frame.setOrigin(DrawProperties.Origin);
    frame.setColor(DrawProperties.Color);
    GlobSpriteBatch.Draw(frame, DrawProperties.Depth);
}

void ETG::SpriteBatch::AddDebugCircle(const ETG::Vector2f& pos, const float radius, const ETG::Color& color, const float thickness)
{
    ETG::CircleShape circle{radius};
    circle.setPosition(pos.x - radius, pos.y - radius);
    circle.setFillColor(ETG::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(thickness);
    RenderContext::Window->draw(circle);
}


void ETG::SpriteBatch::drawRectOutline(const ETG::FloatRect& rect, const ETG::Color& color, float thickness, float depth)
{
    // Get a shared pixel texture
    static std::shared_ptr<ETG::Texture> pixelTex = GetPixelTexture();

    // Top edge
    ETG::Sprite topEdge;
    topEdge.setTexture(*pixelTex);
    topEdge.setPosition(rect.left, rect.top);
    topEdge.setScale(rect.width, thickness);
    topEdge.setColor(color);
    Draw(topEdge, depth);

    // Bottom edge
    ETG::Sprite bottomEdge;
    bottomEdge.setTexture(*pixelTex);
    bottomEdge.setPosition(rect.left, rect.top + rect.height - thickness);
    bottomEdge.setScale(rect.width, thickness);
    bottomEdge.setColor(color);
    Draw(bottomEdge, depth);

    // Left edge
    ETG::Sprite leftEdge;
    leftEdge.setTexture(*pixelTex);
    leftEdge.setPosition(rect.left, rect.top);
    leftEdge.setScale(thickness, rect.height);
    leftEdge.setColor(color);
    Draw(leftEdge, depth);

    // Right edge
    ETG::Sprite rightEdge;
    rightEdge.setTexture(*pixelTex);
    rightEdge.setPosition(rect.left + rect.width - thickness, rect.top);
    rightEdge.setScale(thickness, rect.height);
    rightEdge.setColor(color);
    Draw(rightEdge, depth);
}

bool ETG::SpriteBatch::DrawSinglePixelAtLoc(const ETG::Vector2f& Loc, const ETG::Vector2i scale, const float rotation)
{
    static ETG::Texture tex;
    static bool isLoaded = false;
    if (!isLoaded)
    {
        ETG::Image greenPixel;
        greenPixel.create(1, 1, ETG::Color::Green);
        if (!tex.loadFromImage(greenPixel)) return true;
        isLoaded = true;
    }

    // Set up the sprite with the 1x1 green texture
    //NOTE: Order of sprite setting should be Texture -> Origin -> Scale -> Position
    ETG::Sprite frame;
    frame.setTexture(tex);
    frame.setOrigin(0.5f, 0.5f); // Center of 1x1 pixel
    frame.setScale(scale.x, scale.y);
    frame.setPosition(Loc); // Position it at the specified location
    frame.setRotation(rotation);

    // Draw the sprite
    GlobSpriteBatch.Draw(frame, -1);
    return false;
}
