#pragma once

#include "lvgl.h"
#include <string>

namespace
{
    inline std::string normalizeSpinnerSrc(const char *src)
    {
        if (!src || !*src)
            return {};

        if (((src[0] >= 'A' && src[0] <= 'Z') ||
             (src[0] >= 'a' && src[0] <= 'z')) &&
            src[1] == ':')
        {
            return src;
        }

        return std::string("S:") + src;
    }

    inline bool isImageSpinner(lv_obj_t *obj)
    {
        return obj && lv_obj_check_type(obj, &lv_image_class);
    }
}

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
    static lv_obj_t *create(
        lv_obj_t *parent,
        lv_coord_t width,
        lv_coord_t height,
        const char *imagePath = nullptr)
    {
        if (!parent)
            return nullptr;

        lv_obj_t *spinner = nullptr;

        if (imagePath)
        {
            // Create image spinner
            spinner = lv_image_create(parent);

            const std::string resolvedSrc = normalizeSpinnerSrc(imagePath);

            lv_image_set_src(spinner, resolvedSrc.c_str());
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

        return spinner;
    }

    /**
     * @brief Start spinning animation
     * @param spinner Spinner object created by create()
     * @param duration Time in milliseconds for one full rotation (360°)
     * @param repeatCount Number of times to repeat (LV_ANIM_REPEAT_INFINITE for endless)
     */
    static void start(
        lv_obj_t *spinner,
        uint32_t duration = 600,
        uint32_t repeatCount = LV_ANIM_REPEAT_INFINITE)
    {
        if (!spinner)
            return;

        stop(spinner);

        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, spinner);
        lv_anim_set_exec_cb(
            &anim,
            [](void *obj, int32_t angle)
            {
                lv_obj_t *spinnerObj = static_cast<lv_obj_t *>(obj);
                if (isImageSpinner(spinnerObj))
                {
                    lv_image_set_rotation(spinnerObj, angle);
                }
                else
                {
                    lv_obj_set_style_transform_rotation(spinnerObj, angle, 0);
                }
            });

        lv_anim_set_values(&anim, 0, 3600);
        lv_anim_set_time(&anim, duration);
        lv_anim_set_repeat_count(&anim, repeatCount);
        lv_anim_set_repeat_delay(&anim, 0);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear);

        lv_anim_start(&anim);
    }

    /**
     * @brief Stop spinning animation
     * @param spinner Spinner object
     */
    static void stop(lv_obj_t *spinner)
    {
        if (!spinner)
            return;

        lv_anim_del(spinner, nullptr);

        if (isImageSpinner(spinner))
        {
            lv_image_set_rotation(spinner, 0);
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
    static void setColor(lv_obj_t *spinner, lv_color_t color)
    {
        if (!spinner)
            return;

        if (!isImageSpinner(spinner))
        {
            lv_obj_set_style_bg_color(spinner, color, 0);
            lv_obj_set_style_border_color(spinner, color, 0);
        }
    }

    /**
     * @brief Set spinner border width (for shape spinners)
     * @param spinner Spinner object
     * @param borderWidth Border width in pixels
     */
    static void setBorderWidth(lv_obj_t *spinner, lv_coord_t borderWidth)
    {
        if (!spinner)
            return;

        if (!isImageSpinner(spinner))
        {
            lv_obj_set_style_border_width(spinner, borderWidth, 0);
        }
    }

    /**
     * @brief Clean up spinner resources
     * Call this before deleting the spinner or in view destroy()
     * @param spinner Spinner object
     */
    static void cleanup(lv_obj_t *spinner)
    {
        if (!spinner)
            return;

        lv_anim_del(spinner, nullptr);
    }

    /**
     * @brief Set animation duration at runtime
     * Useful for speed control (user can change)
     * @param spinner Spinner object
     * @param duration Time in milliseconds for one rotation
     *
     * Note: Stops current animation and restarts with new duration
     */
    static void setDuration(lv_obj_t *spinner, uint32_t duration)
    {
        if (!spinner)
            return;

        // Stop current animation and restart with new duration
        stop(spinner);
        start(spinner, duration);
    }
};