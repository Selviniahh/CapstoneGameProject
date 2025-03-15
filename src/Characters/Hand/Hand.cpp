#include "Hand.h"
#include <iostream>
#include <filesystem>

//(std::filesystem::path(RESOURCE_PATH) / "Player" / "rogue_hand_001.png").generic_string())
ETG::Hand::Hand(const std::string& handTexture)
{
    Texture = std::make_shared<sf::Texture>();

    if (!Texture->loadFromFile(handTexture))
        std::cerr << "Failed to load hand texture" << std::endl;

    Hand::Initialize();
}

void ETG::Hand::Initialize()
{
    
    
    GameObjectBase::Initialize();
}

void ETG::Hand::Draw()
{
    GameObjectBase::Draw();
}

void ETG::Hand::Update()
{
    GameObjectBase::Update();
}
