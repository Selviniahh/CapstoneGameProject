#include "Character.h"
#include <cmath>
#include "Hero/Hand/Hand.h"
#include "../Guns/Base/GunBase.h"
#include "../Guns/Base/HandRig.h"
#include "../Items/Active/ActiveItemBase.h"
#include "../Items/Passive/PassiveItemBase.h"
#include "../../Engine/Core/Components/BaseHealthComp.h"
#include "../../Engine/Core/Components/BaseMoveComp.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Utils/Math.h"

namespace ETG
{
    //Characters are the one thing in the dungeon drawn through the grayscale sprite program
    //(fs_sprite_grayscale). Everything else - guns, projectiles, items, the HUD - keeps its colour,
    //which is what makes the effect visible at a glance. The strength is global and live-tweakable
    //through GraphicsDevice::SetGrayscaleAmount.
    Character::Character()
    {
        SetGrayscaleEnabled(true);
    }

    Character::~Character() = default;

    void Character::SetGrayscaleEnabled(const bool enabled)
    {
        Effect = enabled ? ShaderEffect::Grayscale : ShaderEffect::None;
    }

    //<---------- The held-gun rig: the one decision everything else reads ---------->
    bool Character::IsGunOnRightSide() const
    {
        //A gun that names its own swap angle is asked directly, so the side it is held on can turn over somewhere
        //other than where the body's 8-way facing does. Everything else falls back to the facing, which is what
        //every gun did before there was a swap angle at all
        if (CurrentGun && CurrentGun->DecidesOwnHandSide())
            return CurrentGun->IsHeldOnRightSide(AimAngle);

        return ETG::IsFacingRight(CurrentDir);
    }

    //<---------- The rig: shared geometry ---------->
    ETG::Vector2f Character::BodyRestPosition(const ETG::Vector2f& offset) const
    {
        //Only the magnitude of the scale goes in. FlipSpritesX mirrors the body by setting Scale.x = -1, and the
        //caller has already chosen a left or right offset, so passing the sign in would mirror it twice
        const ETG::Vector2f scaleMagnitude{std::abs(Scale.x), std::abs(Scale.y)};

        return Position + Math::RotateVector(Rotation, scaleMagnitude, offset);
    }

    ETG::Vector2f Character::MirroredHeldOffset() const
    {
        if (!CurrentGun) return {};

        //The mirroring rule itself lives on the gun, because the gun needs it too: WorldPointOnGun has to
        //undo the same slide to locate a pixel of its own artwork. PublishHeldSideToGun has already told the
        //gun which side it is on, so both answers come out of the one decision IsGunOnRightSide made
        return CurrentGun->MirroredHeldOffset();
    }

    //<---------- The rig: step 1, the joint ---------->
    void Character::UpdateHoldPoint()
    {
        if (!Hand) return;

        const ETG::Vector2f facingOffset = IsGunOnRightSide() ? HandOffsetRight : HandOffsetLeft;

        HoldPoint = BodyRestPosition(Hand->HandOffset + facingOffset);
    }

    //<---------- The rig: step 2, the gun and then the hands ---------->
    void Character::UpdateGuns()
    {
        //Hand is required as well as the gun: its GunOffset is part of where the gun hangs
        if (CurrentGun && Hand)
        {
            //Read the order downwards. Each line needs the ones above it and nothing below it, which is why they
            //are separate calls rather than one function: the sequence is the design, so it should be legible
            PublishHeldSideToGun(); //the gun learns which hand it is in
            PlaceHeldGun();         //onto the hold point, aimed
            UpdateHeldGunDepth();   //in front of the body or behind it
            MirrorHeldGun();        //needs the side; the pin below needs the mirror
            ApplyGripPin();         //needs the rotation and the mirror both
        }

        TickEquippedGuns();

        //After the guns, never before: a gun's Origin is rewritten from its current animation frame inside its own
        //Update, and that Origin is what every grip anchor is measured against
        UpdateHands();
    }

    //Each step guards its own precondition rather than trusting UpdateGuns' outer check, because they are
    //protected: a boss reusing three of the five gets the same safety the rig itself has
    void Character::PublishHeldSideToGun() const
    {
        if (!CurrentGun) return;

        CurrentGun->IsHeldOnRightHand = IsGunOnRightSide();
    }

    void Character::PlaceHeldGun() const
    {
        if (!CurrentGun || !Hand) return;

        CurrentGun->SetPosition(HoldPoint + Hand->GunOffset + MirroredHeldOffset());
        CurrentGun->Rotation = AimAngle;
    }

    void Character::UpdateHeldGunDepth() const
    {
        if (!CurrentGun) return;

        //Same rule the hands follow, and for the same reason: with the body drawn from behind, whatever it is
        //holding is on the far side of it
        CurrentGun->Depth = ETG::IsFacingBack(CurrentDir)
                                ? CurrentGun->HeldDepthBehindBody
                                : CurrentGun->HeldDepthInFront;
    }

    void Character::MirrorHeldGun() const
    {
        //A character that may not flip its animations keeps the mirror it already had - mid-dash or dead, the
        //sprite is not tracking the aim any more
        if (!CurrentGun || !CanFlipAnims()) return;

        //NOTE: this used to be an AnimationComp->FlipSpritesY(CurrentDir, gun) call in Hero and another in
        //BulletMan, two copies of the decision, and both read the body's 8-way facing while the hold point read
        //IsGunOnRightSide. A gun with its own swap angle therefore had its sprite turn over 22.5 degrees away
        //from the hand holding it
        ETG::Vector2f gunScale = CurrentGun->GetScale();
        gunScale.y = IsGunOnRightSide() ? std::abs(gunScale.y) : -std::abs(gunScale.y);
        CurrentGun->SetScale(gunScale);
    }

