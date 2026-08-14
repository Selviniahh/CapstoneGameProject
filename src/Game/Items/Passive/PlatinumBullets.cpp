#include <iostream>
#include <filesystem>
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "PlatinumBullets.h"
#include "../../Characters/Character.h"
#include "../../../Engine/Core/Factory.h"
#include "../../Guns/Base/GunBase.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::PlatinumBullets::PlatinumBullets(): PassiveItemBase(AssetManager::Resolve("Items/Passive/platinum_bullets_001.png"))
{
    ItemDescription = "Increase the fire rate %20";
    ModifierSource = "PlatinumBullets"; //Every modifier this item attaches is filed - and removed - under this name
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 15.f;

    //Enemy as well as Hero: the pickup listener casts to Character, so any character can carry this
    CollisionComp->Layer = CollisionLayer::Pickup;
    CollisionComp->Mask = CollisionLayer::Hero | CollisionLayer::Enemy;

    CollisionComp->SetCollisionEnabled(true);
    Position = {100, +30};

    PlatinumBullets::Initialize();

    //Last statement, and the constructor's alone: see GameObjectBase::BindEvents
    PlatinumBullets::BindEvents();
}

void ETG::PlatinumBullets::Initialize()
{
    PassiveItemBase::Initialize();
}

void ETG::PlatinumBullets::BindEvents()
{
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        //Any character can carry this, not only the hero
        if (auto* character = eventData.Other->As<Character>())
        {
            Owner = character; //In UI move from scene to whoever picked it up
            if (!IsVisible) return;

            Holder = character;
            IsPickedUp = true;

            //Files the item and hands the perk to every gun the holder already owns; EquipGun catches later ones
            character->PickUpPassiveItem(this);

            //Play a random pickup sound when collision occurs
            std::uniform_int_distribution<int> dist(0, Sounds.size() - 1);
            const int soundIndex = dist(rng);
            Sounds[soundIndex].play();
            IsVisible = false;
        }
    });
}

void ETG::PlatinumBullets::Update()
{
    PassiveItemBase::Update();

    //Check if the FireRateIncreasePerc has changed (with only through UI).
    //NOTE: gated on IsPickedUp. Without it, dragging the percentage in the editor buffs a gun the player has not
    //earned the item for yet - which the old code also did, except it only reached the one gun in hand
    if (IsPickedUp && FireRateIncreasePerc != PreviousFireRatePerc)
        ApplyToAllGuns();

    PreviousFireRatePerc = FireRateIncreasePerc;
}

void ETG::PlatinumBullets::Draw()
{
    PassiveItemBase::Draw();
    CollisionComp->Visualize(*ETG::RenderContext::Window);
}

void ETG::PlatinumBullets::ApplyGunPerk(GunBase& gun)
{
    //FireRate is the delay between shots, so raising the rate by 20% means cutting the delay by 20%.
    //Re-attaching is safe: the modifier is keyed on ModifierSource, so this overwrites rather than compounds
    gun.FireRate.AddModifier(ModifierSource, StatOp::Percent, -FireRateIncreasePerc / 100.f);
}

void ETG::PlatinumBullets::ApplyToAllGuns()
{
    if (!Holder) return;

    for (GunBase* gun : Holder->EquippedGuns)
        if (gun) ApplyGunPerk(*gun);
}
