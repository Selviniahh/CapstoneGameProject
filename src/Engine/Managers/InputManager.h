#pragma once

#include "../Platform/Platform.h"

namespace ETG
{
    class Hero;
    class InputManager
    {
    private:
        inline static float MinScaleSpeed = 0.001f;
        inline static float MaxScaleSpeed = 0.008f;

        inline static float ZoomFactor = 0.01f;
        inline static float MoveFactor = 5.0f;
        inline static float MinMoveSpeed = 0.25f;
        inline static float MaxMoveSpeed = 3.f;

        //Hero Pointer. To avoid reference fetching in every frame, Hero ptr defined as member variable
        inline static Hero* HeroPtr = nullptr;

    public:
        inline static ETG::Vector2f direction{};
        inline static float ZoomScale{};
        inline static ETG::Text debugText;
        inline static ETG::Vector2f textPos{0.f, -20.f}; // Start within the window
        inline static bool LeftClickRequired;

        //Mouse wheel movement accumulated for the current frame (set by GameManager::ProcessEvents)
        inline static float MouseWheelDelta = 0.f;

        //Relative to view's top left (0,0)
        inline static ETG::Vector2f ViewLocalMousePos;

        //Mouse world position that doesn't account for View's transformations  
        inline static ETG::Vector2f WorldMousePos;

        static ETG::Vector2f GetDirection() { return direction; }
        static bool IsMoving() { return direction != ETG::Vector2f(0.f, 0.f); }

        static void Update();
        static void InitializeDebugText();
        static float GetZoomScale(const ETG::View& currentView, const ETG::RenderWindow& window);

        static float AdjustMoveFactor();

        static float AdjustZoomFactor();

        // In InputManager.h
        static float GetMouseAngleRelativeToHero();

        //I spent so much time to correctly get mouse position after zoom or move with View.  
        static ETG::Vector2f GetRelativeMousePos();

    private:
        //Private constructor to prevent instantiation
        InputManager() = delete;
    };
}
