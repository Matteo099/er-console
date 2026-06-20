local counter = 0

system.info("Script loaded")

function init()
    system.info("Init called")
end

function update(dt, paused)
    counter = counter + dt

    if counter >= 1.0 then
        counter = 0
        
        local state = paused and "inactive" or "active"
        system.info(string.format("Update running, game %s", state))
    end
end
