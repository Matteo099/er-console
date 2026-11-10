-- Centrale Nucleare Dolomiti: sequenza completa degli enigmi.
-- Contratto firmware ancora da implementare, oltre ai binding esistenti:
-- input.adc.read(channel) -> millivolt, canali 0..7 (ADS1115 0x48/0x49).
-- input.servo.setAngle(channel, degrees), PCA9685 canali 0..3.
-- input.tm1637.display(display, text), display 0..7; i pin MCP sono in config.
-- input.ledstrip.setColor(index, r, g, b), input.ledstrip.show(), LED 0..23.
-- input.channels deve supportare anche MCP3/4 e il bus I2C configurabile.
-- RFID deve usare il bus TFT condiviso e rispettarne la sincronizzazione.
-- I campi aggiuntivi di config.json richiedono un parser firmware aggiornato.

local messages = {
    elevator = {it = "Inserire la sequenza dell'ascensore.", en = "Enter the elevator sequence."},
    door = {it = "Ascensore aperto. Inserire il codice della sala controllo.", en = "Elevator open. Enter the control room code."},
    valves = {it = "CRIOSTATO OFFLINE. Aprire i circuiti criogenici nell'ordine corretto.", en = "CRYOSTAT OFFLINE. Open the cryogenic circuits in order."},
    reset = {it = "Sequenza errata. Riportare i comandi alla posizione iniziale.", en = "Incorrect sequence. Return controls to their initial positions."},
    badge = {it = "CRIOSTATO STABILIZZATO. Inserire il badge operatore.", en = "CRYOSTAT STABILIZED. Scan the operator badge."},
    cabinet = {it = "ARCHIVIO TECNICO: lega superconduttrice Nb₃Sn. Aprire l'armadio.", en = "TECHNICAL ARCHIVE: superconductor Nb₃Sn. Open the cabinet."},
    fuse = {it = "ALIMENTAZIONE MAGNETI INTERROTTA. Inserire il fusibile e il codice tecnico.", en = "MAGNET POWER INTERRUPTED. Insert the fuse and technical code."},
    modules = {it = "SETTORI DANNEGGIATI: TF-2, TF-5, TF-7. Sostituire i moduli indicati.", en = "DAMAGED SECTORS: TF-2, TF-5, TF-7. Replace the indicated modules."},
    coils = {it = "MODULI RIPRISTINATI. Riallineare le bobine opposte.", en = "MODULES RESTORED. Align the opposing coils."},
    rings = {it = "CAMPO MAGNETICO STABILE. Sigillare la camera a vuoto.", en = "MAGNETIC FIELD STABLE. Seal the vacuum chamber."},
    route = {it = "CAMERA SIGILLATA. Instradare il plasma verso il serbatoio.", en = "CHAMBER SEALED. Route plasma to the tank."},
    final = {it = "SERBATOIO COLLEGATO. Eseguire la procedura di scarico.", en = "TANK CONNECTED. Perform the discharge procedure."},
    victory = {it = "PLASMA SCARICATO. CENTRALE NUCLEARE DOLOMITI SALVATA.", en = "PLASMA DISCHARGED. DOLOMITI POWER PLANT SAVED."},
}
local function text(key) return system.translate(messages[key]) end
local function show(key)
    scene.setGameObjects({{title = system.translate({it = "Centrale Nucleare Dolomiti", en = "Dolomiti Nuclear Power Plant"}),
        content = {{type = "text", value = text(key)}}}})
end

