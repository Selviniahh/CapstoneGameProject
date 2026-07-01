#pragma once
#include "../ComponentBase.h"
#include "../../Managers/SpriteBatch.h"
#include "../../Managers/Globals.h"


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

        BOOST_DESCRIBE_CLASS(ArrowComp, (ComponentBase), (arrowOriginOffset, arrowOffset),(),())
    };

    inline ArrowComp::ArrowComp(const std::string& texturePath)
    {
        if (!Texture) Texture = std::make_shared<ETG::Texture>();
        if (!Texture->loadFromFile(texturePath))
            throw std::runtime_error(texturePath + " not found");

        Origin = {
            static_cast<float>(Texture->getSize().x / 2),
            static_cast<float>(Texture->getSize().y / 2)
        };

        IsVisible = false;
        arrowOriginOffset = {-2.f, 0.f};
    }

    inline void ArrowComp::Draw()
    {
        if (!IsVisible) return;
        ComponentBase::Draw();
        SpriteBatch::Draw(GetDrawProperties());
        Globals::DrawSinglePixelAtLoc(Position, {1, 1}, Rotation);
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
