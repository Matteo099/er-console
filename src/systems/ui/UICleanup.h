#pragma once

#include "lvgl.h"
#include <utility>
#include <algorithm>

/**
 * @brief Utilities for managing LVGL object lifecycles
 * 
 * Provides helper functions for safe object cleanup patterns
 */
namespace UICleanup
{
    /**
     * @brief Safely delete an LVGL object and nullify its pointer
     * @param obj Reference to object pointer (will be set to nullptr)
     * 
     * Usage:
     *   UICleanup::deleteAndNull(myButton);  // myButton becomes nullptr
     */
    inline void deleteAndNull(lv_obj_t*& obj)
    {
        if (obj)
        {
            lv_obj_del(obj);
            obj = nullptr;
        }
    }

    /**
     * @brief Clear all children of an object
     * @param parent The parent object whose children to clear
     * 
     * Usage:
     *   UICleanup::clearChildren(container);
     */
    inline void clearChildren(lv_obj_t* parent)
    {
        if (parent)
        {
            lv_obj_clean(parent);
        }
    }

    /**
     * @brief Recursively delete object hierarchy and nullify all pointers in the group
     * This is useful when you have related objects that form a logical group
     * 
     * @param root The root object of the hierarchy
     * @param objects Variadic list of object pointers to nullify
     * 
     * Usage:
     *   UICleanup::deleteHierarchy(screen, btnStart, btnCancel, lblTitle);
     */
    inline void deleteHierarchy(lv_obj_t*& root)
    {
        deleteAndNull(root);
    }

    // Template recursion for variadic cleanup
    template<typename First, typename... Rest>
    inline void deleteHierarchy(lv_obj_t*& root, First& obj, Rest&... rest)
    {
        deleteAndNull(obj);
        deleteHierarchy(root, rest...);
    }

    /**
     * @brief Helper to delete root object, which cascades to all children
     * Automatically nullifies the pointer
     * 
     * Usage:
     *   UICleanup::deleteRoot(myScreen);
     */
    inline void deleteRoot(lv_obj_t*& root)
    {
        if (root)
        {
            // Deleting root cascades to all children due to LVGL hierarchy
            lv_obj_del(root);
            root = nullptr;
        }
    }

    /**
     * @brief Batch nullify multiple object pointers (useful after cascade delete)
     * 
     * Usage:
     *   UICleanup::nullifyAll(btn1, btn2, btn3, lbl);
     */
    inline void nullifyAll() {}  // Base case for recursion

    template<typename First, typename... Rest>
    inline void nullifyAll(First& obj, Rest&... rest)
    {
        obj = nullptr;
        nullifyAll(rest...);
    }

    /**
     * @brief Check if object is still valid (not deleted by parent)
     * @return true if parent exists (object is in hierarchy), false if orphaned/deleted
     */
    inline bool isValid(lv_obj_t* obj)
    {
        return obj && lv_obj_get_parent(obj) != nullptr;
    }

    /**
     * @brief Count the depth of an object in the hierarchy
     * @return Depth from root (0 = root, 1 = direct child, etc.)
     */
    inline int getDepth(lv_obj_t* obj)
    {
        int depth = 0;
        while (obj && lv_obj_get_parent(obj) != nullptr)
        {
            obj = lv_obj_get_parent(obj);
            depth++;
        }
        return depth;
    }

    /**
     * @brief Find common parent of two objects
     * @return nullptr if no common parent or either input is invalid
     */
    inline lv_obj_t* findCommonParent(lv_obj_t* obj1, lv_obj_t* obj2)
    {
        if (!obj1 || !obj2)
            return nullptr;

        // Get all ancestors of obj1
        std::vector<lv_obj_t*> ancestors1;
        lv_obj_t* current = obj1;
        while (current)
        {
            ancestors1.push_back(current);
            current = lv_obj_get_parent(current);
        }

        // Walk up obj2's hierarchy until we find a common ancestor
        current = obj2;
        while (current)
        {
            if (std::find(ancestors1.begin(), ancestors1.end(), current) != ancestors1.end())
                return current;
            current = lv_obj_get_parent(current);
        }

        return nullptr;
    }
}
