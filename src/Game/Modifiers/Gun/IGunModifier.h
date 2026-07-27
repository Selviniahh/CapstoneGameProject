#pragma once

namespace ETG
{
    //Type tag for the gun modifier family. There is deliberately nothing to implement: its only job is to give
    //ModifierManager a common base to store, and to make ModifierManager<IGunModifier> reject a hero modifier at
    //compile time. A modifier's identity is its concrete type, so no name or id is needed here
    //Everything one pull of the trigger is allowed to vary. The gun fills this in with its own defaults, hands it
    //round the modifiers, then fires whatever comes back
    struct ShotParams
    {
        int ShotCount = 1;
        float Spread = 0.f;
    };

    class IGunModifier
    {
    public:
        virtual ~IGunModifier() = default;

        //Fired once per shot, before any bullet is queued. Modifiers see the params in an unspecified order and
        //each one may adjust them, so prefer adding to a field over overwriting it when the effect is meant to
        //stack with others.
        //NOTE: this is for the shape of the shot only. A flat "+2 damage" is not a modifier at all - it is a
        //StatModifier on the gun's Damage stat, which needs no class and no hook (see PlatinumBullets)
        virtual void ModifyShot(ShotParams& shot) = 0;
    };
}
