#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "../Core/GameObjectBase.h"
#include "../Managers/StateEnums.h"
#include "../Core/Factory.h"
#include "../Guns/Base/GunBase.h"

namespace ETG
{
    class CollisionComponent;
    class ReloadText;
    class GunBase;
    class Hand;
    class RogueSpecial;
    class HeroAnimComp;
    class InputComponent;
    class HeroMoveComp;
    class ReloadSlider;
    class BaseHealthComp;


    class Hero : public GameObjectBase
    {
    public:
        explicit Hero(sf::Vector2f Position);
        ~Hero() override;
        void Update() override;
        void Initialize() override;
        void Draw() override;
        [[nodiscard]] GunBase* GetCurrentHoldingGun() const;

        static float MouseAngle;
        static Direction CurrentDirection;
        static bool IsShooting;

        HeroStateEnum CurrentHeroState{HeroStateEnum::Idle};

        std::unique_ptr<RogueSpecial> RogueSpecial;
        std::unique_ptr<HeroMoveComp> MoveComp;
        std::unique_ptr<Hand> Hand;
        std::unique_ptr<ReloadText> ReloadText;
        std::unique_ptr<CollisionComponent> CollisionComp;
        std::unique_ptr<BaseHealthComp> HealthComp;

        ActiveItemBase* CurrActiveItem{};

        std::unique_ptr<HeroAnimComp> AnimationComp;
        std::unique_ptr<InputComponent> InputComp;

        //Selected guns 
        std::vector<GunBase*> EquippedGuns; // Array of equipped guns
        GunBase* CurrentGun = nullptr; // Currently selected gun
        int currentGunIndex = 0; // Track the index of current gun
        float KnockBackMagnitude = 150.f;

        //When equipping a new gun pickup
        void EquipGun(GunBase* newGun);

        // When scrolling the mouse wheel, switch back to the default (index 0) gun.
        void SwitchToPreviousGun();
        void SwitchToNextGun();

        BOOST_DESCRIBE_CLASS(Hero, (GameObjectBase),
                             (MouseAngle, CurrentDirection, CurrentHeroState, IsShooting, KnockBackMagnitude),
                             (),
                             ())
    private:
        void UpdateGunVisibility();
    };
}
