// Add these function declarations to your UIUtils.h
#pragma once

class Animation;
class AnimationManager;

#include <memory>
#include <variant>
#include "../../Animation/Animation.h"
#include "../../Animation/AnimationManager.h"

#include "../../Platform/Platform.h"

namespace UIUtils
{
    void DisplayIntRectangle(ETG::IntRect& rect);
    void DisplayAnimation(const char* label, Animation& value);
    void DisplayTexture(const std::shared_ptr<ETG::Texture>& value);
    void DisplayAnimationKey(const AnimationKey& key);
    void DisplayAnimationManager(const char* label, AnimationManager& manager);
    void DisplayColorPicker(const char* label, ETG::Color& color);

    void BeginProperty(const char* label);
    void EndProperty();
}