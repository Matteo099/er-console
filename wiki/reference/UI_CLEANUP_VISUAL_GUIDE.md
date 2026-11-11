# UI Cleanup - Visual Guide

## Object Hierarchy Example

### Static Hierarchy (Typical View)
```
CREATION ORDER:          DELETION ORDER (LIFO):      OBJECT STATE DURING:
                                                      
1. Screen (root)         5. Nullify button    ✓       Step 1: Create
   └─ 2. Button          4. Nullify label     ✓           screen
      └─ 3. Label        3. Delete label      ✓           └─ button
         └─ 4. Dropdown     • Cascade child    ✓              └─ label
            └─ Item       2. Delete button       ✓
                          1. Delete root      ✓

SAFE DELETION:           UNSAFE DELETION:
UICleanup::deleteRoot    lv_obj_del(label)    ← Deletes label
(root)                   lv_obj_del(button)   ← CRASH! Button already
                                                 deleted by root cascade
→ Cascade deletion       
→ Managed LIFO
→ Safe pointers
```

---

## Memory Ownership Diagram

### Bad Pattern (Before Refactor)
```
┌─────────────────────────────────────────┐
│            GameView::destroy()          │
├─────────────────────────────────────────┤
│                                         │
│  if (root) lv_obj_del(root);           │ ← Cascades automatically
│  root = nullptr;         ← Set null     │   but messy management
│  header = nullptr;       ← Manual null  │
│  titleLabel = nullptr;   ← Manual null  │
│  timerLabel = nullptr;   ← Manual null  │
│  startPauseBtn = nullptr;← Manual null  │
│  cancelBtn = nullptr;    ← Manual null  │
│  ... 10+ more manual lines ❌           │
│                                         │
│  Problems:                              │
│  • Error-prone (easy to forget)         │
│  • Hard to maintain (15+ lines)         │
│  • Unclear who owns what                │
│  • Scales poorly with more objects      │
│                                         │
└─────────────────────────────────────────┘
```

### Good Pattern (After Refactor)
```
┌─────────────────────────────────────────┐
│            GameView::destroy()          │
├─────────────────────────────────────────┤
│                                         │
│  getObjectTracker().cleanup();         │ ← Clean dynamic objects
│  UICleanup::deleteRoot(root);          │ ← Delete root
│  header = nullptr;                     │ ← Quick nullify
│  ...minimal lines ✓                    │
│                                         │
│  Benefits:                              │
│  • Clear intent                         │
│  • Automatic cascade                    │
│  • LIFO order guaranteed                │
│  • Easy to add new objects              │
│  • Scales with system                   │
│                                         │
└─────────────────────────────────────────┘
```

---

## Deletion Flow Diagram

### Pattern 1: Simple Root Delete (Most Common)
```
┌──────────────────────────────────┐
│      View::destroy()             │
├──────────────────────────────────┤
│                                  │
│  ┌──────────────────────────┐   │
│  │ getObjectTracker().      │   │
│  │ cleanup()                │   │ ← Delete any dynamic objects
│  │ │                        │   │   (popups, lists, etc.)
│  │ └─→ [dynamic obj 1]      │   │
│  │     [dynamic obj 2]      │   │
│  │     ...                  │   │
│  └──────────────────────────┘   │
│           │                      │
│           ▼                      │
│  ┌──────────────────────────┐   │
│  │ UICleanup::             │   │
│  │ deleteRoot(screen)      │   │ ← Single operation
│  │ │                       │   │   Cascades all children
│  │ └─→ Screen deleted      │   │
│  │     └─→ Button deleted  │   │
│  │         └─→ Label del.  │   │
│  │             └─→ Image   │   │
│  │                 ...     │   │
│  └──────────────────────────┘   │
│           │                      │
│           ▼                      │
│  ┌──────────────────────────┐   │
│  │ Nullify other pointers  │   │ ← Clean up references
│  │ header = nullptr;        │   │
│  │ button = nullptr;        │   │
│  │ label = nullptr;         │   │
│  └──────────────────────────┘   │
│           │                      │
│           ▼                      │
│      ✓ View destroyed            │
│                                  │
└──────────────────────────────────┘
```

