// Problem: Hero, Scene ve Engine gibi tek aktif örneği olan bazı sınıflara
// farklı yerlerden kolayca erişmek istiyoruz.
//
// GameObjectBase'e static Self koyarsak bütün türetilmiş sınıflar aynı pointer'ı
// paylaşır ve son oluşturulan GameObject önceki pointer'ın üstüne yazar.
//
// Her tekil sınıfa ayrı ayrı static pointer ve GetSelf() yazmak da tekrarlı olur.
//
// Çözüm: SingleInstance<T>, her T sınıf türü için ayrı bir static Instance üretir.
// Nesne oluşturulunca pointer otomatik atanır, yok edilince temizlenir.
//
// Not: Bu yapı yalnızca aynı anda tek örneği bulunması gereken sınıflarda kullanılmalıdır.
//Aynı projectile class'ı birden fazla spawn edilirse bu çalışmaz 

// Cozum: SingleInstance<T>, her T tipi icin ayri bir static Instance olusturur. Nesne
// olusturulunca kendisini kaydeder, yok edilince kaydi temizler; T::GetSelf() bu nesneyi verir.
// Not: Ikinci bir nesnenin olusturulmasini engellemez; ayni anda tek ornek olacagini varsayar.

#pragma once
namespace ETG
{
    //CRTP mixin for classes that only ever have one live instance (the hero, the active scene, the editor...).
    //Deriving from SingleInstance<T> gives T its OWN static instance pointer (each T instantiation is a
    //separate static), set on construction, reachable from anywhere as T::GetSelf().
    //NOTE: A static member directly in a shared base wouldn't work: it would be one variable shared by
    //every derived class, overwritten by whichever object was constructed last.
    
    //NOTE: Provides getter for every single object
    template <typename T>
    class SingleInstance
    {
    public:
        [[nodiscard]] static T* Get() { return Instance; }

    protected:
        SingleInstance() { Instance = static_cast<T*>(this); }
        ~SingleInstance() { if (Instance == this) Instance = nullptr; }

    private:
        inline static T* Instance = nullptr;
    };
}
