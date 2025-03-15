#include "PassiveItem.h"
#include "../Managers/Globals.h"
#include "../Managers/GameState.h"
#include "../Characters/Hero.h" // Include Hero header to get the complete class definition
#include <filesystem>
#include <iostream>

namespace ETG {
    class GameState; // Forward declaration of GameState if needed
}

PassiveItem::PassiveItem(const std::string& texturePath)
{
    Texture = std::make_shared<sf::Texture>();
    if (!Texture->loadFromFile(texturePath))
    {
        std::cerr << "Failed to load passive item texture: " << texturePath << std::endl;
    }
    equipSoundBuffer.loadFromFile((std::filesystem::path(RESOURCE_PATH) / "Sounds" / "equip_sound.wav").generic_string());
    equipSound.setBuffer(equipSoundBuffer);
}

void PassiveItem::Initialize()
{
    Origin.x = Texture->getSize().x / 2;
    Origin.y = Texture->getSize().y / 2;
}

void PassiveItem::Update()
{
    // No update logic needed for passive items
}

void PassiveItem::Draw() {
    if (Texture) {
        // Access hero through GameState
        ETG::Hero* hero = ETG::GameState::GetInstance().GetHero();
        if (hero && hero->IsMouseNearHero(*ETG::Globals::Window)) {
            sf::Sprite sprite;
            sprite.setTexture(*Texture);
            sprite.setScale(0.5f, 0.5f);
            sprite.setPosition(GetPosition());
            ETG::Globals::Window->draw(sprite);
        }
    } else {
        std::cerr << "PassiveItem texture is null!" << std::endl;
    }
}

void PassiveItem::PlayEquipSound()
{
    equipSound.play();
}
void PassiveItem::Equip() {
    // Apply 20% faster fire rate
    ETG::Hero* hero = ETG::GameState::GetInstance().GetHero();
    if (hero) {
        hero->SetFireRateMultiplier(1.2f);
    }
}