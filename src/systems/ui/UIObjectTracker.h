#pragma once

#include "lvgl.h"
#include <vector>
#include <memory>
#include <algorithm>

/**
 * @brief Manages LVGL object lifecycle and hierarchical cleanup
 * 
 * Provides RAII-style object management with automatic parent/child tracking.
 * Objects are destroyed in reverse order of creation (leaf to root).
 * 
 * Usage:
 *   UIObjectTracker tracker;
 *   lv_obj_t* btn = tracker.track(Button::create(...));
 *   // On view destruction, tracker auto-destroys all tracked objects
 */
class UIObjectTracker
{
public:
    UIObjectTracker() = default;
    
    // Prevent copying (objects held by tracker shouldn't be duplicated)
    UIObjectTracker(const UIObjectTracker&) = delete;
    UIObjectTracker& operator=(const UIObjectTracker&) = delete;
    
    // Support move semantics
    UIObjectTracker(UIObjectTracker&& other) noexcept 
        : objects(std::move(other.objects)) {}
    
    UIObjectTracker& operator=(UIObjectTracker&& other) noexcept 
    {
        if (this != &other) {
            cleanup();
            objects = std::move(other.objects);
        }
        return *this;
    }
    
    ~UIObjectTracker()
    {
        cleanup();
    }

    /**
     * @brief Track an LVGL object for automatic cleanup
     * @param obj The object to track (typically a root object for a view section)
     * @return The same object pointer (for pass-through usage)
     */
    lv_obj_t* track(lv_obj_t* obj)
    {
        if (obj)
        {
            objects.push_back(obj);
        }
        return obj;
    }

    /**
     * @brief Track multiple objects at once
     * @param objs Vector of objects to track
     */
    void trackAll(const std::vector<lv_obj_t*>& objs)
    {
        for (lv_obj_t* obj : objs)
        {
            track(obj);
        }
    }

    /**
     * @brief Manually delete a specific tracked object and remove from tracking
     * @param obj The object to delete
     * @return true if object was tracked and deleted, false otherwise
     */
    bool deleteTracked(lv_obj_t* obj)
    {
        auto it = std::find(objects.begin(), objects.end(), obj);
        if (it != objects.end())
        {
            lv_obj_del(*it);
            objects.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Clean up all tracked objects in reverse order
     * Deleting in reverse order ensures children are cleaned before parents
     */
    void cleanup()
    {
        // Delete in reverse order (LIFO - last created, first destroyed)
        // This ensures child objects are cleaned up before their parents
        while (!objects.empty())
        {
            lv_obj_t* obj = objects.back();
            objects.pop_back();
            
            // Only delete if still valid (parent deletion cascades to children)
            // Check by trying to access the parent - if parent is nullptr, obj is already deleted
            if (obj && lv_obj_get_parent(obj) != nullptr)
            {
                lv_obj_del(obj);
            }
        }
    }

    /**
     * @brief Get the number of tracked objects
     */
    size_t getObjectCount() const
    {
        return objects.size();
    }

    /**
     * @brief Check if a specific object is being tracked
     */
    bool isTracked(lv_obj_t* obj) const
    {
        return std::find(objects.begin(), objects.end(), obj) != objects.end();
    }

    /**
     * @brief Clear all tracking without deleting objects
     * Use this if you're manually managing object lifetime
     */
    void clearTracking()
    {
        objects.clear();
    }

    /**
     * @brief Get all tracked objects (read-only)
     */
    const std::vector<lv_obj_t*>& getTrackedObjects() const
    {
        return objects;
    }

private:
    std::vector<lv_obj_t*> objects;
};
