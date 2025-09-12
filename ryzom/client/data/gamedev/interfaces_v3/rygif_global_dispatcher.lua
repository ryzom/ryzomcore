-- Global registry for all players, keyed by target groupId
_G._GIFPLAYERS = _G._GIFPLAYERS or {}

function GIF_Register(player)
    if not player or not player.targetGroupId then return end
    _GIFPLAYERS[player.targetGroupId] = player
end

function GIF_UnregisterById(groupId)
    _GIFPLAYERS[groupId] = nil
end

-- Draw all players each frame (used by setOnDraw as string)
function GIF_DrawAll()
    -- Optional: light loader pump (if your engine doesn't call player:draw often enough)
    for _, p in pairs(_GIFPLAYERS) do
        p:draw()
    end
end

-- Per-instance controls usable from string-based hooks
function GIF_Play(groupId)
    local p = _GIFPLAYERS[groupId]
    if p then p:play() end
end

function GIF_Pause(groupId)
    local p = _GIFPLAYERS[groupId]
    if p then p:pause() end
end

function GIF_Reset(groupId)
    local p = _GIFPLAYERS[groupId]
    if p then p:reset() end
end

function GIF_ShowPause(groupId)
    local p = _GIFPLAYERS[groupId]
    --TODO: need add later that we show a Pause button by mouseover the Gif Player
end

function GIF_Toggle(groupId)
    local p = _GIFPLAYERS[groupId]
    if p.paused then
        p:play()
    else
        p:pause()
    end
end

function GIF_IsPaused(groupId)
    local p = _GIFPLAYERS[groupId]
    return p and p.paused == true or false
end


function GIF_SetFPS(groupId, fps)
    local p = _GIFPLAYERS[groupId]
    if p then p:setFPS(tonumber(fps) or 12) end
end