#include "SpawnInitialLevel.h"
#include "../Managers/GameManager.h"
#include "../Characters/Hero.h"
#include "../Guns/AK-47/AK47.h"
#include "../Guns/SawedOff/SawedOff.h"
#include "../Guns/Magnum/Magnum.h"
#include "../Enemy/BulletMan/BulletMan.h"
#include "../Items/Active/DoubleShoot.h"
#include "../Items/Passive/PlatinumBullets.h"

void ETG::SpawnInitialLevel::Spawn(GameManager& game)
{
    game.SpawnGameObject<Hero>(Vector2f{10, 10});
    game.SpawnGameObject<AK47>(Vector2f{-100, 100});
    game.SpawnGameObject<SawedOff>(Vector2f{-150, 100});
    game.SpawnGameObject<Magnum>(Vector2f{-200, 100});
    game.SpawnGameObject<BulletMan>(Vector2f{50, 50});
    game.SpawnGameObject<PlatinumBullets>();
    game.SpawnGameObject<DoubleShoot>();
}
