> Bir şeyi Stat yapan, sonsuza kadar sürmesi değil; sayısal bir değeri değiştirmesi.

### Stat kullanılacak şeyler

Bir sayı artıyor veya azalıyorsa:

- %20 daha hızlı ateş → FireRate
- +1 maksimum can → MaxHealth
- 2x hareket hızı → MaxSpeed
- +2 hasar → Damage
- %15 sekme ihtimali → RicochetChance
- +1 sekme sayısı → BounceCount

Bunların pasif veya geçici olması fark etmez.

Pasif item:

hero->MoveComp->MaxSpeed.AddModifier(
"SpeedBoots",
StatOp::Percent,
0.20f
);


Önce problem

Silahın FireRate = 0.4 (atışlar arası saniye, küçük = hızlı).

Oyuncu PlatinumBullets alıyor: "%20 daha hızlı ateş".

Eski kod:
gun->FireRate = gun->BaseFireRate - %20;   // 0.4 -> 0.32

Neden BaseFireRate diye ikinci bir alan var? Çünkü FireRate'e yazdığın an 0.4 sonsuza dek kayboldu. Geri dönmek için bir yerde saklaman lazım. Tamam neyse çalışıyor.

Şimdi oyuncu ikinci bir item alıyor — BulletTime, "%50 daha hızlı":
gun->FireRate = gun->BaseFireRate - %50;   // 0.4 -> 0.20

PlatinumBullets buhar oldu. Doğru sonuç 0.4 × 0.8 × 0.5 = 0.16 olmalıydı, 0.20 çıktı. Son yazan kazandı.

Daha kötüsü: oyuncu BulletTime'ı düşürsün. FireRate'e ne yazacaksın? 0.4 yazarsan PlatinumBullets de gider. 0.32 yazacağını nereden bileceksin — o bilgi hiçbir yerde yok.

Kök sebep: değişken sadece sonucu tutuyordu. Sonuç sana o sonuca kimlerin katkı yaptığını söylemez.

Market fişi gibi düşün: elinde sadece "toplam 340 TL" yazıyorsa, içinden ekmeği iade edemezsin. Kalemler duruyorsa edebilirsin.

Stat ne yapıyor

Sonucu saklamıyor. Listeyi saklıyor, sonucu okurken hesaplıyor.

FireRate
Taban: 0.4
Liste:
"PlatinumBullets"   Percent  -0.20
"BulletTime"        Percent  -0.50

gun->FireRate okuduğunda:
0.4 × (1 - 0.20) × (1 - 0.50) = 0.16   ✔

BulletTime düşünce, o isimdeki satırı silersin:
gun->FireRate.RemoveModifiersFrom("BulletTime");
0.4 × 0.8 = 0.32   ✔  otomatik doğru

Hepsi bu. Sihir yok. BaseFireRate alanına da gerek kalmadı çünkü taban zaten listenin içinde duruyor.

Source (isim) neden var: silme anahtarı o. "%50'lik olanı sil" diyemezsin — hangisi? "BulletTime'ın koyduğunu sil" diyorsun.

Formül neden (Taban + Σ Flat) × Π (1+Percent): iki tür item var. "+2 hasar" (Flat) ve "+%20 hasar" (Percent). İkisi de lazım.

Yüzdeler neden çarpılıyor, toplanmıyor: çarpma sırası önemsizdir, o yüzden ortadan birini çıkarmak diğerlerini bozmaz. Toplasaydım (1 + 0.2 + 0.5) tek tek geri alamazdım. Test bunu bit düzeyinde doğruluyor.

IGunModifier ne yapıyor

Stat bir sayıyı değiştirir. Peki bunu nasıl sayıyla ifade edersin:

▎ "Mermiler duvardan sekiyor."

Silahta "sekme miktarı" diye bir alan yok. Yeni bir sayı değil — çalışması gereken yeni kod lazım. Mermi duvara çarptığında normalde yok oluyor; artık yön değiştirmesi gerekiyor.

İşte IGunModifier bunun için: sayı değil, davranış.


Silahta "sekme miktarı" diye bir alan yok. Yeni bir sayı değil — çalışması gereken yeni kod lazım. Mermi duvara çarptığında normalde yok oluyor; artık yön değiştirmesi gerekiyor.

İşte IGunModifier bunun için: sayı değil, davranış.

Ayrım tek cümle:

▎ Stat = var olan bir sayı farklı çıkıyor. Okuma yerinde hiç kod eklemezsin.
▎ IGunModifier = yeni kod çalışıyor. Bir yerde if (bu modifier varsa) { ... } eklemek zorundasın.

GunBase::PrepareShooting'de bunu görüyorsun:
if (const auto& multiMod = modifierManager.GetModifier<MultiShotModifier>())
shotCount = multiMod->GetShotCount();
Bu if orada olmak zorunda. Stat'ta böyle bir şey yok — Timer >= FireRate satırı değişmiyor.

Ama dürüst olayım: MultiShotModifier kötü bir örnek

EnqueueProjectiles(shotCount, spread) zaten mermi sayısını parametre alıyor. Yani ShotCount pekâlâ Stat ShotCount = 1 olabilirdi ve MultiShotModifier tamamen silinebilirdi. Sınırda bir vaka — bu yüzden kafan karışmış olabilir, haklısın.

IGunModifier'ın gerçekten gerektiği örnekler şunlar:

- mermiler duvardan seker
- her 10. mermi patlayıcı
- şarjör bittiğinde etraftaki düşmanlar hasar alır
- mermiler en yakın düşmana kilitlenir

Bunların hiçbiri "şu sayıyı şu kadar değiştir" değil.

