#include "GameManager.h"
#include "DebugTexts.h"
#include "InputManager.h"
#include "SpriteBatch.h"
#include "Globals.h"
#include "../Core/Scene/Scene.h"
#include "../Characters/Hero.h"
#include "../UI/UserInterface.h"
#include <filesystem>
#include <imgui-SFML.h>
#include <iostream>
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include "../Enemy/BulletMan/BulletMan.h"

sf::Event ETG::GameManager::GameEvent{};
using namespace ETG::Globals;

ETG::GameManager::~GameManager() = default;

ETG::GameManager::GameManager() {
    Initialize();
}

void ETG::GameManager::Initialize() {
    // Initialize window
    const auto VideoMode = sf::VideoMode::getDesktopMode();
    Window = std::make_shared<sf::RenderWindow>(sf::VideoMode(VideoMode.width / 1.5, VideoMode.height / 1.5), "Enter the Gungeon Clone");
    Window->setFramerateLimit(Globals::FPS);

    // Initialize game state and scene
    GameState::GetInstance();
    GameState::GetInstance().SetSceneObjs(SceneObjects);

    Scene = std::make_unique<class Scene>();
    Scene->SetObjectNameToSelfClassName();
    GameState::GetInstance().SetSceneObj(Scene.get());


    //NOTE: Secondly EngineUI needs to be initialized
    EngineUI.Initialize();
    Globals::Initialize(Window);
    InputManager::InitializeDebugText();

    // Spawn hero and UI
    Hero = ETG::CreateGameObjectDefault<class Hero>(sf::Vector2f{10, 10});
    UI = ETG::CreateGameObjectDefault<UserInterface>();

    BulletMan = ETG::CreateGameObjectDefault<class BulletMan>(sf::Vector2f{50,50});
    BulletMan->Initialize();
    
    //Always initialize debug text last 
    DebugText = std::make_unique<class DebugText>();

    // Spawn items
    SpawnItems();
}

void ETG::GameManager::SpawnItems() {
    // Spawn active and passive items near the hero's initial position
    ActiveItem = std::make_unique<class ActiveItem>((std::filesystem::path(RESOURCE_PATH) / "Items" / "Active" / "Potion_of_Gun_Friendship.png").generic_string());
    PassiveItem = std::make_unique<class PassiveItem>((std::filesystem::path(RESOURCE_PATH) / "Items" / "Passive" / "platinum_bullets_001.png").generic_string());

    // Set initial positions for items
    ActiveItem->SetPosition(Hero->GetPosition() + sf::Vector2f(4, -20));  // 30 pixels to the right
    PassiveItem->SetPosition(Hero->GetPosition() + sf::Vector2f(-4, -20)); // 30 pixels to the left

    // Add items to the scene
    SceneObjects["ActiveItem"] = ActiveItem.get();
    SceneObjects["PassiveItem"] = PassiveItem.get();

    // Debug log to check item positions
    std::cout << "Active Item Position: " << ActiveItem->GetPosition().x << ", " << ActiveItem->GetPosition().y << std::endl;
    std::cout << "Passive Item Position: " << PassiveItem->GetPosition().x << ", " << PassiveItem->GetPosition().y << std::endl;
}

void ETG::GameManager::HandleItemEquip() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
        std::cout << "F key pressed" << std::endl; // Debug log

        if (CurrentSelectionState == ItemSelectionState::NoneSelected) {
            // No item is selected, try to select the active item first
            if (Hero->IsNearItem(ActiveItem.get())) {
                EquipActiveItem();
            } else if (Hero->IsNearItem(PassiveItem.get())) {
                // If the hero is not near the active item but is near the passive item, select it
                EquipPassiveItem();
            }
        } else if (CurrentSelectionState == ItemSelectionState::ActiveItemSelected) {
            // Active item is already selected, unequip it and try to select the passive item
            UnequipActiveItem();
            if (Hero->IsNearItem(PassiveItem.get())) {
                EquipPassiveItem();
            } else {
                CurrentSelectionState = ItemSelectionState::NoneSelected; // No item is selected
            }
        } else if (CurrentSelectionState == ItemSelectionState::PassiveItemSelected) {
            // Passive item is already selected, unequip it and try to select the active item
            UnequipPassiveItem();
            if (Hero->IsNearItem(ActiveItem.get())) {
                EquipActiveItem();
            } else {
                CurrentSelectionState = ItemSelectionState::NoneSelected; // No item is selected
            }
        }
    }
}
void ETG::GameManager::EquipActiveItem() {
    if (ActiveItem && Hero->IsNearItem(ActiveItem.get())) {
        std::cout << "Equipping active item" << std::endl; // Debug log
        ActiveItem->PlayEquipSound();
        DebugText->DrawDebugText("Active Item Equipped: Potion of Gun Friendship", *Window);
        Hero->EquipActiveItem(ActiveItem.get());
        UI->SetActiveItem(ActiveItem.get());
        CurrentSelectionState = ItemSelectionState::ActiveItemSelected;
    }
}

