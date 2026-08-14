#include "DoubleShoot.h"
#include <filesystem>
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Character.h"
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
        //Anyone who can carry a gun can carry this: the hero, or a BulletMan that happened to walk over it
        //TODO: this listener is identical in every active item. It belongs in ActiveItemBase::Initialize
        if (auto* character = eventData.Other->As<Character>())
        {
            Owner = character; //In UI move from scene to whoever picked it up
            if (!IsVisible) return;

            PlayRandomPickupSound();

            character->PickUpActiveItem(this);
        }
    });
}

//TODO: This also must be used from enemies as well so it can also go right into the base ActiveItemBase
void ETG::DoubleShoot::Update()
{
    ActiveItemBase::Update();

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

    //Whoever picked the item up is who it buffs. Asking Hero::Get() instead would have buffed the player's gun no
    //matter which character actually triggered the item
    Character* const holder = Owner ? Owner->As<Character>() : nullptr;
    if (!holder || !holder->GetCurrentHoldingGun()) return;

    ActivateSound.play();

    // Reset state
    ActiveItemState = ActiveItemState::Consuming;
    ConsumeTimer = 0;

    //Hands itself to the gun. From here until the effect runs out, every shot that gun fires comes back to
    //ModifyShot below. Remembering the gun matters: the holder may switch weapons mid-effect, and the modifier has
    //to come off the gun it was put on rather than whatever is in hand when the timer runs out
    AffectedGun = holder->GetCurrentHoldingGun();
    AffectedGun->modifierManager.AddModifier(this);
}

//Every shot the affected gun fires while this item is active passes through here
void ETG::DoubleShoot::ModifyShot(ShotParams& shot)
{
    shot.ShotCount = ShootCount;
    shot.Spread = SpreadAmount;
}
