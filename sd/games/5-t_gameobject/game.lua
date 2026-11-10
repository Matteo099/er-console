-- Authored game text follows the language selected in Settings.
local messages = {
    started = {it = "Test del framework iniziato", en = "Framework test started"},
    inputsReady = {it = "Ingressi configurati", en = "Inputs configured"},
    notReady = {it = "Aggiornamento ignorato: inizializzazione non completata", en = "Update ignored: initialization not complete"},
    channel1 = {it = "Canale 1 PREMUTO", en = "Channel 1 PRESSED"},
    channel2 = {it = "Canale 2 PREMUTO", en = "Channel 2 PRESSED"},
    channel3 = {it = "Canale 3 PREMUTO - %d/50", en = "Channel 3 PRESSED - %d/50"},
    completed = {it = "Test completato, partita risolta", en = "Framework test complete, game unlocked"},
    heartbeat = {it = "Stato - CH1=%s CH2=%s CH3=%s SBLOCCO=%d/50", en = "Heartbeat - CH1=%s CH2=%s CH3=%s UNLOCK=%d/50"},
    exiting = {it = "Uscita dal test del framework", en = "Framework test exiting"},
    map = {it = "Mappa", en = "Map"},
    mapDescription = {it = "Osserva la mappa e confrontala con gli indizi raccolti. Puoi scorrere il contenuto per esaminare tutte le immagini e le annotazioni.", en = "Study the map and compare it with the clues you have collected. Scroll through the content to examine all images and notes."},
    mapHint = {it = "Confronta i simboli e i percorsi. Gli oggetti restano disponibili durante la partita: puoi tornare al timer e riaprire questa sezione quando vuoi.", en = "Compare the symbols and paths. These items remain available during the game: return to the timer and reopen this section whenever you need."},
    board = {it = "Tavola", en = "Board"},
}
local function text(key)
    return system.translate(messages[key])
end

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

local lastLanguage = nil
local function publishObjects()
    scene.setGameObjects({
        {
            title = text("map"),
            content = {
                {
                    type = GameObjectContentType.TEXT,
                    value = text("mapDescription")
                },
                {
                    type = GameObjectContentType.IMAGE,
                    path = "images/map.png",
                    width = 50,
                    height = 50
                },
                {
                    type = GameObjectContentType.TEXT,
                    value = text("mapHint")
                },
                {
                    type = GameObjectContentType.IMAGE,
                    path = "images/map.png",
                    width = 100,
                    height = 100
                },
            }
        },
        {
            title = text("board"),
            content = {
                {
                    type = GameObjectContentType.IMAGE,
                    path = "images/tavola.png",
                    width = 250,
                    height = 166
                }
            }
        }
    })

    lastLanguage = system.language()
end

function init()
    system.info(text("started"))
    input.channels.pinMode(CONFIG.input1, PinMode.INPUT_PULLUP)
    input.channels.pinMode(CONFIG.input2, PinMode.INPUT_PULLUP)
    input.channels.pinMode(CONFIG.input3, PinMode.INPUT_PULLUP)
    state.initialized = true
    publishObjects()
    system.info(text("inputsReady"))
end


--------------------------------------------------
-- UPDATE
--------------------------------------------------

function update(dt, paused)
    if state.initialized and lastLanguage ~= system.language() then
        publishObjects()
    end
    if(paused) then
        return
    end

    if not state.initialized then
        system.warn(text("notReady"))
        return
    end

    local ch1 = input.channels.digitalRead(CONFIG.input1)
    local ch2 = input.channels.digitalRead(CONFIG.input2)
    local ch3 = input.channels.digitalRead(CONFIG.input3)
    
    --------------------------------------------------
    -- Log input changes
    --------------------------------------------------

    if ch1 then
        system.info(text("channel1"))
    end

    if ch2 then
        system.info(text("channel2"))
    end

    if ch3 then
        system.info(string.format(text("channel3"), state.unlockCounter))
        state.unlockCounter = state.unlockCounter + 1
    end


    if state.unlockCounter >= 50 then
        system.info(text("completed"))
        scene.completeGame()
    end

    --------------------------------------------------
    -- Periodic heartbeat
    --------------------------------------------------

    state.lastLogTime = state.lastLogTime + dt

    if state.lastLogTime >= 5.0 then
        state.lastLogTime = 0

        system.info(string.format(text("heartbeat"), tostring(ch1), tostring(ch2), tostring(ch3), state.unlockCounter))
    end
end


function exit()
    system.info(text("exiting"))
    
end
