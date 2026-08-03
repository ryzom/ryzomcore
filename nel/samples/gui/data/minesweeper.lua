-- Minesweeper for the NeL GUI showcase.
--
-- The whole game lives in this file plus minesweeper_sample.xml: cells are
-- plain button/text/bitmap widgets and the logic below drives them through
-- the reflection system (getUI is provided by the sample bootstrap; NLGUI
-- leaves widget lookup to the embedder). No C++ game code.

ms = {}

ms.W = 9
ms.H = 9
ms.MINES = 10
ms.WINDOW = "ui:sample:minesweeper"
ms.PREFIX = ms.WINDOW .. ":content:"

-- classic minesweeper digit colors, brightened for the dark w_ skin
ms.NUMBER_COLORS = {
	"120 170 255 255", -- 1
	"120 220 120 255", -- 2
	"255 110 110 255", -- 3
	"170 140 255 255", -- 4
	"230 180 110 255", -- 5
	"110 220 220 255", -- 6
	"230 230 230 255", -- 7
	"160 160 160 255", -- 8
}
ms.BG_NORMAL = "255 255 255 255"
ms.BG_BOOM = "255 80 80 255"

-- state; mines are placed on the first reveal so the first click is safe
ms.mine = {}
ms.adj = {}
ms.open = {}
ms.flagged = {}
ms.opened = 0
ms.flags = 0
ms.placed = false
ms.over = false
ms.flagMode = false
ms.running = false
ms.startTime = 0
ms.shownTime = -1

local function cell(i)
	return getUI(ms.PREFIX .. "grid:c" .. i)
end

local function setText(id, text)
	getUI(ms.PREFIX .. id).hardtext = text
end

local function setMinesLeft()
	setText("mines", string.format("%03d", ms.MINES - ms.flags))
end

function ms.neighbors(i)
	local result = {}
	local x = math.fmod(i, ms.W)
	local y = math.floor(i / ms.W)
	for dy = -1, 1 do
		for dx = -1, 1 do
			local nx = x + dx
			local ny = y + dy
			if (dx ~= 0 or dy ~= 0) and nx >= 0 and nx < ms.W and ny >= 0 and ny < ms.H then
				table.insert(result, ny * ms.W + nx)
			end
		end
	end
	return result
end

-- deal the mines, keeping the first-clicked cell and its ring clear
function ms.place(safe)
	local forbidden = {}
	forbidden[safe] = true
	for _, n in ipairs(ms.neighbors(safe)) do
		forbidden[n] = true
	end
	local count = 0
	while count < ms.MINES do
		local i = math.random(0, ms.W * ms.H - 1)
		if not ms.mine[i] and not forbidden[i] then
			ms.mine[i] = true
			count = count + 1
		end
	end
	for i = 0, ms.W * ms.H - 1 do
		local adj = 0
		for _, n in ipairs(ms.neighbors(i)) do
			if ms.mine[n] then adj = adj + 1 end
		end
		ms.adj[i] = adj
	end
	ms.placed = true
	ms.startTime = nltime.getPreciseLocalTime()
	ms.shownTime = -1
	ms.running = true
	setOnDraw(getUI(ms.WINDOW), "ms.tick()")
	setText("status", "Minefield live. Good luck.")
end

local function showOpen(i)
	local c = cell(i)
	c.cover.active = false
	if ms.adj[i] > 0 then
		c.t.hardtext = tostring(ms.adj[i])
		c.t.color = ms.NUMBER_COLORS[ms.adj[i]]
	end
end

function ms.boom(i)
	ms.over = true
	ms.running = false
	for j = 0, ms.W * ms.H - 1 do
		if ms.mine[j] and not ms.flagged[j] then
			local c = cell(j)
			c.cover.active = false
			c.mine.active = true
		end
	end
	-- solid red floor under the detonated mine (w_box_blank is too dark to tint)
	local hit = cell(i)
	hit.bg.texture = "blank.tga"
	hit.bg.color = ms.BG_BOOM
	setText("status", "BOOM. New game?")
end

function ms.win()
	ms.over = true
	ms.running = false
	for j = 0, ms.W * ms.H - 1 do
		if ms.mine[j] and not ms.flagged[j] then
			cell(j).flag.active = true
		end
	end
	ms.flags = ms.MINES
	setMinesLeft()
	setText("status", string.format("Cleared in %d s!", ms.shownTime < 0 and 0 or ms.shownTime))
end

function ms.reveal(i)
	if ms.flagMode then
		ms.flag(i)
		return
	end
	if ms.over or ms.flagged[i] or ms.open[i] then return end
	if not ms.placed then ms.place(i) end
	if ms.mine[i] then
		ms.boom(i)
		return
	end
	local stack = { i }
	while #stack > 0 do
		local c = table.remove(stack)
		if not ms.open[c] and not ms.flagged[c] then
			ms.open[c] = true
			ms.opened = ms.opened + 1
			showOpen(c)
			if ms.adj[c] == 0 then
				for _, n in ipairs(ms.neighbors(c)) do
					if not ms.open[n] then table.insert(stack, n) end
				end
			end
		end
	end
	if ms.opened == ms.W * ms.H - ms.MINES then ms.win() end
end

function ms.flag(i)
	if ms.over or ms.open[i] then return end
	if ms.flagged[i] then
		ms.flagged[i] = nil
		ms.flags = ms.flags - 1
	else
		ms.flagged[i] = true
		ms.flags = ms.flags + 1
	end
	cell(i).flag.active = ms.flagged[i] ~= nil
	setMinesLeft()
end

function ms.toggleFlagMode()
	ms.flagMode = not ms.flagMode
end

-- per-frame while a game runs (installed with setOnDraw)
function ms.tick()
	if not ms.running then return end
	local t = math.floor(nltime.getPreciseLocalTime() - ms.startTime)
	if t > 999 then t = 999 end
	if t ~= ms.shownTime then
		ms.shownTime = t
		setText("timer", string.format("%03d", t))
	end
end

function ms.new()
	for i = 0, ms.W * ms.H - 1 do
		local c = cell(i)
		c.cover.active = true
		c.flag.active = false
		c.mine.active = false
		c.t.hardtext = ""
		c.bg.texture = "w_box_blank.tga"
		c.bg.color = ms.BG_NORMAL
	end
	ms.mine = {}
	ms.adj = {}
	ms.open = {}
	ms.flagged = {}
	ms.opened = 0
	ms.flags = 0
	ms.placed = false
	ms.over = false
	ms.running = false
	ms.shownTime = -1
	setOnDraw(getUI(ms.WINDOW), "")
	setMinesLeft()
	setText("timer", "000")
	setText("status", "Click a tile. Right-click flags.")
end

math.randomseed(nltime.getSecondsSince1970())
