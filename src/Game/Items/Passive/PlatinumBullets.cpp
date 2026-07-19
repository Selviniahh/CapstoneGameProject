#include <iostream>
#include <filesystem>
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "PlatinumBullets.h"
#include "../../Characters/Hero.h"
#include "../../../Engine/Core/Factory.h"
#include "../../Guns/Base/GunBase.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::PlatinumBullets::PlatinumBullets(): PassiveItemBase(AssetManager::Resolve("Items/Passive/platinum_bullets_001.png"))
{
    ItemDescription = "Increase the fire rate %20";
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 15.f;
    CollisionComp->SetCollisionEnabled(true);
    Position = {100, +30};

    PlatinumBullets::Initialize();
}

void ETG::PlatinumBullets::Initialize()
{
    PassiveItemBase::Initialize();
    Hero = Hero::Get();

    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        if (auto* heroObj = dynamic_cast<class Hero*>(eventData.Other))
        {
            Owner = dynamic_cast<GameObjectBase*>(heroObj); //In UI move from scene to Hero
            if (!IsVisible) return;
            Perk(heroObj);

            //Play a random pickup sound when collision occurs
            std::uniform_int_distribution<int> dist(0, Sounds.size() - 1);
            const int soundIndex = dist(rng);
            Sounds[soundIndex].play();
            IsVisible = false;

            //Add self to the hero's equipped passive items
            heroObj->EquippedPassiveItems.push_back(this);
        }
    });
}

void ETG::PlatinumBullets::Update()
{
    PassiveItemBase::Update();
    CollisionComp->Update();

    //Check if the FireRateIncreasePerc has changed (with only through UI)
    if (FireRateIncreasePerc != PreviousFireRatePerc)
        if (Hero) Perk(Hero);
    PreviousFireRatePerc = FireRateIncreasePerc;
}

void ETG::PlatinumBullets::Draw()
{
    PassiveItemBase::Draw();
    CollisionComp->Visualize(*ETG::RenderContext::Window);
}

void ETG::PlatinumBullets::Perk(const class Hero* hero) const
{
    //Modify the FireRate based on the FireRateIncreasePerc
    auto* currGun = hero->GetCurrentHoldingGun(); 
    currGun->FireRate = currGun->BaseFireRate - Math::CalculatePercentageOfValue(currGun->BaseFireRate,FireRateIncreasePerc);
}
