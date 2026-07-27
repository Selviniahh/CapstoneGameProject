//
// Created by selviniah on 7/27/26.
//

#include "TakeNoDamage.h"

#include "../../../Engine/Managers/AssetManager.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../Characters/Hero.h"
#include "../../Modifiers/Hero/InvulnerabilityModifier.h"


ETG::TakeNoDamage::TakeNoDamage() : ActiveItemBase(AssetManager::Resolve("Items/Active/stuffed_star_001.png"),
                                                   AssetManager::Resolve("Sounds/Consume.ogg"),
                                                   DEFAULT_COOLDOWN, DEFAULT_ACTIVE_TIME)
{
    ItemDescription = "Take no damage for 8 seconds and send enemy bullets back";
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 15.f;
    CollisionComp->SetCollisionEnabled(true);
    Position = {150, -70};
    Origin = Vector2f{(float)Texture->getSize().x / 2, (float)Texture->getSize().y / 2};

    TakeNoDamage::Initialize();
}

void ETG::TakeNoDamage::Initialize()
{
    ActiveItemBase::Initialize();
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        if (auto* heroObj = eventData.Other->As<Hero>())
       {
             Owner = heroObj; //In UI move from scene to Hero
             if (!IsVisible) return;

             PlayRandomPickupSound();

             //Add self to the hero's equipped active items
             heroObj->EquippedActiveItems.push_back(this);
       }
    });
}

void ETG::TakeNoDamage::RequestUsage()
{
    if (ActiveItemState != ActiveItemState::Ready) return;

    ActivateSound.play();
    ActiveItemState = ActiveItemState::Consuming;
    ConsumeTimer = 0;

    //The modifier is pure data: it says "no damage, and bounce the shots". Hero reads it back when a projectile
    //actually reaches it, which is the only moment a projectile exists to be turned around
    Hero::Get()->HeroModifierManager.AddModifier(std::make_shared<InvulnerabilityModifier>(DeflectProjectiles));
    IsEffectActive = true;
}

void ETG::TakeNoDamage::Update()
{
    ActiveItemBase::Update();
    CollisionComp->Update();

    //Watching the state LEAVE Consuming rather than testing for Cooldown: the removal has to happen exactly once,
    //and the item stays in Cooldown for the whole 30 seconds afterwards
    if (IsEffectActive && ActiveItemState != ActiveItemState::Consuming)
    {
        Hero::Get()->HeroModifierManager.RemoveModifier<InvulnerabilityModifier>();
        IsEffectActive = false;
    }
}

void ETG::TakeNoDamage::Draw()
{
    ActiveItemBase::Draw();
    CollisionComp->Visualize(*ETG::RenderContext::Window);
}
 