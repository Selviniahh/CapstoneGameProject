# BulletMan — CarryBody

A BulletMan that spots a dead BulletMan runs over, hoists the corpse onto its head,
carries it to the player and throws it.

Every pixel here comes from the existing BulletMan sprites — the idle body, the run
feet, `bullet_hand_001.png`, and `Death/bullet_death_left_side_004.png` as the corpse.
Nothing was scaled, rotated or recoloured, so the whole set stays on the original
10-colour palette with hard (0/255) alpha.

## Frames

| Animation | Directions | Frames |
|---|---|---|
| `bullet_pickup_<dir>` | right, left | 8 |
| `bullet_carry_<dir>` | right, left, right_back, left_back | 6 |
| `bullet_carry_idle_<dir>` | right, left, right_back, left_back | 2 |
| `bullet_throw_<dir>` | right, left | 8 |

All numbering starts at `001`. The `_back` variants have the face removed from both
the carrier **and** the corpse, matching how every stock `*_back` sprite is drawn.

## Canvas / alignment

All 64 frames are **34×57**, with the stock 12×23 body centred on the canvas.

That size is deliberate. `BaseAnimComp::AddAnimationsForState` takes the origin from
the **first** frame (`width/2, height/2`) and reuses it for every frame, and
`CreateSpriteSheet` stitches frames top-aligned. Centring the stock body means the
origin lands exactly where the stock sprites' origin does, so a BulletMan switching
between Idle/Run and these animations does not jump. And because every frame is the
same size, the origin never drifts between frames — the stock sheets do drift a
little, since their frames vary in size.

No engine change is needed; `Animation::CreateSpriteSheet("Enemy/BulletMan/CarryBody",
"bullet_carry_right_001", "png", 0.12f)` picks the set up as-is.

## Hooking it up

Suggested state flow:

```
Run  ->  PickUp  ->  Carry (walk toward the player)  ->  Throw  ->  Idle
```

Two things the game side has to do:

1. **Pick-up** frames already draw the corpse lying on the floor, so hide/despawn the
   real corpse entity when the pick-up starts. Its last frame is pixel-identical to
   the carry pose, so `PickUp -> Carry` joins without a pop.
2. **Throw** releases on frame 5. From frame 6 the thrower is empty-handed, so spawn
   the flying corpse projectile on that transition.

For the projectile itself you do not need new art —
`Death/bullet_death_left_side_004.png` spun via the `rotation` argument that
`Animation::Draw` already takes reads correctly as a tumbling body.

Suggested frame intervals: pick-up `0.09f`, carry `0.12f` (same as Run), carry-idle
`0.15f` (same as Idle), throw `0.07f`.
