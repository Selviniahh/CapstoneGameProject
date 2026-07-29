#pragma once
#include "../Platform/Platform.h"
#include <memory>
#include <vector>
#include <boost/describe/class.hpp>

#include "../Core/GameClass.h"

//Whether an animation starts over or stops on its last frame. Registering a state's animation
//takes one of these explicitly, so a new animation cannot silently end up looping.
enum class Playback
{
    Loop, //idle, run - runs until something else changes the state
    Once  //shot, reload, dash, hit, death - plays out, then holds its last frame
};

class Animation : public GameClass
{
private:
    float AnimTimeLeft{};
    int CurrentFrame = 0;
    int FrameX;
    int FrameY;
    mutable std::vector<std::shared_ptr<ETG::Texture>> textureCache;


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

    //A one-shot animation - a shot, a reload, a death - holds its last frame instead of starting
    //over. Looping stays the default so an animation keeps its old behaviour until it opts out.
    bool Loops = true;

    //Only meaningful while Loops is false: set once the last frame has had its full time on
    //screen. Cleared by Restart.
    bool HasFinished = false;

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
    bool IsFinished() const;
    float GetTotalAnimationTime() const; //NOTE: If there are 5 frames and each frame interval is 0.1 then this will return (5 * 0.1 = 0.5)

    //Omit FileName's last number. If file's name is "SpriteSheet_001" Give "SpriteSheet_00"
    //There's no Y axis sprite sheet creation. Only X 
    static Animation CreateSpriteSheet(const std::string& RelativePath, const std::string& FileName, const std::string& Extension, float eachFrameSpeed, bool IsSingleSprite = false);

    BOOST_DESCRIBE_CLASS(Animation, (GameClass), (CurrRect, Texture, Origin, FrameRects, IsValid, flipX, Active, Loops, HasFinished), (), (FrameInterval))
};
