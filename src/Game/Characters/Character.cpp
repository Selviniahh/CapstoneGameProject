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

    namespace
    {
        //Where an anchor pixel of the gun ends up in the world. The anchor is measured from the sheet's top-left
        //while the gun draws around its Origin, so their difference is the offset in gun space; feeding the gun's
        //own scale into the rotation is what keeps it on the right pixel when the gun is mirrored to aim left.
        ETG::Vector2f AnchorPositionOnGun(const GunBase& gun, const ETG::Vector2f& anchor,
                                          const ETG::Vector2f& heldOffset)
        {
            const ETG::Vector2f gunLocal = anchor - gun.GetOrigin();
            //HeldOffset slides the gun artwork under stationary hands. Remove that world-space displacement here,
            //otherwise the grip anchors would drag both hands along with the gun.
            return gun.GetPosition() - heldOffset +
                   Math::RotateVector(gun.GetRotation(), gun.GetScale(), gunLocal);
        }

        //Puts one hand on a pixel of the gun. Nothing here knows about pinning: a pinned gun has already been
        //moved so that its grip sits on the pinned point, so a hand placed on that grip lands there for free.
        void PlaceHandOnGun(Hand& hand, const GunBase& gun, const ETG::Vector2f& anchor,
                            const ETG::Vector2f& heldOffset)
        {
            hand.SetPosition(AnchorPositionOnGun(gun, anchor, heldOffset));
            hand.SetRotation(gun.GetRotation());
        }
    }

    //<---------- Per-frame work ---------->
    bool Character::IsGunOnRightSide() const
    {
        //A gun that names its own swap angle is asked directly, so the side it is held on can turn over
        //somewhere other than where the body's 8-way facing does. Everything else falls back to the facing,
        //which is what every gun did before there was a swap angle at all
        if (CurrentGun && CurrentGun->DecidesOwnHandSide())
            return CurrentGun->IsHeldOnRightSide(AimAngle);

        return ETG::IsFacingRight(CurrentDir);
    }

    void Character::ApplyGripPin() const
    {
        //A gun with no measured second grip has no pixel to pin, so there is nothing to ask it
        if (!CurrentGun || !CurrentGun->HasLeftHandAnchor || !CurrentGun->WantsGripPinned()) return;

        const ETG::Vector2f gripLocal = CurrentGun->LeftHandAnchor - CurrentGun->GetOrigin();
        const ETG::Vector2f scale = CurrentGun->GetScale();

        //Sliding the gun by the difference between the two puts the grip exactly where the pinned rotation would
        //have left it, whatever the barrel is doing. The gun therefore turns about its grip rather than about its
        //own origin - which is the whole trick: one pixel stands still, everything else swings around it
        const ETG::Vector2f whereTheGripIs = Math::RotateVector(CurrentGun->GetRotation(), scale, gripLocal);
        const ETG::Vector2f whereItStays = Math::RotateVector(CurrentGun->PinnedGripRotation(), scale, gripLocal);

        CurrentGun->SetPosition(CurrentGun->GetPosition() + whereItStays - whereTheGripIs);
    }

    void Character::UpdateHoldPoint()
    {
        if (!Hand) return;

        const ETG::Vector2f facingOffset = IsGunOnRightSide() ? HandOffsetRight : HandOffsetLeft;

        //Facing is already baked into the ternary above, so feed only the scale magnitude into the rotation:
        //FlipSpritesX flips the body by setting Scale.x = -1, and passing that in would mirror the offset a second time
        const ETG::Vector2f scaleMagnitude{std::abs(Scale.x), std::abs(Scale.y)};
        HoldPoint = Position + Math::RotateVector(Rotation, scaleMagnitude, Hand->HandOffset + facingOffset);
    }

    void Character::UpdateGuns()
    {
        if (CurrentGun && Hand)
        {
            //HeldOffset is authored against the right-held artwork. Mirror only its horizontal component when the
            //gun changes sides, so a negative X offset continues to mean "back" while held on the left.
            ETG::Vector2f heldOffset = CurrentGun->HeldOffset;
            if (!IsGunOnRightSide()) heldOffset.x = -heldOffset.x;

            CurrentGun->SetPosition(HoldPoint + Hand->GunOffset + heldOffset);
            CurrentGun->Rotation = AimAngle;

            //Same rule the hands follow, and for the same reason: with the body drawn from behind, what it is
            //holding is on the far side of it. Written before the gun ticks, because the gun bakes its depth into
            //its draw properties in there
            CurrentGun->Depth = ETG::IsFacingBack(CurrentDir)
                                    ? CurrentGun->HeldDepthBehindBody
                                    : CurrentGun->HeldDepthInFront;

            //The gun is mirrored vertically while it is held on the left, so its sprite is never upside down.
            //
            //NOTE: this used to be an AnimationComp->FlipSpritesY(CurrentDir, gun) call in Hero and in BulletMan,
            //i.e. two copies of the decision, both reading the body's facing while the hold point read this one.
            //A gun with its own swap angle would have had its sprite turn over 22.5 degrees away from its hand
            if (CanFlipAnims())
            {
                ETG::Vector2f gunScale = CurrentGun->GetScale();
                gunScale.y = IsGunOnRightSide() ? std::abs(gunScale.y) : -std::abs(gunScale.y);
                CurrentGun->SetScale(gunScale);
            }

            //Last, because it reads the rotation and the mirror decided just above
            ApplyGripPin();
        }

        //Every equipped gun ticks, not only the one in hand: the holstered ones still have projectiles in flight
        for (GunBase* gun : EquippedGuns)
            if (gun)
                gun->Update();

        //After the guns, never before: a gun's Origin is rewritten from its current animation frame inside its own
        //Update, and that origin is what the grips are measured against
        UpdateHands();
    }

    void Character::UpdateHands() const
    {
        //Written before the hands tick, not after: a hand bakes its depth into its draw properties inside its own
        //Update, so a value set later would be a frame late - and one frame late on a facing change is the frame
        //where the hand is visibly on the wrong side of the body
        const float handDepth = ETG::IsFacingBack(CurrentDir) ? HandDepthBehindBody : HandDepthInFront;
        if (Hand) Hand->Depth = handDepth;
        if (OffHand) OffHand->Depth = handDepth;

        ETG::Vector2f heldOffset{};
        if (CurrentGun)
        {
            heldOffset = CurrentGun->HeldOffset;
            if (!IsGunOnRightSide()) heldOffset.x = -heldOffset.x;
        }

        if (Hand)
        {
            if (CurrentGun && CurrentGun->HasRightHandAnchor)
                PlaceHandOnGun(*Hand, *CurrentGun, CurrentGun->RightHandAnchor, heldOffset);
            else
            {
                //No measured grip, so the hand stays where it always was and the gun sits in it
                Hand->SetPosition(HoldPoint);
                Hand->SetRotation(Rotation);
            }
            Hand->Update();
        }

        if (OffHand)
        {
            if (CurrentGun && CurrentGun->HasLeftHandAnchor)
                PlaceHandOnGun(*OffHand, *CurrentGun, CurrentGun->LeftHandAnchor, heldOffset);
            else
            {
                //A gun without a measured second grip leaves the off hand on the opposite side of the body.
                //The hand therefore remains visible instead of keeping its construction position at world (0,0).
                const ETG::Vector2f offHandOffset = IsGunOnRightSide()
                                                        ? HandOffsetLeft
                                                        : HandOffsetRight;
                const ETG::Vector2f scaleMagnitude{std::abs(Scale.x), std::abs(Scale.y)};
                OffHand->SetPosition(Position + Math::RotateVector(Rotation, scaleMagnitude, offHandOffset));
                OffHand->SetRotation(Rotation);
            }
            OffHand->Update();
        }
    }

    void Character::UpdateHandAndGunVisibility() const
    {
        const bool visible = ShouldShowHeldGun();
        if (Hand) Hand->IsVisible = visible;
        //Both hands belong to the character and remain visible while it can hold a gun. The concrete gun only
        //decides whether the off hand attaches to its LeftHandAnchor or rests against the body.
        if (OffHand) OffHand->IsVisible = visible;
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
