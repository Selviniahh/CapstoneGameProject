#include "BaseHealthComp.h"
#include "TimerComponent.h"
#include "../../Core/GameObjectBase.h"
#include "../../Core/Factory.h"

namespace ETG
{
    BaseHealthComp::BaseHealthComp(const float maxHealth)
        : ComponentBase(), CurrentHealth(maxHealth), MaxHealth(maxHealth), IsDamaged(false)
    {
        //Sub-objects are the constructor's to build, not Initialize's: Initialize is allowed to run again, and a
        //second run used to replace both timers with fresh ones - taking the listeners below down with them
        DamageFeedbackTimer = ETG::CreateGameObjectAttached<TimerComponent>(this, DamagedVisualFeedbackDuration);
        InvulnerabilityTimer = ETG::CreateGameObjectAttached<TimerComponent>(this, InvulnerabilityDuration);

        BaseHealthComp::Initialize();

        //Last statement, and the constructor's alone: see GameObjectBase::BindEvents
        BaseHealthComp::BindEvents();
    }

    BaseHealthComp::~BaseHealthComp() = default;

    //Only what is safe to run twice: this resets the component to full health, nothing more
    void BaseHealthComp::Initialize()
    {
        ComponentBase::Initialize();
        CurrentHealth = MaxHealth;
    }

    void BaseHealthComp::BindEvents()
    {
        InvulnerabilityTimer->OnTimerFinished.AddListener([this]()
        {
            InvulnerabilityEnabled = false;
        });

        // Set up the timer completion event
        DamageFeedbackTimer->OnTimerFinished.AddListener([this]()
        {
            IsDamaged = false;
        });
    }

    void BaseHealthComp::Update()
    {
        ComponentBase::Update();
        DamageFeedbackTimer->Update();
        InvulnerabilityTimer->Update();

        //MaxHealth can drop underneath CurrentHealth while the game is running - a curse item is removed, a modifier
        //expires - and a CurrentHealth above it would read as more than 100% on every health bar. Clamping down is
        //safe; growing MaxHealth deliberately does NOT heal, so a heart container gives capacity, not free health
        if (CurrentHealth > MaxHealth) CurrentHealth = MaxHealth;
    }

    bool BaseHealthComp::IsShowingDamageFeedback() const
    {
        return IsDamaged && DamageFeedbackTimer->IsFinished() == false;
    }

    bool BaseHealthComp::ApplyDamage(const float damage, const float forceMagnitude, GameObjectBase* damageInstigator)
    {
        // Don't process damage if dead or invulnerable
        if (IsDead() || damage <= 0)
            return false;

        //If invulnerability is enabled, ignore damage, timer will start and it's delegate automatically will set this to false
        if (InvulnerabilityEnabled)
        {
            InvulnerabilityTimer->Start();
            return false;
        }

        // Apply damage
        const float previousHealth = CurrentHealth;
        CurrentHealth = std::max(0.0f, CurrentHealth - damage);

        // Trigger damage visual feedback
        IsDamaged = true;
        DamageFeedbackTimer->Reset();
        DamageFeedbackTimer->Start();

        // Broadcast damage event
        OnDamageTaken.Broadcast(damage,forceMagnitude, damageInstigator);

        // Check for death
        if (previousHealth > 0 && CurrentHealth <= 0)
        {
            OnDeath.Broadcast(damageInstigator);
        }

        return true;
    }

    bool BaseHealthComp::Heal(const float amount, GameObjectBase* healInstigator)
    {
        // Don't heal if dead or amount is non-positive
        if (IsDead() || amount <= 0)
            return false;

        // Apply healing
        CurrentHealth = std::min(MaxHealth.Get(), CurrentHealth + amount);

        // Broadcast heal event
        OnHealed.Broadcast(amount, healInstigator);

        return true;
    }
}