### Pattern 2: Tracker + Root Delete (Dynamic Content)
```
┌──────────────────────────────────────┐
│         GameListView::destroy()      │
├──────────────────────────────────────┤
│                                      │
│  Step 1: Clean tracked objects      │
│  ┌────────────────────────────────┐ │
│  │ getObjectTracker().cleanup()   │ │
│  │ Deletes in LIFO order:         │ │
│  │ ┌─── Last created ──┐          │ │
│  │ │ [card N]          │ ✓ DELETE │ │
│  │ │ [card N-1]        │ ✓ DELETE │ │
│  │ │ ...               │ ✓ DELETE │ │
│  │ │ [card 1]          │ ✓ DELETE │ │
│  │ └─ First created ───┘          │ │
│  └────────────────────────────────┘ │
│           │                         │
│           ▼                         │
│  Step 2: Delete static root        │
│  ┌────────────────────────────────┐ │
│  │ UICleanup::deleteRoot(screen)  │ │
│  │ screen                         │ │
│  │ ├─ header                      │ │
│  │ ├─ list (already emptied)      │ │
│  │ └─ btnBack                     │ │
│  │ All deleted hierarchically     │ │
│  └────────────────────────────────┘ │
│           │                         │
│           ▼                         │
│  Step 3: Clear containers          │
│  ┌────────────────────────────────┐ │
│  │ cards.clear();                 │ │
│  │ cardUserData.clear();          │ │
│  │ (Already deleted, just clean)  │ │
│  └────────────────────────────────┘ │
│           │                         │
│           ▼                         │
│       ✓ View destroyed              │
│                                      │
└──────────────────────────────────────┘
```

---

## LIFO Deletion Order Visualization

### Creating Objects (Addition Order)
```
Time ─────────────────────────────────────────────>

Push to tracker:
Create 1: Screen ──────────────────────────► Tracker: [Screen]
Create 2: Button ──────────────────────────► Tracker: [Screen, Button]
Create 3: Label  ──────────────────────────► Tracker: [Screen, Button, Label]
Create 4: Image  ──────────────────────────► Tracker: [Screen, Button, Label, Image]
Create 5: Item   ──────────────────────────► Tracker: [Screen, Button, Label, Image, Item]

Stack structure (LIFO - Last In, First Out):
┌─────────┬─────────┬─────────┬───────────┬──────────┐
│ Screen  │ Button  │ Label   │  Image    │  Item    │  ◄─ Top (Last In)
├─────────┼─────────┼─────────┼───────────┼──────────┤
│   1st   │   2nd   │   3rd   │    4th    │   5th    │
└─────────┴─────────┴─────────┴───────────┴──────────┘
```

### Deleting Objects (Reverse Order)
```
Time ─────────────────────────────────────────────>

Pop from tracker (LIFO):
Delete 1: Item   ◄─ Pop from top ──────────► Tracker: [Screen, Button, Label, Image]
Delete 2: Image  ◄─ Pop from top ──────────► Tracker: [Screen, Button, Label]
Delete 3: Label  ◄─ Pop from top ──────────► Tracker: [Screen, Button]
Delete 4: Button ◄─ Pop from top ──────────► Tracker: [Screen]
Delete 5: Screen ◄─ Pop from top ──────────► Tracker: []

Result: ✓ All children deleted before parents
```

---

## Cascade Deletion Example

### Hierarchy Tree
```
Screen (root)
├── ┌──────────────────────┐
│   │ Container (header)   │
│   ├── Label (title)      │ ◄─ Child
│   └── Button             │ ◄─ Child
│
├── ┌──────────────────────┐
│   │ Container (content)  │
│   ├── List               │ ◄─ Child
│   │   ├── Item 1         │ ◄─ Grandchild
│   │   ├── Item 2         │ ◄─ Grandchild
│   │   └── Item 3         │ ◄─ Grandchild
│   └── Image              │ ◄─ Child
│
└── Button (back)          ◄─ Direct child of root
```

### When You Do This
```cpp
UICleanup::deleteRoot(screen);  // Delete only the root!
```

### LVGL Automatically Cascades
```
Delete Screen
  ├─ LVGL sees children
  │  └─ Recursively delete Container (header)
  │     ├─ Delete Label (title)
  │     └─ Delete Button
  │
  ├─ LVGL sees children
  │  └─ Recursively delete Container (content) 
  │     ├─ Delete List
  │     │  ├─ Delete Item 1
  │     │  ├─ Delete Item 2
  │     │  └─ Delete Item 3
  │     └─ Delete Image
  │
  └─ Delete Button (back)

✓ All 12+ objects deleted in correct order!
```

---

## Tracker Memory Layout

### UIObjectTracker Structure
```cpp
UIObjectTracker tracker;

// Internal state:
// ┌──────────────────────────────────────┐
// │ std::vector<lv_obj_t*> objects;     │
// │ ┌──────────────────────────────────┐ │
// │ │ [0] = screen         ◄─ Index 0  │ │
// │ │ [1] = button         ◄─ Index 1  │ │
// │ │ [2] = label          ◄─ Index 2  │ │
// │ │ [3] = popup          ◄─ Index 3  │ │
// │ │ ...                              │ │
// │ │ [n] = last_object    ◄─ Back     │ │
// │ └──────────────────────────────────┘ │
// └──────────────────────────────────────┘

// When cleanup() called:
// objects.pop_back()  ← Delete [n]
// objects.pop_back()  ← Delete [3]
// ...
// objects.pop_back()  ← Delete [0]
// Result: LIFO order guaranteed!
```

