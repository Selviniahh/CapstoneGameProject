#pragma once
#include "../../Core/ComponentBase.h"
#include "../../Core/Events/EventDelegate.h"

namespace ETG
{
    class GameObjectBase;
    class TimerComponent;

    class BaseHealthComp : public ComponentBase
    {
    public:
        explicit BaseHealthComp(float maxHealth = 100.0f);
        ~BaseHealthComp() override;

        void Initialize() override;
        void Update() override;

        // Apply damage to this health component's owner
        bool ApplyDamage(float damage, float forceMagnitude, GameObjectBase* damageInstigator = nullptr);

        // Heal the owner
        bool Heal(float amount, GameObjectBase* healInstigator = nullptr);

        // Check if entity is dead
        bool IsDead() const { return CurrentHealth <= 0.0f; }

        // Get current health percentage (0.0 to 1.0)
        float GetHealthPercent() const { return CurrentHealth / MaxHealth; }
        
        // Check if currently showing damage feedback
        bool IsShowingDamageFeedback() const;

    public:
        // Events
        EventDelegate<float, float, GameObjectBase*> OnDamageTaken;
        EventDelegate<float, GameObjectBase*> OnHealed;
        EventDelegate<GameObjectBase*> OnDeath;

        float CurrentHealth;
        float MaxHealth;
        bool InvulnerabilityEnabled = false; //For now only hero will use this 
        float InvulnerabilityDuration = 0.75f; //For now only hero will use this 

        // Visual feedback duration for damage
        float DamagedVisualFeedbackDuration = 0.2f;
        
    private:
        std::unique_ptr<TimerComponent> DamageFeedbackTimer;
        std::unique_ptr<TimerComponent> InvulnerabilityTimer;
        bool IsDamaged = false;
        
        BOOST_DESCRIBE_CLASS(BaseHealthComp, (ComponentBase),
                         (CurrentHealth, MaxHealth, InvulnerabilityEnabled, DamagedVisualFeedbackDuration), (), ())
    };
}
