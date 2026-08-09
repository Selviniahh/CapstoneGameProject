#pragma once
#include <boost/describe.hpp>
#include "../../../Engine/Core/Components/BaseAnimComp.h"
#include "../Base/GunBase.h"


namespace ETG
{
    class RogueSpecial : public GunBase
    {
    public:
        explicit RogueSpecial(const ETG::Vector2f& Position);
        ~RogueSpecial() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;
        void Reload() override;

        // Silahın el performansına dair NE VARSA HandRig üzerindedir ve Initialize içinde author edilir; burada
        // ayrı bir alan tutulmaz. Ayarlanacak değerler ImGui'da bu silahın altındaki HandRig node'unda durur.
        BOOST_DESCRIBE_CLASS(RogueSpecial, (GunBase), (), (), ())

    protected:
        // Reload VFX. Her ikisinde de `class` gereklidir: GunBase'in kendi MuzzleFlash *member'ı*
        // bu scope içindeki type adını gizler.
        std::unique_ptr<class MuzzleFlash> ReloadFlash; // Barrel ucundan çıkan yeşil flash
        std::unique_ptr<class MuzzleFlash> ReloadSmoke; // Alttan dışarı çıkan smoke
    };

    class RogueSpecialAnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        RogueSpecialAnimComp();
        void SetAnimations() override;

        // Her state'in kendi authored Origin değerine eklenen ortak offset. Böylece idle/shoot/reload
        // hizalarını birbirine göre değiştirmeden silahın ortak attachment pivot'u ayarlanabilir.
        Vector2f AttachmentOrigin{2.f, 9.f};

        BOOST_DESCRIBE_CLASS(RogueSpecialAnimComp, (BaseAnimComp), (AttachmentOrigin), (), ())
    };
}
