#include "EnemyBase.h"
#include "../Guns/RogueSpecial/RogueSpecial.h"
#include "../Utils/Math.h"
#include "../Core/Components/BaseHealthComp.h"
#include "../Characters/Hand/Hand.h"
#include "../Characters/Hero.h"

namespace ETG
{
    // Add constructor that initializes everything to nullptr
    EnemyBase::EnemyBase(): RogueSpecial(nullptr) , HealthComp(nullptr) , Hand(nullptr), Hero(GameState::GetInstance().GetHero())
    {
    }

    EnemyBase::~EnemyBase() = default;

    void EnemyBase::Initialize()
    {
        // Implementation
    }

    void EnemyBase::Update()
    {
        // Implementation
        GameObjectBase::Update();
    }

    void EnemyBase::Draw()
    {
        // Implementation
        GameObjectBase::Draw();
    }
}
