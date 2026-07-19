#include "../../../Engine/Managers/Time.h"
#include "ActiveItemBase.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::ActiveItemBase::ActiveItemBase(const std::string& resourcePath, const std::string& activateResPath, const float& cooldownTime, const float& activeTime) : TotalCooldownTime(cooldownTime), TotalConsumeTime(activeTime)
{
    //Load the texture
    Texture = std::make_shared<ETG::Texture>();

    if (!Texture->loadFromFile(resourcePath))
        std::cerr << "Failed to load hand texture" << std::endl;
    
    Origin = ETG::Vector2f{(float)Texture->getSize().x / 2, (float)Texture->getSize().y / 2};

    // Load sound effects
    if (!PickupSoundBuffers[0].loadFromFile(AssetManager::Resolve("Sounds/Pickup1.ogg")))
        std::cerr << "Failed to load Pickup1.ogg sound" << std::endl;

    if (!PickupSoundBuffers[1].loadFromFile(AssetManager::Resolve("Sounds/Pickup2.ogg")))
        std::cerr << "Failed to load Pickup2.ogg sound" << std::endl;

    if (!ActivateSoundBuffer.loadFromFile(AssetManager::Resolve("Sounds/Consume.ogg")))
        std::cerr << "Failed to load Pickup2.ogg sound" << std::endl;

    if (!ReadySoundBuffer.loadFromFile(AssetManager::Resolve("Sounds/ItemReady.ogg")))
        std::cerr << "Failed to load ItemReady.ogg sound" << std::endl;

    // Connect sounds to their buffers
    PickupSounds[0].setBuffer(PickupSoundBuffers[0]);
    PickupSounds[1].setBuffer(PickupSoundBuffers[1]);

    PickupSounds[0].setVolume(40.0f);
    PickupSounds[1].setVolume(40.0f);

    ActivateSound.setBuffer(ActivateSoundBuffer);
    ReadySound.setBuffer(ReadySoundBuffer);
}

void ETG::ActiveItemBase::RequestUsage()
{
}

void ETG::ActiveItemBase::Update()
{
    GameObjectBase::Update();

    //Just increment the corresponding timer normally first
    if (ActiveItemState == ActiveItemState::Consuming) ConsumeTimer += Time::FrameTick;
    else if (ActiveItemState == ActiveItemState::Cooldown) CoolDownTimer += Time::FrameTick;

    //If current state is consuming and consume time is up, set ActiveItemState to cooldown state 
    if (ActiveItemState == ActiveItemState::Consuming && ConsumeTimer >= TotalConsumeTime)
    {
        ActiveItemState = ActiveItemState::Cooldown;
        ConsumeTimer = 0;
        CoolDownTimer = 0;
    }

    //If current state is cooldown and cooldown timer is up, set current state to be ready
    else if (ActiveItemState == ActiveItemState::Cooldown && CoolDownTimer >= TotalCooldownTime)
    {
        ActiveItemState = ActiveItemState::Ready;
        ReadySound.play();
        ConsumeTimer = 0;
        CoolDownTimer = 0;
    }
}

void ETG::ActiveItemBase::PlayRandomPickupSound()
{
    std::uniform_int_distribution<int> dist(0, PickupSounds.size() - 1);
    const int soundIndex = dist(rng);
    PickupSounds[soundIndex].play();
    IsVisible = false;
}
