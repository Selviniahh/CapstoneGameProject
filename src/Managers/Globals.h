#pragma once
#include <memory>
#include <filesystem>
#include <ostream>
#include "Platform/Platform.h"

//A forward declaration I've never seen before
namespace ETG
{
    class SpriteBatch;
}

namespace ETG::Globals
{
    //Elapsed time in seconds. AKA Delta Time
    extern float FrameTick;
    extern float ElapsedTimeSeconds;

    //Window and rendering resources
    extern std::shared_ptr<ETG::RenderWindow> Window;
    extern std::unique_ptr<ETG::Font> Font;
    extern ETG::Vector2u ScreenSize;
    extern float DefaultScale;
    static int FPS = 170;

    //For Zooming
    extern ETG::View MainView;

    //Function to update elapsed time
    void Update();

    //Initialize global variables
    void Initialize(const std::shared_ptr<ETG::RenderWindow>& window);

    bool DrawSinglePixelAtLoc(const ETG::Vector2f& Loc, ETG::Vector2i scale = {1, 1}, float rotation = 0);
}

//Operator overloads
std::ostream& operator<<(std::ostream& lhs, const ETG::Vector2<int>& rhs);
std::ostream& operator<<(std::ostream& lhs, const ETG::IntRect& rhs);
std::ostream& operator<<(const std::ostream& lhs, const ETG::Vector2<float>& rhs);
std::ostream& operator<<(std::ostream& lhs, const ETG::FloatRect& rhs);
