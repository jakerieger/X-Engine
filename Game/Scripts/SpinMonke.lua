local rotY = 0.0
local rotSpeed = 45.0

function onAwake(entity)
    print('onAwake() called for: ' .. entity.name)
end

function onUpdate(deltaTime, entity)
    local transform = entity.transform
    rotY = rotY + deltaTime
    transform:SetRotationY(rotY * rotSpeed)
end

function onDestroyed(entity)
    print('onDestroyed() called for: ' .. entity.name)
end

