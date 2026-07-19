#include "Globals.h"
#include <chrono>
#include "AssetManager.h"
#include "SpriteBatch.h"

namespace ETG::Globals
{
    float FrameTick = 0.0f;
    float DeltaTime = 0.0f;
    float ElapsedTimeSeconds = 0.0f;
    float DefaultScale = 1;
    std::shared_ptr<ETG::RenderWindow> Window = nullptr;
    std::unique_ptr<ETG::Font> Font;
    ETG::Vector2u ScreenSize;
    ETG::View MainView;

    using Clock = std::chrono::steady_clock;
    static Clock::time_point StartTime;
    static Clock::time_point LastTickTime;

    void Initialize(const std::shared_ptr<ETG::RenderWindow>& window)
    {
        Window = window;
        ScreenSize = window->getSize();
        GlobSpriteBatch.begin();

        StartTime = Clock::now();
        LastTickTime = StartTime;

        //Load font
        Font = std::make_unique<ETG::Font>();
        if (!Font->loadFromFile(AssetManager::Resolve("Fonts/SegoeUI.ttf")))
        {
            throw std::runtime_error("Failed to load font");
        }

        //NOTE: Set camera Location and zoom. After enemy, UI, Gun, Hero are handled, better camera and hero locations will be implemented.
        MainView = window->getDefaultView();
        MainView.setCenter(0.f, 0.f);
        MainView.zoom(0.2f);
    }

    void Update()
    {
        const auto now = Clock::now();

        //Counter starts the beginning in runtime and never stops
        ElapsedTimeSeconds = std::chrono::duration<float>(now - StartTime).count();

        //Calculate tick. In 60fps it should be: 0.016
        FrameTick = std::chrono::duration<float>(now - LastTickTime).count();
        LastTickTime = now;
        DeltaTime = FrameTick;
    }

    void ResetTick()
    {
        LastTickTime = Clock::now();
    }

    bool DrawSinglePixelAtLoc(const ETG::Vector2f& Loc, const ETG::Vector2i scale, const float rotation)
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
}

//Operator overloads
std::ostream& operator<<(std::ostream& lhs, const ETG::Vector2<int>& rhs)
{
    return lhs << "X: " << rhs.x << " Y: " << rhs.y << std::endl;
}

std::ostream& operator<<(std::ostream& lhs, const ETG::IntRect& rhs)
{
    return lhs << "Size: " << rhs.getSize()
        << "Height: " << rhs.height
        << " Width: " << rhs.width
        << " Top: " << rhs.top
        << " Left:" << rhs.left
        << std::endl << "Position: " << rhs.getPosition() << std::endl;
}

std::ostream& operator<<(std::ostream& lhs, const ETG::FloatRect& rhs)
{
    return lhs << "Left: " << rhs.left
        << ", Top: " << rhs.top
        << ", Width: " << rhs.width
        << ", Height: " << rhs.height << " Size: " << std::endl;
}

std::ostream& operator<<(std::ostream& lhs, const ETG::Vector2<float>& rhs)
{
    return lhs << "X: " << rhs.x << " Y: " << rhs.y << std::endl;
};
