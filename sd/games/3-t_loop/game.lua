local counter = 0

system.info("Script loaded")

function init()
    system.info("Init called")
end

function update(dt)
    counter = counter + dt

    if counter >= 1.0 then
        counter = 0
        system.info("Update running")
    end
end