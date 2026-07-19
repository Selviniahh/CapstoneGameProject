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

        void Draw(ETG::RenderWindow& window)
        {
            const auto& SceneObjects = Scene::Get()->SceneObjs;

            // Reset textPos to starting position each frame
            InputManager::textPos = {0.f, -20.f};

            // Draw debug information
            DrawDebugText("Total Game Objects in scene: " + std::to_string(SceneObjects.size()), window);
            DrawDebugText("Direction: " + std::to_string(InputManager::direction.x) + ", " + std::to_string(InputManager::direction.y), window);

            //Mouse position that my monitor's top left point will be (0,0)
            DrawDebugText("Screen Mouse Position: " + std::to_string(ETG::Mouse::getPosition().x) + ", " + std::to_string(ETG::Mouse::getPosition().y), window);

            //NOTE: IMPORTANT!!! Mouse position relative to view. This is so important because when hero rotating around mouse, if view zoomed or moved, we need to take View into account
            DrawDebugText("View Relative Mouse world Position: " + std::to_string(InputManager::ViewLocalMousePos.x) + " " + std::to_string(InputManager::ViewLocalMousePos.y), window);

            //Represents the mouse position as if the View were neither zoomed nor moved
            DrawDebugText("View ignored relative Mouse world Position AKA World Mouse Pos: " + std::to_string(InputManager::WorldMousePos.x) + " " + std::to_string(InputManager::WorldMousePos.y), window);

            // Moving state
            DrawDebugText("Moving: " + std::string(InputManager::IsMoving() ? "true" : "false"), window);
            DrawDebugText("WindowSize: " + std::to_string(RenderContext::ScreenSize.x) + " " + std::to_string(RenderContext::ScreenSize.y), window);

            DrawDebugText("FPS: " + std::to_string(1 / Time::FrameTick), window);
            DrawDebugText("Zoom Scale: " + std::to_string(InputManager::ZoomScale), window);

            DrawDebugText("View Center: " + std::to_string(RenderContext::MainView.getCenter().x) + " " + std::to_string(RenderContext::MainView.getCenter().y), window);
            DrawDebugText("View Size: " + std::to_string(RenderContext::MainView.getSize().x) + " " + std::to_string(RenderContext::MainView.getSize().y), window);

            DebugTextManager::DrawQueuedTexts(window);
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
