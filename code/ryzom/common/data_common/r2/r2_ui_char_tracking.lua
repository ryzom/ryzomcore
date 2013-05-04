-- chars tracking window

CharTracking = 
{
	CurrList = {},
	CurrActiveList = {},
	SortDir = {},			
	SelectedCharId = nil,
	TextCache = {}, -- cache for text widgets
	CenteredTextCache = {},
	NumberCache = {}, -- cache for number widgets
	BitmapCache = {}, -- cache for bitmap widgets

	-- Map scenario flags to the bitmaps displayed in the first column
	FlagsToTex =
	{
		[0] = { Bitmap = "blank.tga",              Color = "0   0   0   0  " },
		[1] = { Bitmap = "r2ed_connected_char.tga", Color = "255 255 255 255" },
		[2] = { Bitmap = "r2ed_kicked_char.tga", Color = "255 255 255 255" },
	},

	WaitingList = false,
	FirstShow = true,
	RefreshPeriod = 10,
	WaitingPeriod = 15,
	MinRefreshPeriod = 4,
	LastRefreshTime = 0,	
	LastRefreshQuerryTime = 0,
	PendingRefresh = false,
	ListReceived = false,

	--
	RaceTypeToUtf8 = {},
	ReligionTypeToUtf8 = {},
	LevelTypeToUtf8 = {},
}

local sampleList1 = 
{
}
table.insert(sampleList1,  { Id=0, Char = "plop",		Guild="Guild1",			Race=0,	Religion=0,	Shard="aniro",	Level=0, Flags=0})
table.insert(sampleList1,  { Id=1, Char = "titi",		Guild="Guild1",			Race=1,	Religion=1, Shard="leanon",	Level=1, Flags=1})
table.insert(sampleList1,  { Id=2, Char = "bob",		Guild="",				Race=0,	Religion=2, Shard="aniro",	Level=0, Flags=1})
table.insert(sampleList1,  { Id=3, Char = "bobette",	Guild="Guild2",			Race=0,	Religion=1, Shard="aniro",	Level=2, Flags=1})
table.insert(sampleList1,  { Id=4, Char = "nico",		Guild="Super guild",	Race=3,	Religion=0,	Shard="aniro",	Level=2, Flags=0})
table.insert(sampleList1,  { Id=5, Char = "toto2",	Guild="GG",				Race=2,	Religion=1, Shard="aniro",	Level=2, Flags=1})
table.insert(sampleList1,  { Id=6, Char = "titi2",	Guild="Plop guild",		Race=3,	Religion=1, Shard="aniro",	Level=2, Flags=1})
table.insert(sampleList1,  { Id=7, Char = "bob2",		Guild="Plop guild",		Race=3,	Religion=1, Shard="leanon",	Level=3, Flags=0})
table.insert(sampleList1,  { Id=8, Char = "bobette2", Guild="Guild1",			Race=1,	Religion=2, Shard="leanon",	Level=3, Flags=0})
table.insert(sampleList1,  { Id=9, Char = "nico2",	Guild="Super guild",	Race=3,	Religion=1, Shard="leanon",	Level=4, Flags=0})
table.insert(sampleList1,  { Id=10, Char = "toto3",	Guild="Guild2",			Race=2,	Religion=2, Shard="leanon",	Level=4, Flags=1})
table.insert(sampleList1,  { Id=11, Char = "titi3",	Guild="GG",				Race=0,	Religion=0,	Shard="leanon",	Level=5, Flags=1})
table.insert(sampleList1,  { Id=12, Char = "bob3",	Guild="Plop guild",		Race=3,	Religion=1, Shard="leanon",	Level=5, Flags=1})
table.insert(sampleList1,  { Id=13, Char = "bobette3",Guild="GG",				Race=3,	Religion=2, Shard="aniro",	Level=5, Flags=0})
table.insert(sampleList1,  { Id=14, Char = "nico3",	Guild="GG",				Race=3,	Religion=1, Shard="aniro",	Level=2, Flags=1})
table.insert(sampleList1,  { Id=15, Char = "toto4",	Guild="",				Race=2,	Religion=0,	Shard="aniro",	Level=3, Flags=0})
table.insert(sampleList1,  { Id=16, Char = "titi'",	Guild="TSuper guild",	Race=1,	Religion=0,	Shard="aniro",	Level=1, Flags=1})
table.insert(sampleList1,  { Id=17, Char = "bob4",	Guild="GG",				Race=1,	Religion=2, Shard="aniro",	Level=3, Flags=0})
table.insert(sampleList1,  { Id=18, Char = "bobette4",Guild="Guild2",			Race=2,	Religion=1, Shard="leanon",	Level=0, Flags=1})
table.insert(sampleList1,  { Id=19, Char = "nico4",	Guild="",				Race=0,	Religion=0,	Shard="leanon",	Level=1, Flags=1})


