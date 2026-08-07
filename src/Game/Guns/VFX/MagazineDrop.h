#pragma once
#include <string>
#include "../../../Engine/Core/GameObjectBase.h"

namespace ETG
{
    //The magazine a gun throws away while it reloads. Purely cosmetic: it is dropped at the magazine
    //well, given a shove, and from there it is only gravity, a spin and a fade - nothing collides with
    //it and nothing reads it back.
    //
    //NOTE: deliberately NOT parented to the gun the way MuzzleFlash is. A flash is painted on the barrel
    //and has to follow it wherever it points; a dropped magazine has left the gun, so it keeps the world
    //position it was dropped at while the hero walks on. That difference is the whole reason this is its
    //own class rather than another MuzzleFlash with a different sheet
    class MagazineDrop : public GameObjectBase
    {
    public:
        //Starts out with no texture: which magazine falls out is the owning gun's business, so the gun
        //hands it its sprite rather than this guessing one for everybody
        MagazineDrop();
        ~MagazineDrop() override = default;

        //Loads the sprite and centres the origin on it, so the drop spins about its middle
        void SetSprite(const std::string& relativePath);
        [[nodiscard]] bool HasSprite() const { return Texture != nullptr; }

        //Throws one magazine. Everything a fall needs is decided here, so a gun that reloads again before
        //the last magazine has faded simply restarts this one instead of leaking a second object
        void Drop(const ETG::Vector2f& worldPos, float rotation, const ETG::Vector2f& velocity, float depth);

        void Update() override;
        void Draw() override;

        [[nodiscard]] bool IsFalling() const { return TimeLeft > 0.f; }

        //Pixels per second squared. The dungeon is drawn top-down but the drop reads as a real object
        //falling to the floor, so it accelerates straight down the screen rather than along the gun
        float Gravity{260.f};

        //Degrees per second. A magazine leaving a gun tumbles; without this it slides down looking pasted on
        float SpinSpeed{220.f};

        //How long the whole drop lasts, and how much of its tail is spent fading out. The fade is what
        //stands in for the magazine landing - there is no floor to land on, so it leaves instead
        float LifeTime{0.9f};
        float FadeTime{0.35f};

    private:
        ETG::Vector2f Velocity{};
        float TimeLeft{};

        BOOST_DESCRIBE_CLASS(MagazineDrop, (GameObjectBase),
                             (Gravity, SpinSpeed, LifeTime, FadeTime),
                             (),
                             (Velocity, TimeLeft))
    };
}
