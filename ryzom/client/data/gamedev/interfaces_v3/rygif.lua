-- =================== rygif.lua (auto-frame + spritesheet support) ===================

rygif = {}
rygif.__index = rygif

-- =================== Global loader state ===================
LoadedStems   = {}
PendingStems  = {}
MaxAtlasesPerFrame = 1
LoadTimeBudgetSec  = 0.002

-- =================== Utility ===================
local function warm_texture_once_for(targetGroupPath, targetGroupId, texname)
    local grp = getUI(targetGroupPath)
    if grp then
        local g = grp:find(targetGroupId)
        if g then
            local img = g:find("texture_img")
            if img then
                img.texture = texname
            end
        end
    end
end

local function enqueue_stems(stems, prioritize_first)
    if prioritize_first and #stems > 0 then
        local first = stems[1]
        if not LoadedStems[first] then
            table.insert(PendingStems, 1, first)
            LoadedStems[first] = "pending"
        end
        for i = 2, #stems do
            local s = stems[i]
            if not LoadedStems[s] then
                table.insert(PendingStems, s)
                LoadedStems[s] = "pending"
            end
        end
        return
    end
    for _, s in ipairs(stems) do
        if not LoadedStems[s] then
            table.insert(PendingStems, s)
            LoadedStems[s] = "pending"
        end
    end
end

-- =================== Loader pumping ===================
local function pump_loader_slice()
    local start_clock = os.clock()
    local loaded_this_frame = 0
    while loaded_this_frame < MaxAtlasesPerFrame and #PendingStems > 0 do
        local elapsed = os.clock() - start_clock
        if loaded_this_frame > 0 and elapsed >= LoadTimeBudgetSec then break end
        local stem = table.remove(PendingStems, 1)
        loadTextures(stem, true)
        LoadedStems[stem] = true
        loaded_this_frame = loaded_this_frame + 1
    end
end

function rygif.pumpGlobalLoader()
    pump_loader_slice()
end

function rygif.setLoadBudgets(max_per_frame, time_budget_ms)
    if type(max_per_frame) == "number" and max_per_frame >= 1 then
        MaxAtlasesPerFrame = math.floor(max_per_frame)
    end
    if type(time_budget_ms) == "number" and time_budget_ms >= 0 then
        LoadTimeBudgetSec = time_budget_ms / 1000.0
    end
end

-- =================== Player ===================
function rygif.new(base, fps, loop, opts)
    local self = setmetatable({}, rygif)
    self.fps  = fps or 12
    self.loop = (loop ~= false)
    self.targetGroupPath = (opts and opts.groupPath)
    self.targetGroupId   = (opts and opts.groupId)

    if not (opts and opts.frameCount and opts.sheetCount) then
        error("You must provide opts.frameCount and opts.sheetCount")
    end

    local prefix = base:gsub("%.jpg$", "") -- strip accidental .jpg

    -- generate frame list
    self.frames = {}
    for i = 1, opts.frameCount do
        local fname = string.format("%s_%05d.jpg", prefix, i)
        table.insert(self.frames, fname)
    end

    -- enqueue all spritesheets (without extension, client adds .png)
    local stems = {}
    for s = 1, opts.sheetCount do
        table.insert(stems, string.format("%s_%d", prefix, s))
    end
    enqueue_stems(stems, true)

    self.frameCount   = #self.frames
    self.ready        = false
    self.warmed       = false
    self.playAfterReady = true

    self.startTime    = os.clock()
    self.paused       = false
    self.pauseStart   = 0
    self.pauseAccum   = 0
    self.currentIndex = 1
    self.pausedIndex  = 1

    return self
end

function rygif:isReady()
    for stem, state in pairs(LoadedStems) do
        if state ~= true then return false end
    end
    return true
end

function rygif:play()
    self.playAfterReady = true
    if self.paused then
        self.paused = false
        if self.pauseStart and self.pauseStart ~= 0 then
            self.pauseAccum = (self.pauseAccum or 0) + (os.clock() - self.pauseStart)
            self.pauseStart = 0
        end
    end
end