local boolToInt = 
{ 
}
boolToInt[false] = 0
boolToInt[true] = 1


--*********************************
-- standard comparison
local function less(lhs, rhs)
	if type(lhs) == "boolean" then
		return boolToInt[lhs] < boolToInt[rhs]
	else
		return lhs < rhs
	end
end

--*********************************
-- reversed comparison
local function more(lhs, rhs)
	return not less(lhs,rhs)
end

--*********************************
-- sorting by race type
local function lessRaceType(lhs, rhs)
	return CharTracking.RaceTypeToUtf8[lhs] < CharTracking.RaceTypeToUtf8[rhs]
end

--*********************************
-- sorting by religion type
local function lessReligionType(lhs, rhs)
	return CharTracking.ReligionTypeToUtf8[lhs] < CharTracking.ReligionTypeToUtf8[rhs]
end


-- init sort dir
table.insert(CharTracking.SortDir, { Var="Flags", Up=false, Cmp = more })
table.insert(CharTracking.SortDir, { Var="Char", Up=false, Cmp = less})
table.insert(CharTracking.SortDir, { Var="Guild", Up=false, Cmp = less })
table.insert(CharTracking.SortDir, { Var="Race", Up=false, Cmp = lessRaceType })
table.insert(CharTracking.SortDir, { Var="Religion", Up=false, Cmp = lessReligionType })
table.insert(CharTracking.SortDir, { Var="Shard", Up=false, Cmp = less })
table.insert(CharTracking.SortDir, { Var="Level", Up=false, Cmp = less })

--***********************************************************************
function CharTracking:getWindow()
	return getUI("ui:interface:ring_chars_tracking")
end

--***********************************************************************
function CharTracking:initRaceTypes()
	for k = 0, 3 do	
		self.RaceTypeToUtf8[k] = i18n.get("uiRAP_CharRace_" .. tostring(k)):toUtf8()	
	end
end

--***********************************************************************
function CharTracking:initReligionTypes()
	for k = 0, 2 do		
		self.ReligionTypeToUtf8[k] = i18n.get("uiRAP_CharReligion_" .. tostring(k)):toUtf8()		
	end
end

--***********************************************************************
function CharTracking:initLevelTypes()
	for k = 0, 5 do	
		self.LevelTypeToUtf8[k] = i18n.get("uiRAP_CharLevel_" .. tostring(k)):toUtf8()		
	end
end


--***********************************************************************
function CharTracking:isConnected(flags)
	return flags == 1
end

--***********************************************************************
function CharTracking:isKicked(flags)
	return flags == 2
end

--***********************************************************************
function CharTracking:getColumn(name)
	return getUI("ui:interface:ring_chars_tracking:content:main:enclosing:columns:getw:column_group:" .. name .. ":values")
end


--***********************************************************************
function CharTracking:getSelectList()
	return getUI("ui:interface:ring_chars_tracking:content:main:enclosing:columns:getw:select")
end


local scratchUCStr = ucstring()

--***********************************************************************
function CharTracking:newTemplate(name, cache)	
	local group
	if table.getn(cache) ~= 0 then
		group = cache[table.getn(cache)]
		table.remove(cache, table.getn(cache))
	else		
		group = createGroupInstance(name, "", {})	
	end
	return group
end

--***********************************************************************
-- build a new text group from utf8 text
function CharTracking:newTextLabel(value)		
	local group = self:newTemplate("rap_text", self.TextCache)
	scratchUCStr:fromUtf8(value)
	group:find("t").uc_hardtext_single_line_format = scratchUCStr
	return group
