//
// Created by selviniah on 10/03/25.
//

#include "Hand.h"

#include <iostream>
#include <filesystem>
#include "../../../Engine/Managers/AssetManager.h"

ETG::Hand::Hand()
{
    Hand::Initialize();
}

void ETG::Hand::Initialize()
{
    Texture = AssetManager::LoadTexture("Player/rogue_hand_001.png");

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
