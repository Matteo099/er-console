-- Anno 2039: Centrale Nucleare Dolomiti - Physical Escape Room Game
-- Fusion reactor emergency: Physical room with props and connected buttons

-- Game state
local gameState = {
    currentRoom = "anticamera",  -- anticamera, controllo, nocciolo
    initialized = false,
    paused = false,
    gameTime = 0,  -- seconds elapsed
    
    -- Inventory (physical items collected)
    inventory = {},
    
    -- Puzzle states
    elevatorSequence = {1, 4, 1, 6},  -- N(1), H(4), Pt(1), Cl(6) -> line, square, line, hexagon
    elevatorInput = {},
    elevatorOpen = false,
    
    controlDoorCode = "4150",  -- Nb(41) + Sn(50)
    controlDoorInput = "",
    controlDoorOpen = false,
    
    -- Control room
    valves = {false, false, false, false},  -- Physical valves (simulated)
    cryostatPressure = 0,
    coolingRestored = false,
    
    -- Messages
    messages = {},
    warnings = {},
    errors = {}
}

-- Button mappings (physical buttons connected to ESP32)
local buttons = {
    -- Shape buttons for elevator
    line = 1,        -- I (1 line)
    twoLines = 2,    -- II (2 lines) 
    square = 4,      -- [] (square)
    hexagon = 6,     -- O (hexagon)
    
    -- Numeric buttons for door code
    num0 = 0, num1 = 1, num2 = 2, num3 = 3, num4 = 4,
    num5 = 5, num6 = 6, num7 = 7, num8 = 8, num9 = 9,
    
    -- Control buttons
    quit = 10,
    pause = 11
}

-- Initialize the game
function init()
    print("CND 2039: Physical escape room initializing...")
    gameState.initialized = true
    
    -- Initial display setup
    display.clear()
    
    -- Add initial messages
    addMessage("Reactor emergency activated")
    addWarning("Cryogenic system failure detected")
    addMessage("Maintenance team deployed")
    
    print("Physical game initialized - Starting in anticamera")
end

-- Update is called every frame
function update(dt)
    if not gameState.initialized then return end
    
    gameState.gameTime = gameState.gameTime + dt
    
    display.clear()
    
    -- Always show header with time and controls
    drawHeader()
    
    if gameState.paused then
        drawPaused()
        return
    end
    
    -- Show current room
    if gameState.currentRoom == "anticamera" then
        drawAnticamera()
    elseif gameState.currentRoom == "controllo" then
        drawControlRoom()
    elseif gameState.currentRoom == "nocciolo" then
        drawCoreRoom()
    end
    
    -- Show messages/warnings/errors
    drawMessages()
end

-- Draw header with time and control buttons
function drawHeader()
    -- Time
    local timeStr = time.get()
    display.text(10, 5, "Time: " .. timeStr, 12, 0xFFFFFF)
    
    -- Control buttons (shown as text since physical buttons exist)
    display.text(200, 5, "[Q] Quit  [P] Pause", 10, 0xCCCCCC)
    
    -- Game time
    local gameMinutes = math.floor(gameState.gameTime / 60)
    local gameSeconds = math.floor(gameState.gameTime % 60)
    display.text(10, 20, string.format("Game: %02d:%02d", gameMinutes, gameSeconds), 10, 0xFFFF00)
end

-- Draw paused screen
function drawPaused()
    display.text(80, 100, "GAME PAUSED", 20, 0xFFFF00)
    display.text(60, 130, "Press PAUSE button to resume", 12, 0xFFFFFF)
    display.text(70, 150, "Press QUIT to exit game", 12, 0xFFFFFF)
end

