#include "RegisterGameTypes.h"
#include "../../Engine/Managers/TypeRegistry.h"
#include "../../Engine/Core/GameObjectBase.h"
#include "../../Engine/Core/Scene/Scene.h"
#include "../../Engine/Core/Components/BaseMoveComp.h"
#include "../../Engine/Core/Components/ArrowComp.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../UI/UserInterface.h"
#include "../Characters/Character.h"
#include "../Characters/Hero/Hero.h"
#include "../Characters/Enemy/EnemyBase.h"
#include "../Characters/Hero/Components/InputComponent.h"
#include "../Characters/Hero/Components/HeroMoveComp.h"
#include "../Characters/Hero/Components/HeroAnimComp.h"
#include "../Characters/Hero/Hand/Hand.h"
#include "../Guns/Base/GunBase.h"
#include "../Guns/RogueSpecial/RogueSpecial.h"
#include "../Guns/VFX/MuzzleFlash.h"
#include "../Guns/VFX/MagazineDrop.h"
#include "../Guns/AK-47/AK47.h"
#include "../Guns/Magnum/Magnum.h"
#include "../Guns/SawedOff/SawedOff.h"
#include "../Projectile/ProjectileBase.h"
#include "../UI/UIObjects/AmmoBarUI.h"
#include "../UI/UIObjects/AmmoIndicatorsUI.h"
#include "../UI/UIObjects/AmmoCounter.h"
#include "../UI/UIObjects/ReloadSlider.h"
#include "../Items/Active/DoubleShoot.h"
#include "../Items/Passive/PlatinumBullets.h"
#include "../Characters/Enemy/BulletMan/BulletMan.h"
#include "../Characters/Enemy/BulletMan/Components/BulletManAnimComp.h"
#include "Game/Items/Active/TakeNoDamage.h"

void ETG::RegisterGameTypes()
{
    TypeRegistry::RegisterType<GameClass>();
    TypeRegistry::RegisterType<GameObjectBase>();
    TypeRegistry::RegisterType<ComponentBase>();
    REGISTER_BASE_CLASS(ComponentBase, GameObjectBase);
    TypeRegistry::RegisterType<Scene>();
    REGISTER_BASE_CLASS(Scene, GameObjectBase);
    TypeRegistry::RegisterType<Character>();
    REGISTER_BASE_CLASS(Character, GameObjectBase);
    TypeRegistry::RegisterType<Hero>();
    REGISTER_BASE_CLASS(Hero, Character);
    TypeRegistry::RegisterType<EnemyBase>();
    REGISTER_BASE_CLASS(EnemyBase, Character);
    TypeRegistry::RegisterType<UserInterface>();
    REGISTER_BASE_CLASS(UserInterface, GameObjectBase);
    TypeRegistry::RegisterType<InputComponent>();
    REGISTER_BASE_CLASS(InputComponent, ComponentBase);
    TypeRegistry::RegisterType<HeroMoveComp>();
    REGISTER_BASE_CLASS(HeroMoveComp, BaseMoveComp);
    TypeRegistry::RegisterType<Animation>();
    REGISTER_BASE_CLASS(Animation, GameClass);
    TypeRegistry::RegisterType<HeroAnimComp>();
    REGISTER_BASE_CLASS(HeroAnimComp, BaseAnimComp<HeroStateEnum>);
    TypeRegistry::RegisterType<GunBase>();
    REGISTER_BASE_CLASS(GunBase, GameObjectBase);
    TypeRegistry::RegisterType<RogueSpecial>();
    REGISTER_BASE_CLASS(RogueSpecial, GunBase);
    TypeRegistry::RegisterType<RogueSpecialAnimComp>();
    REGISTER_BASE_CLASS(RogueSpecialAnimComp, BaseAnimComp<GunStateEnum>);
    TypeRegistry::RegisterType<ProjectileBase>();
    REGISTER_BASE_CLASS(ProjectileBase, GameObjectBase);
    TypeRegistry::RegisterType<Hand>();
    REGISTER_BASE_CLASS(Hand, GameObjectBase);
    TypeRegistry::RegisterType<ArrowComp>();
    REGISTER_BASE_CLASS(ArrowComp, ComponentBase);
    TypeRegistry::RegisterType<MuzzleFlash>();
    REGISTER_BASE_CLASS(MuzzleFlash, GameObjectBase);
    TypeRegistry::RegisterType<MagazineDrop>();
    REGISTER_BASE_CLASS(MagazineDrop, GameObjectBase);
    TypeRegistry::RegisterType<AmmoBarUI>();
    REGISTER_BASE_CLASS(AmmoBarUI, GameObjectBase);
    TypeRegistry::RegisterType<AmmoIndicatorsUI>();
    REGISTER_BASE_CLASS(AmmoIndicatorsUI, Hero);
    TypeRegistry::RegisterType<AmmoCounter>();
    REGISTER_BASE_CLASS(AmmoCounter, GameObjectBase);
    TypeRegistry::RegisterType<ReloadSlider>();
    REGISTER_BASE_CLASS(ReloadSlider, GameObjectBase);
    TypeRegistry::RegisterType<CollisionComponent>();
    REGISTER_BASE_CLASS(CollisionComponent, ComponentBase);
    TypeRegistry::RegisterType<ActiveItemBase>();
    REGISTER_BASE_CLASS(ActiveItemBase, GameObjectBase);
    TypeRegistry::RegisterType<DoubleShoot>();
    REGISTER_BASE_CLASS(DoubleShoot, ActiveItemBase);
    TypeRegistry::RegisterType<PassiveItemBase>();
    REGISTER_BASE_CLASS(PassiveItemBase, GameObjectBase);
    TypeRegistry::RegisterType<PlatinumBullets>();
    REGISTER_BASE_CLASS(PlatinumBullets, PassiveItemBase);
    TypeRegistry::RegisterType<AK47>();
    REGISTER_BASE_CLASS(AK47, GunBase);
    TypeRegistry::RegisterType<AK47AnimComp>();
    REGISTER_BASE_CLASS(AK47AnimComp, BaseAnimComp<GunStateEnum>);
    TypeRegistry::RegisterType<SawedOff>();
    REGISTER_BASE_CLASS(SawedOff, GunBase);
    TypeRegistry::RegisterType<Magnum>();
    REGISTER_BASE_CLASS(Magnum, GunBase);
    TypeRegistry::RegisterType<SawedOffAnimComp>();
    REGISTER_BASE_CLASS(SawedOffAnimComp, BaseAnimComp<GunStateEnum>);
    TypeRegistry::RegisterType<BulletMan>();
    REGISTER_BASE_CLASS(BulletMan, EnemyBase);
    TypeRegistry::RegisterType<BulletManAnimComp>();
    REGISTER_BASE_CLASS(BulletManAnimComp, BaseAnimComp<EnemyStateEnum>);
    TypeRegistry::RegisterType<EnemyMoveCompBase>();
    REGISTER_BASE_CLASS(EnemyMoveCompBase, BaseMoveComp);
    TypeRegistry::RegisterType<TakeNoDamage>();
    REGISTER_BASE_CLASS(TakeNoDamage, ActiveItemBase);
}
