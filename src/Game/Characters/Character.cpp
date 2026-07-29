#include "Character.h"
#include <cmath>
#include "Hero/Hand/Hand.h"
#include "../Guns/Base/GunBase.h"
#include "../Items/Active/ActiveItemBase.h"
#include "../Items/Passive/PassiveItemBase.h"
#include "../../Engine/Core/Components/BaseHealthComp.h"
#include "../../Engine/Core/Components/BaseMoveComp.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Utils/Math.h"

namespace ETG
{
    Character::Character() = default;

    Character::~Character() = default;

    //<---------- Per-frame work ---------->
    void Character::UpdateHand() const
    {
        if (!Hand) return;

        const ETG::Vector2f facingOffset = ETG::IsFacingRight(CurrentDir) ? HandOffsetRight : HandOffsetLeft;

        //Facing is already baked into the ternary above, so feed only the scale magnitude into the rotation:
        //FlipSpritesX flips the body by setting Scale.x = -1, and passing that in would mirror the offset a second time
        const ETG::Vector2f scaleMagnitude{std::abs(Scale.x), std::abs(Scale.y)};
        Hand->SetPosition(Position + Math::RotateVector(Rotation, scaleMagnitude, Hand->HandOffset + facingOffset));
        Hand->SetRotation(Rotation); //hand sprite turns with the body
        Hand->Update();
    }

    void Character::UpdateGuns() const
    {
        if (CurrentGun && Hand)
        {                                                                   //just in case if gun needs a bit tweak
            CurrentGun->SetPosition(Hand->GetPosition() + Hand->GunOffset + CurrentGun->HeldOffset);
            CurrentGun->Rotation = AimAngle;
        }

        //Every equipped gun ticks, not only the one in hand: the holstered ones still have projectiles in flight
        for (GunBase* gun : EquippedGuns)
            if (gun)
                gun->Update();
    }

    void Character::UpdateHandAndGunVisibility() const
    {
        const bool visible = ShouldShowHeldGun();
        if (Hand) Hand->IsVisible = visible;
        if (CurrentGun) CurrentGun->IsVisible = visible;
    }

    void Character::UseActiveItem() const
    {
        if (CurrActiveItem && CanUseActiveItems())
            CurrActiveItem->RequestUsage();
    }

    //<---------- Gun inventory ---------->
    void Character::EquipGun(GunBase* newGun)
    {
        if (!newGun) return;

        EquippedGuns.push_back(newGun);
        CurrentGun = newGun; //a freshly picked up gun goes straight into the hand
        CurrentGunIndex = static_cast<int>(EquippedGuns.size()) - 1;
        ShowOnlyCurrentGun();

        //NOTE: A passive item can only reach the guns that exist when it is picked up, so the guns picked up
        //afterwards have to come and collect their perks. Without this, the order the player finds things in decides
        //whether their items work - which is exactly the bug the old code had, except it lost the perk on every
        //weapon switch too
        for (PassiveItemBase* item : EquippedPassiveItems)
            if (item)
                item->ApplyGunPerk(*newGun);

        OnGunChanged(CurrentGun);
    }

    void Character::SwitchGun(const int offset)
    {
        if (EquippedGuns.empty()) return;

        //Wraps in both directions. The modulo guarantees a valid index for any offset as long as the list is not
        //empty, so no separate bounds check is needed
        const int count = static_cast<int>(EquippedGuns.size());
        CurrentGunIndex = ((CurrentGunIndex + offset) % count + count) % count;
        CurrentGun = EquippedGuns[CurrentGunIndex];
        ShowOnlyCurrentGun();

        OnGunChanged(CurrentGun);
    }

    void Character::SwitchToPreviousGun()
    {
        SwitchGun(-1);
    }

    void Character::SwitchToNextGun()
    {
        SwitchGun(1);
    }

    void Character::ShowOnlyCurrentGun() const
    {
        for (GunBase* gun : EquippedGuns)
            if (gun) gun->IsVisible = (gun == CurrentGun);
    }

    //<---------- Items ---------->
    void Character::PickUpActiveItem(ActiveItemBase* item)
    {
        if (!item) return;

        EquippedActiveItems.push_back(item);
        CurrActiveItem = item; //the item just picked up is the one the trigger fires
    }

    void Character::PickUpPassiveItem(PassiveItemBase* item)
    {
        if (!item) return;

        EquippedPassiveItems.push_back(item);

        //The perks reach the guns already owned here; EquipGun catches the ones picked up later
        for (GunBase* gun : EquippedGuns)
            if (gun) item->ApplyGunPerk(*gun);
    }
}
