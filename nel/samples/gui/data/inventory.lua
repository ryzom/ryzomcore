-- Drag-and-drop inventory for the NeL GUI showcase.
--
-- Real press-drag-release gestures in interface data only: the sample
-- bootstrap exposes getMousePos/getMouseDown (same signatures the client
-- gives its scripts), and a per-frame tick polls them to move a ghost
-- bitmap, highlight drop targets and apply the drop. Slots are plain
-- groups; everything is driven through reflected properties.

inv = {}

inv.WINDOW = "ui:sample:inventory"
inv.PREFIX = inv.WINDOW .. ":content:"

inv.ITEMS = {
	helmet     = { tex = "w_ar_helmet.tga",     name = "Helmet",  kind = "head"  },
	gilet      = { tex = "w_ar_gilet.tga",      name = "Vest",    kind = "chest" },
	hand       = { tex = "w_ar_hand.tga",       name = "Gloves",  kind = "hands" },
	armpad     = { tex = "w_ar_armpad.tga",     name = "Sleeves", kind = "arms"  },
	pantabotte = { tex = "w_ar_pantabotte.tga", name = "Pants",   kind = "legs"  },
	botte      = { tex = "w_ar_botte.tga",      name = "Boots",   kind = "feet"  },
}

-- equip slots accept one kind; bag slots (kind nil) accept anything
inv.SLOTS = {}
inv.slotById = {}
local function addSlot(id, path, kind)
	local s = { id = id, path = path, kind = kind }
	table.insert(inv.SLOTS, s)
	inv.slotById[id] = s
end
for _, e in ipairs({ { "eq_head", "head" }, { "eq_chest", "chest" }, { "eq_hands", "hands" },
	{ "eq_arms", "arms" }, { "eq_legs", "legs" }, { "eq_feet", "feet" } }) do
	addSlot(e[1], inv.PREFIX .. "equip:" .. e[1], e[2])
end
for i = 0, 11 do
	addSlot("b" .. i, inv.PREFIX .. "bag:b" .. i)
end

inv.contents = {} -- slot id -> item key
inv.drag = nil    -- { item = key, src = slot id } while a drag is live
inv.wasDown = false
inv.hlSlot = nil

local function setStatus(text)
	getUI(inv.PREFIX .. "status").hardtext = text
end

local function paintSlot(s)
	local icon = getUI(s.path).icon
	local item = inv.contents[s.id]
	if item then
		icon.texture = inv.ITEMS[item].tex
		icon.active = true
	else
		icon.active = false
	end
end

local function repaint()
	for _, s in ipairs(inv.SLOTS) do
		paintSlot(s)
	end
end

local function setHighlight(s)
	if inv.hlSlot == s then return end
	if inv.hlSlot then getUI(inv.hlSlot.path).hl.active = false end
	if s then getUI(s.path).hl.active = true end
	inv.hlSlot = s
end

local function accepts(s, item)
	return s.kind == nil or s.kind == inv.ITEMS[item].kind
end

local function slotAt(mx, my)
	for _, s in ipairs(inv.SLOTS) do
		local g = getUI(s.path)
		if mx >= g.x_real and mx < g.x_real + g.w_real
			and my >= g.y_real and my < g.y_real + g.h_real then
			return s
		end
	end
	return nil
end

local function checkEquipped()
	for _, s in ipairs(inv.SLOTS) do
		if s.kind and not inv.contents[s.id] then return end
	end
	setStatus("Fully equipped!")
end

-- per-frame while the window is active (installed with setOnDraw)
function inv.tick()
	local down = getMouseDown()
	local mx, my = getMousePos()
	local ghost = getUI(inv.PREFIX .. "ghost")
	if down and not inv.wasDown then
		local s = slotAt(mx, my)
		if s and inv.contents[s.id] then
			inv.drag = { item = inv.contents[s.id], src = s.id }
			inv.contents[s.id] = nil
			paintSlot(s)
			ghost.texture = inv.ITEMS[inv.drag.item].tex
			ghost.active = true
			setStatus("Dragging " .. inv.ITEMS[inv.drag.item].name .. "...")
		end
	elseif inv.drag and not down and inv.wasDown then
		local s = slotAt(mx, my)
		local d = inv.drag
		inv.drag = nil
		ghost.active = false
		setHighlight(nil)
		if s and s.id == d.src then
			inv.contents[d.src] = d.item
			setStatus("Drag armor onto its slot.")
		elseif s and accepts(s, d.item) then
			local other = inv.contents[s.id]
			if other and not accepts(inv.slotById[d.src], other) then
				inv.contents[d.src] = d.item
				setStatus("Can't swap: " .. inv.ITEMS[other].name .. " won't fit there.")
			else
				inv.contents[s.id] = d.item
				inv.contents[d.src] = other
				if s.kind then
					setStatus(inv.ITEMS[d.item].name .. " equipped.")
				else
					setStatus(inv.ITEMS[d.item].name .. " stowed.")
				end
			end
		elseif s then
			inv.contents[d.src] = d.item
			setStatus(inv.ITEMS[d.item].name .. " doesn't go there.")
		else
			inv.contents[d.src] = d.item
			setStatus(inv.ITEMS[d.item].name .. " returned.")
		end
		repaint()
		checkEquipped()
	end
	if inv.drag then
		local content = getUI(inv.WINDOW .. ":content")
		ghost.x = mx - content.x_real - 20
		ghost.y = my - content.y_real - 20
		local s = slotAt(mx, my)
		if s and accepts(s, inv.drag.item) then
			setHighlight(s)
		else
			setHighlight(nil)
		end
	end
	inv.wasDown = down
end

function inv.reset()
	inv.contents = {}
	inv.drag = nil
	setHighlight(nil)
	getUI(inv.PREFIX .. "ghost").active = false
	-- scattered around the bag rather than packed in a row
	inv.contents.b0 = "helmet"
	inv.contents.b2 = "gilet"
	inv.contents.b4 = "hand"
	inv.contents.b7 = "armpad"
	inv.contents.b9 = "pantabotte"
	inv.contents.b11 = "botte"
	repaint()
	setStatus("Drag armor onto its slot.")
end

inv.reset()
setOnDraw(getUI(inv.WINDOW), "inv.tick()")
