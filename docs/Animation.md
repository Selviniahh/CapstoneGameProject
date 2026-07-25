## **Overview**
The animation system uses a state-driven approach to manage hero animations. Key components:
- **`AnimationComponent`**: Handles sprite updates, flipping, and animation transitions.
- **`AnimationManager`**: Manages a collection of animations for a specific state (e.g., `Run`, `Idle`).
- **`Hero`**: Controls the hero’s position, state, and delegates animation logic to the `AnimationComponent`.

---

## **Core Components**

### **1. `AnimationComponent`**
#### Responsibilities:
- Stores animations for each `HeroStateEnum` (e.g., `Run`, `Dash`).
- Updates the current frame based on the hero’s state and direction.
- Flips sprites horizontally and adjusts hand positions.

#### Key Methods:
| Method | Description |  
|--------|-------------|  
| **`Update()`** | Updates the current animation frame based on `HeroStateEnum` and `AnimationKey`. |  
| **`FlipSprites()`** | Flips sprites horizontally and adjusts `relativeHandLoc` for hand positioning. |  
| **`SetAnimations()`** | Initializes all animations for `Run`, `Idle`, and `Dash` states. |  

---

### **2. `Hero` Class**
#### Responsibilities:
- Tracks the hero’s position, direction, and state.
- Maps `Direction` enums to animation states (e.g., `RunEnum`, `IdleEnum`).
- Delegates animation updates to the `AnimationComponent`.

#### Key Methods:
| Method | Description |  
|--------|-------------|  
| **`HeroDirections::GetRunEnum()`** | Maps `Direction` to the correct `RunEnum` animation (e.g., `Direction::Up` → `Run_Back`). |  
| **`HeroDirections::GetIdleEnum()`** | Maps `Direction` to the correct `IdleEnum` animation. |  
| **`SetHandTexLoc()`** | Positions the hand sprite relative to the hero’s body. |  

---

## **Enum Relationships**

### **1. `HeroStateEnum`**
Defines the hero’s overarching state:
```cpp  
enum class HeroStateEnum {  
    Idle,  
    Run,  
    Dash  
    // Add new states here (e.g., Attack, Jump)  
};  
```  

### **2. `Direction`**
Eight-way facing in screen space (0° is right, the angle grows clockwise because +y points down). Each value owns
the 45° arc centred on itself, and `DirectionUtils::GetDirectionFromAngle` is the only thing that assigns one:
```cpp  
enum class Direction {  
    Right,      //   0  
    DownRight,  //  45  
    Down,       //  90  
    DownLeft,   // 135  
    Left,       // 180  
    UpLeft,     // 225  
    Up,         // 270  
    UpRight     // 315  
};  
```  
These used to be named `FrontHandRight`, `BackDiagonalLeft` and so on — four Left/Right pairs, because
`IsFacingRight` decided the flip by substring-matching the enum's *name*. Eight arcs centred on the compass
points only form three mirror pairs (straight down and straight up are their own mirror image), so four pairs
could not be placed symmetrically and the left half of the compass ended up rotated 45° against the right half:
pointing the mouse straight left drew the back-diagonal sprite while pointing it straight right drew the side
one. The names carry neither the sprite nor the flip now — `HeroDirections` and `BulletManDirections` state both
out loud, mirror-symmetrically.

### **3. Animation Keys**
Each `HeroStateEnum` maps to a subset of animations via `AnimationKey` enums:

| State (`HeroStateEnum`) | Sub-Animation (`AnimationKey`)         | Example File Path                     |
|-------------------------|----------------------------------------|---------------------------------------|
| `Run`                 | `Run_Back`, `Run_BackWard`, etc.        | `rogue_run_back_hands_001.png`        |
| `Idle`                | `Idle_Back`, `Idle_Right`, etc.         | `rogue_idle_back_hand_left_001.png`   |
| `Dash`                | `Dash_Back`, `Dash_Left`, etc.          | `rogue_dodge_back_001.png`            |

___

## **Sprite Flipping Logic**
### **`FlipSprites()` Behavior**
- **Right-facing directions** (e.g., `Direction::Right`):
    - `flipX = 1.0` (no flip).
    - Hand position: `{8.f, 5.f}`.

- **Left-facing directions** (e.g., `Direction::Left`):
    - `flipX = -1.0` (horizontal flip).
    - Hand position: `{-8.f, 5.f}`.

- **Edge Cases**:
    - `Down` and `Up` are their own mirror image, so no flip is "correct" for them. `ETG::IsFacingRight` picks one
      each and says so; changing either is free, as long as both animation components keep agreeing with it.