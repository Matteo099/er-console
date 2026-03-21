-- The Abandoned Lab - Lua Game Script
-- This script demonstrates the Lua game system

-- Game state
local gameState = {
    initialized = false,
    puzzleSolved = false,
    attemptCount = 0,
    maxAttempts = 3
}

-- Initialize the game
function init()
    print("Lab01 Game: Initializing...")
    gameState.initialized = true
    
    -- Clear display and show initial message
    display.clear()
    display.text(10, 50, "Welcome to the Abandoned Lab", 20, 0xFFD700)
    display.text(10, 90, "Find the code to unlock the exit", 14, 0xFFFFFF)
    display.text(10, 130, "The code is: 1234", 14, 0x00FF00)
    
    print("Lab01 Game: Initialized successfully")
end

-- Update is called every frame when game is running
function update(dt)
    -- dt is delta time in seconds
    
    -- Draw game content
    if gameState.initialized then
        -- Draw background grid pattern
        display.rect(20, 180, 280, 120, 0x333333, false)
        
        -- Show current attempt count
        local attemptText = string.format("Attempts: %d/%d", gameState.attemptCount, gameState.maxAttempts)
        display.text(30, 200, attemptText, 14, 0xFFFFFF)
        
        -- Show puzzle status
        if gameState.puzzleSolved then
            display.text(30, 230, "PUZZLE SOLVED! Exit is unlocked.", 14, 0x00FF00)
        else
            display.text(30, 230, "Enter the code to proceed...", 14, 0xFFCC00)
        end
        
        -- Show instructions
        display.text(30, 270, "Use the buttons to enter digits", 12, 0x888888)
    end
end

-- Called when a button is pressed (can be extended)
function onButtonPress(buttonId)
    print("Button pressed: " .. buttonId)
    
    gameState.attemptCount = gameState.attemptCount + 1
    
    if gameState.attemptCount >= gameState.maxAttempts then
        print("Game Over - Too many attempts!")
    end
end

-- Puzzle solving function (example)
function solvePuzzle(code)
    if code == 1234 then
        gameState.puzzleSolved = true
        print("Puzzle solved!")
        return true
    else
        gameState.attemptCount = gameState.attemptCount + 1
        print("Wrong code! Attempts remaining: " .. (gameState.maxAttempts - gameState.attemptCount))
        return false
    end
end

print("Lab01 Game script loaded successfully")
