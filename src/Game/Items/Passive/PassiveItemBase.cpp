#include "PassiveItemBase.h"
#include "../../Guns/Base/GunBase.h"

namespace ETG
{
    //Every modifier this item attached is filed under its source name, so undoing the item does not require the item
    //to have kept a list of which stats it touched. An item that overrides ApplyGunPerk normally needs no override here
    void PassiveItemBase::RemoveGunPerk(GunBase& gun)
    {
        gun.RemoveAllModifiersFrom(ModifierSource);
    }
}
