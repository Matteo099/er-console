--------------------------------------------------
-- ESCAPE ROOM GAME
-- Prototype
--------------------------------------------------
--------------------------------------------------
-- GAME STATE
--------------------------------------------------
local state = {
    running = false,
    completed = false,

    startTime = 0,
    elapsed = 0,

    -- puzzles
    powerRestored = false,
    rfidAccepted = false,
    buttonsSolved = false,

    -- electronic locks
    doorUnlocked = false,

    -- timers
    messageTimer = 0,

    -- input memory
    lastRFID = nil,

    -- buttons
    button1 = false,
    button2 = false,
    button3 = false
}

--------------------------------------------------
-- HARDWARE CONFIGURATION
--------------------------------------------------
local CONFIG = {
    -- RFID
    accessCard = "A1B2C3D4",

    -- Channels
    button1 = 1,
    button2 = 2,
    button3 = 3,

    -- Relay
    doorRelay = 0,

    -- Power monitor threshold
    minVoltage = 11.5
}

--------------------------------------------------
-- UTILITIES
--------------------------------------------------
local function display(text)
    input.oled.clear()
    input.oled.showMessage(text)
end

local function logState()
    log.debug("STATE power=%s rfid=%s buttons=%s door=%s", tostring(state.powerRestored), tostring(state.rfidAccepted),
        tostring(state.buttonsSolved), tostring(state.doorUnlocked))
end

local function unlockDoor()
    if state.doorUnlocked then
        return
    end

    log.info("Unlocking escape room door")
    input.relays.set(CONFIG.doorRelay, true)
    state.doorUnlocked = true
    display("DOOR OPEN")
end

--------------------------------------------------
-- INITIALIZATION
--------------------------------------------------
function init()
    log.info("Escape room initialization")
    state.running = true
    state.startTime = time.millis()

    --
    -- Configure channels
    --
    input.channels.pinMode(CONFIG.button1, INPUT)
    input.channels.pinMode(CONFIG.button2, INPUT)
    input.channels.pinMode(CONFIG.button3, INPUT)

    --
    -- Lock door
    --
    input.relays.set(CONFIG.doorRelay, false)
    display("ESCAPE ROOM\nSTART")
    log.info("Game started")
end

--------------------------------------------------
-- UPDATE LOOP
--------------------------------------------------

function update(dt)
    if not state.running then
        return
    end
    state.elapsed = time.millis() - state.startTime

    --------------------------------------------------
    -- POWER PUZZLE
    --------------------------------------------------
    local power = input.power.metrics()
    if power ~= nil then
        if power.voltage > CONFIG.minVoltage then
            if not state.powerRestored then
                log.info("Power restored")
                state.powerRestored = true
                display("POWER OK")
            end
        end
    end

    --------------------------------------------------
    -- RFID PUZZLE
    --------------------------------------------------
    local tag = input.rfid.consume()
    if tag ~= nil then
        log.info("RFID detected %s", tag.uid)
        state.lastRFID = tag.uid
        if tag.uid == CONFIG.accessCard then
            state.rfidAccepted = true
            display("CARD ACCEPTED")
            log.info("Correct RFID")
        else
            display("WRONG CARD")
            log.warn("Invalid RFID")
        end
    end

    --------------------------------------------------
    -- BUTTON PUZZLE
    --------------------------------------------------
    state.button1 = input.channels.digitalRead(CONFIG.button1)
    state.button2 = input.channels.digitalRead(CONFIG.button2)
    state.button3 = input.channels.digitalRead(CONFIG.button3)
    if state.button1 and state.button2 and state.button3 then
        if not state.buttonsSolved then
            state.buttonsSolved = true
            log.info("Button sequence solved")
            display("BUTTONS OK")
        end
    end

    --------------------------------------------------
    -- FINAL CONDITION
    --------------------------------------------------
    if state.powerRestored and state.rfidAccepted and state.buttonsSolved then
        unlockDoor()
        state.completed = true
    end

    --------------------------------------------------
    -- DEBUG
    --------------------------------------------------
    if state.elapsed % 10000 < 20 then
        logState()
    end
end