void ETG::GameManager::EquipPassiveItem() {
    if (PassiveItem && Hero->IsNearItem(PassiveItem.get())) {
        std::cout << "Equipping passive item" << std::endl; // Debug log
        PassiveItem->PlayEquipSound();
        DebugText->DrawDebugText("Passive Item Equipped: Platinum Bullets", *Window);
        Hero->EquipPassiveItem(PassiveItem.get());
        UI->SetPassiveItem(PassiveItem.get());
        CurrentSelectionState = ItemSelectionState::PassiveItemSelected;
    }
}

void ETG::GameManager::UnequipActiveItem() {
    if (Hero->HasActiveItem()) {
        std::cout << "Unequipping active item" << std::endl; // Debug log
        Hero->UnequipActiveItem();

    }
}

void ETG::GameManager::UnequipPassiveItem() {
    if (Hero->HasPassiveItem()) {
        std::cout << "Unequipping passive item" << std::endl; // Debug log
        Hero->UnequipPassiveItem();

    }
}
void ETG::GameManager::UpdateItems() {
    if (ActiveItem) {
        ActiveItem->Update();
    }
    if (PassiveItem) {
        PassiveItem->Update();
    }
}

void ETG::GameManager::Update() {
    float deltaTime = Globals::ElapsedTimeSeconds;
    if (HasFocus) {
        EngineUI.Update();
        Globals::Update();
        InputManager::Update();
        Hero->Update();
        UI->Update();
        BulletMan->Update();
        HandleItemEquip();
        UpdateItems();
    }
}

void ETG::GameManager::Draw() {
    if (!HasFocus) return;
    Window->clear(sf::Color::Black);

    // Draw the main game scene
    Window->setView(Globals::MainView);
    GlobSpriteBatch.begin();
    Hero->Draw();
    if (ActiveItem) ActiveItem->Draw(); // Draw active item
    if (PassiveItem) PassiveItem->Draw();
    ETG::GlobSpriteBatch.end(*Window);
    BulletMan->Draw();
    GlobSpriteBatch.end(*Window);

    // Draw UI
    Window->setView(Window->getDefaultView());
    ETG::GlobSpriteBatch.begin();
    UI->Draw();
    ETG::GlobSpriteBatch.end(*Window);

    // Draw debug text
    DebugText->Draw(*Window);

    EngineUI.Draw();
    Window->display();
}

void ETG::GameManager::ProcessEvents() {
    while (Window->pollEvent(GameEvent)) {
        if (GameEvent.type == sf::Event::LostFocus) HasFocus = false;
        if (GameEvent.type == sf::Event::GainedFocus) HasFocus = true;
        if (GameEvent.type == sf::Event::Closed) {
            Font.reset();
            Window->close();
            return;
        }

        // Handle window resize
        if (GameEvent.type == sf::Event::Resized) {
            ScreenSize = {GameEvent.size.width, GameEvent.size.height};
            sf::View defaultView(sf::FloatRect(0.f, 0.f, GameEvent.size.width, GameEvent.size.height));
            Window->setView(defaultView);
            UI->Initialize();
        }

        // Poll and process events for ImGUI
        ImGui::SFML::ProcessEvent(*Window, GameEvent);
    }
}