end

--***********************************************************************
-- build a new text group from utf8 text
function CharTracking:newCenteredTextLabel(value)		
	local group = self:newTemplate("rap_text_centered", self.CenteredTextCache)
	scratchUCStr:fromUtf8(value)
	group:find("t").uc_hardtext_single_line_format = scratchUCStr
	return group
end

--***********************************************************************
function CharTracking:newNumberLabel(value)	
	local group = self:newTemplate("rap_number", self.NumberCache)
	group:find("t").uc_hardtext_single_line_format = tostring(value)
	return group
end

--***********************************************************************
function CharTracking:newBitmap(texName, color)	
	if color == nil then color = "255 255 255 255" end
	local group = self:newTemplate("rap_bitmap", self.BitmapCache)	
	group.f.texture = texName
	group.f.color = color
	return group
end


--***********************************************************************
function CharTracking:newBooleanLabel(value)	
	local group = self:newTemplate("rap_bitmap", self.BitmapCache)
	if value == true then
		group.f.texture="patch_on.tga"
		group.f.color="255 255 255 255"
	else
		group.f.texture="blank.tga"
		group.f.color="0 0 0 0"		
	end
	return group
end

--***********************************************************************
function CharTracking:addLine(line)	
	self:getColumn("char"):addChild(self:newTextLabel(line.Char))	
	self:getColumn("guild"):addChild(self:newTextLabel(line.Guild))	
	self:getColumn("race"):addChild(self:newTextLabel(self.RaceTypeToUtf8[line.Race]))	
	self:getColumn("religion"):addChild(self:newTextLabel(self.ReligionTypeToUtf8[line.Religion]))	
	self:getColumn("shard"):addChild(self:newTextLabel(line.Shard))	
	self:getColumn("level"):addChild(self:newNumberLabel(self.LevelTypeToUtf8[line.Level]))	
	self:getColumn("flags"):addChild(self:newBitmap(self.FlagsToTex[line.Flags].Bitmap, self.FlagsToTex[line.Flags].Color))	
end

--***********************************************************************
function CharTracking:putColumnInCache(column, cache)	
	local childrenCount = column.childrenNb
	for i = 0, childrenCount - 1 do
		local child = column:getChild(column.childrenNb - 1)
		table.insert(cache, child)
		column:detachChild(child)
	end
end


--***********************************************************************
function CharTracking:putMixedColumnInCache(column, textCache, bitmapCache)	
	local childrenCount = column.childrenNb
	for i = 0, childrenCount - 1 do
		local child = column:getChild(column.childrenNb - 1)
		if child:find("t") then			
			table.insert(textCache, child)
		else
			table.insert(bitmapCache, child)
		end
		column:detachChild(child)
	end
end

--***********************************************************************
function CharTracking:putInCache()
	self.TextCache = {}
	self.CenteredTextCache = {}
	self.NumberCache = {}
	self.BitmapCache = {}
	self:putColumnInCache(self:getColumn("char"), self.TextCache)
	self:putColumnInCache(self:getColumn("guild"), self.TextCache)
	self:putColumnInCache(self:getColumn("race"), self.TextCache)
	self:putColumnInCache(self:getColumn("shard"), self.TextCache)
	self:putColumnInCache(self:getColumn("religion"), self.TextCache)
	--
	self:putColumnInCache(self:getColumn("level"), self.NumberCache)
	--	
	self:putColumnInCache(self:getColumn("flags"), self.BitmapCache)	
end

--***********************************************************************
function CharTracking:clear()
	self:getColumn("flags"):clear()
	self:getColumn("char"):clear()
	self:getColumn("guild"):clear()
	self:getColumn("race"):clear()
	self:getColumn("religion"):clear()
	self:getColumn("shard"):clear()
	self:getColumn("level"):clear()
	
	self.TextCache = {}
	self.CenteredTextCache = {}
	self.NumberCache = {}
	self.BitmapCache = {}
	self:getSelectList():clear()
	self:getSelectList().active = false
	self.CurrList = {}
	self.CurrActiveList = {}
	self.ListReceived = false
end

--***********************************************************************
function CharTracking:testFill1()
	self:onCharsListReceived(sampleList1)
