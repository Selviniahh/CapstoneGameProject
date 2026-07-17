#pragma once
#include <filesystem>
#include <random>
#include <array>
#include <iostream>
#include "Platform/Platform.h"
#include "../../Core/GameObjectBase.h"
#include "Managers/AssetManager.h"

namespace ETG
{
    class ComponentBase;

    class PassiveItemBase : public GameObjectBase
    {
    public:
        explicit PassiveItemBase(const std::string& resourcePath)
        {
            //Load the texture
            Texture = std::make_shared<ETG::Texture>();

            if (!Texture->loadFromFile(resourcePath))
                std::cerr << "Failed to load hand texture" << std::endl;

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

    protected:
        //Why did I choose array over vector? std::aray is fixed-size with zero overhead. There's no dynamic memory allocation.
        //It's stack allocated. So use std::array if the collection size is known at compile time
        std::array<ETG::SoundBuffer, 2> PickupSoundBuffers;
        std::array<ETG::Sound, 2> Sounds;

        //Random number generator
        std::mt19937 rng{std::random_device{}()};

        BOOST_DESCRIBE_CLASS(PassiveItemBase, (GameObjectBase), (), (), ())
    };
}
