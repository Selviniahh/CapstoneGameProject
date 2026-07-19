#include "BaseHealthComp.h"
#include "TimerComponent.h"
#include "../../Core/GameObjectBase.h"
#include "../../Core/Factory.h"

namespace ETG
{
    BaseHealthComp::BaseHealthComp(const float maxHealth)
        : ComponentBase(), CurrentHealth(maxHealth), MaxHealth(maxHealth), IsDamaged(false)
    {
        BaseHealthComp::Initialize();
    }

    BaseHealthComp::~BaseHealthComp() = default;

    void BaseHealthComp::Initialize()
    {
        ComponentBase::Initialize();
        CurrentHealth = MaxHealth;

        // Create the damage feedback timer
        DamageFeedbackTimer = ETG::CreateGameObjectAttached<TimerComponent>(this, DamagedVisualFeedbackDuration);
        InvulnerabilityTimer = ETG::CreateGameObjectAttached<TimerComponent>(this, InvulnerabilityDuration);

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
        CurrentHealth = std::min(MaxHealth, CurrentHealth + amount);

        // Broadcast heal event
        OnHealed.Broadcast(amount, healInstigator);

        return true;
    }
}
