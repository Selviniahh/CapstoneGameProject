#pragma once
#include <cstdint>
#include "../../Engine/Platform/Color.h"

namespace ETG
{
    //HARITANIN BIR HUCRESI NEDIR
    //
    //Isimler, hucrenin uzerine basanA NE YAPTIGINA gore verildi; neye BENZEDIGINE gore degil. Bu bir stil tercihi
    //degil: bunlari okuyan tek sey hareket kodu, ve onun uzerine islem yapabilecegi tek cevap kumesi "duvar",
    //"cukur" ve "zemin". Gorsel, hucreyi cizen seyin isi; tamamen farkli iki tileset'ten cizilmis iki hucre,
    //ikisinden de seken varsa ayni TileType'tir.
    //
    //NOTE: haritalar Tiled'dan gelmeye basladiginda oda bu sekli koruyacak. Tiled bir tam sayi izgarasi ve o
    //sayilarin yazarin ne kastettigine karsilik geldigi bir tablo export ediyor; o tablo bu enum'a baglaniyor ve
    //asagisindaki hicbir seyin degismesi gerekmiyor. Enum'u export'cu daha yokken yazmanin sebebi de bu -
    //leveldeki elle konmus bloklar, harita verisi geldiginde konusulacak dili simdiden konusuyor
    enum class TileType : std::uint8_t
    {
        //Buraya henuz hicbir sey yazilmamis. Kati kabul ediliyor, ve bilerek: yazilmamis bir hucre neredeyse her
        //zaman odanin disidir, ve haritanin kenarinda durdurulmak; zemini, duvari ve geri donus yolu olmayan bir
        //dunyaya yurumekten cok daha ucuz bir hatadir
        Default = 0,

        //Zemin. Uzerinden dumduz yurunur - burada gercekten "hicbir sey" var
        Nothing,

        //Kati. Govde yuzunde durur, aciyla geldiyse de yuzey boyunca kayar
        Block,

        //Cukur. Gecirgen, cunku icine YUREBILMEN gerekiyor - dusmek zaten etkilesimin kendisi, ve yazilmasi
        //gereken sey dusme; deligin etrafina duvar ormek degil.
        //NOTE: simdilik sadece gecirgen. Henuz hicbir sey dusmuyor
        Fall,
    };

    //Oyundaki her hucre bir kenari bu kadar dunya birimi. Bir odanin eni ve boyu bunun tam kati olmak zorunda;
    //aksi halde son sutun, hicbir index'in adresleyemeyecegi bir tile parcasi olurdu
    constexpr float TileSize = 16.f;

    //Bu hucre bir govdeyi durdurur mu. Buna karar veren TEK yer burasi - tile'in tasidigi collider buradan acilip
    //kapaniyor, boylece bir tile "yurunurken kati, bakarken gecirgen" olamiyor
    constexpr bool IsTileSolid(const TileType type)
    {
        return type == TileType::Block || type == TileType::Default;
    }

    //Kategori basina tek renk; boylece bir odaya bakinca kimsenin hucreye tiklayip "bu ne" diye sormasina gerek
    //kalmiyor. Sadece debug icin - texture'siz gecici tile'lar bu renklerle ciziliyor
    inline ETG::Color TileDebugColor(const TileType type)
    {
        switch (type)
        {
        case TileType::Block: return {90, 95, 110}; //tas grisi, ve seni durduran tek renk
        case TileType::Fall: return {25, 20, 35}; //siyaha yakin: delik, zeminin YOKLUGU olarak okunuyor
        case TileType::Nothing: return {70, 60, 55}; //mat kahve zemin, uzerinden gecilecek kadar sessiz
        case TileType::Default: return {200, 60, 200}; //eksik texture morartisi - yazilmamis, ve bunu bagira bagira soyluyor
        }

        return ETG::Color::White;
    }
}