---

## Common Error Scenarios

### Scenario 1: Dangling Pointer After Delete
```
WRONG:
┌─────────────────────────┐
│ lv_obj_del(button);    │ ← Deletes button
│ button->hidden = true; │ ← ERROR! Dangling pointer
└─────────────────────────┘

RIGHT:
┌────────────────────────────────────┐
│ UICleanup::deleteAndNull(button); │ ← Deletes AND nullifies
│ // button is now nullptr, safe    │
└────────────────────────────────────┘
```

### Scenario 2: Double-Delete via Cascade
```
WRONG:
┌─────────────────────────┐
│ Screen = [Button]      │ (parent relationship)
│ lv_obj_del(screen);   │ ← Cascades to button
│ lv_obj_del(button);   │ ← ERROR! Already deleted
└─────────────────────────┘

RIGHT:
┌──────────────────────────────────┐
│ Screen = [Button]               │
│ UICleanup::deleteRoot(screen); │ ← Single delete!
│ button = nullptr;               │ ← Safe to reference
└──────────────────────────────────┘
```

### Scenario 3: Forgetting to Track Dynamic Objects
```
WRONG:
┌────────────────────────────────────┐
│ for (int i = 0; i < 100; i++) {  │
│   lv_obj_t* card = ...create();  │ ← Created but not tracked
│   // No tracking!                │
│ }                                │
│ destroy() { /* cards leaking */ }│ ← Memory leak!
└────────────────────────────────────┘

RIGHT:
┌────────────────────────────────────┐
│ for (int i = 0; i < 100; i++) {  │
│   lv_obj_t* card = ...create();  │
│   getObjectTracker().track(card);│ ← Track it
│ }                                │
│ destroy() {                      │
│   getObjectTracker().cleanup();  │ ← Auto-delete all 100!
│ }                                │
└────────────────────────────────────┘
```

---

## Cleanup Pattern Decision Matrix

```
                           ┌─ Is it dynamic? ──────────┐
                           │ (created at runtime)       │
                           │                            │
                      YES  │  NO                        │
                           │                            │
                    ┌──────┴─────────┐                   │
                    │                │                   │
              Use tracker        No tracker              │
                    │                │                   │
          ┌──────────────────┐  ┌─── ├─────┐            │
          │ Do you have      │  │    │ Use │            │
          │ >10 dynamic?     │  │    │ Pat │            │
          │                 │  │    │ tern│            │
          │  YES    NO      │  │    │  1  │            │
          │                 │  │    ├─────┘            │
          ├─ ├──────┐       │  │    │ Use              │
          │  │Use   │       │  │    │ Pattern 1        │
          │  │Pattern       │  │    │ (Root delete)    │
          │  │  2           │  │    │                  │
          │  │(Tracker)     │  │    │ OR Pattern 2     │
          │  │              │  │    │ if mixed         │
          │  └──────┘       │  │    │ dynamic/          │
          └─────────────────┘  │    │ static           │
                               └────┴─────────────────── │
```

---

## File Reference Guide

```
src/systems/ui/
├── View.h                    ◄─ Base class (add tracker)
├── UIObjectTracker.h         ◄─ Object tracking (RAII)
├── UICleanup.h               ◄─ Helper functions
└── UICleanupPatterns.h       ◄─ Pattern documentation

src/scenes/
├── game/GameView.cpp         ◄─ Reference: Pattern 1
├── gamelist/GameList...cpp   ◄─ Reference: Pattern 2 (tracker)
├── boot/BootView.cpp         ◄─ Reference: Pattern 1 + animation
└── [others]/...View.cpp      ◄─ All use Pattern 1

docs/
├── UI_CLEANUP_SYSTEM.md      ◄─ Full documentation
└── UI_CLEANUP_QUICK_...md    ◄─ Quick reference
```

---

## Summary

```
Old Way:                          New Way:
────────────────────────────────────────────────
lv_obj_del(root);              getObjectTracker().cleanup();
root = nullptr;         ──►     UICleanup::deleteRoot(root);
button = nullptr;               button = nullptr;
label = nullptr;                label = nullptr;
[repeat 10+ times]

Result:                          Result:
- Error-prone                    - Clear, safe
- Hard to maintain               - Auto-cascade
- Manual cascade                 - LIFO guaranteed
- Use-after-free risk            - Scales well
```

Now you understand the full picture! 🎯
