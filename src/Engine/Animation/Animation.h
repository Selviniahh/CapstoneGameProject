#pragma once
#include "../Platform/Platform.h"
#include <memory>
#include <vector>
#include <boost/describe/class.hpp>

#include "../Core/GameClass.h"

class Animation : public GameClass
{
private:
    float AnimTimeLeft{};
    int CurrentFrame = 0;
    int FrameX;
    int FrameY;
    mutable std::vector<std::shared_ptr<ETG::Texture>> textureCache;

    float OriginalFrameInterval{}; // Store original interval when paused
    bool IsOnLastFrame = false;    // Track if we're paused on last frame

public:
    std::string AnimPathName;
    float FrameInterval{};
    ETG::IntRect CurrRect;
    std::shared_ptr<ETG::Texture> Texture;
    mutable ETG::Vector2f Origin;
    std::vector<ETG::Rect<int>> FrameRects;
    bool IsValid = true;
    float flipX = 1.0f;
    bool Active = true;

    //NOTE: Rule of Five: Destructor, Copy Constructor, Copy Assignment, Move Constructor, Move Assignment
    Animation(const std::shared_ptr<ETG::Texture>& texture, float eachFrameSpeed, int frameX, int frameY, int row = 1);
    ~Animation() = default;
    Animation(const Animation& other) = default; // Copy constructor
    Animation(Animation&& other) = default; // Move constructor
    Animation& operator=(const Animation& other) = default; // Copy assignment operator

    ///Empty constructor. The point of this is let member Animation variables default constructed.
    ///So that I can easily initialize in Initialize() function instead of directly through constructor
    Animation() = default;

    /// \brief decrement `AnimTimeLeft` If AnimTimeLeft is 0, increment CurrentFrame, restart CurrentFrame counter
    void Update();
    void Draw(ETG::Vector2f position, float layerDepth, float rotation = 0) const;
    void Draw(const std::shared_ptr<ETG::Texture>& texture, ETG::Vector2f position, ETG::Color color, float rotation, ETG::Vector2f origin, ETG::Vector2f scale, const float depth) const;

    /// Restart the animation. Set `CurrentFrame` = 0, `AnimTimeLeft` = `EachFrameSpeed`
    void Restart();
    std::shared_ptr<ETG::Texture> GetCurrentFrameAsTexture() const;
    bool IsAnimationFinished() const;
    float GetTotalAnimationTime() const; //NOTE: If there are 5 frames and each frame interval is 0.1 then this will return (5 * 0.1 = 0.5)
    void PlayOnlyLastFrame(); //Jump to last frame and pause the animation there
    void StopPlayingLastFrame(); //Resume normal animation playback after stopping on the last frame
    bool IsPlayingLastFrame() const; //Check if the animation is paused on the last frame

    //Omit FileName's last number. If file's name is "SpriteSheet_001" Give "SpriteSheet_00"
    //There's no Y axis sprite sheet creation. Only X 
    static Animation CreateSpriteSheet(const std::string& RelativePath, const std::string& FileName, const std::string& Extension, float eachFrameSpeed, bool IsSingleSprite = false);

    BOOST_DESCRIBE_CLASS(Animation, (GameClass), (CurrRect, Texture, Origin, FrameRects, IsValid, flipX, Active), (), (FrameInterval))
};
