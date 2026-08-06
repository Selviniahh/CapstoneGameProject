#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Scene/Scene.h"
#include "Engine/Platform/Platform.h"
#include "Game/Managers/GameManager.h"

//=====================================================================================================================
//  TEST ENVIRONMENT - the world an interactive test builds for itself, and the handle it reads that world through.
//
//  The rule the whole design rests on: A TEST NEVER TOUCHES THE GAME'S LEVEL. There is no level here at all - the
//  interactive test host boots the engine with an EMPTY world (GameManager::LevelSpawnOverride) and hands each test
//  this object to fill it. That is why a new mechanic can be tested by writing a test instead of editing
//  SpawnInitialLevel, and why two tests can want two completely different worlds.
//
//  Anything spawned through this object is remembered and destroyed when the test ends, so leaving a test never
//  leaks a hero, an enemy or a gun into the next one.
//=====================================================================================================================

namespace ETG
{
    class Character;
    class GunBase;
    class Hero;

    namespace Testing
    {
        class TestEnvironment
        {
        public:
            explicit TestEnvironment(GameManager& game) : Game(&game)
            {
            }

            //=========================================================================================================
            //  SPAWNING - build the world
            //=========================================================================================================

            //The general one: spawns any world object exactly the way the game's level does, and files it for
            //cleanup. Everything below is a convenience wrapper around this.
            //
            //  env.Spawn<AK47>(Vector2f{-100.f, 100.f});
            //  env.Spawn<PlatinumBullets>();
            template <typename T, typename... Args>
            T* Spawn(Args&&... args)
            {
                T* object = Game->SpawnGameObject<T>(std::forward<Args>(args)...);
                Spawned.push_back(object);
                return object;
            }

            //The playable character. Call this FIRST in SetUp if the test needs anyone else: enemies capture
            //Hero::Get() in their constructor, and an enemy built without a hero is a null dereference.
            //
            //Tune the hero right after spawning it - that is the point of a per-test world:
            //  Hero* hero = env.SpawnHero();
            //  hero->GetMoveComp()->MaxSpeed = 400.f;        //a faster hero for this test only
            //  hero->HealthComp->CurrentHealth = 99.f;       //...that cannot die while you measure something
            Hero* SpawnHero(const ETG::Vector2f& position = {0.f, 0.f});

            //An enemy of any type, positioned in world coordinates. Throws with a readable message if no hero has
            //been spawned yet, instead of crashing inside the enemy's constructor
            //
            //  env.SpawnEnemy<BulletMan>({60.f, 40.f});
            template <typename TEnemy>
            TEnemy* SpawnEnemy(const ETG::Vector2f& position)
            {
                RequireHero("SpawnEnemy");
                return Spawn<TEnemy>(position);
            }

            //A gun, spawned and put straight into the hero's hands (it becomes CurrentGun). This is how a test says
            //"the mechanic I am testing is about the AK", without walking over to pick it up:
            //
            //  AK47* ak = env.GiveHeroGun<AK47>();
            //  ak->FireRate = 0.05f;                          //...and made stupidly fast, for this test only
            template <typename TGun>
            TGun* GiveHeroGun(const ETG::Vector2f& spawnPosition = {0.f, 0.f})
            {
                RequireHero("GiveHeroGun");
                TGun* gun = Spawn<TGun>(spawnPosition);
                EquipOnHero(gun);
                return gun;
            }

            //=========================================================================================================
            //  QUERYING - read the world back
            //=========================================================================================================

            //The hero this test spawned, or null if it did not spawn one
            [[nodiscard]] Hero* GetHero() const { return HeroPtr; }

            //The gun currently in the hero's hands (null if there is no hero, or it holds nothing)
            [[nodiscard]] GunBase* GetHeroGun() const;

            //Every live object of a type currently in the world - including the ones the test did not spawn itself,
            //which is the point: projectiles are created by the gun, not by the test.
            //
            //  for (ProjectileBase* bullet : env.FindAll<ProjectileBase>()) ...
            //
            //NOTE: the scene registry holds non-owning pointers, so entries are checked against GameClass::IsValid
            //before being handed out - a bullet destroyed earlier this frame never reaches the caller
            template <typename T>
            [[nodiscard]] std::vector<T*> FindAll() const
            {
                std::vector<T*> found;
                for (GameObjectBase* object : Scene::Get()->SceneObjs)
                {
                    if (!GameClass::IsValid(object)) continue;
                    if (T* typed = object->As<T>()) found.push_back(typed);
                }
                return found;
            }

            //The first live object of a type, or null. Handy for "did a bullet appear yet?"
            template <typename T>
            [[nodiscard]] T* FindFirst() const
            {
                for (GameObjectBase* object : Scene::Get()->SceneObjs)
                {
                    if (!GameClass::IsValid(object)) continue;
                    if (T* typed = object->As<T>()) return typed;
                }
                return nullptr;
            }

            //How many live objects of a type exist. "Did the enemy actually die?" is a count going to zero
            template <typename T>
            [[nodiscard]] size_t CountOf() const { return FindAll<T>().size(); }

            //=========================================================================================================
            //  TIME - for anything measured against the clock
            //=========================================================================================================

            //This frame's delta time, in seconds. The same value the whole game moves by
            [[nodiscard]] static float DeltaSeconds();

            //Seconds since this test's SetUp ran. Reset when the test is restarted, so a test can say
            //"three seconds after the world was built, the enemy should have reached me"
            [[nodiscard]] float SecondsSinceSetUp() const { return Elapsed; }

            //=========================================================================================================
            //  Used by the runner
            //=========================================================================================================

            //Marks everything this test spawned for destruction. The objects are actually deallocated by
            //GameManager's sweep at the end of the frame, which is why the runner waits a frame before setting the
            //next test up
            void DestroyEverythingSpawned();

            //Called once per frame by the runner, before the active test's Update
            void AdvanceClock();

            [[nodiscard]] GameManager& GetGame() const { return *Game; }

        private:
            //Guns are equipped through Character, whose header the templates above must not have to include
            void EquipOnHero(GunBase* gun) const;

            //Throws a std::runtime_error naming the caller, so the message says what to fix instead of what crashed
            void RequireHero(const char* caller) const;

            GameManager* Game{nullptr};

            //Non-owning: GameManager owns every world object. This list only exists so cleanup does not have to
            //guess which objects belonged to the test
            std::vector<GameObjectBase*> Spawned;

            Hero* HeroPtr{nullptr};
            float Elapsed{0.f};
        };
    }
}
