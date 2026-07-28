HeroRoot                                      composite
│  Grants: -
│  Revokes: -
│
├── Alive                                     composite
│   │  Grants: CanTakeDamage
│   │  Revokes: -
│   │
│   ├── NormalMovement                        composite
│   │   │  Grants:
│   │   │    CanMove
│   │   │    CanShoot
│   │   │    CanSwitchGuns
│   │   │    CanUseActiveItems
│   │   │    CanFlipAnims
│   │   │  Revokes: -
│   │   │
│   │   ├── Idle                              leaf
│   │   │      Grants: -
│   │   │      Revokes: -
│   │   │      Effective: All
│   │   │
│   │   └── Run                               leaf
│   │          Grants: -
│   │          Revokes: -
│   │          Effective: All
│   │
│   ├── Dash                                  leaf
│   │      Grants: CanFlipAnims
│   │      Revokes: CanTakeDamage
│   │      Effective: CanFlipAnims
│   │
│   └── Hit                                   leaf
│          Grants: -
│          Revokes:
│            CanTakeDamage
│            CanFlipAnims
│          Effective: None
│
└── Dead                                      composite
│  Grants: -
│  Revokes: All
│
└── Die                                   leaf
Grants: -
Revokes: -
Effective: None


Locomotion şu yetenekleri veriyor:

LocomotionNode->Grants =
Cap::CanMove |
Cap::CanShoot |
Cap::CanSwitchGuns |
Cap::CanUseActiveItems |
Cap::CanFlipAnims;

Locomotion şu yetenekleri veriyor:

LocomotionNode->Grants =
Cap::CanMove |
Cap::CanShoot |
Cap::CanSwitchGuns |
Cap::CanUseActiveItems |
Cap::CanFlipAnims;

Eğer Dash, Locomotionun çocuğu olsaydı aktif yol şöyle olurdu:

Root → Alive → Locomotion → Dash

Bu durumda Dash:

- CanMove alırdı.
- CanShoot alırdı.
- Silah değiştirebilirdi.
- Aktif item kullanabilirdi.
- LocomotionNode->OnTick() çalışırdı.
- Ardından DashNode->OnTick() da çalışırdı.

Yani aynı frame’de iki hareket sistemi çalışabilirdi:

hero.MoveComp->UpdateMovement();              // Locomotion
hero.MoveComp->MakeDashMovement(TimeInState()); // Dash

Bunlar birbiriyle çatışabilir. Bu nedenle Dash, Locomotionun çocuğu değil, kardeşi:

Alive
├── Locomotion
│   ├── Idle
│   └── Run
├── Dash
└── Hit


Hit için durum daha net. Hit sırasında karakter normal oyuncu kontrolünde olmamalı:

- Hareket etmemeli.
- Ateş etmemeli.
- Normal movement update çalışmamalı.
- Yalnızca knockback/hit davranışı uygulanmalı.

Bu nedenle Hit de Locomotion dışında.

birseyler birinin child'i oluyorsa otomatik olarak parent'in en basinda koydugu
kurallara otomatik olarak uyacak

Örneğin Run aktifken:

HeroRoot → Alive → Locomotion → Run

Dolayısıyla Run, Alive ve Locomotion üzerine konmuş ortak kurallardan etkilenir.

Otomatik uygulananlar:

- Parent’ın Grants capability’leri.
- Parent’ın Revokes capability’leri.
- Parent’ın transition’ları.
- Parent’ın OnTick fonksiyonu.
  Örneğin Run, hiçbir capability tanımlamıyor:

  RunNode->Grants = Cap::None;

  Ama parent’ı Locomotion verdiği için şunlara sahip:

  CanMove
  CanShoot
  CanSwitchGuns
  CanUseActiveItems
  CanFlipAnims

Ayrıca Alivedan şunu alıyor:
CanTakeDamage

Bu yüzden Runın etkili capability sonucu All oluyor.

Parent transition’ları da geçerlidir. Alive üzerinde:

AliveNode->AddTransition(DeadNode, isDead);
AliveNode->AddTransition(HitNode, hitRequested);
AliveNode->AddTransition(DashNode, dashRequested);

tanımlandığı için karakter Idle veya Rundayken bunların hepsi kontrol edilir.

----------------------------------------------------------------------------------------------------------------

//3 farkli kural var:
//Capabilities → Karakter ne yapabilir?
// Transitions  → Ne zaman başka state'e geçer?
// Actions      → State aktifken ne yapılır?