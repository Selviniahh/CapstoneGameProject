#pragma once
#include <filesystem>
#include <random>
#include <array>
#include <iostream>
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/GameObjectBase.h"

//TODO: is active item base class necessary or I should remove it and rename PassiveItemBase to ItemBase and use it as base class for both active and passive classes 
namespace ETG
{
    enum class ActiveItemState;

    class ComponentBase;
    class CollisionComponent;


    class ActiveItemBase : public GameObjectBase
    {
    public:
        ActiveItemBase(const std::string& resourcePath);

    public:
        float TotalCooldownTime;
        float TotalConsumeTime;
        float ConsumeTimer; //Will be increased when the item is consuming
        float CoolDownTimer; //Will be increased when the item is in cooldown
        bool IsEffectActive{};
        ActiveItemState ActiveItemState{};

        //Every Active item must have a collision component otherwise how would it get picked up 
        std::unique_ptr<CollisionComponent> CollisionComp;
        
        ETG::SoundBuffer ActivateSoundBuffer;
        ETG::Sound ActivateSound;

        ETG::SoundBuffer ReadySoundBuffer;
        ETG::Sound ReadySound;

        virtual void RequestUsage();
        void Update() override;

    protected:
        void PlayRandomPickupSound();

        std::string ItemDescription{};

        std::array<ETG::SoundBuffer, 2> PickupSoundBuffers;
        std::array<ETG::Sound, 2> PickupSounds;

        //Random number generator
        std::mt19937 rng{std::random_device{}()};

        BOOST_DESCRIBE_CLASS(ActiveItemBase, (GameObjectBase), (TotalCooldownTime, TotalConsumeTime, ConsumeTimer, CoolDownTimer, ActiveItemState), (ItemDescription), ())
        
    private:
        float DefaultCooldownTime = 15.0f;
        float DefaultActiveTime = 10.0f;
    };

    enum class ActiveItemState
    {
        Ready, //Item is ready to be consumed
        Consuming, //Item is currently being consumed
        Cooldown //Item is in Cooldown state
    };

    BOOST_DESCRIBE_ENUM(ActiveItemState, Ready, Consuming, Cooldown)
}
