#include "Animation.h"

#include <iostream>

#include "../Managers/Globals.h"
#include "../Managers/SpriteBatch.h"
#include <memory>

Animation::Animation(const std::shared_ptr<ETG::Texture>& texture, const float eachFrameSpeed, const int frameX, const int frameY, const int row)
    : FrameX(frameX), FrameY(frameY), FrameInterval(eachFrameSpeed), Texture(texture)
{
    //This line fixed very very important bug. Since last month, sometimes just one animation frame not playing and sometimes all animation frames are working. The problem was just order of initialization in constructor
    //I tried to assign `AnimTimeLeft` to `FrameInterval` when `FrameInterval`is not initialized yet. This causing all arbitrary number and somehow like %70 everything works and %30 almost always just Hero's Back_Diagonal animation is not playing. Instead assigning in Constructor body fixed it  
    AnimTimeLeft = FrameInterval;

    const int frameXSize = Texture->getSize().x / frameX;
    const int frameYSize = Texture->getSize().y / frameY;

    for (int i = 0; i < frameX; ++i)
    {
        int frameWidth = i * frameXSize;
        int frameHeight = (row - 1) * frameYSize;

        FrameRects.emplace_back(frameWidth, frameHeight, frameXSize, frameYSize);
    }
}

void Animation::Update()
{
    if (!Active || !Texture || Texture->getSize().x == 0) throw std::runtime_error("Something is wrong");
    if (AnimTimeLeft > 9999999.0f || AnimTimeLeft < -1000) throw std::runtime_error("Animation Time is so big");

    //NOTE: Skip playing next frame if we only want to play the last frame
    if (IsOnLastFrame) return;
    
    AnimTimeLeft -= ETG::Globals::FrameTick;
    if (AnimTimeLeft <= 0)
    {
        CurrentFrame++;
        CurrentFrame = CurrentFrame >= FrameX ? 0 : CurrentFrame;
        AnimTimeLeft = FrameInterval;
    }

    CurrRect = FrameRects[CurrentFrame];
}

void Animation::Draw(const ETG::Vector2f position, const float layerDepth, const float rotation) const
{
    if (!Active || !Texture) return;

    ETG::Sprite frame;
    frame.setTexture(*Texture);
    frame.setTextureRect(FrameRects[CurrentFrame]);
    frame.setPosition(position);
    frame.setColor(ETG::Color::White);
    frame.setRotation(rotation);
    frame.setOrigin(Origin);
    frame.setScale(ETG::Globals::DefaultScale * flipX, ETG::Globals::DefaultScale);

    ETG::GlobSpriteBatch.Draw(frame, layerDepth);
}

void Animation::Draw(const std::shared_ptr<ETG::Texture>& texture, const ETG::Vector2f position, const ETG::Color color, const float rotation, const ETG::Vector2f origin, const ETG::Vector2f scale, const float depth) const
{
    if (!Active || !Texture) return;

    ETG::Sprite frame;
    frame.setTexture(*Texture);
    frame.setTextureRect(FrameRects[CurrentFrame]);
    frame.setPosition(position);
    frame.setColor(color);
    frame.rotate(rotation);
    frame.setOrigin(origin);
    frame.setScale(scale);

    ETG::GlobSpriteBatch.Draw(frame, depth);
}

void Animation::Restart()
{
    CurrentFrame = 0;
    AnimTimeLeft = FrameInterval;
}

std::shared_ptr<ETG::Texture> Animation::GetCurrentFrameAsTexture() const
{
    if (!Texture) return nullptr;

    // Ensure the cache is large enough
    if (textureCache.size() <= CurrentFrame) textureCache.resize(CurrentFrame + 1);

    // Create texture if it doesn't exist yet
    if (!textureCache[CurrentFrame] || textureCache[CurrentFrame]->getSize().x == 0)
    {
        const ETG::IntRect sourceRectangle = FrameRects[CurrentFrame];
        ETG::Image frameImage;
        frameImage.create(sourceRectangle.width, sourceRectangle.height);
        frameImage.copy(Texture->copyToImage(), 0, 0, sourceRectangle);

        textureCache[CurrentFrame] = std::make_shared<ETG::Texture>();
        textureCache[CurrentFrame]->loadFromImage(frameImage);
    }
    return textureCache[CurrentFrame];
}