-- MCP A: 100..115; B: 120..135; C: 140..155; D: 160..175.
local pins = {
    elevator = {100, 101, 102, 103}, cabinet = {104, 105, 106, 107, 108},
    valves = {109, 110, 111, 112}, green = 113, red = 114, alarm1 = 115,
    doorRows = {120, 121, 122, 123}, doorCols = {124, 125, 126},
    powerRows = {127, 128, 129, 130}, powerCols = {131, 132, 133},
    alarm2 = 134, moduleLeds = {135, 152, 153},
    rings = {140, 141, 142, 143}, route = {144, 145, 146, 147},
    levers = {148, 149, 150}, cover = 151, seal = 154, tank = 155, fuse = 42,
}
local keypad = {{"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}, {"*", "0", "#"}}
local state = {
    stage = "elevator", inputs = {}, keys = {}, position = 1, code = "",
    resetting = false, indicatorUntil = 0, coils = {}, displayCache = {},
    animation = 0, finishing = false,
}
local function led(pin, on) input.channels.digitalWrite(pin, not on) end
local function feedback(ok)
    led(pins.green, ok); led(pins.red, not ok)
    state.indicatorUntil = time.now() + 350
    audio.beep(ok and 1000 or 300, 100)
end
local function advance(stage)
    state.stage, state.position, state.code = stage, 1, ""
    state.inputs, state.keys = {}, {}
    state.resetting = false
    show(stage)
end

-- Stabilizza ingressi e genera un fronte soltanto dopo 35 ms.
local function readSwitch(pin)
    local now, raw = time.now(), not input.channels.digitalRead(pin)
    local s = state.inputs[pin]
    if not s then s = {raw = raw, value = raw, since = now}; state.inputs[pin] = s; return raw, false end
    if raw ~= s.raw then s.raw, s.since = raw, now end
    if s.value ~= s.raw and now - s.since >= 35 then s.value = s.raw; return s.value, true end
    return s.value, false
end
local function allOff(pinsList)
    local off = true
    for _, pin in ipairs(pinsList) do local active = readSwitch(pin); if active then off = false end end
    return off
end
local function scanKeypad(rows, cols, id)
    local found, count = nil, 0
    for row = 1, 4 do
        input.channels.digitalWrite(rows[row], false); input.channels.pinMode(rows[row], PinMode.OUTPUT)
        for col = 1, 3 do
            if not input.channels.digitalRead(cols[col]) then found, count = keypad[row][col], count + 1 end
        end
        input.channels.pinMode(rows[row], PinMode.INPUT)
    end
    if count ~= 1 then found = nil end
    local now = time.now()
    local s = state.keys[id] or {since = now}; state.keys[id] = s
    if found ~= s.candidate then s.candidate, s.since = found, now end
    if found ~= s.value and now - s.since >= 35 then s.value = found; return found end
end
local function codeEntered(key, code)
    if key == "*" then state.code = ""; return false end
    if not key or key == "#" then return false end
    state.code = state.code .. key
    if #state.code < #code then return false end
    local ok = state.code == code; state.code = ""; feedback(ok); return ok
end
local function sequenceButtons(buttons, sequence)
    for i, pin in ipairs(buttons) do
        local on, changed = readSwitch(pin)
        if changed and on then
            local ok = i == sequence[state.position]; feedback(ok)
            state.position = ok and state.position + 1 or 1
            if state.position > #sequence then return true end
        end
    end
    return false
end
local function sequenceSwitches(switches, sequence)
    if state.resetting then
        if allOff(switches) then state.resetting = false; state.position = 1 end
        return false
    end
    for i, pin in ipairs(switches) do
        local on, changed = readSwitch(pin)
        if changed and on then
            if i ~= sequence[state.position] then
                feedback(false); state.resetting = true; show("reset"); return false
            end
            feedback(true); state.position = state.position + 1
            if state.position > #sequence then return true end
        elseif changed and not on and i < state.position then
            feedback(false); state.resetting = true; show("reset"); return false
        end
    end
    return false
end
local function servo(index) input.servo.setAngle(index, 90) end

-- Enigma 1: formula chimica e apertura dell'ascensore.
local function enigmaAscensore()
    if sequenceButtons(pins.elevator, {1, 3, 2, 1, 4}) then servo(0); advance("door") end
end

-- Enigma 2: tastiera della porta, codice Nb/Sn = 4150.
local function enigmaPortaControllo()
    if codeEntered(scanKeypad(pins.doorRows, pins.doorCols, "door"), "4150") then
        servo(1); advance("valves"); state.resetting = true
    end
end

-- Enigma 3: circuiti criogenici e autenticazione del badge.
local function enigmaCriostato()
    if sequenceSwitches(pins.valves, {1, 2, 3, 4}) then
        led(pins.alarm1, false); led(pins.alarm2, false)
        -- Scarta eventuali letture precedenti alla richiesta del badge.
        while input.rfid.consume() do end
        advance("badge")
    end
end
local function enigmaBadge()
    local tag = input.rfid.consume()
    -- Un badge valido qualsiasi: configurare un UID specifico nel firmware se necessario.
    if tag and tag.uid and tag.uid ~= "" then servo(2); advance("cabinet") end
end

-- Enigma 4: pulsanti Nb, Sn, Fe, Cu, Ti; Nb3Sn apre l'armadio.
local function enigmaArmadioMagneti()
    if sequenceButtons(pins.cabinet, {1, 1, 1, 2}) then servo(3); advance("fuse") end
end

-- Enigma 5: presenza del fusibile e codice 413.
local function enigmaAlimentazione()
    local inserted = readSwitch(pins.fuse)
    if not inserted then state.code = ""; return end
    if codeEntered(scanKeypad(pins.powerRows, pins.powerCols, "power"), "413") then advance("modules") end
end

-- Enigma 6: identificazione dei ricambi mediante partitore da 10 kohm.
-- Tensioni nominali a 3,3 V: 1k=300 mV, 2k2=595 mV, 4k7=1055 mV.
-- Soglie e tensione di riferimento vanno calibrate sul montaggio reale.
local function enigmaSostituzioneBobine()
    local expected = {300, 595, 1055}
    local ready = true
    for i = 1, 3 do
        local mv = input.adc.read(i - 1)
        local correct = type(mv) == "number" and math.abs(mv - expected[i]) <= 60
        led(pins.moduleLeds[i], correct)
        if not correct then ready = false end
    end
    if ready then advance("coils") end
end

-- Enigma 7: quattro potenziometri regolano coppie opposte a somma costante.
local function enigmaRiallineamentoBobine()
    local first = {32, 42, 28, 35}
    local second = {36, 38, 24, 39}
    local balanced = true
    for i = 1, 4 do
        local mv = input.adc.read(i + 2)
        if type(mv) ~= "number" then return end
        -- Intervallo di regolazione: -4..+4, centro = valore iniziale.
        local offset = math.floor(math.max(0, math.min(3300, mv)) * 8 / 3300 + 0.5) - 4
        local a, b = first[i] + offset, second[i] - offset
        local values = {{i - 1, a}, {i + 3, b}}
        for _, item in ipairs(values) do
            if state.displayCache[item[1]] ~= item[2] then
                input.tm1637.display(item[1], tostring(item[2])); state.displayCache[item[1]] = item[2]
            end
        end
        if a ~= b then balanced = false end
    end
    if balanced then advance("rings"); state.resetting = true end
end

-- Enigma 8: chiusura progressiva cerchio, triangolo, quadrato, esagono.
local function enigmaCameraVuoto()
    if sequenceSwitches(pins.rings, {1, 2, 3, 4}) then led(pins.seal, true); advance("route") end
end

-- Enigma 9: posizioni esemplificative da riportare sulla mappa definitiva.
-- false/true = posizione 0/1; il contatto chiuso verso GND corrisponde a true.
local function enigmaInstradamentoPlasma()
    local target = {false, true, false, true}
    for i, pin in ipairs(pins.route) do
        local active = readSwitch(pin)
        if active ~= target[i] then return end
    end
    led(pins.tank, true); advance("final"); state.resetting = true
end

-- Enigma 10: iniettori ON, magneti OFF e scarico chiuso sono la posizione iniziale.
-- Gli ingressi sono attivi bassi: ON/OPEN = true. Le transizioni devono essere ordinate.
local function enigmaScaricoPlasma()
    local values, changes = {}, {}
    for i, pin in ipairs(pins.levers) do values[i], changes[i] = readSwitch(pin) end
    if state.resetting then
        if values[1] and not values[2] and not values[3] then state.resetting = false; state.position = 1 end
        return
    end
    local target = {{false, false, false}, {false, true, false}, {false, true, true}}
    local changed = 0
    for i = 1, 3 do if changes[i] then changed = changed + 1 end end
    if changed == 0 then return end
    local expected = target[state.position]
    if changed ~= 1 or values[1] ~= expected[1] or values[2] ~= expected[2] or values[3] ~= expected[3] then
        feedback(false); state.resetting = true; show("reset"); return
    end
    feedback(true); state.position = state.position + 1
    if state.position == 4 then state.finishing = true; state.animation = 0; state.stage = "animation" end
end

local function animateVictory(dt)
    state.animation = state.animation + dt
    local progress = math.min(1, state.animation / 4)
    for i = 0, 11 do input.ledstrip.setColor(i, math.floor(120 * (1 - progress)), 0, 0) end
    for i = 12, 19 do input.ledstrip.setColor(i, i - 12 < math.floor(progress * 8) and 80 or 0, 0, 0) end
    for i = 20, 22 do input.ledstrip.setColor(i, 0, 0, math.floor(100 * progress)) end
    input.ledstrip.setColor(23, 0, progress >= 1 and 100 or 0, 0)
    input.ledstrip.show()
    if progress >= 1 then state.finishing = false; state.stage = "done"; show("victory"); scene.completeGame() end
end

function init()
    if not (input.adc and input.adc.read and input.servo and input.servo.setAngle and
        input.tm1637 and input.tm1637.display and input.ledstrip and input.ledstrip.setColor and input.ledstrip.show) then
        error("Centrale Dolomiti: binding hardware richiesti non disponibili; vedere game.lua e config.json")
    end
    for _, group in ipairs({pins.elevator, pins.cabinet, pins.valves, pins.rings, pins.route, pins.levers, {pins.cover, pins.fuse}}) do
        for _, pin in ipairs(group) do input.channels.pinMode(pin, PinMode.INPUT_PULLUP) end
    end
    for _, group in ipairs({pins.doorRows, pins.powerRows}) do for _, pin in ipairs(group) do input.channels.pinMode(pin, PinMode.INPUT) end end
    for _, group in ipairs({pins.doorCols, pins.powerCols}) do for _, pin in ipairs(group) do input.channels.pinMode(pin, PinMode.INPUT_PULLUP) end end
    for _, pin in ipairs({pins.green, pins.red, pins.alarm1, pins.alarm2, pins.moduleLeds[1], pins.moduleLeds[2], pins.moduleLeds[3], pins.seal, pins.tank}) do
        input.channels.digitalWrite(pin, true); input.channels.pinMode(pin, PinMode.OUTPUT)
    end
    for i = 0, 3 do input.servo.setAngle(i, 0) end
    for i = 0, 23 do input.ledstrip.setColor(i, 0, 0, 0) end
    for i = 0, 11 do input.ledstrip.setColor(i, 120, 0, 0) end
    input.ledstrip.show()
    led(pins.alarm1, true); led(pins.alarm2, true); show("elevator")
end

function update(dt, paused)
    if paused then return end
    if state.indicatorUntil > 0 and time.now() >= state.indicatorUntil then
        led(pins.green, false); led(pins.red, false); state.indicatorUntil = 0
    end
    if state.stage == "elevator" then enigmaAscensore()
    elseif state.stage == "door" then enigmaPortaControllo()
    elseif state.stage == "valves" then enigmaCriostato()
    elseif state.stage == "badge" then enigmaBadge()
    elseif state.stage == "cabinet" then enigmaArmadioMagneti()
    elseif state.stage == "fuse" then enigmaAlimentazione()
    elseif state.stage == "modules" then enigmaSostituzioneBobine()
    elseif state.stage == "coils" then enigmaRiallineamentoBobine()
    elseif state.stage == "rings" then enigmaCameraVuoto()
    elseif state.stage == "route" then enigmaInstradamentoPlasma()
    elseif state.stage == "final" then enigmaScaricoPlasma()
    elseif state.stage == "animation" then animateVictory(dt) end
end

function exit()
    for _, pin in ipairs({pins.green, pins.red, pins.alarm1, pins.alarm2, pins.moduleLeds[1], pins.moduleLeds[2], pins.moduleLeds[3], pins.seal, pins.tank}) do led(pin, false) end
    for i = 0, 23 do input.ledstrip.setColor(i, 0, 0, 0) end
    input.ledstrip.show()
end
