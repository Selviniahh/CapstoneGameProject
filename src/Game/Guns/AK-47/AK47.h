#pragma once
#include "../Base/GunBase.h"

namespace ETG
{
    class CollisionComponent;

    class AK47 : public GunBase
    {
    public:
        explicit AK47(const ETG::Vector2f& pos);
        ~AK47() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;
        void Reload() override;

    public:
        // <---------- Magazine drop ---------->
        // 26x10 reload frame'inin sol üstünden okunan pixel'den HandRig::AnchorOrigin, yani 27x7 idle frame'inin
        // merkezi {13.5,3.5} çıkarılarak elde edilir. Silahtaki her authored nokta bu tek referansa göre yazılır;
        // reload frame'i çizilirken oluşan Origin farkını WorldPointOnGun geri ekler.
        //
        // NOTE: Ellerin nereye uzandığı burada değil, HandRig::ReloadReach üzerinde authored'dır ve AK47::Initialize
        // içinde doldurulur. Burada yalnızca magazine'e ait olan kalır.
        ETG::Vector2f MagazineEjectPoint{-2.5f, 2.5f}; // Reload pixel {11,6} - AnchorOrigin {13.5,3.5}

        // Magazine'in ayrılırken aldığı itme, saniye başına pixel cinsindendir. Gravity yönünü değiştirmeden önce
        // silahtan uzaklaşması için hafifçe geriye ve yukarı gider. X silahla birlikte mirror edildiğinden hero'nun
        // içinden düşmez.
        ETG::Vector2f MagazineEjectVelocity{-24.f, -34.f};

        BOOST_DESCRIBE_CLASS(AK47, (GunBase), (MagazineEjectPoint, MagazineEjectVelocity), (), ())

    private:
        // Magazine'i well'den dışarı fırlatır. Her reload için el well'e ulaştığı anda bir kez çağrılır
        void EjectMagazine();
    };

    class AK47AnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        AK47AnimComp();
        void SetAnimations() override;

        float ReloadAnimInterval = 1.f; // Frame Count / Reload Time = Reload Time

        // Shoot ve Recoil one-shot'tır; bu nedenle geçiş yapmadan önce her biri 3 * interval süresince oynar.
        // Trigger basılı tutulduğunda ikili her 0.4 saniyelik fire tick'te yeniden başlar. İkisinin de tamamen
        // görülebilmesi için bu tick içine sığmaları gerekir:
        //
        //    3*Shoot + 3*Recoil <= 0.4   ->   Shoot + Recoil <= 0.133
        //
        // 0.05 + 0.08 bu sürenin 0.39 saniyesini kullanır. Sınırı aşmak crash oluşturmaz; sonraki shot yalnızca
        // kick'i kısa keser ve bu rifle için kabul edilebilir görünür. Yine de sınır kazara değil, bilinçli aşılmalıdır.
        float ShootAnimInterval = 0.05f;
        float RecoilAnimInterval = 0.08f;
        BOOST_DESCRIBE_CLASS(AK47AnimComp, (BaseAnimComp), (), (), ());
    };
}
