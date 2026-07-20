#include "../Managers/Time.h"
#include "Animation.h"

#include <iostream>

#include "../Managers/AssetManager.h"
#include "../Managers/RenderContext.h"
#include "../Managers/SpriteBatch.h"
#include <memory>
#include <unordered_map>

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
    
    AnimTimeLeft -= ETG::Time::FrameTick;
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
    frame.setScale(ETG::RenderContext::DefaultScale * flipX, ETG::RenderContext::DefaultScale);

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

namespace
{
    struct SheetData
    {
        std::shared_ptr<ETG::Texture> Texture;
        std::vector<ETG::IntRect> FrameRects;
    };

    struct LoadedFrames
    {
        std::vector<ETG::Image> Images;
        int TotalWidth{};
        int MaxHeight{};
    };

    std::string MakeSheetCacheKey(const std::string& relativePath,
                                  const std::string& fileName,
                                  const std::string& extension,
                                  const bool isSingleSprite)
    {
        return relativePath + "/" + fileName + "." + extension + (isSingleSprite ? "|single" : "");
    }

    LoadedFrames LoadFrames(const std::string& relativePath,
                            const std::string& fileName,
                            const std::string& extension,
                            const bool isSingleSprite)
    {
        LoadedFrames loadedFrames;
        int counter = 0;
        std::string basePath = ETG::AssetManager::Resolve(std::filesystem::path(relativePath) / fileName);
        const char lastChar = basePath.back();
        std::string filePath;

        if (lastChar >= '0' && lastChar <= '9')
        {
            counter = lastChar - '0';
            basePath.pop_back();
            filePath = basePath + std::to_string(counter) + "." + extension;
        }
        else
        {
            filePath = basePath + "." + extension;
        }

        if (!std::filesystem::exists(filePath))
            throw std::runtime_error("File not found at: " + filePath);

        while (true)
        {
            filePath = isSingleSprite
                           ? basePath + "." += extension
                           : basePath + std::to_string(counter) + "." + extension;
            if (!std::filesystem::exists(filePath)) break;

            ETG::Image image;
            if (!image.loadFromFile(filePath))
                throw std::runtime_error("Failed to load image: " + filePath);

            loadedFrames.TotalWidth += static_cast<int>(image.getSize().x);
            loadedFrames.MaxHeight = std::max(loadedFrames.MaxHeight, static_cast<int>(image.getSize().y));
            loadedFrames.Images.push_back(std::move(image));

            ++counter;
            if (isSingleSprite) break;
        }

        return loadedFrames;
    }

    SheetData StitchFrames(const LoadedFrames& loadedFrames)
    {
        ETG::Image spriteImage;
        spriteImage.create(loadedFrames.TotalWidth, loadedFrames.MaxHeight, ETG::Color::Transparent);

        unsigned int xOffset = 0;
        std::vector<ETG::IntRect> frameRects;
        frameRects.reserve(loadedFrames.Images.size());

        for (const auto& image : loadedFrames.Images)
        {
            spriteImage.copy(image, xOffset, 0);
            frameRects.emplace_back(xOffset, 0, image.getSize().x, image.getSize().y);
            xOffset += image.getSize().x;
        }

        auto texture = std::make_shared<ETG::Texture>();
        texture->loadFromImage(spriteImage);
        return {std::move(texture), std::move(frameRects)};
    }

    Animation MakeAnimation(const SheetData& sheet,
                            const float frameSpeed,
                            const std::string& animationPath)
    {
        Animation animation(sheet.Texture, frameSpeed, static_cast<int>(sheet.FrameRects.size()), 1);
        animation.FrameRects = sheet.FrameRects;
        animation.AnimPathName = animationPath;
        return animation;
    }
}

Animation Animation::CreateSpriteSheet(const std::string& RelativePath, const std::string& FileName, const std::string& Extension, const float eachFrameSpeed, bool IsSingleSprite)
{
    //Stitched sheets are cached process-wide: every AnimComp instance of the same type shares one
    //texture, so identical objects can batch together (SpriteBatch batches by texture pointer) and
    //spawning at runtime causes no disk IO.
    static std::unordered_map<std::string, SheetData> sheetCache;

    const std::string cacheKey = MakeSheetCacheKey(RelativePath, FileName, Extension, IsSingleSprite);
    const std::string animationPath = RelativePath + FileName;

    if (const auto it = sheetCache.find(cacheKey); it != sheetCache.end())
        return MakeAnimation(it->second, eachFrameSpeed, animationPath);

    LoadedFrames loadedFrames = LoadFrames(RelativePath, FileName, Extension, IsSingleSprite);
    SheetData sheet = StitchFrames(loadedFrames);
    const auto cacheEntry = sheetCache.emplace(cacheKey, std::move(sheet));

    return MakeAnimation(cacheEntry.first->second, eachFrameSpeed, animationPath);
}
