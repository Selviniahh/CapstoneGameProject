# Hierarchical State Machine

*Türkçesi: [StateMachine.tr.md](StateMachine.tr.md)*

How the hero's states work after the refactor, why it was done this way, and what to do when you want the
enemies on the same system.

---

## 1. What was wrong

The hero's state used to be one field, `HeroStateEnum CurrentHeroState`, plus a `SetState()` that anyone could
call. Six places called it: two health listeners in `Hero.cpp`, three branches in `HeroMoveComp::UpdateMovement`,
and two spots in `HeroAnimComp::Update`. Whoever ran last that frame won.

Because nothing owned the state, priority had to be faked with defensive checks. `HeroMoveComp::UpdateMovement`
repeated `GetState() != Die && GetState() != Hit` in **all three** of its branches, and the same guard appeared
again in `IsDashAvailable()` and in the damage listener. Adding one state meant finding and updating every one
of those spots.

The animation component had drifted into being the gameplay authority. `HeroAnimComp` held `IsDashing`,
`DashTimer` and `MinDashDuration`; `StartDash()` set the hero's state, `EndDash()` started the move component's
cooldown, and `Update()` decided when a hit ended. Dash had three separate sources of truth at once
(`HeroStateEnum::Dash`, `HeroAnimComp::IsDashing`, `HeroMoveComp::DashTimer`), which is why
`HeroMoveComp.cpp` contained the line `HeroPtr->MoveComp->HeroPtr->AnimationComp->IsDashing`.

`HeroStateFlags` was a hierarchy already, just written out by hand as bitmasks. `CanShoot = StateIdle|StateRun`
is a long way of saying "Idle and Run are the same kind of state". Every rule had to be spelled out twice, once
as `Prevent*` and once as `Can*`.

### Two real bugs found on the way

**The animation restart never ran.** `BaseAnimComp::Update` assigned `CurrentAnimStateKey = animKey` and *then*
called `ChangeAnimStateIfRequired(animKey)`, which compares `newKey != CurrentAnimStateKey`. That comparison was
always false, so the restart path was dead code. This is why `StartDash` and the hit handler had to call
`Restart()` by hand. Fixed by calling `ChangeAnimStateIfRequired` first and letting it own the assignment.

**The flag operators were unconstrained.** `template<typename T> T operator|(T, T)` in namespace `ETG` was an
overload candidate for *every* type in that namespace via ADL. Now constrained with `requires std::is_enum_v<T>`
and moved to `Managers/Enum/FlagOperators.h` so both flag headers share one definition.

---

## 2. The shape

```
HeroRoot
├── Alive                    grants CanTakeDamage
│   ├── Locomotion           grants CanMove | CanShoot | CanSwitchGuns | CanUseActiveItems | CanFlipAnims
│   │   ├── Idle   (default)
│   │   └── Run
│   ├── Dash                 grants CanFlipAnims, revokes CanTakeDamage
│   └── Hit                  revokes CanTakeDamage | CanFlipAnims
└── Dead                     revokes everything, declares no outgoing transitions
    └── Die
```

Files:

| File | Role |
|---|---|
| `Engine/Core/StateMachine/StateNode.h` | One node: guards, enter/exit/tick hooks, grants/revokes, transitions |
| `Engine/Core/StateMachine/HierarchicalStateMachine.h` | Generic machine. Depends only on `std` and `EventDelegate.h` |
| `Game/Characters/HeroStateMachine.h/.cpp` | The hero's tree. Every transition the hero can make is in `Build()` |
| `Game/Managers/Enum/HeroCapability.h` | The permission bits, replacing `HeroStateFlags` |
| `Game/Managers/Enum/FlagOperators.h` | Constrained bitwise helpers shared by the flag enums |
| `Game/Characters/HeroStates.h` | The hero's enums: `HeroStateEnum` plus its animation keys. Only the hero includes it |
| `Game/Characters/HeroDirections.h/.cpp` | Facing → the hero's animation keys, and the dash keys |

