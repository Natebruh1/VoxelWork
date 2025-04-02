--print("Script started!") -- Check if script even runs

--print("Checking if Node exists:", Node)  -- Should print "table: 0x..."
--print("Checking if Node.tick exists:", Node and Node.tick) -- Should print "function: 0x..."
local a=true
local lifetime=0.0
print("In Script")
function easeOutQuad(t)
    return {t[0] * (2 - t[0]),t[1] * (2 - t[1]),t[2] * (2 - t[2])} -- Ease-out quadratic interpolation
end

function onTick(deltaTime)
    --print("Tick function called!") -- If this prints, tick is working
    if a then
        a=false
        --[[
            AddAnimation (
                name -> String
                property -> String
                value -> table(for 3d Vectors) or number
                time -> number
                PartID -> table [{} == Root, {0}==Root First Child, {1}==Root Second Child, {0,1}
                ==Root.FirstChild.SecondChild]
            )
        
        ]]
        Model:AddAnimation("Rotate","rotation",{0,900.0,0},10.0,{0})
        
        print("Adding animation from Lua")
        Model:PlayAnimation("Rotate")
        Model:SetAnimationInterpolation("Rotate",easeOutQuad)
    end

    if Model then
        
        lifetime=lifetime+deltaTime
        
        Model:Move(0.0,1.0*deltaTime,0.0)
       
        if lifetime>4.0 then
            Node.AttachScript("scripts/changed_script.lua")
        end
    else
        print("Error: Model is nil!")
    end
end
Node.tick(onTick)
