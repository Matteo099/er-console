---@meta

-- AUTO GENERATED
-- DO NOT EDIT

---@alias DigitalValue integer

DigitalValue = {
    LOW = 0x0,
    HIGH = 0x1,
}

---@alias GameObjectContentType integer

GameObjectContentType = {
    TEXT = Text,
    IMAGE = Image,
}

---@alias PinMode integer

PinMode = {
    INPUT = 0x01,
    OUTPUT = 0x03,
    PULLUP = 0x04,
    INPUT_PULLUP = 0x05,
    PULLDOWN = 0x08,
    INPUT_PULLDOWN = 0x09,
    OPEN_DRAIN = 0x10,
    OUTPUT_OPEN_DRAIN = 0x13,
    ANALOG = 0xC0,
}

---@alias TriggerMode integer

TriggerMode = {
    RISING = 0x01,
    FALLING = 0x02,
    CHANGE = 0x03,
}

---@class audio
audio = {}

--- Play a beep.
---
---
--- Example:
--- audio.beep(1000, 250)
---@param frequency integer Tone frequency in Hz.
---@param duration integer Duration in milliseconds.
function audio.beep(frequency, duration) end

--- Get the current audio volume
---
---
--- Example:
--- currentVolume = audio.getVolume()
---@return integer Current volume value (0-100)
function audio.getVolume() end

--- Play a wav audio file from storage
---
---
--- Example:
--- audio.play("beep.wav")
--- audio.play("music.wav", true)
---@param path string Path to the audio file in storage
---@param loop any Optional. True to loop playback (default: false)
function audio.play(path, loop) end

--- Set the audio volume
---
---
--- Example:
--- audio.setVolume(80)
---@param volume integer The volume level (0-100)
function audio.setVolume(volume) end

--- Stop the currently playing audio
---
---
--- Example:
--- audio.stop()
function audio.stop() end

---@class input
input = {}

---@class input.channels
input.channels = {}

--- Read an analog value from a pin
---
---@param channel integer The channel number to read from
---@return integer The analog value read from the pin (0-255)
function input.channels.analogRead(channel) end

--- Write an analog value to a pin
---
---@param channel integer The channel number to write to
---@param value integer The value to write (0-255)
function input.channels.analogWrite(channel, value) end

--- Read a digital value from a pin
---
---@param channel integer The channel number to read from
---@return boolean The digital value read from the pin (true or false)
function input.channels.digitalRead(channel) end

--- Write a digital value to a pin
---
---@param channel integer The channel number to write to
---@param value any The value to write (true or false)
function input.channels.digitalWrite(channel, value) end

--- Set the mode of a digital pin.
---
---
--- Example:
--- input.channels.pinMode(4, PinMode.INPUT_PULLUP)
---@param channel integer The channel number to set the mode for.
---@param mode PinMode The mode to set.
function input.channels.pinMode(channel, mode) end

---@class input.oled
input.oled = {}

--- Clear the OLED display
---
---
--- Example:
--- input.oled.clear()
function input.oled.clear() end

--- Get the current message displayed on the OLED display
---
---
--- Example:
--- local message = input.oled.currentMessage()
---@return string
function input.oled.currentMessage() end

--- Check if the OLED display is supported
---
---
--- Example:
--- if input.oled.isSupported() then print("OLED is supported") else print("OLED is not supported") end
---@return boolean true if the OLED display is supported, false otherwise
function input.oled.isSupported() end

--- Display a message on the OLED display
---
---
--- Example:
--- input.oled.show("Hello, World!")
---@param text string The message to display
function input.oled.show(text) end

---@class input.relays
input.relays = {}

--- Get the state of a relay
---
---
--- Example:
--- local state = input.relays.get(0) -- get the state of relay 0
---@param relay integer The relay number (0-3)
---@return boolean The state of the relay (true for on, false for off)
function input.relays.get(relay) end

--- Pulse a relay for a specified duration
---
---
--- Example:
--- input.relays.pulse(0, 1000) -- pulse relay 0 for 1 second
---@param relay integer The relay number (0-3)
---@param duration integer The duration of the pulse in milliseconds
function input.relays.pulse(relay, duration) end

--- Set the state of a relay
---
---
--- Example:
--- input.relays.set(0, true) -- turn on relay 0
---@param relay integer The relay number (0-3)
---@param state boolean The state to set (true for on, false for off)
function input.relays.set(relay, state) end

--- Toggle the state of a relay
---
---
--- Example:
--- input.relays.toggle(0) -- toggle the state of relay 0
---@param relay integer The relay number (0-3)
function input.relays.toggle(relay) end

---@class input.rfid
input.rfid = {}

