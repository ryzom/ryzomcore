-- players tracking window

PlayerTracking = 
{
	CurrList = {},
	CurrActiveList = {},
	SortDir = {},			
	SelectedPlayerId = nil,
	TextCache = {}, -- cache for text widgets
	CenteredTextCache = {},
	NumberCache = {}, -- cache for number widgets
	BitmapCache = {}, -- cache for bitmap widgets

	-- Map scenario flags to the bitmaps displayed in the first column
	FlagsToTex =
	{
		[0] = { Bitmap = "blank.tga",              Color = "0   0   0   0  " },
		[1] = { Bitmap = "rap_not_invited_dm.tga", Color = "255 255 255 255" },
	},

	WaitingList = false,
	FirstShow = true,
	RefreshPeriod = 10,
	MinRefreshPeriod = 4,
	LastRefreshTime = 0,	
	LastRefreshQuerryTime = 0,
	PendingRefresh = false,
	ListReceived = false,

	--
	RaceTypeToUtf8 = {},
	ReligionTypeToUtf8 = {},
	ShardTypeToUtf8 = {},
	LevelTypeToUtf8 = {},
}

local sampleList1 = 
{
}
table.insert(sampleList1,  { Id=0, Player = "plop",		Guild="Guild1",			Race=1,	Religion=1,	Shard=2,	Level=1, Flags=0})
table.insert(sampleList1,  { Id=1, Player = "titi",		Guild="Guild1",			Race=2,	Religion=2, Shard=4,	Level=2, Flags=1})
table.insert(sampleList1,  { Id=2, Player = "bob",		Guild="",				Race=1,	Religion=4, Shard=1,	Level=1, Flags=1})
table.insert(sampleList1,  { Id=3, Player = "bobette",	Guild="Guild2",			Race=1,	Religion=2, Shard=1,	Level=4, Flags=1})
table.insert(sampleList1,  { Id=4, Player = "nico",		Guild="Super guild",	Race=8,	Religion=1,	Shard=2,	Level=4, Flags=0})
table.insert(sampleList1,  { Id=5, Player = "toto2",	Guild="GG",				Race=4,	Religion=2, Shard=2,	Level=4, Flags=1})
table.insert(sampleList1,  { Id=6, Player = "titi2",	Guild="Plop guild",		Race=8,	Religion=2, Shard=4,	Level=4, Flags=1})
table.insert(sampleList1,  { Id=7, Player = "bob2",		Guild="Plop guild",		Race=8,	Religion=2, Shard=1,	Level=8, Flags=0})
table.insert(sampleList1,  { Id=8, Player = "bobette2", Guild="Guild1",			Race=2,	Religion=4, Shard=1,	Level=8, Flags=0})
table.insert(sampleList1,  { Id=9, Player = "nico2",	Guild="Super guild",	Race=8,	Religion=2, Shard=1,	Level=16, Flags=0})
table.insert(sampleList1,  { Id=10, Player = "toto3",	Guild="Guild2",			Race=4,	Religion=4, Shard=2,	Level=16, Flags=1})
table.insert(sampleList1,  { Id=11, Player = "titi3",	Guild="GG",				Race=1,	Religion=1,	Shard=4,	Level=32, Flags=1})
table.insert(sampleList1,  { Id=12, Player = "bob3",	Guild="Plop guild",		Race=8,	Religion=2, Shard=1,	Level=32, Flags=1})
table.insert(sampleList1,  { Id=13, Player = "bobette3",Guild="GG",				Race=8,	Religion=4, Shard=2,	Level=32, Flags=0})
table.insert(sampleList1,  { Id=14, Player = "nico3",	Guild="GG",				Race=8,	Religion=2, Shard=2,	Level=4, Flags=1})
table.insert(sampleList1,  { Id=15, Player = "toto4",	Guild="",				Race=4,	Religion=1,	Shard=1,	Level=8, Flags=0})
table.insert(sampleList1,  { Id=16, Player = "titi'",	Guild="TSuper guild",	Race=2,	Religion=1,	Shard=4,	Level=2, Flags=1})
table.insert(sampleList1,  { Id=17, Player = "bob4",	Guild="GG",				Race=2,	Religion=4, Shard=4,	Level=8, Flags=0})
table.insert(sampleList1,  { Id=18, Player = "bobette4",Guild="Guild2",			Race=4,	Religion=2, Shard=1,	Level=1, Flags=1})
table.insert(sampleList1,  { Id=19, Player = "nico4",	Guild="",				Race=1,	Religion=1,	Shard=1,	Level=2, Flags=1})


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
	return PlayerTracking.RaceTypeToUtf8[lhs] < PlayerTracking.RaceTypeToUtf8[rhs]
