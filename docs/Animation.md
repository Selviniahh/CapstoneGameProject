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
| **`GetRunEnum()`** | Maps `Direction` to the correct `RunEnum` animation (e.g., `Direction::BackHandRight` → `Run_Back`). |  
| **`GetIdleDirectionEnum()`** | Maps `Direction` to the correct `IdleEnum` animation. |  
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
Defines the hero’s facing direction (used for flipping and hand positioning):
```cpp  
enum class Direction {  
    Right, Left, FrontHandRight, FrontHandLeft,  
    BackHandRight, BackHandLeft,  
    BackDiagonalRight, BackDiagonalLeft  
};  
```  

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
    - `BackDiagonalRight` uses `{-8.f, 5.f}` to align hands with asymmetrical sprites.
    - `BackDiagonalLeft` uses `{8.f, 5.f}` for the same reason.