local rotSpeed = 45.0

function onAwake(entity)
end

function onUpdate(deltaTime, entity)
    local transform = entity.transform
    local rotation = transform:GetRotation()
    rotation.y = rotation.y + (rotSpeed * deltaTime)
    transform:SetRotation(rotation)
end

function onDestroyed(entity)
end





