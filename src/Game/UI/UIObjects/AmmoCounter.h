#pragma once
#include "../../../Engine/Platform/Platform.h"
#include <string>
#include <filesystem>
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Managers/Globals.h"
#include "../../../Engine/Managers/AssetManager.h"

namespace ETG
{
    class AmmoCounter : public GameObjectBase
    {
    public:
        explicit AmmoCounter(ETG::Vector2f position);
        ~AmmoCounter() override = default;

        void Initialize() override;
        void Draw() override;

        // Set the ammo values to display
        void SetAmmo(int currentAmmo, int maxAmmo);

    private:
        ETG::Text ammoText;
        ETG::Font font;
        int currentAmmo = 0;
        int maxAmmo = 0;
        ETG::Vector2f screenPosition; // Position on screen

        // Update the text content
        void UpdateText();

        BOOST_DESCRIBE_CLASS(AmmoCounter, (GameObjectBase),(),(),(currentAmmo, maxAmmo, screenPosition))
    };

    inline AmmoCounter::AmmoCounter(ETG::Vector2f position)
    {
        AmmoCounter::Initialize();
        ammoText.setPosition(position);
    }

    inline void AmmoCounter::Initialize()
    {
        GameObjectBase::Initialize();

        // Load font - make sure the path is correct for your project
        if (!font.loadFromFile(AssetManager::Resolve("Fonts/alagard.ttf")))
        {
            throw std::runtime_error("Failed to load font");
        }

        // Setup text properties
        ammoText.setFont(font);
        ammoText.setCharacterSize(30);
        ammoText.setFillColor(ETG::Color::White);
        ammoText.setPosition(screenPosition);
        ammoText.setString("0 / 0"); // Default text

        //Set origin the center of the enter text
        const ETG::FloatRect bounds = ammoText.getLocalBounds();
        ammoText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    }

    inline void AmmoCounter::SetAmmo(int current, int max)
    {
        if (current != currentAmmo || max != maxAmmo)
        {
            currentAmmo = current;
            maxAmmo = max;
            UpdateText();
        }
    }

    inline void AmmoCounter::UpdateText()
    {
        // Format the string
        const std::string displayText = std::to_string(currentAmmo) + " / " + std::to_string(maxAmmo);
        ammoText.setString(displayText);
    }

    inline void AmmoCounter::Draw()
    {
        // Draw text directly to the window
        // Note: This doesn't use SpriteBatch since text works differently
        ETG::RenderWindow* window = Globals::Window.get();
        if (window)
        {
            window->draw(ammoText);
        }
    }
}
