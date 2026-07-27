#pragma once

namespace ETG
{
    //Type tag for the hero modifier family. There is deliberately nothing to implement: its only job is to give
    //ModifierManager a common base to store, and to make ModifierManager<IHeroModifier> reject a gun modifier at
    //compile time. A modifier's identity is its concrete type, so no name or id is needed here
    
    //Turkce yazacak olursam, Farklı farklı bir ton modifiler bir ton silah modifier, hero modifier'ımız olacak değil mi? Bir tür modifier'ı diğerinden compile time ayırmak istiyorum 
    //Bunun en kolay yolu direkt, interface mantığıyla modifierlar eklemek. Bu class direk bomboş olacak 
    //Game/Modifiers/Hero  içerisindeki Hero modifierları illa bu class'ı inherit etmek ZORUNDA OLACAK
    //Daha sonra hero içerisine 
    //                             ModifierManager<IHeroModifier> HeroModifierManager;
    //Bir modifier vermem gerektiğinde paşa paşa zorla "Game/Modifiers/Hero" vereceğim. Basit mantık olmasa'da olur ama olması bir tık daha profosyonel yapıyor  
    class Hero;
    class ProjectileBase;

    class IHeroModifier
    {
    public:
        virtual ~IHeroModifier() = default;

        //Fired for every hit that is about to land on the hero. `projectile` is null for contact damage (walking
        //into an enemy) and set for a shot, so a modifier that only cares about bullets can bail on null.
        //Returning true CONSUMES the hit: the hero takes no damage and no further modifier is asked about it.
        //NOTE: pure, so this header stays a contract with no code in it. The price is that a modifier with
        //nothing to do with damage still has to write `return false;` - one line, in exchange for Hero never
        //learning a single concrete modifier name
        virtual bool ReflectProjectile(Hero& hero, ProjectileBase* projectile) = 0;
    };
}