**The enums stayed.** Leaf nodes carry a `HeroStateEnum`, so `AnimManagerDict`, the `AnimationKey` variant and
the boost::describe editor UI are all untouched. Switching to polymorphic state classes would have meant
rewriting all three for no gain.

---

## 3. How it works

### Two rules

1. **Transitions are evaluated root → leaf.** An interrupt declared on `Alive` is checked before `Idle → Run`
   deep inside `Locomotion`. That is where priority comes from: no ordering table, no `!= Die` checks. Within a
   single node, declaration order decides — on `Alive` that is Dead, then Hit, then Dash.
2. **A node with no outgoing transitions is terminal.** `Dead` declares none, so resurrection is not *blocked*,
   it is unreachable. Nothing can put the hero back on his feet, however many requests pile up.

### Capabilities

`HasCapability()` walks the active path, unions every `Grants`, unions every `Revokes`, and lets revokes win.
`Idle` and `Run` declare nothing at all — they inherit the whole on-foot set from `Locomotion`. `Dash` sits
outside `Locomotion`, so it never gets `CanMove`; it grants `CanFlipAnims` back on its own and revokes the
`CanTakeDamage` its parent handed out.

`Hero::CanMove()` and friends still have exactly the signatures they always had. Every caller is unchanged.

### Requests, not commands

Nothing outside the machine assigns a state. Input and listeners file a one-shot intent:

```cpp
hero.RequestDash(HeroDirections::GetDashEnum());            // InputComponent
hero.RequestHit(knockbackDir, forceMagnitude);             // the damage listener
```

A guard reads the flag, and the target node's `OnEnter` consumes it. `InputComponent` no longer needs to know
whether the hero is already dashing, dead or mid-hit — the machine either has a legal transition right now or
it doesn't.

**A request lives for exactly one tick.** `Hero::ExpireRequests()` runs immediately after `Tick()` and drops
whatever nobody acted on. Input re-files `RequestDash` on every frame the button is held, so holding it still
chains dashes as it always did — the cooldown is what paces them. What expiry prevents is a request that had no
legal transition when it was filed (already dashing, cooldown not up) sitting around and spending itself half a
second later, long after the player let go.

### A transition has to change something

A transition whose target resolves to the state we are already in is never taken. Composites are not states, so
"resolves to" means descending default children until a leaf: re-entering `Locomotion` while in `Run` lands on
`Idle` and is therefore a real change, while `Alive → Dash` while already inside `Dash` is not.

This is not a micro-optimisation, it is what makes rule 1 safe. A transition declared on a parent keeps being
evaluated while a *child* is active — that is the entire point of putting `Alive → Dash` on `Alive` — so it is
also evaluated on every frame the hero spends inside `Dash`, against a request input keeps re-filing. Taking it
runs no `OnExit` and no `OnEnter` at all (both walks stop at the lowest common ancestor, which for a self
transition is the leaf itself) but still resets `TimeInState()`. The hero ended up in a dash that restarted every
frame: it never moved, because `MakeDashMovement` samples the bell curve at `t = 0` and `sin(0)` is zero, and it
never ended, because `Dash → Locomotion` waits on a timer that never got to grow. The old code had the same rule,
written as `if (IsDashing) return;` at the top of `HeroAnimComp::StartDash` — moving the decision into the machine
is what dropped it.

### One decision per tick

`Tick()` takes **at most one new decision**, then lets the tree settle inwards. The settling pass only considers
transitions declared inside the subtree it just entered.

This restriction is load bearing in both directions:

- It **must** settle inwards: leaving `Dash` lands on `Locomotion`, whose default child is `Idle`, but if the
  player is still holding a movement key then `Idle → Run` has to fire in the same tick or the hero renders one
  frame of the wrong animation.
