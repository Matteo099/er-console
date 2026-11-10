-- Authored game text follows the language selected in Settings.
local messages = {
    loaded = {it = "Script caricato", en = "Script loaded"},
    initialized = {it = "Inizializzazione eseguita", en = "Initialization called"},
    inactive = {it = "in pausa", en = "paused"},
    active = {it = "attivo", en = "active"},
    updating = {it = "Aggiornamento in corso, gioco %s", en = "Update running, game %s"},
}
local function text(key)
    return system.translate(messages[key])
end

local counter = 0

system.info(text("loaded"))

function init()
    system.info(text("initialized"))
end

function update(dt, paused)
    counter = counter + dt

    if counter >= 1.0 then
        counter = 0
        
        local state = paused and text("inactive") or text("active")
        system.info(string.format(text("updating"), state))
    end
end
