#pragma once

#include <string>
#include <memory>

/**
 * @brief Base class for all UI events
 * 
 * UI events are emitted by views and handled by scenes.
 * Each event type should inherit from this base class.
 */
class UIEvent
{
public:
    virtual ~UIEvent() = default;

    /**
     * @brief Get the event type identifier
     * @return String identifier for the event type
     */
    virtual const char* getType() const = 0;

    /**
     * @brief Clone the event (for queueing)
     * @return Unique pointer to cloned event
     */
    virtual std::unique_ptr<UIEvent> clone() const = 0;
};

/**
 * @brief Template for creating event types
 * 
 * Usage:
 *   class ButtonClickedEvent : public UIEventType<ButtonClickedEvent> {
 *   public:
 *       static constexpr const char* TYPE = "button_clicked";
 *       std::string buttonId;
 *   };
 */
template<typename T>
class UIEventType : public UIEvent
{
public:
    const char* getType() const override {
        return T::TYPE;
    }

    std::unique_ptr<UIEvent> clone() const override {
        return std::unique_ptr<UIEvent>(new T(static_cast<const T&>(*this)));
    }
};

/**
 * @brief Event handler interface
 * 
 * Scenes implement this to handle UI events from their views.
 */
class UIEventHandler
{
public:
    virtual ~UIEventHandler() = default;

    /**
     * @brief Handle a UI event
     * @param event The event to handle
     * @return true if event was handled, false otherwise
     */
    virtual bool handleUIEvent(const UIEvent& event) = 0;
};

/**
 * @brief Event emitter interface
 * 
 * Views implement this to emit events to their scenes.
 */
class UIEventEmitter
{
public:
    virtual ~UIEventEmitter() = default;

    /**
     * @brief Set the event handler (typically the scene)
     * @param handler The handler to receive events
     */
    virtual void setEventHandler(UIEventHandler* handler) = 0;

    /**
     * @brief Emit an event to the registered handler
     * @param event The event to emit
     */
    virtual void emitEvent(const UIEvent& event) = 0;
};
