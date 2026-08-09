#include "HandRig.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include "../../../Utils/Math.h"

namespace ETG
{
    // ============================== DirectionFlags ==============================
    bool DirectionFlags::operator[](const Direction direction) const
    {
        switch (direction)
        {
        case Direction::Right: return Right;
        case Direction::DownRight: return DownRight;
        case Direction::Down: return Down;
        case Direction::DownLeft: return DownLeft;
        case Direction::Left: return Left;
        case Direction::UpLeft: return UpLeft;
        case Direction::Up: return Up;
        case Direction::UpRight: return UpRight;
        }

        return false;
    }

    DirectionFlags DirectionFlags::BackFacings()
    {
        // IsFacingBack ile aynı üç arc. Predicate'i çağırmak yerine alanlar açıkça yazılır; bu bir varsayılan
        // set'tir, kuralın kendisi değildir ve silah dilediği checkbox'ı tek tek değiştirebilir.
        DirectionFlags flags{};
        flags.Up = true;
        flags.UpLeft = true;
        flags.UpRight = true;
        return flags;
    }

    // ============================== BreathMotion ==============================
    ETG::Vector2f BreathMotion::Advance(const float deltaSeconds)
    {
        if (Height == 0.f || Period <= 0.f) return {};

        Elapsed += deltaSeconds;

        // Period'a göre wrap edilir. Aksi hâlde accumulator oturum boyunca büyür ve float precision, saatler sonra
        // nefesi gözle görülür biçimde kekelemeye başlatırdı.
        while (Elapsed >= Period) Elapsed -= Period;

        const float phase = Elapsed / Period;

        // Uçlara sıkışan bir ratio, o yarıyı sıfır süreye indirip hard cut oluştururdu
        const float fall = std::clamp(FallRatio, 0.01f, 0.99f);

        // İki yarı da smoothstep ile taşınır: türevi iki uçta da sıfır olduğundan el ne dibe çarpar ne de tepede
        // köşe yapar. İniş ve çıkış hızlarını ayıran tek şey hangi yarının daha uzun sürdüğüdür.
        const float depth = phase < fall
                                ? Math::SmoothStep(phase / fall) // rest'ten aşağı
                                : 1.f - Math::SmoothStep((phase - fall) / (1.f - fall)); // aşağıdan rest'e

        // +Y aşağıdır: nefes eli rest pose'unun altına indirir, üstüne çıkarmaz
        return {0.f, Height * depth};
    }

    // ============================== ShotKickMotion ==============================
    void ShotKickMotion::Trigger()
    {
        Elapsed = 0.f;
    }

    void ShotKickMotion::Advance(const float deltaSeconds)
    {
        if (Elapsed < 0.f) return;

        Elapsed += deltaSeconds;

        // Bittiğinde negatife çekilir; böylece aşağıdaki iki offset de tam olarak zero'ya döner ve silah da el de
        // authored idle pose'una oturur
        if (Elapsed >= Duration) Elapsed = -1.f;
    }

    ETG::Vector2f ShotKickMotion::GunOffset() const
    {
        if (Elapsed < 0.f || GunCircleRadius == 0.f) return {};

        // Tam bir tur: kavrayış geriye-yukarıya çıkar, ileriye-aşağıya döner ve tam olarak başladığı yerde biter.
        // Turun iki ucunda da offset zero olduğundan kick idle pose ile kesintisiz birleşir.
        const float sweep = Math::Progress01(Elapsed, Duration) * 2.f * std::numbers::pi_v<float>;

        // Gun-local uzayda +X muzzle'a doğru, +Y aşağıdır. cos-1 dairenin merkezini bir radius geriye kaydırır;
        // böylece hareket ileri atılmak yerine silahın geri tepmesiyle başlar. sin'in negatiflenmesi turu yukarıdan
        // başlatır.
        return {GunCircleRadius * (std::cos(sweep) - 1.f), -GunCircleRadius * std::sin(sweep)};
    }

    ETG::Vector2f ShotKickMotion::OffHandOffset() const
    {
        if (Elapsed < 0.f || OffHandRise == 0.f) return {};

        // Bell curve iki uçta da sıfır olduğundan vuruş, elin altındaki nefes hattından kopmadan ayrılır ve tam
        // olarak ona geri oturur
        return {0.f, -OffHandRise * Math::BellCurve(Math::Progress01(Elapsed, Duration))};
    }