bool Animation::IsAnimationFinished() const
{
    return CurrentFrame == FrameX - 1;
}

float Animation::GetTotalAnimationTime() const
{
    return (float)FrameX * FrameInterval;
}

void Animation::PlayOnlyLastFrame()
{
    if (IsOnLastFrame) return; // Already on last frame

    OriginalFrameInterval = FrameInterval; //Set original frame interval so that when we StopPlayingLastFrame, we can return back to initial FrameInterval 
    CurrentFrame = FrameX - 1; // Set to last frame
    AnimTimeLeft = 9999999.0f; // Effectively pause. Idk if there's a better way t handle this 
    CurrRect = FrameRects[CurrentFrame]; // Update the current rectangle
    IsOnLastFrame = true;
}

void Animation::StopPlayingLastFrame()
{
    if (!IsOnLastFrame) return;

    FrameInterval = OriginalFrameInterval;
    AnimTimeLeft = FrameInterval; // Reset the animation timer
    IsOnLastFrame = false;
}

bool Animation::IsPlayingLastFrame() const
{
    return IsOnLastFrame;
}

Animation Animation::CreateSpriteSheet(const std::string& RelativePath, const std::string& FileName, const std::string& Extension, const float eachFrameSpeed, bool IsSingleSprite)
{
    std::vector<ETG::Image> imageArr;
    int counter = 0;
    int totalWidth = 0, maxHeight = 0;
    std::string basePath = (std::filesystem::path(RESOURCE_PATH) / RelativePath / FileName).string();
    char LastChar = basePath[basePath.length() - 1];
    std::string filePath;

    if (static_cast<int>(LastChar) >= 48 && static_cast<int>(LastChar) <= 57)
    {
        counter = LastChar - '0';
        basePath[basePath.length() - 1] = counter + '0';
        basePath.erase(basePath.length() - 1, basePath.length() - 2);
        filePath = basePath + std::to_string(counter) + "." + Extension;
    }
    else
    {
        filePath = basePath + "." + Extension;
    }

    // Check firstly if filepath is valid
    if (!std::filesystem::exists(filePath)) throw std::runtime_error("File not found at: " + filePath);

    // Load all the given textures 
    while (true)
    {
        ETG::Image singleImage;
        filePath = IsSingleSprite ? basePath + "." + Extension : basePath + std::to_string(counter) + "." + Extension;
        if (!std::filesystem::exists(filePath)) break;

        if (!singleImage.loadFromFile(filePath))
            throw std::runtime_error("Failed to load image: " + filePath);

        imageArr.push_back(singleImage);
        totalWidth += int(singleImage.getSize().x);
        maxHeight = std::max(maxHeight, int(singleImage.getSize().y));
        counter++;
        if (IsSingleSprite) break;
    }

    // Create the spritesheet as image
    ETG::Image spriteImage;
    spriteImage.create(totalWidth, maxHeight, ETG::Color::Transparent);

    // Copy images into the image
    unsigned int xOffset = 0;
    std::vector<ETG::Rect<int>> customFrameRects;
    for (auto& image : imageArr)
    {
        spriteImage.copy(image, xOffset, 0);
        // Create frame rect based on the actual image size
        ETG::IntRect frameRect(xOffset, 0, image.getSize().x, image.getSize().y);
        customFrameRects.push_back(frameRect);
        xOffset += image.getSize().x;
    }

    auto spriteTex = std::make_shared<ETG::Texture>();
    spriteTex->loadFromImage(spriteImage);

    Animation anim(spriteTex, eachFrameSpeed, int(imageArr.size()), 1);
    // Overwrite the auto-generated frame rectangles with our custom ones
    anim.FrameRects = customFrameRects;
    anim.AnimPathName = RelativePath + FileName;
    return anim;
}
