print("Spawning Slimes!")
local lifetime=0.0
local spawnSlimeCountdown=5.0
function onTick(deltaTime)
    lifetime=lifetime+deltaTime
    spawnSlimeCountdown=spawnSlimeCountdown-deltaTime
    if spawnSlimeCountdown<0.0 then
        --spawn slime
        spawnSlimeCountdown=5.0 + spawnSlimeCountdown*-1.0
        Model.SpawnModel("mods/models/slime.txt")
        print("Spawning Slime")
    end
end

--Assign the tick callback
Node.tick(onTick)