#pragma once
#include "InputManager.h"
#include "../Core/Scene/Scene.h"
#include "RenderContext.h"
#include "Time.h"
#include "AssetManager.h"
#include "../../Utils/StrManipulateUtil.h"

namespace ETG
{
    class DebugText;

    //Game code queues its own lines each frame (hero state, AI info etc.) via QueueText;
    //the engine only draws generic engine-level info itself.
    class DebugTextManager
    {
        inline static std::vector<std::string> queuedTexts;
    public:
        static void QueueText(const std::string& text)
        {
            queuedTexts.push_back(text);
        };
        static void DrawQueuedTexts(ETG::RenderWindow& window);
        static void ClearQueue()
        {
            queuedTexts.clear();
        };
    };

    class DebugText
    {
    public:
        //The debug overlay owns the engine font; nothing else needs a global font
        inline static std::unique_ptr<ETG::Font> Font;

        static void LoadFont()
        {
            Font = std::make_unique<ETG::Font>();
            if (!Font->loadFromFile(AssetManager::Resolve("Fonts/SegoeUI.ttf")))
                throw std::runtime_error("Failed to load font");
        }

        static void DrawDebugText(const std::string& str, ETG::RenderWindow& window)
        {
            if (!window.isOpen()) return; //For no reason, suddenly I had to write this otherwise, the game starts crashing when I close the game
            InputManager::debugText.setString(str);
            InputManager::debugText.setPosition(SetDebugTextPos(InputManager::textPos));
            window.draw(InputManager::debugText);
        }

        static ETG::Vector2f& SetDebugTextPos(ETG::Vector2f& pos)
        {
            pos.y += 20.f; // Increment Y position for next line
            return pos;
        }
    };

    inline void DebugTextManager::DrawQueuedTexts(ETG::RenderWindow& window)
    {
        for (const auto& text : queuedTexts)
        {
            DebugText::DrawDebugText(text, window);
        }
        ClearQueue();
    }
}
