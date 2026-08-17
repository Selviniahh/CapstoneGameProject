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
#include "TileType.h"

namespace
{
    //ODANIN KENDISI, TEK BIR YERDE, OKUNABILIR HALDE
    //
    //Oda 128x128 dunya birimi; TileSize 16 oldugu icin bu tam olarak 8x8 hucre eder. Boyutu burada sabit sayi
    //olarak degil bolme olarak yaziyoruz: TileSize degisirse odanin metrik boyutu ayni kalir, izgara sıklığı
    //degisir - ki iki sayidan hangisinin "gercek" oldugu sorusunun cevabi da budur
    constexpr float RoomSize = 128.f;
    constexpr int RoomTiles = static_cast<int>(RoomSize / ETG::TileSize); //8

    //Odayi orijine ortaliyoruz: -64..+64. Hero {10,10}'da spawn oluyor ve bu, icerideki bir zemin hucresine
    //dusuyor - yani oyun ilk frame'de zaten odanin icinde basliyor
    constexpr float RoomLeft = -RoomSize / 2.f;
    constexpr float RoomTop = -RoomSize / 2.f;

    //Odanin yerlesim plani. Kod olarak degil RESIM olarak yaziliyor; cunku "cevresi duvar, icinde uc cukur"
    //cumlesi tam olarak boyle GORUNUYOR ve bir dongu ile uretilmis hali, plani okumak icin kafadan calistirmayi
    //gerektirirdi. Tiled'dan gelecek veri de zaten bunun aynisi: bir karakter izgarasi ve onun ne demek oldugunu
    //soyleyen bir tablo (asagidaki ToTileType)
    //
    //  # = duvar   . = zemin   F = cukur
    constexpr const char* RoomPlan[RoomTiles] = {
        "########",
        "#......#",
        "#.FF...#",
        "#......#",
        "#......#",
        "#....F.#",
        "#......#",
        "########",
    };

    //Plandaki karakterin ne oldugu. Tanimsiz bir karakter Default'a dusuyor - ki o da mor cizilip kati davranir,
    //yani plandaki bir yazim hatasi sessizce zemine donusmek yerine ekranda bagirir
    constexpr ETG::TileType ToTileType(const char glyph)
    {
        switch (glyph)
        {
        case '#': return ETG::TileType::Block;
        case '.': return ETG::TileType::Nothing;
        case 'F': return ETG::TileType::Fall;
        default: return ETG::TileType::Default;
        }
    }

    void SpawnRoom(ETG::GameManager& game)
    {
        for (int row = 0; row < RoomTiles; ++row)
        {
            for (int col = 0; col < RoomTiles; ++col)
            {
                //Block'un Position'i hucrenin KOSESI degil MERKEZI (bkz. Block.cpp'deki Origin {0.5,0.5}), o yuzden
                //index * TileSize'in uzerine yarim hucre ekleniyor. Bu yarim hucre unutulursa oda butun olarak
                //8 birim sol ust'e kayar ve collision kutulari cizimle ayni yerde durmaz
                const ETG::Vector2f center{
                    RoomLeft + static_cast<float>(col) * ETG::TileSize + (ETG::TileSize / 2.f), //sol üst değil merkez olması için 
                    RoomTop + static_cast<float>(row) * ETG::TileSize + (ETG::TileSize / 2.f)
                };

                //Tip constructor'a veriliyor; renk oradan geliyor. Block, Color'i TileDebugColor(Type) ile set
                //ediyor - yani hucre basina renk ayrica yazilmiyor, TIPTEN TUREYOR. Bir hucrenin rengi ile
                //katiligi bu sayede asla birbirinden ayrilamiyor
                game.SpawnGameObject<ETG::Block>(center, ToTileType(RoomPlan[row][col]));
            }
        }
    }
}

void ETG::SpawnInitialLevel::Spawn(GameManager& game)
{
    game.SpawnGameObject<Hero>(Vector2f{10, 10});

    //Onceki elle konmus uc hucrenin yerini aliyor: odanin duvarlari, o uc blogun kanitladigi her seyi zaten
    //iceriyor - yatay yuz, dikey yuz ve ikisinin bulustugu ic kose (kose hucreleri), ustelik dort yonun hepsinde
    SpawnRoom(game);

    //Hepsi odanin ICINE, zemin hucrelerinin merkezlerine tasindi. Eski yerleri artik duvarin disi (silahlar) ya da
    //duvarin kendisi (BulletMan {50,50}, ki o hucre sag duvarin 48..64 araligina dusuyor): oda cevrelendigi anda
    //"su koordinat" demek yetmiyor, koordinatin hangi hucre oldugu onemli hale geliyor
    game.SpawnGameObject<AK47>(Vector2f{-40, 40});
    game.SpawnGameObject<SawedOff>(Vector2f{-24, 40});
    game.SpawnGameObject<Magnum>(Vector2f{-8, 40});
    game.SpawnGameObject<BulletMan>(Vector2f{40, -40});
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
