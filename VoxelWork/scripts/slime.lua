print("I am a slime!")
local xInc=math.random(-5.0,5.0)
local zInc=math.random(-5.0,5.0)
function onTick(deltaTime)
    Model:Move(xInc*deltaTime,0.0,zInc*deltaTime)
end

Node.tick(onTick)