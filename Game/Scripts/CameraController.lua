function onUpdate(deltaTime, camera)
    local yaw = Input:GetMouseDeltaX()
    local pitch = Input:GetMouseDeltaY()
    camera:Rotate(pitch, yaw)
end
