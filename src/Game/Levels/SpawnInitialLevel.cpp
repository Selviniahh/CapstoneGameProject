#include "SpawnInitialLevel.h"
#include <imgui.h>
#include "../Managers/GameManager.h"
#include "../Characters/Hero/Hero.h"
#include "../Guns/AK-47/AK47.h"
#include "../Guns/SawedOff/SawedOff.h"
#include "../Guns/Magnum/Magnum.h"
#include "../Characters/Enemy/BulletMan/BulletMan.h"
#include "../Items/Active/DoubleShoot.h"
#include "../Items/Active/TakeNoDamage.h"
#include "../Items/Passive/PlatinumBullets.h"
#include "Block.h"

void ETG::SpawnInitialLevel::Spawn(GameManager& game)
{
    game.SpawnGameObject<Hero>(Vector2f{10, 10});

    //Elle konmus uc hucre; sira halinde degil L seklinde, cunku sira, kanitlanmaya deger iki seyden sadece
    //birini kanitlar. Ustteki cift, icine yukari dogru yurunecek YATAY bir yuz veriyor (W, sonra W+A: hero yana
    //giden hizinin tamamini korumali, yukari giden yarisi ise cope gitmeli). Ucuncusu koseyi donup ayni testin
    //eksenleri degistirilmis hali icin DIKEY bir yuz veriyor. Bulustuklari yer ise ic kose - iki eksenin ayni
    //anda bloklandigi durum, ki calisan bir cozumu titreyen bir cozumden ayiran vaka da budur.
    //
    //Pozisyonlar hucre MERKEZLERI ve hepsi 16'nin tam kati; harita bu uc satir yerine gercek veri oldugunda bir
    //grid index'inin TileSize ile carpimi da tam olarak bunlari uretecek
    game.SpawnGameObject<Block>(Vector2f{0, -32});
    game.SpawnGameObject<Block>(Vector2f{16, -32});
    game.SpawnGameObject<Block>(Vector2f{16, -16});
    game.SpawnGameObject<AK47>(Vector2f{-100, 100});
    game.SpawnGameObject<SawedOff>(Vector2f{-150, 100});
    game.SpawnGameObject<Magnum>(Vector2f{-200, 100});
    game.SpawnGameObject<BulletMan>(Vector2f{50, 50});
    game.SpawnGameObject<PlatinumBullets>();
    game.SpawnGameObject<DoubleShoot>();
    game.SpawnGameObject<TakeNoDamage>();

    //Game-specific editor widgets on the Scene panel (the engine's Scene only exposes the hook)
    Scene::Get()->PopulateGameWidgets = [&game]
    {
        static float spawnPos[2] = {0.f, 0.f};
        ImGui::InputFloat2("Spawn Pos", spawnPos);

        if (ImGui::Button("Spawn BulletMan"))
        {
            game.SpawnGameObject<BulletMan>(Vector2f{spawnPos[0], spawnPos[1]});
        }

        // Display count of active enemies
        int enemyCount = 0;
        for (const auto* obj : Scene::Get()->SceneObjs)
        {
            if (GameClass::IsValid(obj) && dynamic_cast<const BulletMan*>(obj))
                enemyCount++;
        }
        ImGui::Text("Active enemies: %d", enemyCount);
    };
}