end

--*********************************
-- sorting by religion type
local function lessReligionType(lhs, rhs)
	return PlayerTracking.ReligionTypeToUtf8[lhs] < PlayerTracking.ReligionTypeToUtf8[rhs]
end


-- init sort dir
table.insert(PlayerTracking.SortDir, { Var="Flags", Up=false, Cmp = more })
table.insert(PlayerTracking.SortDir, { Var="Player", Up=false, Cmp = less})
table.insert(PlayerTracking.SortDir, { Var="Guild", Up=false, Cmp = less })
table.insert(PlayerTracking.SortDir, { Var="Race", Up=false, Cmp = lessRaceType })
table.insert(PlayerTracking.SortDir, { Var="Religion", Up=false, Cmp = lessReligionType })
table.insert(PlayerTracking.SortDir, { Var="Shard", Up=false, Cmp = less })
table.insert(PlayerTracking.SortDir, { Var="Level", Up=false, Cmp = less })

--***********************************************************************
function PlayerTracking:getWindow()
	return getUI("ui:login:ring_players_tracking")
end

--***********************************************************************
function PlayerTracking:initRaceTypes()
	for k = 0, 3 do	
		self.RaceTypeToUtf8[math.pow(2, k)] = i18n.get("uiRAP_PlayerRace_" .. tostring(k)):toUtf8()	
	end
end

--***********************************************************************
function PlayerTracking:initReligionTypes()
	for k = 0, 2 do		
		self.ReligionTypeToUtf8[math.pow(2, k)] = i18n.get("uiRAP_PlayerReligion_" .. tostring(k)):toUtf8()		
	end
end

--***********************************************************************
function PlayerTracking:initShardTypes()
	for k = 0, 2 do		
		self.ShardTypeToUtf8[math.pow(2, k)] = i18n.get("uiRAP_PlayerShard_" .. tostring(k)):toUtf8()		
	end
end

--***********************************************************************
function PlayerTracking:initLevelTypes()
	for k = 0, 5 do	
		self.LevelTypeToUtf8[math.pow(2, k)] = i18n.get("uiRAP_PlayerLevel_" .. tostring(k)):toUtf8()		
	end
end


--***********************************************************************
function PlayerTracking:isConnected(flags)
	return flags == 1
end

--***********************************************************************
function PlayerTracking:getColumn(name)
	return getUI("ui:login:ring_players_tracking:content:main:enclosing:columns:getw:column_group:" .. name .. ":values")
end


--***********************************************************************
function PlayerTracking:getSelectList()
	return getUI("ui:login:ring_players_tracking:content:main:enclosing:columns:getw:select")
end


local scratchUCStr = ucstring()

--***********************************************************************
function PlayerTracking:newTemplate(name, cache)	
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
function PlayerTracking:newTextLabel(value)		
	local group = self:newTemplate("rap_text", self.TextCache)
	scratchUCStr:fromUtf8(value)
	group:find("t").uc_hardtext_single_line_format = scratchUCStr
	return group
end

