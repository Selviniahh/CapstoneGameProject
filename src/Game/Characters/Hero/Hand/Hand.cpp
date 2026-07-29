//
// Created by selviniah on 10/03/25.
//

#include "Hand.h"

#include <iostream>
#include <filesystem>
#include "../../../../Engine/Managers/AssetManager.h"

ETG::Hand::Hand()
{
    //NOTE: no Depth here on purpose. Where a hand draws is the character's call, because it changes with facing -
    //a hand is in front of the body from the front and behind it from the back - so the owner writes it every
    //frame in Character::UpdateHands from its own HandDepthInFront / HandDepthBehindBody
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