    void Character::ApplyGripPin() const
    {
        //A gun with no measured second grip has no pixel to pin, so there is nothing to ask it
        if (!CurrentGun || !CurrentGun->Hands->HasLeftHandAnchor || !CurrentGun->WantsGripPinned()) return;

        //Through the rig as well, so the pinned pixel is the one the artwork is actually showing this frame
        const ETG::Vector2f gripLocal = CurrentGun->Hands->FrameAdjusted(CurrentGun->Hands->LeftHandAnchor,
                                                                          CurrentGun->GetOrigin());
        const ETG::Vector2f scale = CurrentGun->GetScale();

        //Sliding the gun by the difference between these two puts the grip exactly where the pinned rotation would
        //have left it, whatever the barrel is doing. The gun therefore turns about its grip rather than about its
        //own Origin - which is the whole trick: one pixel stands still and everything else swings around it
        const ETG::Vector2f whereTheGripIs = Math::RotateVector(CurrentGun->GetRotation(), scale, gripLocal);
        const ETG::Vector2f whereItStays = Math::RotateVector(CurrentGun->PinnedGripRotation(), scale, gripLocal);

        CurrentGun->SetPosition(CurrentGun->GetPosition() + whereItStays - whereTheGripIs);
    }

    void Character::TickEquippedGuns() const
    {
        for (GunBase* gun : EquippedGuns)
            if (gun)
                gun->Update();
    }

    //<---------- The rig: step 2b, the hands onto the gun ---------->
    void Character::UpdateHands() const
    {
        UpdateHandDepths();

        const bool gunNamesRightGrip = CurrentGun && CurrentGun->Hands->HasRightHandAnchor;
        const bool gunNamesLeftGrip = CurrentGun && CurrentGun->Hands->HasLeftHandAnchor;
        const bool gunOnRight = IsGunOnRightSide();

        //The rig combines the authored anchor, the standing offset and this frame's gesture; the sum arrives here
        //already made, so a gun that authors none of them is placed on exactly the pixel it always was.
        //
        //A gun can also act with a hand it does not hold - the revolver's free hand rides the shot while it is
        //still resting against the body - so both branches take the rig's displacement. Feeding it only into the
        //grip branch would leave every one-handed gun unable to move or offset its off hand at all
        if (Hand)
            PlaceHand(*Hand, gunNamesRightGrip,
                      gunNamesRightGrip ? CurrentGun->Hands->RightHandGrip() : ETG::Vector2f{},
                      HoldPoint + (CurrentGun ? CurrentGun->Hands->RightHandBodyShift(gunOnRight) : ETG::Vector2f{}));

        //The off hand falls back to the opposite side of the body, so it stays visible on a one-handed gun
        //instead of sitting at its construction position at world (0,0)
        if (OffHand)
        {
            const ETG::Vector2f restOffset = gunOnRight ? HandOffsetLeft : HandOffsetRight;

            PlaceHand(*OffHand, gunNamesLeftGrip,
                      gunNamesLeftGrip ? CurrentGun->Hands->LeftHandGrip() : ETG::Vector2f{},
                      BodyRestPosition(restOffset) +
                      (CurrentGun ? CurrentGun->Hands->LeftHandBodyShift(gunOnRight) : ETG::Vector2f{}));
        }
    }

    void Character::UpdateHandDepths() const
    {
        //Written before the hands tick, not after: a hand bakes its depth into its draw properties inside its own
        //Update, so a value set later would be a frame late - and one frame late on a facing change is exactly the
        //frame where the hand is visibly on the wrong side of the body
        const float handDepth = ETG::IsFacingBack(CurrentDir) ? HandDepthBehindBody : HandDepthInFront;

        if (Hand) Hand->Depth = handDepth;
        if (OffHand) OffHand->Depth = handDepth;
    }

    void Character::PlaceHand(class Hand& hand, const bool gunNamesGrip, const ETG::Vector2f& gripAnchor,
                              const ETG::Vector2f& bodyRestPosition) const
    {
        if (gunNamesGrip)
        {
            //Nothing here knows about pinning, and it does not need to: a pinned gun has already been slid so
            //that its grip sits on the pinned point, so a hand placed on that grip lands there for free
            hand.SetPosition(CurrentGun->WorldPointOnGun(gripAnchor));
            hand.SetRotation(CurrentGun->GetRotation()); //a hand turns with the gun it is holding
        }
        else
        {
            hand.SetPosition(bodyRestPosition);
            hand.SetRotation(Rotation); //resting against the body, so it turns with the body
        }

        hand.Update();
    }

    void Character::UpdateHandAndGunVisibility() const
    {
        const bool visible = ShouldShowHeldGun();
        if (Hand) Hand->IsVisible = visible;

        //Both hands belong to the character and remain visible while it can hold a gun. The concrete gun decides
        //whether the off hand attaches to its LeftHandAnchor or rests against the body - and, through the rig,
        //whether this facing is one where it should be out of sight entirely. From behind an unhidden free hand
        //reads as passing through the body, which is why the rig's default set is the three back facings
        if (OffHand)
        {
            const bool hiddenForThisFacing = CurrentGun && CurrentGun->Hands->HideOffHandIn[CurrentDir];
            OffHand->IsVisible = visible && !hiddenForThisFacing;
        }

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