- It **must not** take a second outside decision: with a hit and a dash both pending, the hero would enter `Hit`
  and leave for `Dash` before the hit animation ever drew a frame. *(This was caught by the test driver, not by
  reading the code — the first version of `Tick` did exactly that.)*

`TimeInState()` is 0 for the whole entry frame, which is what stops `Hit → Locomotion` from firing before the
animation component has had its chance to restart the animation the guard is waiting on.

### Order within a frame

```cpp
UpdateComponents();                         // input gathers, forces resolve, health ticks
StateMachine->Tick(*this, Time::FrameTick); // decide the state, then run the behaviour that belongs to it
UpdateAnimations();                         // draw whatever state we ended up in
```

`HeroMoveComp::Update()` still runs in the first phase, but only to resolve forces — knockback has to keep
working in every state. Actual walking (`Locomotion::OnTick`) and dashing (`Dash::OnTick`) are node behaviour.

---

## 4. Where the old code went

| Was | Is now |
|---|---|
| `HeroAnimComp::StartDash` | `Dash::OnEnter` + `HeroMoveComp::BeginDash` |
| `HeroAnimComp::EndDash` | `Dash::OnExit` |
| `HeroMoveComp::ApplyDashImpulse` + `OnDashStart`/`OnDashEnd` delegates | gone; the enter/exit hooks are the event |
| Two separate `DashTimer` fields | `StateMachine->TimeInState()` |
| Hit-finished block in `HeroAnimComp::Update` | `Hit → Locomotion` transition |
| Death freeze block in `HeroAnimComp::Update` | `Die::OnTick` |
| Knockback in the damage listener | `Hit::OnEnter` |
| `HealthComp->OnDeath` listener | the `Alive → Dead` guard reads `IsDead()` directly |
| The `switch` in `HeroAnimComp::Update` | `SetKeyResolver()` registrations |
| `if (state != Die && state != Hit)` × 5 | the tree shape |

`HeroAnimComp` is now just an animation component: resolve a key, apply the editor-tweakable frame interval,
call the base. Nothing after the base call.

---

## 5. Migrating the enemies

The machine is generic, so this is mostly writing a tree. The suggested shape:

```
EnemyRoot
├── Alive                    grants CanFlipAnims
│   ├── Combat               grants CanMove | CanShoot
│   │   ├── Idle   (default)
│   │   ├── Run
│   │   └── Shooting
│   └── Hit                  revokes CanMove | CanShoot
└── Dead                     revokes everything, no outgoing transitions
    └── Die
```

Note the enemy is **not** the hero: `EnemyStateFlag::CanFlipAnims` includes `StateHit`, so `Hit` must not revoke
`CanFlipAnims` here. Read the existing flags before copying the hero's grants across.

### Steps

1. **Add `EnemyCapability.h`** next to `HeroCapability.h`: `CanMove`, `CanShoot`, `CanFlipAnims`, `All`. Only
   the positive side — `Revokes` already covers the `Prevent*` half.

2. **Add `EnemyStateMachine`** deriving from
   `HierarchicalStateMachine<EnemyStateEnum, EnemyBase, EnemyCapability>`, with a `virtual void Build()` so
   `BulletMan` (and later enemies) can add their own nodes on top of the shared shape.

3. **One machine per enemy instance.** The hero is a singleton, an enemy is not. `EnemyBase` owns a
   `std::unique_ptr<EnemyStateMachine>`, built and started in its constructor. Do **not** make the tree static
   or shared — node pointers and the active path are per-instance state.

4. **Move the transitions in, one call site at a time.** The current `SetState` calls map like this:

   | Current call site | Becomes |
   |---|---|
   | `MoveComp->OnForceStart` → `Hit` | `Alive → Hit` guard on `MoveComp->IsBeingForced` |
   | `MoveComp->OnForceEnd` → `Idle` | `Hit → Combat` guard on `!IsBeingForced` |
   | `HealthComp->OnDeath` → `Die` | `Alive → Dead` guard on `HealthComp->IsDead()` |
   | `EnemyMoveCompBase` → `Run` / `Idle` | `Idle ↔ Run` inside `Combat`, on distance to hero |
   | `BulletMan::BulletManShoot` → `Shooting` | `Combat → Shooting` on cooldown elapsed |
   | `BulletMan::UpdateShooting` → `Idle` | `Shooting → Combat` when the gun animation finishes |
   | `BulletMan::HandleHitForce` → `Hit` | already covered by `Alive → Hit` |