--***********************************************************************
-- build a new text group from utf8 text
function PlayerTracking:newCenteredTextLabel(value)		
	local group = self:newTemplate("rap_text_centered", self.CenteredTextCache)
	scratchUCStr:fromUtf8(value)
	group:find("t").uc_hardtext_single_line_format = scratchUCStr
	return group
end

--***********************************************************************
function PlayerTracking:newNumberLabel(value)	
	local group = self:newTemplate("rap_number", self.NumberCache)
	group:find("t").uc_hardtext_single_line_format = tostring(value)
	return group
end

--***********************************************************************
function PlayerTracking:newBitmap(texName, color)	
	if color == nil then color = "255 255 255 255" end
	local group = self:newTemplate("rap_bitmap", self.BitmapCache)	
	group.f.texture = texName
	group.f.color = color
	return group
end


--***********************************************************************
function PlayerTracking:newBooleanLabel(value)	
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
function PlayerTracking:addLine(line)

	self:getColumn("player"):addChild(self:newTextLabel(line.Player))
	self:getColumn("guild"):addChild(self:newTextLabel(line.Guild))
	self:getColumn("race"):addChild(self:newTextLabel(self.RaceTypeToUtf8[line.Race]))
	self:getColumn("religion"):addChild(self:newTextLabel(self.ReligionTypeToUtf8[line.Religion]))
	self:getColumn("shard"):addChild(self:newTextLabel(self.ShardTypeToUtf8[line.Shard]))
	self:getColumn("level"):addChild(self:newNumberLabel(self.LevelTypeToUtf8[line.Level]))
	self:getColumn("flags"):addChild(self:newBitmap(self.FlagsToTex[line.Flags].Bitmap, self.FlagsToTex[line.Flags].Color))
end

--***********************************************************************
function PlayerTracking:putColumnInCache(column, cache)	
	local childrenCount = column.childrenNb
	for i = 0, childrenCount - 1 do
		local child = column:getChild(column.childrenNb - 1)
		table.insert(cache, child)
		column:detachChild(child)
	end
end


--***********************************************************************
function PlayerTracking:putMixedColumnInCache(column, textCache, bitmapCache)	
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
function PlayerTracking:putInCache()
	self.TextCache = {}
	self.CenteredTextCache = {}
	self.NumberCache = {}
	self.BitmapCache = {}
	self:putColumnInCache(self:getColumn("player"), self.TextCache)
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
function PlayerTracking:clear()
	self:getColumn("flags"):clear()
	self:getColumn("player"):clear()
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
function PlayerTracking:testFill1()
	self:onPlayersListReceived(sampleList1)
end

--***********************************************************************
function PlayerTracking:onPlayersListReceived(list)
	self.WaitingList = false
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.ListReceived = true
	self:fill(list)	
end	

--***********************************************************************
function PlayerTracking:fill(list)	

	self:initRaceTypes()
	self:initReligionTypes()
	self:initShardTypes()
	self:initLevelTypes()
	
				
	-- if the window is not active, then maybe an old msg -> ignore
	if not self:getWindow().active then return end	
	self:enableButtons(true)	
	--	
	local startTime = nltime.getPreciseLocalTime()
	debugInfo(tostring(self.SelectedPlayerId))
	--
	self:getWindow():find("teleport").frozen = true
	self:getWindow():find("kick").frozen = true
	self:getWindow():find("tell").frozen = true	
	--
	self:putInCache()
	--
	debugInfo("***********************")
	debugInfo("TextCache size =  " .. table.getn(self.TextCache))
	debugInfo("CenteredTextCache size =  " .. table.getn(self.CenteredTextCache))
	debugInfo("NumberCache size =  " .. table.getn(self.NumberCache))
	debugInfo("BitmapCache size =  " .. table.getn(self.BitmapCache))	
	--self:clear()
	self.CurrList = list
	self.CurrActiveList = {}
	self:sort()	
	local selectList = self:getSelectList()
	selectList:clear()
	local count = 0
	local displayedCount = 0
	for k, v in pairs(self.CurrList) do
		count = count + 1
	    local active

		self:addLine(v)
		local newGroup = createGroupInstance("rap_select_line", "", { id=tostring(v.Id)})

		selectList:addChild(newGroup)		
		newGroup:find("but").pushed = (v.Id == self.SelectedPlayerId)
		table.insert(self.CurrActiveList, v)
		displayedCount = displayedCount + 1
	end
	
	if displayedCount == 0 then				
		if count ~= 0 then 
			self:setErrorMessage("uiRAP_NoSessionForLangFilter")
		else 
			--self:setErrorMessage(i18n.get("uiRAP_NoSessionFound"))
			self:setErrorMessage(ucstring())
			self:enableButtons(false)
		end
	else
		self:clearMessage()
	end
	local endTime = nltime.getPreciseLocalTime()
	debugInfo(string.format("time ellapsed =  %d", 1000 * (endTime - startTime)))
