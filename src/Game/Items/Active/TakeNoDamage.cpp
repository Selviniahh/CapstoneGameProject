//
// Created by selviniah on 7/27/26.
//

#include "TakeNoDamage.h"

#include "../../../Engine/Managers/AssetManager.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../Characters/Character.h"
#include "../../Characters/Hero/Hero.h"
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
        //Picked up by any character, same as every other active item
        if (auto* character = eventData.Other->As<Character>())
        {
            Owner = character; //In UI move from scene to whoever picked it up
            if (!IsVisible) return;

            PlayRandomPickupSound();

            character->PickUpActiveItem(this);
        }
    });
}

//Whoever is carrying this. Null when an enemy carries it: the damage-modifier machinery
//(ModifierManager<IHeroModifier>) still lives on Hero, so this is the one effect an enemy cannot run yet
ETG::Hero* ETG::TakeNoDamage::GetHolder() const
{
    return Owner ? Owner->As<Hero>() : nullptr;
}

void ETG::TakeNoDamage::RequestUsage()
{
    Hero* const holder = GetHolder();
    if (!holder) return;

    ActiveItemBase::RequestUsage();

    //Hands itself to the holder. From here until the effect runs out, every hit they take comes back to
    //ReflectProjectile below
    holder->HeroModifierManager.AddModifier(this);
}

void ETG::TakeNoDamage::Update()
{
    ActiveItemBase::Update();

    //Watching the state LEAVE Consuming rather than testing for Cooldown: the removal has to happen exactly once,
    //and the item stays in Cooldown for the whole 30 seconds afterwards
    if (IsEffectActive && ActiveItemState != ActiveItemState::Consuming)
    {
        if (Hero* const holder = GetHolder())
            holder->HeroModifierManager.RemoveModifier<TakeNoDamage>();

        IsEffectActive = false;
    }
}

void ETG::TakeNoDamage::Draw()
{
    ActiveItemBase::Draw();
    CollisionComp->Visualize(*ETG::RenderContext::Window);
}

//Every hit that reaches the hero while this item is active ends up here, and none of them get through
//TODO: Reflect random 120-240 derece arasi reflect etsin
//TODO: Velocity hizini %10-30 arasi da rastgele hizlandir
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
