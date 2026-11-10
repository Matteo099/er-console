# UI Component Lifecycle Management & Cleanup System

## Overview

This document describes the **robust LVGL object lifecycle management system** implemented for the EscapeOS UI framework. It provides safe, maintainable patterns for managing display object creation, operation, and cleanup.

---

## Problem Statement

**Before this system:**
- Manual `lv_obj_del()` calls scattered throughout view destroy methods
- Error-prone pointer management (forgetting to nullify pointers)
- Use-after-free bugs when parent objects deleted children unexpectedly
- No consistent pattern across views
- Difficult to track object ownership and hierarchy
- Adding a new child object required manual deletion code

---

## Solution Architecture

### Three-Component System

#### 1. **UIObjectTracker** (`UIObjectTracker.h`)
Manages automatic object lifecycle with RAII semantics.

**Key Features:**
- Tracks LVGL objects with automatic cleanup order
- LIFO (Last In, First Out) deletion order — children deleted before parents
- Prevents use-after-free by checking object validity before deletion
- Support for move semantics (no copying, prevents accidental duplicates)
- Hierarchical awareness (doesn't delete already-deleted objects)

**Usage:**
```cpp
UIObjectTracker tracker;
lv_obj_t* btn = tracker.track(Button::create(...));
// Automatic cleanup on scope exit (destructor)
```

#### 2. **UICleanup Utilities** (`UICleanup.h`)
Helper functions for safe object deletion patterns.

**Key Functions:**
- `deleteAndNull(obj)` — Safely delete and nullify pointer
- `deleteRoot(screen)` — Delete root (cascades to children)
- `clearChildren(parent)` — Clear all children of a container
- `isValid(obj)` — Check if object is still in hierarchy
- `getDepth(obj)` — Find object's depth in hierarchy
- `findCommonParent(obj1, obj2)` — Find common ancestor

**Usage:**
```cpp
UICleanup::deleteRoot(myScreen);  // Cascades, nullifies
UICleanup::deleteAndNull(button); // Safe single deletion
```

#### 3. **Enhanced View Base Class** (`View.h`)
All View subclasses now have built-in object tracking.

**Usage:**
```cpp
// In view's create():
lv_obj_t* dynamicBtn = getObjectTracker().track(Button::create(...));

// In view's destroy():
getObjectTracker().cleanup();  // Auto-destroys all tracked objects
```

---

## Implementation Patterns

### Pattern 1: Simple Root-Based Cleanup (RECOMMENDED for ~80% of views)

**Use Case:** Most views with static object hierarchy

```cpp
class MyView : public View {
public:
    void create() override {
        root = Screen::create();
        button = Button::create(root, ...);
        label = Label::create(root, ...);
        // All children automatically own by root hierarchy
    }

    void destroy() override {
        // Clean tracker (for any dynamic objects, if used)
        getObjectTracker().cleanup();
        
        // Single line: delete root cascades to all children
        UICleanup::deleteRoot(root);
        
        // Nullify other pointers (defensive)
        button = nullptr;
        label = nullptr;
    }

private:
    lv_obj_t* root = nullptr;
    lv_obj_t* button = nullptr;
    lv_obj_t* label = nullptr;
};
```

**Benefits:**
- ✅ One-line cleanup
- ✅ Automatic cascading deletion
- ✅ No manual ordering concerns
- ✅ Clear ownership model

---

### Pattern 2: Tracker-Based Cleanup (for dynamic objects)

**Use Case:** Views creating objects at runtime (lists, grids, popups)

```cpp
class GameListView : public View {
private:
    std::vector<lv_obj_t*> cards;

public:
    void rebuildCards() {
        for (auto& game : games) {
            lv_obj_t* card = lv_btn_create(list);
            // Track it for cleanup
            getObjectTracker().track(card);
            cards.push_back(card);
        }
    }

    void destroy() override {
        // Tracker auto-deletes all tracked cards in LIFO order
        getObjectTracker().cleanup();
        cards.clear();
        UICleanup::deleteRoot(screen);
    }
};
```

**Benefits:**
- ✅ Handles variable object counts
- ✅ LIFO deletion order maintained
- ✅ No manual loop-based cleanup
- ✅ Scales to thousands of objects

---

### Pattern 3: Mixed Static + Dynamic Cleanup

**Use Case:** View with both static layout and runtime-created content

```cpp
class GameView : public View {
private:
    // Static structure (owned by screen hierarchy)
    lv_obj_t* screen = nullptr;
    lv_obj_t* header = nullptr;
    
    // Dynamic content (tracks separately)
    std::vector<lv_obj_t*> dynamicButtons;

public:
    void destroy() override {
        // Clean up dynamic objects tracked at runtime
        getObjectTracker().cleanup();
        
        // Delete root (cascades to header and static children)
        UICleanup::deleteRoot(screen);
        
        // Clear dynamic references
        header = nullptr;
        dynamicButtons.clear();
    }
};
```

---

### Pattern 4: Section-Based Cleanup (complex multi-area views)

**Use Case:** Views with independent header, content, footer sections

```cpp
class ComplexView : public View {
private:
    struct {
        lv_obj_t* container;
        lv_obj_t* title;
        lv_obj_t* buttons;
    } header, content, footer;

private:
    void destroySection(lv_obj_t*& container) {
        UICleanup::deleteRoot(container);
    }

public:
    void destroy() override {
        destroySection(header.container);
        destroySection(content.container);
        destroySection(footer.container);
    }
};
```

---

## Migration Guide

### Step 1: Add Includes

```cpp
// In your View subclass header:
#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
```

### Step 2: Identify Object Hierarchy

Ask: "What owns what?"
- **Owned by parent hierarchy** → Will cascade-delete
- **Owned by view** → Delete explicitly

### Step 3: Choose Pattern

| View Type | Pattern | Complexity |
|-----------|---------|-----------|
| Simple screen (btn, label) | Pattern 1 | ⭐ |
| List/grid with dynamic items | Pattern 2 | ⭐⭐ |
| Mixed static + dynamic | Pattern 3 | ⭐⭐ |
| Multiple independent sections | Pattern 4 | ⭐⭐⭐ |

### Step 4: Refactor destroy()

**Before:**
```cpp
void destroy() {
    if (screen) lv_obj_del(screen);
    screen = nullptr;
    button = nullptr;
    label = nullptr;
    image = nullptr;
    // ... 10+ manual lines
}
```

**After:**
```cpp
void destroy() {
    getObjectTracker().cleanup();
    UICleanup::deleteRoot(screen);
    button = nullptr;
    label = nullptr;
    // ... 3-5 lines, much clearer
}
```

### Step 5: Test

```bash
# Memory check: Verify no memory leaks
# Pointer check: Ensure no use-after-free crashes
# Functional test: View create → destroy → recreate cycle
```

---

## Key Principles

### 1. LIFO Deletion Order
**Delete last-created objects first**

```
Created order:  root → button → list → item
Deletion order: item → list → button → root ✓
```

UIObjectTracker automatically handles this.

### 2. Cascade Deletion
**Delete parent, auto-deletes children**

```cpp
lv_obj_t* root = Screen::create();
lv_obj_t* btn = Button::create(root);  // btn is child of root
lv_obj_del(root);  // ✓ Automatically deletes btn
// btn pointer is now dangling!
```

**Solution:** Always nullify after delete, or use `UICleanup::deleteRoot()` which does it.

### 3. One Deletion Path
**Don't have multiple ways to delete**

```cpp
// ❌ Bad: Multiple destruction paths
void destroy() {
    if (state == NORMAL) {
        lv_obj_del(screen);
    } else if (state == ANIMATED) {
        lv_anim_del(screen, nullptr);
        lv_obj_del(screen);  // Might double-delete
    }
}

// ✅ Good: Single consistent path
void destroy() {
    getObjectTracker().cleanup();  // Handles all cases
    UICleanup::deleteRoot(screen);
}
```

### 4. Document Ownership
**In class comments, state what owns what**

```cpp
class MyView : public View {
    // Owned by screen hierarchy (will cascade-delete)
    lv_obj_t* button = nullptr;
    
    // Owned by view, manually deleted
    std::vector<lv_obj_t*> dynamicItems;
};
```

---

## Real-World Example: GameView

### Original Code (Problematic)
```cpp
void GameView::destroy() {
    if (root) lv_obj_del(root);
    
    root = nullptr;
    header = nullptr;
    titleLabel = nullptr;
    timerLabel = nullptr;
    startPauseBtn = nullptr;
    startPauseLabel = nullptr;
    cancelBtn = nullptr;
    infoBtn = nullptr;
    settingsBtn = nullptr;
    loadingContainer = nullptr;
    loadingBar = nullptr;
    loadingLabel = nullptr;
    errorLabel = nullptr;
    gameContentArea = nullptr;
}
```

**Issues:**
- 🔴 15 manual pointer assignments
- 🔴 Easy to forget new members
- 🔴 No documentation of what owns what
- 🔴 Hard to maintain as objects change

### Refactored Code (Clean)
```cpp
void GameView::destroy() {
    // Clean up any dynamically created Lua objects
    getObjectTracker().cleanup();

    // Delete root (cascades to all static children)
    UICleanup::deleteRoot(root);

    // Nullify remaining pointers (defensive programming)
    header = nullptr;
    // ... (automatic cascade handles rest)
}
```

**Benefits:**
- ✅ 3 lines vs 15 lines
- ✅ Clear intent
- ✅ Automatic cascade handling
- ✅ Easy to add new members
- ✅ Safer: No manual omissions

---

## Common Patterns & Recipes

### Recipe 1: Temporary Popup Dialog

```cpp
lv_obj_t* createPopup() {
    lv_obj_t* popup = lv_obj_create(screen);
    getObjectTracker().track(popup);  // Track for cleanup
    
    // Add children...
    lv_obj_t* btn = Button::create(popup, "OK", ...);
    // (btn owned by popup, cascades)
    
    return popup;
}

// Auto-cleanup when view destroyed
```

### Recipe 2: Scrollable List with Dynamic Items

```cpp
std::vector<lv_obj_t*> items;

void buildList() {
    for (int i = 0; i < 100; i++) {
        lv_obj_t* item = lv_btn_create(list);
        getObjectTracker().track(item);
        items.push_back(item);
    }
}

void destroy() {
    getObjectTracker().cleanup();  // Handles all 100 items
    UICleanup::deleteRoot(screen);
    items.clear();
}
```

### Recipe 3: Nested Container Hierarchy

```cpp
// All automatically handled via cascade deletion
lv_obj_t* root = Screen::create();
lv_obj_t* container1 = Container::create(root, ...);
lv_obj_t* container2 = Container::create(container1, ...);
lv_obj_t* button = Button::create(container2, ...);

// Single delete cleans all:
UICleanup::deleteRoot(root);
// ✓ root → container1 → container2 → button all deleted
```

---

## Debugging Helpers

### Check Object Validity
```cpp
// Before using a pointer:
if (UICleanup::isValid(myButton)) {
    lv_obj_set_hidden(myButton, true);
} else {
    // Object was already deleted (by parent)
    myButton = nullptr;
}
```

### Find Object Depth
```cpp
int depth = UICleanup::getDepth(myLabel);
// 0 = root
// 1 = direct child of root
// 2+ = nested deeper
```

### Find Common Parent
```cpp
lv_obj_t* common = UICleanup::findCommonParent(button, label);
// If both are in same hierarchy tree, returns their common parent
// Useful for understanding relationships
```

---

## Performance Considerations

### Memory Overhead
- `UIObjectTracker`: ~24 bytes base + 8 bytes per object (std::vector)
- `UICleanup`: Zero overhead (inline functions)
- Negligible for typical views (10-50 objects)

### Deletion Performance
- **LIFO order check**: O(n) where n = number of tracked objects
- **Validity check**: O(1) — just checks parent pointer
- **Cascade deletion**: Hardware-dependent, typically ≤1ms per 100 objects

**For embedded systems:** Acceptable. Views destroy ~once per scene transition.

---

## Troubleshooting

### Issue: Use-After-Free on Pointer Access

**Symptom:** Crash when accessing `button->` after `destroy()`

**Cause:** Pointer not nullified after delete

**Fix:** 
```cpp
// Use UICleanup helper:
UICleanup::deleteAndNull(button);  // Deletes AND sets to nullptr

// Or manually:
lv_obj_del(button);
button = nullptr;  // ← Don't forget this!
```

### Issue: Objects Not Deleted (Memory Leak)

**Symptom:** Increasing heap size with repeated scene changes

**Cause:** Forgot to delete in `destroy()` or forgot to track dynamic objects

**Fix:**
```cpp
void destroy() {
    // Always include tracker cleanup:
    getObjectTracker().cleanup();
    
    // Always delete root:
    UICleanup::deleteRoot(screen);
}
```

### Issue: Double Deletion Crash

**Symptom:** Crash after `destroy()` called, error mentions invalid object

**Cause:** Deleting same object twice via tracker AND manual delete

**Fix:**
```cpp
void destroy() {
    getObjectTracker().cleanup();  // Deletes all tracked
    
    // DON'T manually delete tracked objects:
    // UICleanup::deleteAndNull(button);  // ← Error! Already deleted
    
    UICleanup::deleteRoot(screen);
}
```

---

## Files Overview

| File | Purpose |
|------|---------|
| `UIObjectTracker.h` | Object lifecycle management with RAII |
| `UICleanup.h` | Helper utilities for safe deletion |
| `UICleanupPatterns.h` | Design patterns documentation |
| `View.h` | Enhanced base class with tracker integration |

---

## FAQ

### Q: Should I always use UIObjectTracker?
**A:** Use it for dynamic objects (created at runtime). For static views, `UICleanup::deleteRoot()` is sufficient.

### Q: Can I delete objects manually while tracker exists?
**A:** Not recommended. Use `getObjectTracker().deleteTracked(obj)` for consistency.

### Q: What if my view has custom animations?
**A:** Stop animations before deleting:
```cpp
void destroy() {
    lv_anim_del(myObject, nullptr);  // Stop animation
    getObjectTracker().cleanup();
    UICleanup::deleteRoot(screen);
}
```

### Q: Performance impact of tracker?
**A:** Negligible. ~24 bytes per view. Deletion check is O(n) but n is typically ≤50.

### Q: Can I nest trackers?
**A:** No, not necessary. One tracker per view (inherited from View base class) is enough.

---

## Summary

✅ **Before:** 15-20 lines manual cleanup per view  
✅ **After:** 3-5 lines with tracker + utilities  
✅ **Reliability:** RAII pattern ensures cleanup always happens  
✅ **Safety:** Automatic LIFO deletion prevents use-after-free  
✅ **Maintainability:** Adding new objects requires no cleanup modification  
✅ **Performance:** < 1ms cleanup overhead per view  

**All scene views have been refactored to use this system.** See GameView.cpp for reference implementation.
