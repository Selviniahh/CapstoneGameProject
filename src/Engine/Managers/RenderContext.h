#pragma once
#include <memory>
#include "../Platform/Platform.h"

//Global window/render context: the window, its screen size and the world view.
//Frame timing lives in Time.h, the debug font in DebugTexts.h.
namespace ETG::RenderContext
{
    extern std::shared_ptr<ETG::RenderWindow> Window;
    extern ETG::Vector2u ScreenSize;
    extern float DefaultScale;

    //For Zooming
    extern ETG::View MainView;

    //Initialize global variables
    void Initialize(const std::shared_ptr<ETG::RenderWindow>& window);
}