5. **Delete the "don't change state if shooting" guard** in `EnemyMoveCompBase::UpdateAIMovement`. `Shooting` is
   a sibling of `Idle`/`Run` under `Combat`, so the `Idle ↔ Run` transitions are simply not on the active path
   while shooting. That is the whole reason to do this.

6. **Move the death side effects into `Dead::OnEnter`**: the depth change, the knockback, clearing the delegates
   and disabling collision. Clearing `OnForceStart`/`OnForceEnd` becomes optional once `Dead` is terminal, but
   keep it if you want the force system to stay quiet.

7. **Replace `BulletManAnimComp::Update`'s switch** with `SetKeyResolver` registrations, exactly as
   `HeroAnimComp::SetKeyResolvers` does. The `BulletManDirections::Get*Enum` helpers are unchanged, they
   just get wired up instead of being called from case labels.

8. **Guards receive `const EnemyBase&`.** For anything BulletMan-specific, use `owner.As<BulletMan>()` inside
   the guard, or register that transition from `BulletMan`'s own `Build()` override where the type is known.

9. **Then delete `EnemyStateFlag` from `StateFlags.h`**, which leaves that file empty and deletable.

### Test it the same way

`docs/` has no test harness, but the two throwaway drivers used for the hero are worth recreating:

- A standalone driver with a **fake owner struct**, compiled against nothing but
  `HierarchicalStateMachine.h`, to assert transition ordering and enter/exit sequences. This is what caught the
  two-decisions-in-one-tick flaw.
- A driver linked against `libetgcore` that calls `Build()` and asserts on the node graph — parents, default
  children, grants/revokes, transition targets, and that `Dead` has no way out. `Build()` needs no window and
  no assets, so it runs headless.

---

## 6. Rules of thumb

- **Never add a `SetState`.** If something needs to push the character into a state, it files a request and a
  guard picks it up. Otherwise you are back to "whoever ran last wins".
- **Put a rule on the node that owns it.** If two sibling states share a permission or a transition, it belongs
  on their parent. If you find yourself writing the same guard on two siblings, you have found a missing
  composite node.
- **Prefer shape over checks.** "X cannot happen while Y" is usually a statement that X's transition should live
  somewhere Y is not on the active path.
- **One-shot requests must be consumed in `OnEnter`, and expired after the tick.** A flag that stays set will
  re-fire its transition later, on a tick nobody was thinking about when it was filed.
- **A guard must not mutate.** It takes `const OwnerT&` for that reason. Side effects belong in
  `OnEnter`/`OnTick`/`OnExit`.
- **Watch the frame order.** Guards run before `UpdateAnimations()`, so anything asking an animation a question
  is reading last frame's answer. `HeroMoveComp::GetDashDuration` looks the animation up by state and direction
  instead of asking for "the current animation" for exactly this reason.

---

## 7. Not done yet

- `GunStateEnum` (Idle / Shoot / Reload) is still a flat enum. `ReloadSlider::FinishAnimation` — a UI object —
  assigns `Gun->CurrentGunState`, which is the same layering violation the hero just got rid of.
- `Hero::MouseAngle`, `CurrentDirection` and `IsShooting` are `static`. They work because there is one hero, but
  they are global mutable state and the machine's guards read them.
- `AnimationKey` is type-erased now, so a new direction enum no longer has to be added to a central list. What
  is left is that it carries a `std::string Name` in every key, and that string takes part in both `operator==`
  and the hash — every animation lookup hashes a string.