end

--***********************************************************************
function CharTracking:onCharsListReceived(list)
	self.WaitingList = false
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.ListReceived = true
	self:fill(list)	
end	

--***********************************************************************
function CharTracking:fill(list)	
		
	-- if the window is not active, then maybe an old msg -> ignore
	if not self:getWindow().active then return end	
	self:enableButtons(true)	
	--	
	local startTime = nltime.getPreciseLocalTime()
	--
	self:getWindow():find("teleport").frozen = true
	self:getWindow():find("kick").frozen = true
	self:getWindow():find("unkick").frozen = true
	self:getWindow():find("tell").frozen = true	
	--
	self:putInCache()
	--	
	self.CurrList = list
	self.CurrActiveList = {}
	self:sort()	
	local selectList = self:getSelectList()
	selectList:clear()
	local count = 0
	local displayedCount = 0
	local lastCharFound = false
	for k, v in pairs(self.CurrList) do
		count = count + 1
	    local active

		self:addLine(v)
		local newGroup = createGroupInstance("rap_select_line", selectList.id, { id=tostring(v.Id),  
			params_l="CharTracking:onLineLeftClick()", params_r="CharTracking:onLineRightClick()", 
			params_dblclick_l="CharTracking:onLineLeftDblClick()",
			on_tooltip_params="CharTracking:onLineCharacterTooltip()", 
			tooltip="uiRAP_InvitedAndConnected"})

		selectList:addChild(newGroup)		
		newGroup:find("but").pushed = (v.Id == self.SelectedCharId)
		if v.Id == self.SelectedCharId then
			lastCharFound = true
		end
		table.insert(self.CurrActiveList, v)
		displayedCount = displayedCount + 1
	end

	if not lastCharFound then
		self.SelectedCharId = nil
	end

	if displayedCount == 0 then				
		if count == 0 then 
			self:setErrorMessage(i18n.get("uiRAP_NoCharFound"))
			self:enableButtons(false)
		end
	else
		self:clearMessage()
	end
	local endTime = nltime.getPreciseLocalTime()
	self:updateJoinAndTellButtons()		
end

--***********************************************************************
function CharTracking:onLineCharacterTooltip()

	if self:isConnected(self.CurrActiveList[self:getSelectList():getElementIndex(getUICaller().parent) + 1].Flags) then
		setContextHelpText(i18n.get("uiRAP_InvitedAndConnected"))
	else
		setContextHelpText(i18n.get("uiRAP_InvitedAndNotConnected"))
	end
end

--***********************************************************************
function CharTracking:setMessage(msg, color)		
	-- display the error msg at the bottom of the window
	local errorTxt = self:getWindow():find("refreshText")	

	-- The version below set the msg in the middle of the window
	--local errorTxt = self:getWindow():find("errorMsg")

	errorTxt.uc_hardtext = msg
	errorTxt.color = color
	errorTxt.active=true
	--self:getWindow():invalidateCoords()
end

--***********************************************************************
function CharTracking:clearMessage()
	--local errorTxt = self:getWindow():find("errorMsg")
	local errorTxt = self:getWindow():find("refreshText")
	errorTxt.active = false
end

--***********************************************************************
function CharTracking:setErrorMessage(msg)
	self:setMessage(msg, "192 64 0 255")
end

--***********************************************************************
function CharTracking:setInfoMessage(msg)
	self:setMessage(msg, "255 255 255 255")
end


--***********************************************************************
function CharTracking:sort(list)
	local sortDir = self.SortDir			
	local function sorter(lhs, rhs)
		for k = 1, table.getn(sortDir) do
			if lhs[sortDir[k].Var] ~= rhs[sortDir[k].Var] then
				if sortDir[k].Up then
					return not sortDir[k].Cmp(lhs[sortDir[k].Var], rhs[sortDir[k].Var])					
				else
					return sortDir[k].Cmp(lhs[sortDir[k].Var], rhs[sortDir[k].Var])			
				end
			end
		end
		return false
	end
	table.sort(self.CurrList, sorter)
end

