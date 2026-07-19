#pragma once
#include "../Engine/Platform/Platform.h"
#include "../Engine/Platform/Platform.h"
#include <memory>

// A small free function to retrieve a 1×1 pixel texture (or any color).
namespace ETG
{
    inline std::shared_ptr<ETG::Texture> GetPixelTexture()
    {
        static std::shared_ptr<ETG::Texture> whiteTex = nullptr;
        if (!whiteTex)
        {
            whiteTex = std::make_shared<ETG::Texture>();
            ETG::Image img;
            img.create(1, 1, ETG::Color::White);
            whiteTex->loadFromImage(img);
        }
        return whiteTex;
    };
}
