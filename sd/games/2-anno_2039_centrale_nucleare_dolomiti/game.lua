-- Authored game text follows the language selected in Settings.
local messages = {
    title = {it = "Anno 2039: Centrale Dolomiti", en = "Year 2039: Dolomiti Power Plant"},
    quit = {it = "ESCI", en = "QUIT"},
    pause = {it = "PAUSA", en = "PAUSE"},
    resume = {it = "RIPRENDI", en = "RESUME"},
    edward = {it = "Edward sa, Edward sa, Edward sa, Edward sa, Edward sa", en = "Edward knows, Edward knows, Edward knows, Edward knows, Edward knows"},
    loaded = {it = "Anno 2039: script del gioco caricato", en = "Year 2039: game script loaded"},
}
local function text(key)
    return system.translate(messages[key])
end

-- Anno 2039: Centrale Nucleare Dolomiti - Physical Escape Room Game

local gameState = {
    initialized = false,
    paused = false,
    gameTime = 0,
    enigma1Solved = false,
    enigma2Solved = false,
    enigma3Solved = false,
    enigma4Solved = false,
    elevatorSequence = {1, 4, 1, 6},  -- line, square, line, hexagon
    elevatorInput = {},
    controlDoorInput = "",
    badgeDetected = false,
    lastBadgeId = "",
    doorOpenTimer = 0
}

-- GPIO pins (adjust as needed)
local pins = {
    buttonLine = 13,
    buttonSquare = 12,
    buttonHexagon = 14,
    buttonTwoLines = 27,  -- not used in sequence
    numPad4 = 4,
    numPad1 = 1,
    numPad5 = 5,
    numPad0 = 0,
    ledControlDoor = 15,
    actuatorElectroDoor = 16,  -- elevator door
    actuatorControlDoor = 17,
    sensorValve1 = 18,
    sensorValve2 = 19,
    sensorValve3 = 20,
    sensorValve4 = 21,
    rfidSensor = 22  -- placeholder
}

function init()
    display.text(10, 10, text("title"), 16, 0xFFD700)
    
    -- Quit button bottom left
    display.rect(10, 270, 100, 40, 0xFF0000, true)
    display.text(20, 280, text("quit"), 14, 0xFFFFFF)
    
    -- Pause/Resume button bottom right
    display.rect(210, 270, 100, 40, 0x00FF00, true)
    display.text(220, 280, text("pause"), 14, 0xFFFFFF)
    
    -- Set background black
    display.clear()
    
    -- Timer in center
    display.text(120, 150, "00:00:00", 20, 0xFFFFFF)
    
    -- Initialize GPIO
    input.pinMode(pins.buttonLine, 0)  -- INPUT
    input.pinMode(pins.buttonSquare, 0)
    input.pinMode(pins.buttonHexagon, 0)
    input.pinMode(pins.buttonTwoLines, 0)
    input.pinMode(pins.numPad4, 0)
    input.pinMode(pins.numPad1, 0)
    input.pinMode(pins.numPad5, 0)
    input.pinMode(pins.numPad0, 0)
    input.pinMode(pins.ledControlDoor, 1)  -- OUTPUT
    input.pinMode(pins.actuatorElectroDoor, 1)
    input.pinMode(pins.actuatorControlDoor, 1)
    input.pinMode(pins.sensorValve1, 0)
    input.pinMode(pins.sensorValve2, 0)
    input.pinMode(pins.sensorValve3, 0)
    input.pinMode(pins.sensorValve4, 0)
    
    gameState.initialized = true
end

function update(dt, paused)
    if not gameState.initialized then return end
    
    if not gameState.paused then
        gameState.gameTime = gameState.gameTime + dt
        -- Update timer display
        local timeStr = string.format("%02d:%02d:%02d", math.floor(gameState.gameTime / 3600), math.floor((gameState.gameTime % 3600) / 60), gameState.gameTime % 60)
        display.text(120, 150, timeStr, 20, 0xFFFFFF)
    end
    
    -- Check pause/resume button (assume touch or button press)
    -- For simplicity, assume onButtonPress handles it
    
    -- Check enigmas
    checkEnigma1()
    checkEnigma2()
    checkEnigma3()
    checkEnigma4()
    
    -- Handle door timers
    if gameState.doorOpenTimer > 0 then
        gameState.doorOpenTimer = gameState.doorOpenTimer - dt
        if gameState.doorOpenTimer <= 0 then
            input.digitalWrite(pins.actuatorControlDoor, 0)  -- close door
        end
    end
end

function checkEnigma1()
    -- Check elevator button sequence
    local input = {}
    if input.digitalRead(pins.buttonLine) == 1 then table.insert(input, 1) end
    if input.digitalRead(pins.buttonSquare) == 1 then table.insert(input, 4) end
    if input.digitalRead(pins.buttonHexagon) == 1 then table.insert(input, 6) end
    
    -- Simple sequence check (in practice, need debouncing and order)
    if #input >= #gameState.elevatorSequence then
        local match = true
        for i, v in ipairs(gameState.elevatorSequence) do
            if input[i] ~= v then match = false break end
        end
        if match and not gameState.enigma1Solved then
            gameState.enigma1Solved = true
            input.digitalWrite(pins.actuatorElectroDoor, 1)  -- open elevator
            audio.beep(1000, 500)
        end
    end
end

function checkEnigma2()
    -- Check numeric keypad sequence 4 1 5 0
    local input = ""
    if input.digitalRead(pins.numPad4) == 1 then input = input .. "4" end
    if input.digitalRead(pins.numPad1) == 1 then input = input .. "1" end
    if input.digitalRead(pins.numPad5) == 1 then input = input .. "5" end
    if input.digitalRead(pins.numPad0) == 1 then input = input .. "0" end
    
    if input == "4150" and not gameState.enigma2Solved then
        gameState.enigma2Solved = true
        input.digitalWrite(pins.ledControlDoor, 1)  -- turn on LED
        
        -- Check for badge
        local badgeId = input.rfidRead()
        if badgeId == "0001" then  -- specific ID
            input.digitalWrite(pins.actuatorControlDoor, 1)  -- open door
            gameState.doorOpenTimer = 5  -- 5 seconds
            audio.beep(1200, 300)
        end
    end
end

function checkEnigma3()
    if not (gameState.enigma1Solved and gameState.enigma2Solved) then return end
    
    -- Check if all 4 valve sensors are HIGH
    local allHigh = input.digitalRead(pins.sensorValve1) == 1 and
                     input.digitalRead(pins.sensorValve2) == 1 and
                     input.digitalRead(pins.sensorValve3) == 1 and
                     input.digitalRead(pins.sensorValve4) == 1
    
    if allHigh and not gameState.enigma3Solved then
        gameState.enigma3Solved = true
        input.oledPrint(text("edward"))
        audio.beep(800, 1000)
    end
end

function checkEnigma4()
    -- Placeholder for additional enigmas
end

function onButtonPress(buttonId)
    if buttonId == buttons.quit then
        game.quit()
    elseif buttonId == buttons.pause then
        gameState.paused = not gameState.paused
        display.text(220, 280, gameState.paused and text("resume") or text("pause"), 14, 0xFFFFFF)
    end
end

print(text("loaded"))