--***********************************************************************
function CharTracking:headerLeftClick(down, criterion)	
	-- change column sort order
	local parent = getUICaller().parent		
	parent.tdown.active = down
	parent.tup.active = not down		
	-- insert 		
	local sortDir = self.SortDir
	for k = 1, table.getn(sortDir) do
		if sortDir[k].Var == criterion then			
			sortDir[k].Up = not sortDir[k].Up			
			table.insert(sortDir, 1, sortDir[k])			
			table.remove(sortDir, k + 1)			
			if self.ListReceived then
				self:fill(self.CurrList) -- update only if list has been received
			end
			return
		end
	end	
end

--***********************************************************************
function CharTracking:getCharFromId(id)
	for k, v in pairs(self.CurrList) do
		if v.Id == id then return v end
	end
	return nil
end

--***********************************************************************
function CharTracking:onLineLeftClick()
	self.SelectedCharId = self.CurrActiveList[self:getSelectList():getElementIndex(getUICaller().parent) + 1].Id	
	local selectList = self:getSelectList()
	for k = 0, selectList.childrenNb - 1 do
		local but = selectList:getChild(k):find("but")
		if but == getUICaller() then
			but.pushed = true	
		else
			but.pushed = false
		end
	end 
	--self:getWindow():find("kick").frozen = not self:isConnected(self:getCharFromId(self.SelectedCharId).Flags)
	--self:getWindow():find("unkick").frozen = not self:isKicked(self:getCharFromId(self.SelectedCharId).Flags)
	--self:getWindow():find("teleport").frozen = not self:isConnected(self:getCharFromId(self.SelectedCharId).Flags)
	--self:getWindow():find("tell").frozen = false

	self:updateJoinAndTellButtons()	
end

--***********************************************************************
function CharTracking:updateJoinAndTellButtons()
	if 	self.SelectedCharId then
		self:getWindow():find("kick").frozen = not self:isConnected(self:getCharFromId(self.SelectedCharId).Flags)
		self:getWindow():find("unkick").frozen = not self:isKicked(self:getCharFromId(self.SelectedCharId).Flags)
		self:getWindow():find("teleport").frozen = not self:isConnected(self:getCharFromId(self.SelectedCharId).Flags) or (r2.Mode=="r2ed_anim_test")
		self:getWindow():find("tell").frozen = false
	else
		self:getWindow():find("kick").frozen = true
		self:getWindow():find("unkick").frozen = true
		self:getWindow():find("teleport").frozen = true
		self:getWindow():find("tell").frozen = true
	end
end


--***********************************************************************
function CharTracking:onLineRightClick()
	self:onLineLeftClick()	
	local menu = getUI("ui:interface:ring_char_menu")

	menu:find("tell").uc_hardtext = ucstring(i18n.get("uiRAP_MenuCharTell"):toUtf8() .. "'" .. self:getCharFromId(self.SelectedCharId).Char .."'")
	menu:find("teleport").uc_hardtext = ucstring(i18n.get("uiRAP_MenuTeleportTo"):toUtf8() .. "'" .. self:getCharFromId(self.SelectedCharId).Char.."'")
	menu:find("kick").uc_hardtext = ucstring(i18n.get("uiRAP_MenuKick"):toUtf8() .. "'" .. self:getCharFromId(self.SelectedCharId).Char.."'")
	menu:find("unkick").uc_hardtext = ucstring(i18n.get("uiRAP_MenuUnkick"):toUtf8() .. "'" .. self:getCharFromId(self.SelectedCharId).Char.."'")

	menu:find("unkick").grayed = not self:isKicked(self:getCharFromId(self.SelectedCharId).Flags)
	menu:find("kick").grayed = not self:isConnected(self:getCharFromId(self.SelectedCharId).Flags)
	menu:find("teleport").grayed = not self:isConnected(self:getCharFromId(self.SelectedCharId).Flags) or (r2.Mode=="r2ed_anim_test")
	launchContextMenuInGame("ui:interface:ring_char_menu")
end

--***********************************************************************
function CharTracking:onTell()
	debugInfo("tell to owner of session" .. self.SelectedCharId)
	char = ucstring()	
	char:fromUtf8(self:getCharFromId(self.SelectedCharId).Char)
	tell(char, i18n.get("uiRAP_AskForTall"))
end