Yan yana

┌───────────────────────────┬─────────────────────────────────────┬───────────────────────────────┐
│                           │                Stat                 │         IGu
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Değiştirdiği              │ bir sayı                            │ ne olduğu                     │
├───────────────────────────┼─────────────────────────────────────┼────────────
│ Örnek                     │ +%20 ateş hızı, +2 hasar, +1 şarjör │ mermi seker, patlayıcı mermi  │
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Üst üste binebilir mi     │ evet, sınırsız                      │ hayır, isim başına bir tane   │
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Tek tek kaldırılabilir mi │ evet, matematiksel olarak kesin     │ evet ama sadece varlık olarak │
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Yeni item eklerken        │ kod eklemezsin, sadece kayıt        │ kullanım ye
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Sende kaç tane olacak     │ yüzlerce                            │ onlarca
└───────────────────────────┴─────────────────────────────────────┴───────────────────────────────┘

Gungeon'daki itemlerin çoğunluğu (tahminen %70-80'i) Stat tarafında. IGunModifier tarafı azınlık ama var olmak zorunda.

Ve şu an ikisi de "modifier" adını taşıdığı için hangisini ne zaman kullanacağıbu, o yüzden birine Behaviour demeyi önerdim.

Şimdi oyuncu ikinci bir item alıyor — BulletTime, "%50 daha hızlı":
gun->FireRate = gun->BaseFireRate - %50;   // 0.4 -> 0.20


Kök sebep: değişken sadece sonucu tutuyordu. Sonuç sana o sonuca kimlerin katkı yaptığını söylemez.

Market fişi gibi düşün: elinde sadece "toplam 340 TL" yazıyorsa, içinden ekmeği iade edemezsin. Kalemler duruyorsa edebilirsin.

Stat ne yapıyor

Sonucu saklamıyor. Listeyi saklıyor, sonucu okurken hesaplıyor.

FireRate
Taban: 0.4
Liste:

0.4 × (1 - 0.20) × (1 - 0.50) = 0.16   ✔

BulletTime düşünce, o isimdeki satırı silersin:
gun->FireRate.RemoveModifiersFrom("BulletTime");
0.4 × 0.8 = 0.32   ✔  otomatik doğru

Hepsi bu. Sihir yok. BaseFireRate alanına da gerek kalmadı çünkü taban zaten li

Source (isim) neden var: silme anahtarı o. "%50'lik olanı sil" diyemezsin — hangisi? "BulletTime'ın koyduğunu sil" diyorsun.

Formül neden (Taban + Σ Flat) × Π (1+Percent): iki tür item var. "+2 hasar" (Fl. İkisi de lazım.

IGunModifier ne yapıyor

Stat bir sayıyı değiştirir. Peki bunu nasıl sayıyla ifade edersin:

▎ "Mermiler duvardan sekiyor."

Silahta "sekme miktarı" diye bir alan yok. Yeni bir sayı değil — çalışması gereara çarptığında normalde yok oluyor; artık yön değiştirmesi gerekiyor.

İşte IGunModifier bunun için: sayı değil, davranış.

Ayrım tek cümle:

▎ IGunModifier = yeni kod çalışıyor. Bir yerde if (bu modifier varsa) { ... } eklemek zorundasın.

GunBase::PrepareShooting'de bunu görüyorsun:
if (const auto& multiMod = modifierManager.GetModifier<MultiShotModifier>())
shotCount = multiMod->GetShotCount();
Bu if orada olmak zorunda. Stat'ta böyle bir şey yok — Timer >= FireRate satırı, üzerinde 5 item olsa da hiç değişmiyor.

Ama dürüst olayım: MultiShotModifier kötü bir örnek

EnqueueProjectiles(shotCount, spread) zaten mermi sayısını parametre alıyor. YaCount = 1 olabilirdi ve MultiShotModifier tamamen silinebilirdi. Sınırda birvaka — bu yüzden kafan karışmış olabilir, haklısın.

IGunModifier'ın gerçekten gerektiği örnekler şunlar:

- mermiler duvardan seker
- her 10. mermi patlayıcı
- şarjör bittiğinde etraftaki düşmanlar hasar alır
- mermiler en yakın düşmana kilitlenir

Bunların hiçbiri "şu sayıyı şu kadar değiştir" değil.

Yan yana

┌───────────────────────────┬─────────────────────────────────────┬────────────
│                           │                Stat                 │         IGunModifier          │
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Değiştirdiği              │ bir sayı                            │ ne olduğu                     │
MinorNameChange├───────────────────────────┼─────────────────────────────────────┼────────────
│ Örnek                     │ +%20 ateş hızı, +2 hasar, +1 şarjör │ mermi seker
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Üst üste binebilir mi     │ evet, sınırsız                      │ hayır, isim
├───────────────────────────┼─────────────────────────────────────┼───────────────────────────────┤
│ Tek tek kaldırılabilir mi │ evet, matematiksel olarak kesin     │ evet ama sa
├───────────────────────────┼─────────────────────────────────────┼────────────
│ Yeni item eklerken        │ kod eklemezsin, sadece kayıt        │ kullanım ye
├───────────────────────────┼─────────────────────────────────────┼────────────
│ Sende kaç tane olacak     │ yüzlerce                            │ onlarca
└───────────────────────────┴─────────────────────────────────────┴────────────

Gungeon'daki itemlerin çoğunluğu (tahminen %70-80'i) Stat tarafında. IGunModifiak zorunda.

Ve şu an ikisi de "modifier" adını taşıdığı için hangisini ne zaman kullanacağıbu, o yüzden birine Behaviour demeyi önerdim.