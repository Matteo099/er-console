--------------------------------------------------
-- FRAMEWORK TEST
--------------------------------------------------
local state = {
    initialized = false,
    lastLogTime = 0,
    unlockCounter = 0,
}

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

    input.channels.pinMode(CONFIG.input1, PinMode.INPUT_PULLUP)
    input.channels.pinMode(CONFIG.input2, PinMode.INPUT_PULLUP)
    input.channels.pinMode(CONFIG.input3, PinMode.INPUT_PULLUP)

    state.initialized = true

    system.info("Inputs configured")
end


--------------------------------------------------
-- UPDATE
--------------------------------------------------

function update(dt, paused)
    
    if(paused) then
        return
    end

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

    if ch1 then
        system.info("Channel 1 PRESSED")
    end

    if ch2 then
        system.info("Channel 2 PRESSED")
    end

    if ch3 then
        system.info(string.format("Channel 3 PRESSED - %d/50", state.unlockCounter))
        state.unlockCounter = state.unlockCounter + 1
    end


    if state.unlockCounter >= 50 then
        system.info("Framework test complete, unlocking game")
        scene.completeGame()
    end

    --------------------------------------------------
    -- Periodic heartbeat
    --------------------------------------------------

    state.lastLogTime = state.lastLogTime + dt

    if state.lastLogTime >= 5.0 then
        state.lastLogTime = 0

        system.info(string.format("Heartbeat - CH1=%s CH2=%s CH3=%s UNLOCK=%d/50", tostring(ch1), tostring(ch2), tostring(ch3), state.unlockCounter))
    end
end


function exit()
    system.info("Framework test exiting")
    
end