--***********************************************************************
function CharTracking:onKick()
	r2:kickCharacter(self.SelectedCharId)
end

--***********************************************************************
function CharTracking:onUnkick()
	r2:unkickCharacter(self.SelectedCharId)
end

--***********************************************************************
function CharTracking:onTeleportTo()
	r2:teleportToCharacter(self.SelectedCharId)
end

--***********************************************************************
function CharTracking:onLineLeftDblClick()
	local char = self:getCharFromId(self.SelectedCharId)
	if self:isConnected(char.Flags) then
		onTell()		
	end
end

--***********************************************************************
function CharTracking:refresh()

	self.PendingRefresh = true	
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.WaitingList = true
	--debugInfo("*refresh*")
end	

--***********************************************************************
-- called by C++ when someone is invited
function CharTracking:forceRefresh()
	self:refresh()
	self.LastRefreshQuerryTime = 0 -- make believe last refresh was *quite* long ago...
end	


--***********************************************************************
function CharTracking:updatePendingRefresh()

	if self.PendingRefresh then
		local currTime = nltime.getLocalTime() / 1000
		if currTime - self.LastRefreshQuerryTime > self.MinRefreshPeriod and game.getRingCharList then			
			self.LastRefreshQuerryTime = currTime
			self.PendingRefresh = false
			-- when you load an animation, lua state isn't initialized for a short time
			game.getRingCharList()
		end
	end
end

--***********************************************************************
function CharTracking:onShow()
	self:initRaceTypes()
	self:initReligionTypes()
	self:initLevelTypes()
	setOnDraw(self:getWindow(), "CharTracking:onDraw()")		
	self:clear()
	self:enableButtons(false)	
	self:refresh()	
end

local waitTextColor = CRGBA(255, 255, 255, 255)


--***********************************************************************
function CharTracking:enableButtons(enabled)
	local win = self:getWindow()
	local alpha
	if enabled then alpha = 255 else alpha = 128 end
	self:getSelectList().active = enabled	
end

--***********************************************************************
function CharTracking:show()
	-- update msg	
	local win = self:getWindow()	
	win.active = true
	if self.FirstShow then
		local w, h = getWindowSize()
		win.w = w * 5 / 6
		win.h = h * 5 / 6
		win:invalidateCoords()	
		win:updateCoords()
		win:center()
		win:invalidateCoords()	
		win:updateCoords()
		self.FirstShow = false
	end
	win:blink(1)
end

--***********************************************************************
function CharTracking:enlargeColumns()
	self:getWindow():find("header_line"):enlargeColumns(10)
	local enlargeButton = self:getWindow():find("enlarge")
	enlargeButton.frozen = true
end

--***********************************************************************
function CharTracking:onResize()
	local enlargeButton = self:getWindow():find("enlarge")
	enlargeButton.frozen = false
end

--***********************************************************************
function CharTracking:resizeColumnsAndContainer()
	self:getWindow():find("header_line"):resizeColumnsAndContainer(5)
end

--***********************************************************************
function CharTracking:onDraw()
		
	local timeInSec = nltime.getLocalTime() / 1000
	if self.WaitingList then
		if timeInSec - self.LastRefreshTime > self.WaitingPeriod then		
			self.WaitingList = false
			self.LastRefreshTime = nltime.getLocalTime() / 1000
			--self:getWindow():find("refreshText").active = false
		else	
			local waitText = i18n.get("uiRAP_WaitChars" .. math.mod(os.time(), 3))
			self:setInfoMessage(waitText)			
			--local refreshText = self:getWindow():find("refreshText")
			--if not self.ListReceived then
			--	self:setInfoMessage(waitText)
			--	waitTextColor.A = 127 + 127 * (0.5 + 0.5 * math.cos(6 * timeInSec))
			--	local errorTxt = self:getWindow():find("errorMsg")
			--	errorTxt.color_rgba = waitTextColor
			--	refreshText.active = false
			--else			
			--	refreshText.active = true
			--	refreshText.uc_hardtext = waitText 
			--end
		end
	else
		if timeInSec - self.LastRefreshTime > self.RefreshPeriod then		
			self:refresh()
		else
			--debugInfo("pas de refresh")
		end
	end
	self:updatePendingRefresh()
end	




