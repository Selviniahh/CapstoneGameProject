//
// Created by selviniah on 10/03/25.
//

#include "Hand.h"

#include <iostream>
#include <filesystem>

ETG::Hand::Hand()
{
    Hand::Initialize();
}

void ETG::Hand::Initialize()
{
    Texture = std::make_shared<sf::Texture>();

    if (!Texture->loadFromFile((std::filesystem::path(RESOURCE_PATH) / "Player" / "rogue_hand_001.png").generic_string()))
        std::cerr << "Failed to load hand texture" << std::endl;
    
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
