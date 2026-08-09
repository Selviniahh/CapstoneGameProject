#pragma once
#include "../ComponentBase.h"
#include "../../Managers/SpriteBatch.h"
#include "../../Managers/AssetManager.h"


namespace ETG
{
    class ArrowComp : public ComponentBase
    {
    public:
        explicit ArrowComp(const std::string& texturePath);
        ~ArrowComp() override = default;
        void Initialize() override;
        void Draw() override;
        void Update() override;

    public:
        ETG::Vector2f arrowOriginOffset;
        ETG::Vector2f arrowOffset;

        // Texture'ın kendi merkezi. arrowOriginOffset her frame bunun üzerine uygulanır, Origin'e biriktirilmez:
        // böylece editörde değeri oynatmak oku anında kaydırır ve Initialize ikinci kez çalışsa bile offset iki
        // kez eklenmez.
        ETG::Vector2f BaseOrigin{};

        // Ok, silahın iki noktasını birden tarif eder ve ikisi farklı uzaydadır; bu yüzden owner'a devretmek
        // yetmez.
        [[nodiscard]] ETG::Vector2f ResolveDebugPoint(const char* label, const ETG::Vector2f& point) const override
        {
            // Pivot Position'da kalır, kayan artwork'tür: marker da değerin gerçekten oynattığı şeyi, yani ok
            // sprite'ının oturduğu yeri gösterir.
            if (DebugLabelIs(label, "arrowOriginOffset")) return OriginShiftDebugPoint(point);

            // Silahın Position'ına göre yazılır ve silahla döner; GunBase::Update onu tam olarak böyle uygular.
            // WorldPointOnGun DEĞİL: o, held offset'i geri alıp frame Origin'ini düzelttiği için marker okun
            // gerçekte durduğu yerden kayardı.
            if (DebugLabelIs(label, "arrowOffset") && Owner) return Owner->LocalDebugPoint(point);

            return ComponentBase::ResolveDebugPoint(label, point);
        }

        BOOST_DESCRIBE_CLASS(ArrowComp, (ComponentBase), (arrowOriginOffset, arrowOffset),(),())
    };

    inline ArrowComp::ArrowComp(const std::string& texturePath)
    {
        Texture = AssetManager::LoadTexture(texturePath);

        BaseOrigin = {
            static_cast<float>(Texture->getSize().x / 2),
            static_cast<float>(Texture->getSize().y / 2)
        };
        Origin = BaseOrigin;

        IsVisible = false;
        arrowOriginOffset = {-2.f, 0.f};
    }

    inline void ArrowComp::Draw()
    {
        if (!IsVisible) return;
        ComponentBase::Draw();
        SpriteBatch::Draw(GetDrawProperties());
        SpriteBatch::DrawSinglePixelAtLoc(Position, {1, 1}, Rotation);
    }

    inline void ArrowComp::Initialize()
    {
        ComponentBase::Initialize();
    }

    inline void ArrowComp::Update()
    {
        ComponentBase::Update();
    }
}
