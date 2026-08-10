#pragma once
#include "../ComponentBase.h"

namespace ETG
{
    //Plays a fragment program on its owner for a set number of seconds, then puts back whatever the
    //owner was drawing with before. One component, any effect, any object - the hit flash every enemy
    //shows when a bullet lands is just the first caller, spelled out in PlayHitFlash below.
    //
    //The owner is left alone until something calls in: an object with this component attached but never
    //triggered draws exactly as it did before, so attaching it to a base class costs nothing.
    //
    //Two things it deliberately handles, because every caller would otherwise have to:
    //  - retriggering. An AK-47 lands ten hits in the time one flash lasts. Each of those restarts the
    //    timer instead of stacking, and the effect that gets restored at the end is still the owner's
    //    real one (grayscale, for a Character) rather than the flash captured from the previous hit.
    //  - flashes shorter than a frame. A 0.01s flash asked for on a 60 Hz frame (0.016s) would expire
    //    in the same tick it started and never reach the screen. MinFramesShown keeps it alive for at
    //    least one drawn frame, which is what a "one frame flash" means in the first place.
    //
    //Where the owner ticks this matters, and there is one right answer: BEFORE whatever can trigger a
    //flash (the collision pass) and therefore before the owner publishes its draw properties at the end
    //of its own Update. A flash started this tick is then drawn this tick, and only starts being aged
    //on the next one. Ticking it last would let a sub-frame flash be started and expire between two
    //draws. EnemyBase::Update is the worked example.
    class ShaderEffectComponent : public ComponentBase
    {
    public:
        ShaderEffectComponent();
        ~ShaderEffectComponent() override;

        void Update() override;

        //Draw the owner with `effect` for `duration` seconds. Calling it again while one is running
        //restarts the clock and swaps in the new effect; the restored effect stays the pre-flash one.
        void Play(ShaderEffect effect, const ShaderEffectParams& params, float duration);

        //The hit flash, from the knobs below: the owner's silhouette filled with HitFlashColor.
        //This is what an enemy calls from its damage handler
        void PlayHitFlash();

        //Ends the effect early and restores the owner's own. Safe to call when nothing is playing
        void Stop();

        [[nodiscard]] bool IsPlaying() const { return Playing; }

        //<---------- Hit flash knobs ---------->
        //Seconds the flash lasts. Short on purpose: it reads as an impact, not as a colour change
        float HitFlashDuration{0.05f};

        //What the silhouette is filled with, and how far towards it (1 = the flat colour, nothing of
        //the artwork left; lower values keep some of the sprite showing through)
        ETG::Color HitFlashColor{ETG::Color::White};
        float HitFlashStrength{1.f};

        //Frames the effect is guaranteed to be drawn for, however short its duration. 0 disables the
        //guarantee and lets a sub-frame flash be skipped entirely
        int MinFramesShown{1};

    private:
        //Writes an effect onto the owner, if there is one
        void ApplyToOwner(ShaderEffect effect, const ShaderEffectParams& params) const;

        //What the owner draws with when nothing is playing. Captured at the start of a run rather than
        //in the constructor, so a character that turns grayscale on later still gets it back
        ShaderEffect BaseEffect{ShaderEffect::None};
        ShaderEffectParams BaseParams{};

        float Remaining{0.f};
        int FramesShown{0};
        bool Playing{false};

        BOOST_DESCRIBE_CLASS(ShaderEffectComponent, (ComponentBase),
                             (HitFlashDuration, HitFlashColor, HitFlashStrength, MinFramesShown),
                             (), (Playing, Remaining))
    };
}
