#include "DoubleShoot.h"
#include <filesystem>
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero.h"
#include "../../../Engine/Core/Factory.h"
#include "../../Guns/Base/GunBase.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::DoubleShoot::DoubleShoot() : ActiveItemBase(AssetManager::Resolve("Items/Active/Potion_of_Gun_Friendship.png"))
{
    ItemDescription = "Double shoot the item and set Spread 0";
    Position = {100, -70};
    
    TotalCooldownTime = 15.0f;
    TotalConsumeTime = 10.0f;

    DoubleShoot::Initialize();
}

void ETG::DoubleShoot::Initialize()
{
    ActiveItemBase::Initialize();
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        //If collided object is hero:
        //TODO: I want to create a character base class. Hero and EnemyBase will inherit from this base class 
        //TODO: And then ActiveItemBase will add all this Collision Add Listener into it's Base initializer
        //TODO: In base Character class, it'll have EquippedActiveItems both Hero player character and any enemy character
        //TODO: like boss or random Bulletman can come up and equip the item and use it right away 
        //TODO: so in ActiveItemBase, I'll base this listener and this if block will check if it's Character only not hero
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

//TODO: This also must be used from enemies as well so it can also go right into the base ActiveItemBase
void ETG::DoubleShoot::Update()
{
    ActiveItemBase::Update();
    CollisionComp->Update();

    //Watching the state LEAVE Consuming rather than testing for Cooldown: the removal has to happen exactly once,
    //and the item stays in Cooldown long after the effect ended
    if (AffectedGun && ActiveItemState != ActiveItemState::Consuming)
    {
        AffectedGun->modifierManager.RemoveModifier<DoubleShoot>();
        AffectedGun = nullptr;
    }
}

void ETG::DoubleShoot::Draw()
{
    ActiveItemBase::Draw();
    CollisionComp->Visualize(*ETG::RenderContext::Window);
}

void ETG::DoubleShoot::RequestUsage()
{
    // Only allow usage if the cooldown is complete
    if (ActiveItemState != ActiveItemState::Ready) return;

    ActivateSound.play();

    // Reset state
    ActiveItemState = ActiveItemState::Consuming;
    ConsumeTimer = 0;

    //Hands itself to the gun. From here until the effect runs out, every shot that gun fires comes back to
    //ModifyShot below. Remembering the gun matters: the hero may switch weapons mid-effect, and the modifier has
    //to come off the gun it was put on rather than whatever is in hand when the timer runs out
    AffectedGun = Hero::Get()->GetCurrentHoldingGun();
    AffectedGun->modifierManager.AddModifier(this);
}

//Every shot the affected gun fires while this item is active passes through here
void ETG::DoubleShoot::ModifyShot(ShotParams& shot)
{
    shot.ShotCount = ShootCount;
    shot.Spread = SpreadAmount;
}
