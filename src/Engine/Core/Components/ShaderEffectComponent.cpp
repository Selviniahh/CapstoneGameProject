#include "ShaderEffectComponent.h"
#include "../../Managers/Time.h"

namespace ETG
{
    ShaderEffectComponent::ShaderEffectComponent() = default;

    ShaderEffectComponent::~ShaderEffectComponent() = default;

    void ShaderEffectComponent::Play(const ShaderEffect effect, const ShaderEffectParams& params, const float duration)
    {
        //Only the first call of a run reads the owner's effect. A second hit landing mid-flash would
        //otherwise capture the flash itself, and the enemy would stay white for good
        if (!Playing)
        {
            BaseEffect = Owner ? Owner->GetEffect() : ShaderEffect::None;
            BaseParams = Owner ? Owner->GetEffectParams() : ShaderEffectParams{};
            Playing = true;
        }

        Remaining = duration;
        FramesShown = 0;
        ApplyToOwner(effect, params);
    }

    void ShaderEffectComponent::PlayHitFlash()
    {
        Play(ShaderEffect::Flash, MakeFlashParams(HitFlashColor, HitFlashStrength), HitFlashDuration);
    }

    void ShaderEffectComponent::Stop()
    {
        if (!Playing) return;

        ApplyToOwner(BaseEffect, BaseParams);
        Playing = false;
        Remaining = 0.f;
        FramesShown = 0;
    }

    void ShaderEffectComponent::Update()
    {
        if (!Playing) return;

        ++FramesShown;
        Remaining -= Time::FrameTick;

        //Both conditions, not either: a flash shorter than a frame has already run out of time by the
        //first tick, and the frame count is the only thing keeping it on screen long enough to be seen
        if (Remaining <= 0.f && FramesShown >= MinFramesShown)
            Stop();
    }

    void ShaderEffectComponent::ApplyToOwner(const ShaderEffect effect, const ShaderEffectParams& params) const
    {
        if (!Owner) return;
        Owner->SetEffect(effect, params);
    }
}
