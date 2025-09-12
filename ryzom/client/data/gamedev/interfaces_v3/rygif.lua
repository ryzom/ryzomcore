-- ## a smale fake gif player

local rygif = {}
rygif.__index = rygif

-- =================== Global loader state ===================
-- Shared across all rygif instances.
local LoadedStems   = {}
local PendingStems  = {}
local MaxAtlasesPerFrame = 1
local LoadTimeBudgetSec  = 0.002

-- =================== Utility ===================
local function file_exists(path)
    local f = io.open(path, "r")
    if f then f:close() return true end
    return false
end

local function first_existing_path(candidates)
    for _, p in ipairs(candidates) do
        local f = io.open(p, "r")
        if f then f:close(); return p end
    end
    return nil
end

local function list_uv_packs(base)
    local packs = {}
    local i = 1

    -- Try multi-pack: base_1.txt, base_2.txt, ...
    while true do
        local uv = first_existing_path({
            ("user/%s_%d.txt"):format(base, i),  -- prefer user/
            ("%s_%d.txt"):format(base, i),       -- fallback: root
        })
        if not uv then break end

        -- Stem = filename without path + ".txt" (e.g., "trailer_2")
        local stem = uv:match("([^/\\]+)%.txt$")
        table.insert(packs, { stem = stem, uv = uv })
        i = i + 1
    end

    -- Fallback: single-pack base.txt
    if #packs == 0 then
        local uv_single = first_existing_path({
            ("user/%s.txt"):format(base),        -- prefer user/
            ("%s.txt"):format(base),             -- fallback: root
        })
        if uv_single then
            local stem = uv_single:match("([^/\\]+)%.txt$")
            packs = { { stem = stem, uv = uv_single } }
        end
    end

    return packs
end

local function extract_num(s)
    local n = tostring(s):match("(%d+)%.[^%.]+$")
    return n and tonumber(n) or nil
end

local function sort_frames_natural(frames)
    local keyed, all_num = {}, true
    for _, name in ipairs(frames) do
        local n = extract_num(name)
        if not n then all_num = false break end
        table.insert(keyed, { n = n, name = name })
    end
    if all_num then
        table.sort(keyed, function(a,b) return a.n < b.n end)
        local out = {}
        for _, it in ipairs(keyed) do table.insert(out, it.name) end
        return out
    end
    return frames
end

-- Per-instance warm (touches a texture once so the engine uploads it to VRAM)
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
        if loaded_this_frame > 0 and elapsed >= LoadTimeBudgetSec then
            break
        end
        local stem = table.remove(PendingStems, 1)
        loadTextures(stem, true) -- loads <stem>.png + <stem>.txt
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
-- opts (optional): { groupPath = "...", groupId = "..." }
function rygif.new(base, fps, x, y, loop, opts)
    local self = setmetatable({}, rygif)
    self.fps  = fps or 12
    self.x    = x or 0
    self.y    = y or 0
    self.loop = (loop ~= false)
    self.base = base

    -- Per-instance UI targets
    self.targetGroupPath = (opts and opts.groupPath)
    self.targetGroupId   = (opts and opts.groupId)

    self.frames = {}
    self.frameCount = 0
    self.ready = false
    self.warmed = false
    self.playAfterReady = true

    -- Playback/pausing state
    self.startTime   = os.clock()
    self.paused      = false
    self.pauseStart  = 0
    self.pauseAccum  = 0
    self.currentIndex = 1
    self.pausedIndex  = 1

    -- 1) Find packs and enqueue
    self.packs = list_uv_packs(base)
    assert(#self.packs > 0, ("No UV files found for '%s' (expected %s.txt or user/%s.txt, ...)"):format(base, base, base))

    local stems = {}
    for _, p in ipairs(self.packs) do table.insert(stems, p.stem) end
    enqueue_stems(stems, true)

    -- 2) Read UVs
    for _, p in ipairs(self.packs) do
        local f = assert(io.open(p.uv, "r"), "Cannot open: " .. p.uv)
        for line in f:lines() do
            line = line:match("^%s*(.-)%s*$")
            if line ~= "" and line:sub(1,1) ~= "#" then
                local fname = line:match("([^%s]+)")
                if fname then table.insert(self.frames, fname) end
            end
        end
        f:close()
    end

    self.frames = sort_frames_natural(self.frames)
    self.frameCount = #self.frames
    assert(self.frameCount > 0, "No frames found in UV files.")

    return self
end

function rygif:isReady()
    for _, p in ipairs(self.packs) do
        if LoadedStems[p.stem] ~= true then
            return false
        end
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
    -- Pump loader a bit each frame
    pump_loader_slice()

    -- Wait for readiness
    if not self.ready then
        if self:isReady() then
            self.ready = true
            if not self.warmed and #self.frames > 0 then
                warm_texture_once_for(self.targetGroupPath, self.targetGroupId, self.frames[1])
                self.warmed = true
            end
            if self.playAfterReady then self.paused = false end
        else
            -- Not ready yet: show first frame as a stand-in
            if #self.frames > 0 then
                self:TextureDraw(self.frames[1], self.x, self.y)
            end
            return
        end
    end

    -- If paused, draw the frozen frame captured when pausing
    if self.paused then
        local idx = self.pausedIndex or 1
        if idx < 1 then idx = 1 end
        if idx > self.frameCount then idx = self.frameCount end
        self:TextureDraw(self.frames[idx], self.x, self.y)
        return
    end

    -- Running: compute current frame index and draw it
    local now = os.clock()
    local elapsed = now - self.startTime - (self.pauseAccum or 0)
    local idx = math.floor(elapsed * (self.fps or 12))
    if self.loop then
        idx = (idx % self.frameCount) + 1
    else
        idx = math.min(idx + 1, self.frameCount)
    end
    self.currentIndex = idx
    self:TextureDraw(self.frames[idx], self.x, self.y)
end

function rygif:TextureDraw(texture_img, x, y)
    local main_ui = getUI(self.targetGroupPath)
    if not main_ui then return end
    local group = main_ui:find(self.targetGroupId)
    if not group then return end
    local img = group:find("texture_img")
    if not img then return end
    img.texture = texture_img
end

return rygif
