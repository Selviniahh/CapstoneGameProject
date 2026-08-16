#pragma once
#include <memory>
#include "TileType.h"
#include "../../Engine/Core/GameObjectBase.h"

namespace ETG
{
    class CollisionComponent;

    //DUNYANIN 16x16'LIK BIR HUCRESI, KENDI BASINA BIR SAHNE NESNESI OLARAK
    //
    //Bilerek "atilacak" yari. Odalar Tiled'dan gelmeye basladiginda harita, bir TileType izgarasi ve collision
    //sorgusuna verilen birkac dikdortgen olacak - her hucre icin bir sahne nesnesi degil; ki bu, herhangi bir
    //boyuttaki oda icin "ben hala duvarim" demek uzere kendini update edip cizen binlerce nesne demektir.
    //
    //ATILMAYACAK olan ise yuzeyin altindaki her sey: CollisionLayer::Obstacle uzerinde duran kati bir dikdortgen,
    //haritanin da vereceginin tam olarak aynisi. Yani bu isin mover tarafi - MoveAndSlide, projeksiyon, duvar
    //boyunca kosmanin hissi - harita verisinin tek bir byte'i bile ortada yokken bitmis ve test edilmis oluyor
    class Block : public GameObjectBase
    {
    public:
        explicit Block(const ETG::Vector2f& position, TileType type = TileType::Block);
        ~Block() override;

        void Draw() override;

        [[nodiscard]] TileType GetTileType() const { return Type; }

        //Bir hucrenin ne oldugunu degistirmek, hem nasil cizildigini hem de birini durdurup durdurmadigini
        //degistirir; bu ikisi asla ayri ayri set edilmemeli - icinden gecilebilen bir duvar, bulmasi bir saat
        //suren cinsten bir bugdur
        void SetTileType(TileType type);

        //Diger her nesnenin ki ile ayni sebepten public: editor uzerinde geziyor, ve Draw ona pencereyi veriyor
        std::unique_ptr<CollisionComponent> CollisionComp;

    private:
        TileType Type{TileType::Block};

        BOOST_DESCRIBE_CLASS(Block, (GameObjectBase), (), (), ())
    };
}
