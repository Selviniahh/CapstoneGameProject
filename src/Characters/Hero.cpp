#include "Hero.h"
#include <filesystem>
#include "../Managers/GameState.h"
#include "../Managers/SpriteBatch.h"
#include "../Guns/RogueSpecial/RogueSpecial.h"
#include "Components/HeroAnimComp.h"
#include "Components/HeroMoveComp.h"
#include "Components/InputComponent.h"
#include "Hand/Hand.h"

float ETG::Hero::MouseAngle = 0;
ETG::Direction ETG::Hero::CurrentDirection{};
bool ETG::Hero::IsShooting{};

ETG::Hero::Hero(const sf::Vector2f Position)
{
    this->Position = Position;
    Depth = 2;
    GameState::GetInstance().SetHero(this);

    Hand = ETG::CreateGameObjectAttached<class Hand>(this, (std::filesystem::path(RESOURCE_PATH) / "Player" / "rogue_hand_001.png").generic_string());
    RogueSpecial = ETG::CreateGameObjectAttached<class RogueSpecial>(this, Hand->GetRelativePosition());
    AnimationComp = ETG::CreateGameObjectAttached<HeroAnimComp>(this);
    AnimationComp->Initialize();
    AnimationComp->Update(); //Set the Texture during Initialization
    MoveComp = ETG::CreateGameObjectAttached<HeroMoveComp>(this);
    InputComp = ETG::CreateGameObjectAttached<InputComponent>(this);

    Hero::Initialize();
}

void ETG::Hero::Initialize()
{
    GameObjectBase::Initialize();
}

ETG::Hero::~Hero() = default;

void ETG::Hero::Update()
{
    GameObjectBase::Update();

    InputComp->Update(*this);
    MoveComp->Update();

    // Animations - still need direct access to HeroAnimComp for specialized functionality
    AnimationComp->FlipSpritesY<class RogueSpecial>(CurrentDirection, *RogueSpecial);
    AnimationComp->FlipSpritesX(CurrentDirection, *this);
    AnimationComp->Update();

    //Set hand properties
    const sf::Vector2f HandOffsetForHero = AnimationComp->IsFacingRight(CurrentDirection) ? sf::Vector2f{8.f, 5.f} : sf::Vector2f{-7.f, 5.f};
    Hand->SetPosition(Position + Hand->HandOffset + HandOffsetForHero);
    Hand->Update();

    //Gun
    RogueSpecial->SetPosition(Hand->GetPosition() + Hand->GunOffset);
    RogueSpecial->Rotation = MouseAngle;
    RogueSpecial->Update();
    if (IsShooting) RogueSpecial->Shoot();

    //Necessary to call end of update because the texture is created at here. 
    GameObjectBase::Update();
}

void ETG::Hero::Draw()
{
    GameObjectBase::Draw();
    RogueSpecial->Draw();
    SpriteBatch::Draw(GetDrawProperties());

    Hand->Draw();
}

ETG::GunBase* ETG::Hero::GetCurrentHoldingGun() const
{
    return dynamic_cast<GunBase*>(RogueSpecial.get());
}
void ETG::Hero::EquipActiveItem(ActiveItem* item) {
    EquippedActiveItem = item;
}

void ETG::Hero::EquipPassiveItem(PassiveItem* item) {
    EquippedPassiveItem = item;
    // Apply passive effect (e.g., 20% faster fire rate)
}


bool ETG::Hero::IsMouseNearHero(const sf::RenderWindow& window) const {
    // Get the mouse position in the world coordinates
    sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // Calculate distance between mouse cursor and hero
    float distance = std::sqrt(
        std::pow(mousePosition.x - GetPosition().x, 2) +
        std::pow(mousePosition.y - GetPosition().y, 2)
    );

    // Define proximity threshold (e.g., 100 pixels)
    const float proximityThreshold = 10.0f;
    return distance < proximityThreshold;
}
bool ETG::Hero::IsNearItem(GameObjectBase* item) const {
    if (!item) return false;

    // Calculate distance between hero and item
    float distance = std::sqrt(
        std::pow(item->GetPosition().x - GetPosition().x, 2) +
        std::pow(item->GetPosition().y - GetPosition().y, 2)
    );

    // Define proximity threshold (e.g., 50 pixels)
    const float proximityThreshold = 50.0f;
    return distance < proximityThreshold;
}