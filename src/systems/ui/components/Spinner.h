#pragma once

#include "lvgl.h"
#include <utility>

/**
 * @brief Rotating spinner component for loading/progress indication
 * 
 * Factory-style component that creates an animated spinning object.
 * Useful for indicating ongoing processes (loading, processing, etc.)
 * 
 * Usage:
 *   lv_obj_t* spinner = Spinner::create(parent, 50, 50);  // 50x50 spinner
 *   Spinner::start(spinner, 600);  // Rotate 360° in 600ms
 *   
 *   // Later:
 *   Spinner::stop(spinner);  // Stop animation
 *   
 *   // On cleanup:
 *   // Animation auto-stopped when parent deleted (cascade)
 */
class Spinner
{
public:
    /**
     * @brief Create a spinning object (typically an image or shape)
     * @param parent Parent container
     * @param width Width in pixels
     * @param height Height in pixels
     * @param imagePath Optional image file path (e.g., "S:/spinner.png")
     *                   If nullptr, creates a simple rotating shape
     * @return Created spinner object
     * 
     * Note: Spinner starts in idle state (not rotating)
     * Call start() to begin animation
     */
    static lv_obj_t* create(lv_obj_t* parent, lv_coord_t width, lv_coord_t height, 
                             const char* imagePath = nullptr)
    {
        if (!parent)
            return nullptr;

        lv_obj_t* spinner = nullptr;

        if (imagePath)
        {
            // Create image spinner
            spinner = lv_img_create(parent);
            lv_img_set_src(spinner, imagePath);
            lv_obj_set_size(spinner, width, height);
        }
        else
        {
            // Create shape spinner (rotating rectangle)
            spinner = lv_obj_create(parent);
            lv_obj_set_size(spinner, width, height);
            lv_obj_set_style_bg_color(spinner, lv_color_hex(0x00BFD8), 0);
            lv_obj_set_style_radius(spinner, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(spinner, 4, 0);
            lv_obj_set_style_border_color(spinner, lv_color_hex(0x00BFD8), 0);
            lv_obj_set_style_bg_opa(spinner, LV_OPA_TRANSP, 0);
        }

        // Store animation data in user_data for lifecycle management
        lv_anim_t* anim = new lv_anim_t();
        lv_obj_set_user_data(spinner, (void*)anim);

        return spinner;
    }

    /**
     * @brief Start spinning animation
     * @param spinner Spinner object created by create()
     * @param duration Time in milliseconds for one full rotation (360°)
     * @param repeatCount Number of times to repeat (LV_ANIM_REPEAT_INFINITE for endless)
     */
    static void start(lv_obj_t* spinner, uint32_t duration = 600, 
                      uint32_t repeatCount = LV_ANIM_REPEAT_INFINITE)
    {
        if (!spinner)
            return;

        // Stop any existing animation
        stop(spinner);

        lv_anim_t* anim = (lv_anim_t*)lv_obj_get_user_data(spinner);
        if (!anim)
        {
            anim = new lv_anim_t();
            lv_obj_set_user_data(spinner, (void*)anim);
        }

        lv_anim_init(anim);
        lv_anim_set_var(anim, spinner);
        
        // Rotation callback: set angle
        lv_anim_set_exec_cb(anim, [](void* obj, int32_t angle)
        {
            if (lv_img_get_src((lv_obj_t*)obj))
            {
                // Image: use rotation API
                lv_img_set_angle((lv_obj_t*)obj, angle);
            }
            else
            {
                // Shape: use transform rotation
                lv_obj_set_style_transform_rotation((lv_obj_t*)obj, angle, 0);
            }
        });

        // Rotate 360 degrees (3600 in LVGL units = 360 degrees)
        lv_anim_set_values(anim, 0, 3600);
        lv_anim_set_time(anim, duration);
        lv_anim_set_repeat_count(anim, repeatCount);
        lv_anim_set_repeat_delay(anim, 0);  // No delay between repeats
        lv_anim_set_path_cb(anim, lv_anim_path_linear);

        lv_anim_start(anim);
    }

    /**
     * @brief Stop spinning animation
     * @param spinner Spinner object
     */
    static void stop(lv_obj_t* spinner)
    {
        if (!spinner)
            return;

        // Find and delete animation for this object
        lv_anim_del(spinner, nullptr);

        // Reset angle to 0
        if (lv_img_get_src(spinner))
        {
            lv_img_set_angle(spinner, 0);
        }
        else
        {
            lv_obj_set_style_transform_rotation(spinner, 0, 0);
        }
    }

    /**
     * @brief Set spinner color
     * @param spinner Spinner object
     * @param color Color in 0xRRGGBB format
     */
    static void setColor(lv_obj_t* spinner, lv_color_t color)
    {
        if (!spinner)
            return;

        if (!lv_img_get_src(spinner))
        {
            // Shape spinner
            lv_obj_set_style_bg_color(spinner, color, 0);
            lv_obj_set_style_border_color(spinner, color, 0);
        }
    }

    /**
     * @brief Set spinner border width (for shape spinners)
     * @param spinner Spinner object
     * @param borderWidth Border width in pixels
     */
    static void setBorderWidth(lv_obj_t* spinner, lv_coord_t borderWidth)
    {
        if (!spinner)
            return;

        if (!lv_img_get_src(spinner))
        {
            lv_obj_set_style_border_width(spinner, borderWidth, 0);
        }
    }

    /**
     * @brief Clean up spinner resources
     * Call this before deleting the spinner or in view destroy()
     * @param spinner Spinner object
     */
    static void cleanup(lv_obj_t* spinner)
    {
        if (!spinner)
            return;

        stop(spinner);

        // Free allocated animation
        lv_anim_t* anim = (lv_anim_t*)lv_obj_get_user_data(spinner);
        if (anim)
        {
            delete anim;
            lv_obj_set_user_data(spinner, nullptr);
        }
    }
    /**
     * @brief Set animation duration at runtime
     * Useful for speed control (user can change)
     * @param spinner Spinner object
     * @param duration Time in milliseconds for one rotation
     * 
     * Note: Stops current animation and restarts with new duration
     */
    static void setDuration(lv_obj_t* spinner, uint32_t duration)
    {
        if (!spinner)
            return;

        // Stop current animation and restart with new duration
        stop(spinner);
        start(spinner, duration);
    }
};
