------------------------------------------------
-- ARDUINO-LIKE CONSTANTS
------------------------------------------------
local Arduino = {
    LOW = 0x0,
    HIGH = 0x1,

    INPUT = 0x01,
    OUTPUT = 0x03,
    PULLUP = 0x04,
    INPUT_PULLUP = 0x05,
    PULLDOWN = 0x08,
    INPUT_PULLDOWN = 0x09,
    OPEN_DRAIN = 0x10,
    OUTPUT_OPEN_DRAIN = 0x13,
    ANALOG = 0xC0
}

--------------------------------------------------
-- FRAMEWORK TEST
--------------------------------------------------
local state = {
    initialized = false,
    lastLogTime = 0
}
local timer = 0

--------------------------------------------------
-- CONFIG
--------------------------------------------------

local CONFIG = {
    input1 = 40,
    input2 = 41,
    input3 = 42
}

--------------------------------------------------
-- INIT
--------------------------------------------------

function init()
    system.info("Framework test started")

    input.channels.pinMode(CONFIG.input1, Arduino.INPUT_PULLUP)
    input.channels.pinMode(CONFIG.input2, Arduino.INPUT_PULLUP)
    input.channels.pinMode(CONFIG.input3, Arduino.INPUT_PULLUP)

    state.initialized = true

    system.info("Inputs configured")
end

--------------------------------------------------
-- UPDATE
--------------------------------------------------

function update(dt)

    if not state.initialized then
        system.warn("Update called before initialization, ignoring")
        return
    end

    local ch1 = input.channels.digitalRead(CONFIG.input1)
    local ch2 = input.channels.digitalRead(CONFIG.input2)
    local ch3 = input.channels.digitalRead(CONFIG.input3)
    
    --------------------------------------------------
    -- Log input changes
    --------------------------------------------------

    if not ch1 then
        system.info("Channel 1 ACTIVE")
    end

    if not ch2 then
        system.info("Channel 2 ACTIVE")
    end

    if not ch3 then
        system.info("Channel 3 ACTIVE")
    end

    --------------------------------------------------
    -- Periodic heartbeat
    --------------------------------------------------

    timer = timer + dt

    if timer >= 5.0 then
        timer = 0

        system.info(string.format("Heartbeat - CH1=%s CH2=%s CH3=%s", tostring(ch1), tostring(ch2), tostring(ch3)))
    end
end
