#include "InputManager.h"
#include <complex>
#include "RenderContext.h"
#include "DebugTexts.h"
#include "../Editor/Engine.h"

void ETG::InputManager::Update()
{
    if (!Engine::IsGameWindowFocused()) return;


    // Update PreviousGameFocus at the end of the frame to track state correctly
    Engine::PreviousGameFocus = Engine::CurrentGameFocus;

    ZoomScale = GetZoomScale(RenderContext::MainView, *RenderContext::Window);

    const float adjustedZoomFactor = AdjustZoomFactor();
    const float adjustedMoveFactor = AdjustMoveFactor();

    //Calculate directions. It can only be -1 or 1 
    direction = ETG::Vector2f(0.f, 0.f);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A)) direction.x--;
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D)) direction.x++;
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::W)) direction.y--;
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::S)) direction.y++;

    //Camera Effects:
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::E)) RenderContext::MainView.zoom(1.0f - adjustedZoomFactor);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Q)) RenderContext::MainView.zoom(1.0f + adjustedZoomFactor);

    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Up)) RenderContext::MainView.move(0, -adjustedMoveFactor);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Down)) RenderContext::MainView.move(0, +adjustedMoveFactor);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Right)) RenderContext::MainView.move(+adjustedMoveFactor, 0);
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Left)) RenderContext::MainView.move(-adjustedMoveFactor, 0);

    ViewLocalMousePos = GetRelativeMousePos();
    WorldMousePos = RenderContext::Window->mapPixelToCoords(ETG::Mouse::getPosition(*RenderContext::Window), RenderContext::MainView);
}


void ETG::InputManager::InitializeDebugText()
{
    DebugText::LoadFont();
    debugText.setFont(*DebugText::Font);
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

ETG::Vector2f ETG::InputManager::GetRelativeMousePos()
{
    //Mouse world position
    const ETG::Vector2f MousePos = RenderContext::Window->mapPixelToCoords(ETG::Mouse::getPosition(*RenderContext::Window), RenderContext::MainView);

    // Calculate the top-left corner of the view in world coordinates
    const ETG::Vector2f viewTopLeft = RenderContext::MainView.getCenter() - (RenderContext::MainView.getSize() / 2.0f);

    // Subtract the view's top-left to get relative mouse position
    return MousePos - viewTopLeft;
}
