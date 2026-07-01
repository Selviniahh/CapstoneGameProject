# How to create game object

- All the game drawable objects needs to be declared as `std::unique_ptr` by default.
- If the game object doesn't need to be attached to anything, call the global function `ETG::CreateGameObject` to create the object.
- If the game object needs to be attached to another game object, call the global function `ETG::CreateGameObjectAttached` to create the object.

# Create  a non attached object

An example from `GameManager`'s `Initialize` function:

```cpp
std::unique_ptr<Hero> Hero;
std::unique_ptr<UserInterface> UI;

UI = ETG::CreateGameObjectDefault<UserInterface>();
Hero = ETG::CreateGameObjectDefault<class Hero>(ETG::Vector2f{10,10});
```

The objects not attached to anything will be attached to `Scene` object by default.

# Create an attached object

- Attached objects are mainly components. For instance, `Hero` object can have `Move` `Input` `Anim`  component attached to it.
- But other game objects can be attached as well. For instance, `RogueSpecial` gun is attached to `Hero` object.

An example from `Hero`'s  constructor:
```cpp
std::unique_ptr<RogueSpecial> RogueSpecial;
std::unique_ptr<HeroMoveComp> MoveComp;
std::unique_ptr<HeroAnimComp> AnimationComp;
std::unique_ptr<InputComponent> InputComp;

RogueSpecial = ETG::CreateGameObjectAttached<class RogueSpecial>(this,HandPos);
AnimationComp = ETG::CreateGameObjectAttached<HeroAnimComp>(this);
MoveComp = ETG::CreateGameObjectAttached<HeroMoveComp>(this);
InputComp = ETG::CreateGameObjectAttached<InputComponent>(this);
```

First argument is the attachment object. It's given `this` pointer to attach to self in this case.

