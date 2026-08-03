Here’s a revised and improved version of your rendering guidelines with clearer explanations, structure, and emphasis on critical rules:

---

# Rendering Guidelines

## **Why Use SpriteBatch?**
- **Performance**: Reduces draw calls by grouping sprites that share a texture *and* a shader into a single bgfx submit.
- **Best Practice**: Direct calls to `Window->draw()` are **prohibited** except for non-batched debug elements (e.g., `ETG::Text`, `ETG::RectangleShape`).

---

## **How to Use SpriteBatch**
### **Basic Workflow**
```cpp
SpriteBatch.begin();     // Start a batch
// Add sprites here (e.g., Hero, UI elements)
SpriteBatch.end(*Window); // Render all batched sprites in one go
```

### **Rules to Follow**
1. **Never call `Window->draw()` directly**:
   ```cpp
   // ❌ BAD: Direct draw calls
   Window->draw(sprite);
   
   // ✅ GOOD: Add to SpriteBatch
   SpriteBatch.begin();
   SpriteBatch.draw(sprite); // Batched
   SpriteBatch.end(*Window);
   ```

2. **Group by View**:
    - All sprites in a batch **must use the same view** (camera).
    - **Change the view *before* starting a batch**:
      ```cpp
      Window->setView(MainView); // Set view FIRST
      SpriteBatch.begin();       // Then start the batch
      ```

3. **Batch Lifecycle**:
    - `begin()`: Resets the batch buffer.
    - `end()`: Flushes the batch to the GPU. **Call this *before* changing the view**.

4. **Shaders**: a sprite carries an `ETG::ShaderEffect` (see [BgfxRenderer.md](BgfxRenderer.md)). A run of quads is only merged into one submit while both the texture and the effect stay the same, so mixing effects has the same cost as mixing textures.

---

## **GameManager Draw Phases**
The `GameManager::Draw()` method is structured into **3 phases** to handle different rendering needs:

### **1. Scaled (MainView)**
- **Purpose**: Render game objects (Hero, enemies) using a zoomed/moved camera.
- **Workflow**:
  ```cpp
  Window->setView(Globals::MainView); // Set zoomed view
  SpriteBatch.begin();
  Hero.Draw();     // Batched under MainView
  Enemy.Draw();
  SpriteBatch.end(*Window);
  ```

### **2. Unscaled (DefaultView)**
- **Purpose**: Render UI elements (HUD, menus) in screen coordinates (persistent position).
- **Workflow**:
  ```cpp
  Window->setView(Window->getDefaultView()); // Reset to screen coords
  SpriteBatch.begin();
  UI.Draw(); // Batched under DefaultView
  SpriteBatch.end(*Window);
  ```

### **3. Non-Batched**
- **Purpose**: Debug elements (text, shapes) drawn directly.
- **Workflow**:
  ```cpp
  DebugText::Draw(*Window); // Direct draw (text is drawn immediately)
  ```

---

## **Example: GameManager.cpp**
```cpp
void ETG::GameManager::Draw() {
    Window->clear(ETG::Color::Black);

    // Phase 1: Scaled (Game World)
    Window->setView(Globals::MainView); // Zoomed view
    SpriteBatch.begin();
    Hero.Draw();    // Batched under MainView
    Enemy.Draw();
    SpriteBatch.end(*Window); // Flush before view change

    // Phase 2: Unscaled (UI)
    Window->setView(Window->getDefaultView()); // Screen coords
    SpriteBatch.begin();
    UI.Draw();      // Batched under DefaultView
    SpriteBatch.end(*Window);

    // Phase 3: Non-Batched (Debug)
    DebugText::Draw(*Window); // Direct draw (drawn immediately through the platform layer)

    Window->display();
}
```

---

## **Common Pitfalls**
- **Mixing Views in a Batch**:
  ```cpp
  SpriteBatch.begin();
  Window->setView(ViewA); // ❌ WRONG: View changed mid-batch
  SpriteBatch.end(*Window);
  ```
- **Forgetting to Call `end()`**:
    - Always pair `begin()` with `end()`. Unflushed batches cause rendering issues.

---

## **Debugging Exceptions**
- **Debug Text**: Uses `ETG::Text` directly. The platform layer rasterizes the string once and renders it as one textured quad per string.
- **Shapes/Lines**: Use sparingly. For example:
  ```cpp
  // ❌ Avoid (not batched):
  ETG::RectangleShape rect;
  Window->draw(rect);
  
  // ✅ Better: Convert to a sprite or use a batched quad.
  ```

---

## **Best Practices**
1. **Batch Early, Batch Often**:
    - Always prefer `SpriteBatch` for game objects and UI.
2. **Minimize Texture Swaps**:
    - Group sprites by texture within a batch (e.g., render all "hero" sprites before "enemies").
3. **Profile Performance**:
    - Track submits per frame (bgfx's debug stats, or how often `SpriteBatch::end` flushes) to measure batching efficiency.

---

This version emphasizes **key rules**, provides actionable examples, and clarifies the relationship between views and batches. It’s structured for quick scanning while still being thorough.