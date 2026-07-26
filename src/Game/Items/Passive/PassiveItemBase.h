#pragma once
#include <filesystem>
#include <random>
#include <array>
#include <iostream>
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Managers/AssetManager.h"

namespace ETG
{
    class ComponentBase;
    class GunBase;

    class PassiveItemBase : public GameObjectBase
    {
    public:
        explicit PassiveItemBase(const std::string& resourcePath)
        {
            //Load the texture
            Texture = AssetManager::LoadTexture(resourcePath);

            // Load sound effects
            if (!PickupSoundBuffers[0].loadFromFile(AssetManager::Resolve("Sounds/Pickup1.ogg")))
                std::cerr << "Failed to load Pickup1.ogg sound" << std::endl;

            if (!PickupSoundBuffers[1].loadFromFile(AssetManager::Resolve("Sounds/Pickup2.ogg")))
                std::cerr << "Failed to load Pickup2.ogg sound" << std::endl;

            // Connect sounds to their buffers
            Sounds[0].setBuffer(PickupSoundBuffers[0]);
            Sounds[1].setBuffer(PickupSoundBuffers[1]);
        }

        std::string ItemDescription{};

        //Attaches this item's stat modifiers to a gun. Called on pickup for every gun the hero already owns, and
        //again by Hero::EquipGun for every gun picked up afterwards.
        //
        //NOTE: this hook exists because a perk used to be applied straight to `hero->GetCurrentHoldingGun()`, which
        //meant it only ever reached whichever gun happened to be in hand at pickup time. Switching weapons silently
        //left the perk behind. Re-applying is harmless: a modifier is keyed on ModifierSource, so a second call
        //overwrites the first rather than stacking
        virtual void ApplyGunPerk(GunBase& gun)
        {
        }

        //The exact inverse. Nothing calls it yet - items cannot be dropped - but it is what makes the modifiers worth
        //having, so it stays defined next to the thing it undoes rather than being invented later
        virtual void RemoveGunPerk(GunBase& gun);

        //The name every modifier this item attaches is filed under, and the label the editor shows next to them.
        //Derived items set it in their constructor; it has to be unique per item type and stable across a session
        [[nodiscard]] const std::string& GetModifierSource() const { return ModifierSource; }

    protected:
        std::string ModifierSource{"UnnamedPassiveItem"};

        //Why did I choose array over vector? std::aray is fixed-size with zero overhead. There's no dynamic memory allocation.
        //It's stack allocated. So use std::array if the collection size is known at compile time
        std::array<ETG::SoundBuffer, 2> PickupSoundBuffers;
        std::array<ETG::Sound, 2> Sounds;

        //Random number generator
        std::mt19937 rng{std::random_device{}()};

        BOOST_DESCRIBE_CLASS(PassiveItemBase, (GameObjectBase), (), (), ())
    };
}