-- Draw anticamera (physical corridor room)
function drawAnticamera()
    display.text(10, 40, "ANTICAMERA - Physical Corridor", 16, 0xFFFFFF)
    
    -- Physical props description
    display.text(10, 60, "Physical Props:", 12, 0x00FFFF)
    display.text(20, 75, "- Paper on wall: (NH4)2[PtCl6]", 10, 0xCCCCCC)
    display.text(20, 85, "- Elevator with shape buttons", 10, 0xCCCCCC)
    display.text(20, 95, "- Door with keypad & broken scanner", 10, 0xCCCCCC)
    
    -- Elevator puzzle status
    display.text(10, 115, "Elevator Sequence:", 12, 0xFFFFFF)
    local inputStr = ""
    for i, val in ipairs(gameState.elevatorInput) do
        if val == 1 then inputStr = inputStr .. "I-"
        elseif val == 4 then inputStr = inputStr .. "[]-"
        elseif val == 2 then inputStr = inputStr .. "II-"
        elseif val == 6 then inputStr = inputStr .. "O-"
        end
    end
    display.text(20, 130, inputStr, 12, 0xFFFF00)
    
    if gameState.elevatorOpen then
        display.text(10, 150, "ELEVATOR OPEN!", 14, 0x00FF00)
        display.text(20, 165, "Found: Periodic Table piece, Badge, Key", 10, 0x00FF00)
    else
        display.text(10, 150, "Press shape buttons in correct order", 10, 0xCCCCCC)
    end
    
    -- Control door status
    display.text(10, 185, "Control Door Code:", 12, 0xFFFFFF)
    display.text(20, 200, gameState.controlDoorInput, 12, 0xFFFF00)
    
    if gameState.controlDoorOpen then
        display.text(10, 220, "DOOR OPEN! Entering Control Room...", 12, 0x00FF00)
    else
        display.text(10, 220, "Use numeric buttons for code", 10, 0xCCCCCC)
        display.text(20, 235, "Hint: Niobium + Tin atomic numbers", 10, 0x888888)
    end
end

-- Draw control room
function drawControlRoom()
    display.text(10, 40, "SALA DI CONTROLLO - Physical Control Room", 16, 0xFFFFFF)
    
    -- Physical valves status
    display.text(10, 65, "Physical Valves Status:", 12, 0xFFAA00)
    for i = 1, 4 do
        local status = gameState.valves[i] and "OPEN" or "CLOSED"
        local color = gameState.valves[i] and 0x00FF00 or 0xFF0000
        display.text(20, 80 + (i-1)*12, "Valve " .. i .. ": " .. status, 10, color)
    end
    
    display.text(10, 140, "Cryostat Pressure: " .. gameState.cryostatPressure .. " bar", 12, 0xFFFF00)
    
    if gameState.coolingRestored then
        display.text(10, 160, "COOLING RESTORED!", 14, 0x00FF00)
        display.text(10, 180, "Proceed to Core Room", 12, 0x00FF00)
    else
        display.text(10, 160, "Turn all valves to stabilize system", 10, 0xCCCCCC)
    end
end

-- Draw core room (placeholder)
function drawCoreRoom()
    display.text(10, 40, "STANZA DEL NOCCIOLO - Physical Core Room", 16, 0xFFFFFF)
    display.text(10, 70, "Torus Reactor - Plasma Discharge", 14, 0xFF4444)
    display.text(10, 100, "MISSION: Safely discharge plasma", 12, 0xFFFFFF)
    display.text(10, 120, "Physical controls and indicators", 10, 0xCCCCCC)
end

-- Draw messages/warnings/errors
function drawMessages()
    local y = 250
    for i, msg in ipairs(gameState.messages) do
        display.text(10, y, "INFO: " .. msg, 8, 0xFFFFFF)
        y = y + 10
        if y > 290 then break end
    end
    
    for i, warn in ipairs(gameState.warnings) do
        display.text(10, y, "WARN: " .. warn, 8, 0xFFFF00)
        y = y + 10
        if y > 290 then break end
    end
    
    for i, err in ipairs(gameState.errors) do
        display.text(10, y, "ERROR: " .. err, 8, 0xFF0000)
        y = y + 10
        if y > 290 then break end
    end
end

