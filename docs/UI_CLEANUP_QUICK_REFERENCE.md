# UI Cleanup Quick Reference

## One-Minute Cleanup Pattern

```cpp
#include "systems/ui/UICleanup.h"

class MyView : public View {
    lv_obj_t* screen = nullptr;
    lv_obj_t* button = nullptr;

    void destroy() override {
        getObjectTracker().cleanup();      // Step 1: Clean tracked objects
        UICleanup::deleteRoot(screen);     // Step 2: Delete root (cascades)
        button = nullptr;                   // Step 3: Nullify others
    }
};
```

Done. That's the pattern for 90% of views.

---

## Quick Decision Tree

```
Does your view create objects at RUNTIME?
  ├─ YES → Use getObjectTracker().track(obj) when creating
  │        Use getObjectTracker().cleanup() in destroy()
  └─ NO  → Skip tracker, just use UICleanup::deleteRoot()

Do you have MULTIPLE independent sections?
  ├─ YES → Delete each section separately, or track each
  └─ NO  → Single deleteRoot() call handles all
```

---

## Copy-Paste Template

Use this template for new views:

```cpp
// Header
#pragma once
#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"

/**
 * Object Hierarchy & Cleanup Pattern:
 * - Follows Pattern 1 (Simple Root-Based Cleanup)
 * - Root object: `screen`
 */
class MyView : public View {
public:
    void create() override;
    void update() override;
    void destroy() override;
    lv_obj_t* getScreen() override { return screen; }

private:
    lv_obj_t* screen = nullptr;
    // Add other members here
};

// Implementation
void MyView::destroy() {
    getObjectTracker().cleanup();
    UICleanup::deleteRoot(screen);
    // Nullify other pointers here
}
```

---

## Common Operations

### Delete and Nullify One Object
```cpp
UICleanup::deleteAndNull(button);  // Sets button to nullptr
```

### Delete Root (Cascades All Children)
```cpp
UICleanup::deleteRoot(screen);  // Nullifies screen automatically
```

### Clear Container (Keep Container)
```cpp
UICleanup::clearChildren(container);  // Children deleted, container remains
```

### Track Dynamic Object for Auto-Cleanup
```cpp
while (hasMoreItems()) {
    lv_obj_t* item = lv_btn_create(list);
    getObjectTracker().track(item);  // Will auto-delete on view destroy
}
```

### Check If Object Still Valid
```cpp
if (UICleanup::isValid(button)) {
    lv_obj_set_hidden(button, true);
}
```

---

## The Three Rules

1. **Track what you create dynamically**
   ```cpp
   lv_obj_t* popup = lv_obj_create(root);
   getObjectTracker().track(popup);  // If created at runtime
   ```

2. **Delete root once**
   ```cpp
   UICleanup::deleteRoot(screen);  // Cascades to all children
   ```

3. **Nullify after delete**
   ```cpp
   UICleanup::deleteRoot(screen);  // screen becomes nullptr automatically
   button = nullptr;                // Always nullify other pointers
   ```

---

## Anti-Patterns (❌ Don't Do This)

```cpp
// ❌ Manual cascading delete (error-prone)
lv_obj_del(screen);
lv_obj_del(button);  // Dangerous! button already deleted via cascade
button = nullptr;

// ✅ Correct: Delete root once
UICleanup::deleteRoot(screen);
button = nullptr;

// ❌ Forget to nullify
lv_obj_del(button);
button->hidden = true;  // ← CRASH! Dangling pointer

// ✅ Correct
UICleanup::deleteAndNull(button);  // Safe, nullifies automatically

// ❌ Double-track same object
getObjectTracker().track(obj);
getObjectTracker().track(obj);  // Error: will try to delete twice

// ✅ Track once
lv_obj_t* obj = lv_btn_create(parent);
getObjectTracker().track(obj);
```

---

## Performance Notes

- **Tracker overhead:** 24 bytes per view
- **Deletion time:** Negligible (< 1ms for 100 objects)
- **Memory check:** Yes, LVGL objects are lightweight
- **Safe for embedded:** Yes, designed for ESP32

---

## When to Use Each Pattern

| Pattern | When | Example |
|---------|------|---------|
| 1: Root Delete | Most views | MainMenuView, BootView |
| 2: Tracker | Dynamic lists | GameListView with cards |
| 3: Mixed | Static + dynamic | GameView with Lua objects |
| 4: Sections | Multi-section layout | Complex dialogs |

---

## Files to Know

- `systems/ui/UIObjectTracker.h` — Tracker interface
- `systems/ui/UICleanup.h` — Helper functions
- `systems/ui/View.h` — Base class (has tracker built-in)
- `UI_CLEANUP_SYSTEM.md` — Full documentation

---

## Checklist for View Creation

- [ ] Add `#include "systems/ui/UICleanup.h"` to header
- [ ] Add doc comment explaining cleanup pattern
- [ ] Use `getObjectTracker().track()` for dynamic objects
- [ ] Use `UICleanup::deleteRoot()` in `destroy()`
- [ ] Nullify all other object pointers
- [ ] Test: create → destroy → create cycle without crashes
- [ ] Memory check: No leaks after 10 view transitions

Done! Your view is properly managed.
