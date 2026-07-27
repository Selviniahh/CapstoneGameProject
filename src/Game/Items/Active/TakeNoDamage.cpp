//
// Created by selviniah on 7/27/26.
//

#include "TakeNoDamage.h"

#include "../../../Engine/Managers/AssetManager.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../Characters/Hero.h"
#include "../../Guns/Base/GunBase.h"
#include "../../Projectile/ProjectileBase.h"


ETG::TakeNoDamage::TakeNoDamage() : ActiveItemBase(AssetManager::Resolve("Items/Active/stuffed_star_001.png"))
{
    ItemDescription = "Take no damage for 8 seconds and send enemy bullets back";
    Position = {150, -70};
    
    TotalCooldownTime = 15.0f;
    TotalConsumeTime = 10.0f;

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
    ActiveItemBase::RequestUsage();
    //Hands itself to the hero. From here until the effect runs out, every hit the hero takes comes back to
    //OnIncomingDamage below
    Hero::Get()->HeroModifierManager.AddModifier(this);
}

void ETG::TakeNoDamage::Update()
{
    ActiveItemBase::Update();

    //Watching the state LEAVE Consuming rather than testing for Cooldown: the removal has to happen exactly once,
    //and the item stays in Cooldown for the whole 30 seconds afterwards
    if (IsEffectActive && ActiveItemState != ActiveItemState::Consuming)
    {
        Hero::Get()->HeroModifierManager.RemoveModifier<TakeNoDamage>();
        IsEffectActive = false;
    }
}

void ETG::TakeNoDamage::Draw()
{
    ActiveItemBase::Draw();
    CollisionComp->Visualize(*ETG::RenderContext::Window);
}

//Every hit that reaches the hero while this item is active ends up here, and none of them get through
bool ETG::TakeNoDamage::ReflectProjectile(Hero& hero, ProjectileBase* const projectile)
{
    //Contact damage from walking into an enemy: there is nothing to send back, just eat the hit
    if (!projectile) return true;

    if (DeflectProjectiles)
    {
        projectile->ProjVelocity = -projectile->ProjVelocity;
        projectile->SetRotation(projectile->GetRotation() + 180.f); //sprite has to turn with it

        //NOTE: EnemyBase decides friend-or-foe from projectile->Owner->Owner (EnemyBase.cpp). Reversing only the
        //velocity would send back a bullet that every enemy still recognises as its own and ignores
        projectile->Owner = hero.GetCurrentHoldingGun();
    }
    else
    {
        projectile->MarkForDestroy();
    }

    return true;
}