end


--***********************************************************************
function PlayerTracking:setMessage(msg, color)		
	local errorTxt = self:getWindow():find("errorMsg")
	errorTxt.uc_hardtext = msg
	errorTxt.color = color
	errorTxt.active=true
end

--***********************************************************************
function PlayerTracking:clearMessage()
	local errorTxt = self:getWindow():find("errorMsg")
	errorTxt.active = false
end

--***********************************************************************
function PlayerTracking:setErrorMessage(msg)
	self:setMessage(msg, "255 0 0 255")
end

--***********************************************************************
function PlayerTracking:setInfoMessage(msg)
	self:setMessage(msg, "255 255 255 255")
end


--***********************************************************************
function PlayerTracking:sort(list)
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
function PlayerTracking:headerLeftClick(down, criterion)	
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
function PlayerTracking:getPlayerFromId(id)
	for k, v in pairs(self.CurrList) do
		if v.Id == id then return v end
	end
	return nil
end

--***********************************************************************
function PlayerTracking:onLineLeftClick()
	self.SelectedPlayerId = self.CurrActiveList[self:getSelectList():getElementIndex(getUICaller().parent) + 1].Id	
	local selectList = self:getSelectList()
	for k = 0, selectList.childrenNb - 1 do
		local but = selectList:getChild(k):find("but")
		if but == getUICaller() then
			but.pushed = true	
		else
			but.pushed = false
		end
	end 
	self:getWindow():find("kick").frozen = not self:isConnected(self:getPlayerFromId(self.SelectedPlayerId).Flags)
	self:getWindow():find("teleport").frozen = not self:isConnected(self:getPlayerFromId(self.SelectedPlayerId).Flags)
	self:getWindow():find("tell").frozen = false
end

--***********************************************************************
function PlayerTracking:onLineRightClick()
	self:onLineLeftClick()	
	local menu = getUI("ui:login:ring_player_menu")

	menu:find("tell").uc_hardtext = ucstring(i18n.get("uiRAP_MenuPlayerTell"):toUtf8() .. "'" .. self:getPlayerFromId(self.SelectedPlayerId).Player .."'")
	menu:find("teleport").uc_hardtext = ucstring(i18n.get("uiRAP_MenuTeleportTo"):toUtf8() .. "'" .. self:getPlayerFromId(self.SelectedPlayerId).Player.."'")
	menu:find("kick").uc_hardtext = ucstring(i18n.get("uiRAP_MenuKick"):toUtf8() .. "'" .. self:getPlayerFromId(self.SelectedPlayerId).Player.."'")

	menu:find("kick").grayed = not self:isConnected(self:getPlayerFromId(self.SelectedPlayerId).Flags)
	menu:find("teleport").grayed = not self:isConnected(self:getPlayerFromId(self.SelectedPlayerId).Flags)
	launchContextMenuInGame("ui:login:ring_player_menu")
end

--***********************************************************************
-- called by C++ if session joining failed
function PlayerTracking:onJoinFailed()
	messageBox(i18n.get("uiRAP_JoinFailed"))
end

