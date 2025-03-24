

print("Test gen")
function terrain_rule(voxels)
    local results = {}
    for i, v in ipairs(voxels) do
        local height = GetNoise(v.x, v.z) * 48
        results[i] = (v.y < height)
    end
    return results
end

AddRule(terrain_rule)