#include "InputManager.h"
#include <complex>
#include "../../Game/Managers/GameManager.h"
#include "../../Game/Managers/GameState.h"
#include "../../Game/Characters/Hero.h"
#include "../Editor/Engine.h"

void ETG::InputManager::Update()
{
    if (!HeroPtr) HeroPtr = GameState::GetInstance().GetHero();

    if (!Engine::IsGameWindowFocused()) return;


    // Update PreviousGameFocus at the end of the frame to track state correctly
    Engine::PreviousGameFocus = Engine::CurrentGameFocus;

    ZoomScale = GetZoomScale(Globals::MainView, *Globals::Window);

    const float adjustedZoomFactor = AdjustZoomFactor();
    const float adjustedMoveFactor = AdjustMoveFactor();

    //Calculate directions. It can only be -1 or 1 
    direction = ETG::Vector2f(0.f, 0.f);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A)) direction.x--;
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D)) direction.x++;
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::W)) direction.y--;
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::S)) direction.y++;

    //shooting
    Hero::IsShooting = ETG::Mouse::isButtonPressed(ETG::Mouse::Left);

    //Camera Effects:
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::E)) Globals::MainView.zoom(1.0f - adjustedZoomFactor);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Q)) Globals::MainView.zoom(1.0f + adjustedZoomFactor);

    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Up)) Globals::MainView.move(0, -adjustedMoveFactor);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Down)) Globals::MainView.move(0, +adjustedMoveFactor);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Right)) Globals::MainView.move(+adjustedMoveFactor, 0);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Left)) Globals::MainView.move(-adjustedMoveFactor, 0);

    ViewLocalMousePos = GetRelativeMousePos();
    WorldMousePos = Globals::Window->mapPixelToCoords(ETG::Mouse::getPosition(*Globals::Window), Globals::MainView);
}


void ETG::InputManager::InitializeDebugText()
{
    debugText.setFont(*Globals::Font);
    debugText.setCharacterSize(16);
    debugText.setFillColor(ETG::Color::Yellow);
}

float ETG::InputManager::GetZoomScale(const ETG::View& currentView, const ETG::RenderWindow& window)
{
    //Default view size
    ETG::Vector2f defSize = window.getDefaultView().getSize();

    //Get current view size
    ETG::Vector2f currSize = currentView.getSize();

    return defSize.x / currSize.x;
}

float ETG::InputManager::AdjustMoveFactor()
{
    const float scaleRatio = 10000.f / ZoomScale;
    float adjustedMoveFactor = ZoomFactor * std::sqrt(scaleRatio);

    adjustedMoveFactor = std::clamp(adjustedMoveFactor, MinMoveSpeed, MaxMoveSpeed);
    return adjustedMoveFactor;
}

float ETG::InputManager::AdjustZoomFactor()
{
    const float scaleRatio = 0.1f / ZoomScale;
    float adjustedZoomFactor = ZoomFactor * std::sqrt(scaleRatio);
    adjustedZoomFactor = std::clamp(adjustedZoomFactor, MinScaleSpeed, MaxScaleSpeed);
    return adjustedZoomFactor;
}

float ETG::InputManager::GetMouseAngleRelativeToHero()
{
    const ETG::Vector2f diff = WorldMousePos - HeroPtr->GetPosition();
    return std::atan2(diff.y, diff.x);
}

ETG::Vector2f ETG::InputManager::GetRelativeMousePos()
{
    //Mouse world position
    const ETG::Vector2f MousePos = Globals::Window->mapPixelToCoords(ETG::Mouse::getPosition(*Globals::Window), Globals::MainView);

    // Calculate the top-left corner of the view in world coordinates
    const ETG::Vector2f viewTopLeft = Globals::MainView.getCenter() - (Globals::MainView.getSize() / 2.0f);

    // Subtract the view's top-left to get relative mouse position
    return MousePos - viewTopLeft;
}