    // ============================== ReloadReachMotion ==============================
    float ReloadReachMotion::Evaluate(const float progress,
                                      const ETG::Vector2f& workingAnchor, const ETG::Vector2f& steadyAnchor,
                                      ETG::Vector2f& workingGesture, ETG::Vector2f& steadyGesture) const
    {
        // Çalışan elin ulaşması gereken yer, kendi anchor'ına göre displacement olarak ifade edilir. Sıfırda başlayıp
        // bitmesi, reload'un iki ucunda da elin idle pose ile kesintisiz devam etmesini sağlar.
        const ETG::Vector2f toTarget = WorkingPoint - workingAnchor;

        // Reload'un elleri idle grip'lerinden ne kadar uzaklaştırdığı: iki uçta 0, stroke boyunca 1 değerindedir.
        // Aşağıdaki her beat kendi length'ine bölünür; alt sınırlar, sıfıra sıkıştırılan bir beat'in ele NaN
        // göndermek yerine yalnızca hard cut oluşturmasını sağlar.
        float engagement;

        if (progress < GrabEnd)
        {
            // Aşağı uzanma: el idle grip'inden ayrılır ve hedefe gider
            engagement = progress / std::max(GrabEnd, 0.001f);
            workingGesture = toTarget * engagement;
        }
        else if (progress < StrokeEnd)
        {
            // Stroke'un kendisi: yukarı çekip geri indirme. Half sine sıfırdan ayrılıp sıfıra döner; böylece
            // triangle wave'in aksine elin çekişin tepesinde keskin bir köşesi olmaz.
            const float strokeTime = (progress - GrabEnd) / std::max(StrokeEnd - GrabEnd, 0.001f);

            engagement = 1.f;
            workingGesture = toTarget + ETG::Vector2f{0.f, -StrokeHeight * Math::BellCurve(strokeTime)};
        }
        else
        {
            // Idle grip'e geri döner ve reload bittiği anda tam olarak anchor üzerine yerleşir
            engagement = (1.f - progress) / std::max(1.f - StrokeEnd, 0.001f);
            workingGesture = toTarget * engagement;
        }

        // Sabit kalan el kendi grip'inden hiç ayrılmaz; yalnızca grip'in tilted reload pose içindeki hareketini
        // takip eder. Aynı weight ile fade edildiğinden çalışan elle birlikte ulaşır ve ayrılır.
        steadyGesture = (SteadyPoint - steadyAnchor) * engagement;

        return engagement;
    }

    // ============================== HandRig ==============================
    HandRig::HandRig()
    {
        // Rig'in kendi görseli yoktur; yalnızca silahın ellerine dair veriyi taşır ve Hierarchy'de silahın altında
        // ayarlanabilir bir node olarak görünür
        IsVisible = false;
    }

    void HandRig::CaptureAnchorOriginOnce(const ETG::Vector2f& idleOrigin)
    {
        if (AnchorOriginCaptured) return;

        AnchorOrigin = idleOrigin;
        AnchorOriginCaptured = true;
    }

    void HandRig::OnShotFired()
    {
        ShotKick.Trigger();
    }

    void HandRig::RestHands()
    {
        RightHandGesture = {};
        LeftHandGesture = {};
    }

    void HandRig::Tick(const float deltaSeconds, const float reloadProgress)
    {
        // Nefes ve kick'in saatleri reload sırasında da akmaya devam eder. Reload bitince el, nefesin donduğu
        // yerden değil, o an bulunması gereken yerden devralır; aksi hâlde reload'un bitişinde bir sıçrama olurdu.
        const ETG::Vector2f breath = OffHandBreath.Advance(deltaSeconds);
        ShotKick.Advance(deltaSeconds);

        // Reload performansı diğer her şeyin yerine geçer: iki eli de kendisi yazar. İki el de silahın üzerinde
        // çalıştığı için nefesin oraya karışacak bir işi yoktur.
        if (ReloadReach.Enabled && reloadProgress >= 0.f)
        {
            ReloadReach.Evaluate(reloadProgress, RightHandAnchor, LeftHandAnchor, RightHandGesture, LeftHandGesture);
            return;
        }

        // Silahı tutan elin idle'da yapacak işi yoktur: shot kick'i silahın kendisi taşır ve el ona anchor'lıdır,
        // dolayısıyla kaymayı gesture olmadan da takip eder
        RightHandGesture = {};

        // Boştaki el nefes alır ve shot anında bunun üstüne bir vuruş biner
        LeftHandGesture = breath + ShotKick.OffHandOffset();
    }
}
