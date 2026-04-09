/**
 * @file UICleanupPatterns.h
 * @brief Best practices for managing LVGL object lifecycles in Views
 * 
 * This file demonstrates recommended patterns for object cleanup in view/scene code.
 * 
 * ============================================================================
 * ## PATTERN 1: Simple Root-Based Cleanup (RECOMMENDED for most views)
 * ============================================================================
 * 
 * Use when: View has a simple object hierarchy (screen as root, children as members)
 * 
 * class MyView : public View {
 *     lv_obj_t* screen = nullptr;
 *     lv_obj_t* button = nullptr;
 *     lv_obj_t* label = nullptr;
 *     
 *     void create() override {
 *         screen = Screen::create();
 *         button = Button::create(screen, ...);
 *         label = Label::create(screen, ...);
 *     }
 *     
 *     void destroy() override {
 *         // Option A: Delete root (cascades to all children)
 *         UICleanup::deleteRoot(screen);
 *         
 *         // Option B: Explicit cleanup (if you need special handling)
 *         UICleanup::deleteAndNull(button);    // deletes button's children first
 *         UICleanup::deleteAndNull(label);
 *         UICleanup::deleteAndNull(screen);
 *     }
 * };
 * 
 * ============================================================================
 * ## PATTERN 2: Tracker-Based Cleanup (For dynamic/runtime-created objects)
 * ============================================================================
 * 
 * Use when: View creates objects dynamically (e.g., list items, popup dialogs)
 * 
 * class GameListView : public View {
 *     std::vector<lv_obj_t*> cards;
 *     
 *     void create() override {
 *         lv_obj_t* screen = Screen::create();
 *         getObjectTracker().track(screen);  // Track root
 *         
 *         // Create dynamic items
 *         for (auto& game : games) {
 *             lv_obj_t* card = lv_btn_create(screen);
 *             getObjectTracker().track(card);
 *             cards.push_back(card);
 *         }
 *     }
 *     
 *     void destroy() override {
 *         // Automatic! Tracker cleans up all tracked objects
 *         getObjectTracker().cleanup();
 *         cards.clear();  // Clear our own references
 *     }
 * };
 * 
 * ============================================================================
 * ## PATTERN 3: Mixed Pattern (Static + Dynamic objects)
 * ============================================================================
 * 
 * Use when: View has both static structure and dynamic content
 * 
 * class GameView : public View {
 *     // Static structure
 *     lv_obj_t* screen = nullptr;
 *     lv_obj_t* header = nullptr;
 *     
 *     // Dynamic content (created at runtime)
 *     std::vector<lv_obj_t*> dynamicButtons;
 *     
 *     void create() override {
 *         screen = Screen::create();
 *         header = BrandHeader::create(screen);
 *         
 *         // Track root so everything is cleaned recursively
 *         getObjectTracker().track(screen);
 *     }
 *     
 *     void destroy() override {
 *         // Option 1: Let tracker handle everything (simplest)
 *         getObjectTracker().cleanup();
 *         
 *         // Option 2: Manual cleanup with tracker assistance
 *         for (auto btn : dynamicButtons) {
 *             getObjectTracker().deleteTracked(btn);
 *         }
 *         dynamicButtons.clear();
 *         
 *         UICleanup::deleteAndNull(screen);
 *     }
 * };
 * 
 * ============================================================================
 * ## PATTERN 4: Hierarchical Cleanup (Complex multi-section views)
 * ============================================================================
 * 
 * Use when: View has multiple independent sections that can be destroyed separately
 * 
 * class ComplexView : public View {
 *     // Section 1: Header
 *     struct {
 *         lv_obj_t* container = nullptr;
 *         lv_obj_t* title = nullptr;
 *         lv_obj_t* button = nullptr;
 *     } header;
 *     
 *     // Section 2: Content
 *     struct {
 *         lv_obj_t* container = nullptr;
 *         lv_obj_t* list = nullptr;
 *     } content;
 *     
 *     lv_obj_t* screen = nullptr;
 *     
 *     void destroyHeader() {
 *         UICleanup::deleteRoot(header.container);
 *         header.title = nullptr;
 *         header.button = nullptr;
 *     }
 *     
 *     void destroyContent() {
 *         UICleanup::clearChildren(content.container);
 *         UICleanup::deleteAndNull(content.container);
 *     }
 *     
 *     void destroy() override {
 *         destroyHeader();
 *         destroyContent();
 *         UICleanup::deleteAndNull(screen);
 *     }
 * };
 * 
 * ============================================================================
 * ## PATTERN 5: Exception-Safe Cleanup (For complex error handling)
 * ============================================================================
 * 
 * Use when: View creation can fail mid-way and needs rollback
 * 
 * void create() override {
 *     try {
 *         screen = Screen::create();
 *         if (!screen) throw std::runtime_error("Screen creation failed");
 *         
 *         lv_obj_t* btn = Button::create(screen, ...);
 *         if (!btn) throw std::runtime_error("Button creation failed");
 *         
 *         getObjectTracker().track(screen);  // Only track if creation succeeded
 *     }
 *     catch (const std::exception& e) {
 *         // Clean up partial state
 *         UICleanup::deleteRoot(screen);
 *         throw;
 *     }
 * }
 * 
 * ============================================================================
 * ## KEY PRINCIPLES
 * ============================================================================
 * 
 * 1. **Delete Hierarchy Last**: Always delete parent objects last
 *    - LVGL deletion cascades to children
 *    - Tracking ensures LIFO (Last In, First Out) order
 * 
 * 2. **Use Tracker for Dynamic Objects**: Dynamic lists/grids should be tracked
 *    - Handles variable object counts
 *    - Preserves cleanup order automatically
 * 
 * 3. **Nullify After Delete**: Prevent use-after-free bugs
 *    - deleteAndNull() and deleteRoot() do this automatically
 *    - Manual cleanup: set ptrs to nullptr after lv_obj_del()
 * 
 * 4. **One Cleanup Path**: Don't have multiple destroy() paths
 *    - Easier to maintain
 *    - Less error-prone
 * 
 * 5. **Track Roots, Not Leaves**: Track top-level objects
 *    - Reduces tracking overhead
 *    - Children auto-delete via cascade
 * 
 * 6. **Document Object Ownership**: In class comments, state what owns what
 *    - "Owned by screen (cascades)"
 *    - "Owned by view (manually deleted)"
 * 
 * ============================================================================
 * ## MIGRATION CHECKLIST
 * ============================================================================
 * 
 * When refactoring existing views:
 * 
 * ☐ Add includes: UICleanup.h and UIObjectTracker.h (in View.h)
 * ☐ Identify object hierarchy (what owns what)
 * ☐ Choose cleanup pattern (usually Pattern 1)
 * ☐ Update destroy() to use cleanup helpers
 * ☐ Test that all objects are properly deleted (memory check)
 * ☐ Remove manual std::find and pointer searches
 * ☐ Remove redundant object deletion code
 * 
 * ============================================================================
 */