-- Handle physical button presses
function onButtonPress(buttonId)
    print("Physical button pressed: " .. buttonId)
    
    -- Control buttons
    if buttonId == buttons.quit then
        game.quit()
        return
    elseif buttonId == buttons.pause then
        gameState.paused = not gameState.paused
        if gameState.paused then
            addMessage("Game paused")
        else
            addMessage("Game resumed")
        end
        return
    end
    
    if gameState.paused then return end
    
    -- Room-specific inputs
    if gameState.currentRoom == "anticamera" then
        handleAnticameraInput(buttonId)
    elseif gameState.currentRoom == "controllo" then
        handleControlRoomInput(buttonId)
    end
end

-- Handle anticamera physical inputs
function handleAnticameraInput(buttonId)
    if buttonId >= 0 and buttonId <= 9 then
        -- Numeric keypad for door code
        if #gameState.controlDoorInput < 4 then
            gameState.controlDoorInput = gameState.controlDoorInput .. buttonId
            audio.beep(600, 100)  -- Keypress sound
        end
        
        -- Check code when complete
        if #gameState.controlDoorInput == 4 then
            if gameState.controlDoorInput == gameState.controlDoorCode then
                gameState.controlDoorOpen = true
                addMessage("Control door unlocked")
                audio.beep(1000, 300)  -- Success
                -- Transition to control room after delay
                gameState.currentRoom = "controllo"
            else
                addError("Invalid code entered")
                gameState.controlDoorInput = ""
                audio.beep(200, 500)  -- Error
            end
        end
    elseif buttonId == buttons.line or buttonId == buttons.twoLines or 
           buttonId == buttons.square or buttonId == buttons.hexagon then
        -- Elevator shape buttons
        table.insert(gameState.elevatorInput, buttonId)
        audio.beep(400, 150)  -- Button press sound
        
        -- Check sequence
        local correct = true
        for i, val in ipairs(gameState.elevatorInput) do
            if val ~= gameState.elevatorSequence[i] then
                correct = false
                break
            end
        end
        
        if not correct then
            -- Wrong sequence
            gameState.elevatorInput = {}
            addWarning("Elevator sequence incorrect - restarting")
            audio.beep(200, 300)  -- Error
        elseif #gameState.elevatorInput == #gameState.elevatorSequence then
            -- Correct sequence
            gameState.elevatorOpen = true
            addMessage("Elevator unlocked - items collected")
            audio.beep(1200, 500)  -- Success
            table.insert(gameState.inventory, "periodic_table")
            table.insert(gameState.inventory, "badge")
            table.insert(gameState.inventory, "key")
        end
    end
end

-- Handle control room physical inputs
function handleControlRoomInput(buttonId)
    -- Assume valve buttons are mapped to IDs 12-15
    if buttonId >= 12 and buttonId <= 15 then
        local valveId = buttonId - 11  -- 12->1, 13->2, etc.
        gameState.valves[valveId] = not gameState.valves[valveId]
        
        -- Recalculate pressure
        gameState.cryostatPressure = 0
        for i, on in ipairs(gameState.valves) do
            if on then
                gameState.cryostatPressure = gameState.cryostatPressure + 10
            end
        end
        
        audio.beep(500, 200)  -- Valve toggle sound
        
        -- Check if all valves open
        local allOpen = true
        for i, on in ipairs(gameState.valves) do
            if not on then allOpen = false; break end
        end
        
        if allOpen and not gameState.coolingRestored then
            gameState.coolingRestored = true
            addMessage("Cryostat cooling restored")
            audio.beep(1500, 400)  -- Success
            -- Transition to core room
            gameState.currentRoom = "nocciolo"
        end
    end
end

-- Message management
function addMessage(msg)
    table.insert(gameState.messages, msg)
    if #gameState.messages > 5 then
        table.remove(gameState.messages, 1)
    end
end

function addWarning(msg)
    table.insert(gameState.warnings, msg)
    if #gameState.warnings > 3 then
        table.remove(gameState.warnings, 1)
    end
end

function addError(msg)
    table.insert(gameState.errors, msg)
    if #gameState.errors > 3 then
        table.remove(gameState.errors, 1)
    end
end

-- Utility functions
function hasItem(item)
    for _, invItem in ipairs(gameState.inventory) do
        if invItem == item then
            return true
        end
    end
    return false
end

print("CND 2039 Physical Escape Room script loaded successfully")