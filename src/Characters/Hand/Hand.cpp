//
// Created by selviniah on 10/03/25.
//

#include "Hand.h"

#include <iostream>
#include <filesystem>
#include "Managers/AssetManager.h"

ETG::Hand::Hand()
{
    Hand::Initialize();
}

void ETG::Hand::Initialize()
{
    Texture = std::make_shared<ETG::Texture>();

    if (!Texture->loadFromFile(AssetManager::Resolve("Player/rogue_hand_001.png")))
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