function rygif:pause()
    if not self.paused then
        local now = os.clock()
        local elapsed = now - self.startTime - (self.pauseAccum or 0)
        local idx = math.floor(elapsed * (self.fps or 12))
        if self.loop then
            idx = (idx % self.frameCount) + 1
        else
            idx = math.min(idx + 1, self.frameCount)
        end
        self.currentIndex = idx
        self.pausedIndex = idx
        self.paused = true
        self.pauseStart = now
    end
end

function rygif:setFPS(fps)
    if fps and fps > 0 then self.fps = fps end
end

function rygif:reset()
    self.startTime = os.clock()
    self.pauseAccum = 0
    self.pauseStart = 0
    self.currentIndex = 1
    self.pausedIndex = 1
end

function rygif:draw()
    pump_loader_slice()
    if not self.ready then
        if self:isReady() then
            self.ready = true
            if not self.warmed and #self.frames > 0 then
                warm_texture_once_for(self.targetGroupPath, self.targetGroupId, self.frames[1])
                self.warmed = true
            end
            if self.playAfterReady then self.paused = false end
        else
            if #self.frames > 0 then
                self:TextureDraw(self.frames[1])
            end
            return
        end
    end
    if self.paused then
        local idx = self.pausedIndex or 1
        if idx < 1 then idx = 1 end
        if idx > self.frameCount then idx = self.frameCount end
        self:TextureDraw(self.frames[idx])
        return
    end
    local now = os.clock()
    local elapsed = now - self.startTime - (self.pauseAccum or 0)
    local idx = math.floor(elapsed * (self.fps or 12))
    if self.loop then
        idx = (idx % self.frameCount) + 1
    else
        idx = math.min(idx + 1, self.frameCount)
    end
    self.currentIndex = idx
    self:TextureDraw(self.frames[idx])
end

function rygif:TextureDraw(texture_img)
    local main_ui = getUI(self.targetGroupPath)
    if not main_ui then return end
    local group = main_ui:find(self.targetGroupId)
    if not group then return end
    local img = group:find("texture_img")
    if not img then return end
    img.texture = texture_img
end

function rygif:open_resize_window(window_id, win_h, win_w, render_html_content, close_window_id)
	local mainui = getUI(window_id)
	local mainui_html = mainui:find("html")
	local mainui_close_button = mainui:find("rightbut")

	mainui.active = true
	
	if(mainui.active == false)then
		mainui.active = true
	end
	
	if(mainui.opened == false)then
		mainui.opened = true
	end
	
	if(close_window_id == "")then
		mainui_close_button.active = false
		mainui_close_button.onclick_l = ""
		mainui_close_button.params_l =  ""
	else
		mainui_close_button.active = true
		mainui_close_button.onclick_l = "lua"
		mainui_close_button.params_l = "rygif:close_window('"..close_window_id.."')"
	end
	
	mainui.h = win_h
	mainui.w = win_w

	mainui_heade_open = mainui:find("header_opened")
	mainui_heade_open.h = 10
	mainui_heade_open.w = win_w
	
	mainui_html:renderHtml(render_html_content)
end

function rygif:close_window(window_id)
	if(getUI(window_id) ~= nil)then
		getUI(window_id).active=false
		GIF_UnregisterById(window_id)
	end
end

-- =================== Global Dispatcher ===================
_G._GIFPLAYERS = _G._GIFPLAYERS or {}

function GIF_Register(player)
    if not player or not player.targetGroupId then return end
    _GIFPLAYERS[player.targetGroupId] = player
end

function GIF_UnregisterById(groupId)
    _GIFPLAYERS[groupId] = nil
end

function GIF_DrawAll()
    for _, p in pairs(_GIFPLAYERS) do
        p:draw()
    end
end

function GIF_Play(groupId)
    local p = _GIFPLAYERS[groupId]
    if p then p:play() end
end

function GIF_Pause(groupId)
    local p = _GIFPLAYERS[groupId]
    if p then p:pause() end
end

function GIF_ShowPause(groupId)
    --TODO add a Pause or Play Icon
end

function GIF_Reset(groupId)
    local p = _GIFPLAYERS[groupId]
    if p then p:reset() end
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

-- VERSION --
FILE_RYGIF_VERSION = 137