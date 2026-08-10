#include "SpriteBatch.h"

#include <algorithm>
#include <cmath>
#include <numbers>
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
        texture, depth, drawCounter++, sprite.getEffect(), sprite.getEffectParams()
    };

    sprites.push_back(quad);
}

void ETG::SpriteBatch::end(ETG::RenderWindow& window)
{
    if (sprites.empty()) return;

    // Sort sprites by draw order. If draw order same, draw the one has higher order.
    std::ranges::sort(sprites,
                      [](const SpriteQuad& a, const SpriteQuad& b)
                      {
                          if (a.depth == b.depth)
                              return a.drawOrder < b.drawOrder;
                          return a.depth > b.depth;
                      });

    std::vector<ETG::GfxVertex> vertices;
    std::vector<std::uint16_t> indices;
    vertices.reserve(sprites.size() * 4);
    indices.reserve(sprites.size() * 6);

    //A run of quads sharing a texture *and* a fragment program is one draw call. The view is
    //submitted sequentially, so flushing at every change keeps the sorted order intact.
    const ETG::Texture* currentTexture = sprites[0].texture;
    ETG::ShaderEffect currentEffect = sprites[0].effect;
    ETG::ShaderEffectParams currentEffectParams = sprites[0].effectParams;

    const auto flush = [&]
    {
        if (vertices.empty()) return;
        ETG::GraphicsDevice::DrawIndexed(vertices.data(), static_cast<std::uint32_t>(vertices.size()),
                                         indices.data(), static_cast<std::uint32_t>(indices.size()),
                                         currentTexture, currentEffect, currentEffectParams);
        vertices.clear();
        indices.clear();
    };

    for (const auto& quad : sprites)
    {
        //A batch is also capped by the 16 bit index buffer: 4 vertices per quad, so 16384 quads.
        //The uniform is set once per submit, so quads on the same program but different parameters
        //cannot share one: a flashing enemy is its own draw call for as long as it flashes.
        if (quad.texture != currentTexture || quad.effect != currentEffect
            || quad.effectParams != currentEffectParams || vertices.size() + 4 > 65536)
        {
            flush();
            currentTexture = quad.texture;
            currentEffect = quad.effect;
            currentEffectParams = quad.effectParams;
        }

        const ETG::Vector2u texSize = quad.texture->getSize();
        const float texW = texSize.x > 0 ? static_cast<float>(texSize.x) : 1.f;
        const float texH = texSize.y > 0 ? static_cast<float>(texSize.y) : 1.f;

        const Vertex* quadVertices[4] = {&quad.v0, &quad.v1, &quad.v2, &quad.v3};
        const auto base = static_cast<std::uint16_t>(vertices.size());

        for (const Vertex* v : quadVertices)
        {
            const ETG::Vector2f screenPos = window.worldToScreen(v->position);
            vertices.push_back(ETG::GfxVertex{
                screenPos.x, screenPos.y,
                v->texCoords.x / texW, v->texCoords.y / texH,
                ETG::PackColor(v->color)
            });
        }

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    flush();
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
    //An object whose properties carry no texture draws nothing. DrawProperties holds a RAW pointer copied out
    //of the object's shared_ptr by ComputeDrawProperties, so it is null for anything that has not published
    //its properties since it got a texture - and dereferencing it here is a segfault inside the renderer,
    //several frames of stack away from whichever object actually forgot to republish
    if (!DrawProperties.Texture) return;

    ETG::Sprite frame;
    frame.setTexture(*DrawProperties.Texture);
    frame.setScale(DrawProperties.Scale);
    frame.setPosition(DrawProperties.Position); // Position it at the specified location
    frame.setRotation(DrawProperties.Rotation);
    frame.setOrigin(DrawProperties.Origin);
    frame.setColor(DrawProperties.Color);
    frame.setEffect(DrawProperties.Effect);
    frame.setEffectParams(DrawProperties.EffectParams);
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

//Sizes are in world pixels; MainView is zoomed 5x, so a 5px cross reads clearly without swamping a 27x7 gun
void ETG::SpriteBatch::DrawDebugCross(const ETG::Vector2f& Loc, const ETG::Color& color, const float armLength, const float depth)
{
    static std::shared_ptr<ETG::Texture> pixelTex = GetPixelTexture();

    // A cross rather than a single pixel: on a 27x7 gun sprite a lone dot disappears into the artwork, while the
    // arms stay readable and still point at exactly one pixel - the one where they meet.
    const float span = armLength * 2.f + 1.f;

    //NOTE: Order of sprite setting should be Texture -> Origin -> Scale -> Position
    ETG::Sprite horizontal;
    horizontal.setTexture(*pixelTex);
    horizontal.setOrigin(0.5f, 0.5f);
    horizontal.setScale(span, 1.f);
    horizontal.setPosition(Loc);
    horizontal.setColor(color);
    GlobSpriteBatch.Draw(horizontal, depth);

    ETG::Sprite vertical;
    vertical.setTexture(*pixelTex);
    vertical.setOrigin(0.5f, 0.5f);
    vertical.setScale(1.f, span);
    vertical.setPosition(Loc);
    vertical.setColor(color);
    GlobSpriteBatch.Draw(vertical, depth);
}