--***********************************************************************
function PlayerTracking:onTell()
	debugInfo("tell to owner of session" .. self.SelectedPlayerId)
	player = ucstring()	
	player:fromUtf8(self:getPlayerFromId(self.SelectedPlayerId).Player)
	tell(player)
end

--***********************************************************************
function PlayerTracking:onTeleportTo()

end

--***********************************************************************
function PlayerTracking:onKick()

end

--***********************************************************************
function PlayerTracking:onLineLeftDblClick()
	local player = self:getPlayerFromId(self.SelectedPlayerId)
	if self:isConnected(player.Flags) then
		validMessageBox(i18n.get("uiRAP_JoinConfirm"), "lua", "PlayerTracking:onJoin()", "", "", "ui:login")	
	else
		-- default to a tell
		onTell()		
	end
end

--***********************************************************************
function PlayerTracking:refresh()

	self.PendingRefresh = true	
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.WaitingList = true
	debugInfo("*refresh*")
end	

--***********************************************************************
function PlayerTracking:updatePendingRefresh()

	if self.PendingRefresh then
		local currTime = nltime.getLocalTime() / 1000
		if currTime - self.LastRefreshQuerryTime > self.MinRefreshPeriod then			
			self.LastRefreshQuerryTime = currTime
			self.PendingRefresh = false
			game.getRingPlayerList()
		end
	end
end

--***********************************************************************
function PlayerTracking:onShow()
	self:initRaceTypes()
	self:initReligionTypes()
	self:initShardTypes()
	self:initLevelTypes()
	setOnDraw(self:getWindow(), "PlayerTracking:onDraw()")		
	self:clear()
	self:enableButtons(false)	
	self:refresh()	
end

local waitTextColor = CRGBA(255, 255, 255, 255)


function PlayerTracking:connectError(errorTextId)
	if not self.WaitingList then return end	
	self:clear()
	self:setErrorMessage(i18n.get(errorTextId))	
	self.WaitingList = false
	self.PendingRefresh = false	
	self.LastRefreshTime = nltime.getLocalTime() / 1000 -- force to wait some more
end	
--***********************************************************************
-- called by C++ if retrieving of sessions failed
function PlayerTracking:onConnectionFailed()	
	self:connectError("uiRAP_ConnectionFailed")		
end

--***********************************************************************
-- called by C++ if retrieving of sessions failed
function PlayerTracking:onDisconnection()
	self:connectError("uiRAP_Disconnection")	
end

--***********************************************************************
-- called by C++ if retrieving of sessions failed
function PlayerTracking:onConnectionClosed()
	self:connectError("uiRAP_ConnectionClosed")	
end

--***********************************************************************
function PlayerTracking:enableButtons(enabled)
	local win = self:getWindow()
	local alpha
	if enabled then alpha = 255 else alpha = 128 end
	self:getSelectList().active = enabled	
end

--***********************************************************************
function PlayerTracking:show()
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
function PlayerTracking:enlargeColumns()
	self:getWindow():find("header_line"):enlargeColumns(10)
end


--***********************************************************************
function PlayerTracking:resizeColumnsAndContainer()
	self:getWindow():find("header_line"):resizeColumnsAndContainer(5)
end

--***********************************************************************
function PlayerTracking:onDraw()
		
	local timeInSec = nltime.getLocalTime() / 1000
	if self.WaitingList then		
		local waitText = i18n.get("uiRAP_WaitMsg" .. math.fmod(os.time(), 3))
		if not self.ListReceived then
			self:setInfoMessage(waitText)
			waitTextColor.A = 127 + 127 * (0.5 + 0.5 * math.cos(6 * timeInSec))
			local errorTxt = self:getWindow():find("errorMsg")
			errorTxt.color_rgba = waitTextColor
		end
	else
		if timeInSec - self.LastRefreshTime > self.RefreshPeriod then		
			self:refresh()
		end
	end
	self:updatePendingRefresh()
end	


