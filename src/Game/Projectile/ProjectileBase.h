#pragma once
#include "../../Engine/Animation/Animation.h"
#include "../../Engine/Core/GameObjectBase.h"

namespace ETG
{
    class CollisionComponent;
    class TimerComponent;

    class ProjectileBase : public GameObjectBase
    {
    public:
        ProjectileBase() = default;
        ~ProjectileBase() override;
        ProjectileBase(const ETG::Texture& texture, ETG::Vector2f spawnPos, ETG::Vector2f velocity, float range, float rotation, float damage = 1.f, float force = 1.f);

        void Initialize() override;
        void Update() override;
        void Draw() override;

        //TODO: idk why this is considered as const
        mutable ETG::Vector2f ProjVelocity;
        float Range;
        float Damage;
        float Force; //knockback amount

        std::unique_ptr<CollisionComponent> CollisionComp;
        std::unique_ptr<TimerComponent> TimerComp;

        // <---------- Impact VFX ---------->
        // Projectile artık çarptığı anda kaybolmaz: durur, çarpma noktasında impact animation'ı bir kez oynar ve
        // ancak animation bittiğinde MarkForDestroy edilir. Silah yalnızca hangi sheet'in oynayacağını söyler
        // (GunBase::SetProjectileImpact), oynatmayı ve yok etmeyi bu class yürütür. Böylece yeni bir silah tek
        // satırla aynı davranışı alır; hiçbir şey yazmayan silah ise GunBase'in verdiği ortak impact'i kullanır.
        //
        // NOTE: Şimdilik tek bir animation vardır ve çarpma yönünden bağımsız oynar. Gungeon'ın vertical/horizontal
        // ayrımı duvarın hangi yüzüne çarpıldığıyla ilgilidir; duvar collision'ı girdiğinde ikinci bir sheet ve onu
        // seçen bir kural buraya eklenir, çağıran taraf değişmez.
        void SetImpactAnimation(const Animation& impactAnim);

        // Çarpma anı. Hareketi ve collision'ı durdurup animation'ı `impactPoint` üzerinde başlatır. Impact
        // animation'ı olmayan projectile eskisi gibi o frame yok edilir, yani çağıran taraf ayrım yapmaz.
        void BeginImpact(const ETG::Vector2f& impactPoint);

        [[nodiscard]] bool IsImpacting() const { return Impacting; }

    private:
        float DistanceTraveled = 0.0f; //Track the total distance traveled

        Animation ImpactAnim;
        bool HasImpactAnim{false};

        // Çarpma başladıktan sonra projectile yalnızca VFX'tir: Update onu hareket ettirmez, Draw da mermi
        // sprite'ı yerine animation'ı çizer.
        bool Impacting{false};

        // Animation'ın oynadığı yer. Projectile'ın kendi Position'ı kullanılmaz: enemy'ye çarpan mermi, merminin
        // merkezinde değil iki collision dairesinin değdiği noktada patlamalıdır.
        ETG::Vector2f ImpactPos{};

        BOOST_DESCRIBE_CLASS(ProjectileBase, (GameObjectBase), (ProjVelocity, Range, Damage, Force), (), ())
    };
}