--- Consume the next available RFID tag
---
---
--- Example:
--- local tag = input.rfid.consume()
---@return table A table containing the tag information, or nil if no new tag is available
function input.rfid.consume() end

--- Get the last available RFID tag
---
---
--- Example:
--- local tag = input.rfid.last()
---@return table A table containing the tag information, or nil if no tag is available
function input.rfid.last() end

---@class input.rgb
input.rgb = {}

--- Set the brightness of a specific RGB LED
---
---
--- Example:
--- input.rgb.setBrightness(0, 128) -- sets LED 0 to half brightness
---@param led integer The LED number (0-3)
---@param value integer The brightness value (0-255)
function input.rgb.setBrightness(led, value) end

--- Set the color of a specific RGB LED
---
---
--- Example:
--- input.rgb.setColor(0, 255, 0, 0) -- sets LED 0 to red
---@param led integer The LED number (0-3)
---@param r integer The red component (0-255)
---@param g integer The green component (0-255)
---@param b integer The blue component (0-255)
function input.rgb.setColor(led, r, g, b) end

---@class input.sevenSegment
input.sevenSegment = {}

--- Set the brightness of a specific seven-segment display
---
---
--- Example:
--- input.sevenSegment.brightness(0, 128) -- sets display 0 to half brightness
---@param display integer The display number (0-3)
---@param value integer The brightness value (0-255)
function input.sevenSegment.brightness(display, value) end

--- Clear a specific seven-segment display
---
---
--- Example:
--- input.sevenSegment.clear(0) -- clears display 0
---@param display integer The display number (0-3)
function input.sevenSegment.clear(display) end

--- Display a value on a specific seven-segment display
---
---
--- Example:
--- input.sevenSegment.display(0, "1234") -- displays "1234" on display 0
---@param display integer The display number (0-3)
---@param value string The value to display (string)
function input.sevenSegment.display(display, value) end

---@class scene
scene = {}

--- Complete the escape room successfully
---
---
--- Example:
--- scene.completeGame()
function scene.completeGame() end

--- Set the interactive objects shown in the game object viewer.
--- Each object contains:
--- title: string
--- content: array of blocks
--- A content block can be:
--- {
--- type = GameObjectContentType.TEXT,
--- value = "some paragraph"
--- }
--- or:
--- {
--- type = GameObjectContentType.IMAGE,
--- path = "/images/map.png",
--- width = 200,
--- height = 120
--- }
--- Text and images can be mixed freely.
--- scene.setGameObjects({
--- {
--- title = "Ancient Map",
--- content = {
--- {
--- type = GameObjectContentType.TEXT,
--- value = "A mysterious map is found on the wall."
--- },
--- {
--- type = GameObjectContentType.IMAGE,
--- path = "/games/demo/map.png",
--- width = 220,
--- height = 160
--- },
--- {
--- type = GameObjectContentType.TEXT,
--- value = "The red mark indicates the hidden door."
--- }
--- }
--- }
--- })
---
---@param objects table Array of game objects.
function scene.setGameObjects(objects) end

---@class system
system = {}

--- Clear the recent logs buffer
---
---
--- Example:
--- system.clearLogs()
function system.clearLogs() end

--- Log a debug message to the console
---
---
--- Example:
--- system.debug("Player position: x=10, y=20")
---@param msg string The debug message to log
function system.debug(msg) end

--- Log an error message to the console
---
---
--- Example:
--- system.error("RFID reader unavailable")
---@param msg string The error message to log
function system.error(msg) end

--- Log an info message to the console
---
---
--- Example:
--- system.info("Hello from Lua!")
---@param msg string The info message to log
function system.info(msg) end

--- Get recent logs as a string
---
---
--- Example:
--- system.logs(10)
---@return string A string containing the recent logs
function system.logs() end

--- Log a warning message to the console
---
---
--- Example:
--- system.warn("Low power")
---@param msg string The warning message to log
function system.warn(msg) end

---@class time
time = {}

--- Get the current time in HH:MM:SS format
---
---
--- Example:
--- local currentTime = time.clock()
---@return string The current time in HH:MM:SS format
function time.clock() end

--- Get the frame delta time
---
---
--- Example:
--- local delta = time.delta()
---@return any The frame delta time
function time.delta() end

--- Get the current time in milliseconds since boot
---
---
--- Example:
--- local currentTime = time.now()
---@return any The current time in milliseconds since boot
function time.now() end

--- Get the current time in seconds since boot
---
---
--- Example:
--- local currentSeconds = time.seconds()
---@return any The current time in seconds since boot
function time.seconds() end

--- Get the uptime in seconds
---
---
--- Example:
--- local uptime = time.uptime()
---@return any The uptime in seconds
function time.uptime() end

