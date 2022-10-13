

-- ring access point window

RingAccessPoint = 
{
	CurrList = {},
	CurrActiveList = {},
	SortDir = {},			
	SelectedSessionId = nil,
	TextCache = {}, -- cache for text widgets
	CenteredTextCache = {},
	NumberCache = {}, -- cache for number widgets
	BitmapCache = {}, -- cache for bitmap widgets
	JaugeCache = {},  -- cache for jauge widgets
	-- Map language to the displayed texture
	LangToTex = 
	{
		en = "flag-en.tga",
		fr = "flag-fr.tga",
		de = "flag-de.tga"
	},
	-- Map scenario flags to the bitmaps displayed in the first column
	FlagsToTex =
	{
		[0] = { Bitmap = "blank.tga",              Color = "0   0   0   0  " },
		[1] = { Bitmap = "rap_not_invited_dm.tga", Color = "255 255 255 255" },
		[2] = { Bitmap = "rap_invited_no_dm.tga",  Color = "255 255 255 255" },
		[3] = { Bitmap = "rap_invited_dm.tga",     Color = "255 255 255 255" },
		[4] = { Bitmap = "r2ed_icon_stop.tga",     Color = "255 255 255 255" }, 
	},
	LangFilter =
	{
	  en = true,
	  fr = true,
	  de = true,
	  misc = true
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
	ScenarioTypeToUtf8 = {}
}

local sampleList1 = 
{
}

local function luaDate(y, mo, d, h, m)
	assert(nil) -- a adapter
	return { year = y, month = mo, day = d, hour = h, min = m }
end

local refTime = os.time()

table.insert(sampleList1,  { Id=0, Owner = "", Title="Scenar de toto", Desc="Fight scenario", Level=0, Flags=0, PlayerCount=0, Language="en", LaunchDate=refTime - 1000, ScenarioType = 0,														NbRating=1,	RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=1, Owner = "titi", Title="Titi's scenario", Desc="Un peu de RP", Level=0, Flags=1, PlayerCount=4,  Language="en", LaunchDate=refTime - 2000, ScenarioType = 1,													NbRating=0,	RateFun=0, RateDifficulty=0, RateAccessibility=0, RateOriginality=0, RateDirection=0})
table.insert(sampleList1,  { Id=2, Owner = "bob", Title="Yubo's back", Desc="Chasse aux yubos", Level=0, Flags=1, PlayerCount=10, Language="en", LaunchDate=refTime - 3000, ScenarioType = 2,													NbRating=3,	RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=3, Owner = "bobette", Title="Capriny hill", Desc="Une colline est envahie de capriny, nettoyer la", Level=1, Flags=1, PlayerCount=2, Language="fr", LaunchDate=refTime - 4000, ScenarioType = 3,				NbRating=1,	RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=4, Owner = "nico", Title="Nico test", Desc="Scenario de test de nico", Level=1, Flags=2, PlayerCount=3, Language="fr", LaunchDate=refTime - 10000, ScenarioType = 4,											NbRating=1,	RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=5, Owner = "toto2", Title="Scenar de toto", Desc="Fight scenario", Level=-1, Flags=2, PlayerCount=0, Language="de", LaunchDate=refTime - 20000, ScenarioType = 5,												NbRating=7,	RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=6, Owner = "titi2", Title="Titi's scenario", Desc="Un peu de RP", Level=10, Flags=3, PlayerCount=4, Language="it", LaunchDate=refTime - 40000, ScenarioType = 6,												NbRating=1, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=7, Owner = "bob2", Title="Yubo's back", Desc="Chasse aux yubos", Level=2, Flags=3, PlayerCount=10, Language="cz", LaunchDate=refTime - 60000, ScenarioType = 0,													NbRating=1, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=8, Owner = "bobette2", Title="Capriny hill", Desc="Une colline est envahie de capriny, nettoyer la", Level=2, Flags=0, PlayerCount=2, Language="en", LaunchDate=refTime - 80000, ScenarioType = 1,				NbRating=1, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=9, Owner = "nico2", Title="Nico test", Desc="Scenario de test de nico", Level=3, Flags=0, PlayerCount=3, Language="fr", LaunchDate=refTime - 100000, ScenarioType = 2,											NbRating=100, RateFun=0, RateDifficulty=45, RateAccessibility=78, RateOriginality=123, RateDirection=99})
table.insert(sampleList1,  { Id=10, Owner = "toto3", Title="Scenar de toto", Desc="Fight scenario", Level=3, Flags=0, PlayerCount=0, Language="de", LaunchDate=refTime - 200000, ScenarioType = 3,												NbRating=1, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=11, Owner = "titi3", Title="Titi's scenario", Desc="Un peu de RP", Level=4, Flags=0, PlayerCount=4, Language="en", LaunchDate=refTime - 300000, ScenarioType = 4,												NbRating=1, RateFun=8, RateDifficulty=12, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=12, Owner = "bob3", Title="Yubo's back", Desc="Chasse aux yubos", Level=4, Flags=0, PlayerCount=10, Language="fr", LaunchDate=refTime - 500000, ScenarioType = 5,												NbRating=45, RateFun=8, RateDifficulty=2, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=13, Owner = "bobette3", Title="Capriny hill", Desc="Une colline est envahie de capriny, nettoyer la", Level=4, Flags=0, PlayerCount=2, Language="de", LaunchDate=refTime - 600000, ScenarioType = 6,			NbRating=1, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=14, Owner = "nico3", Title="Nico test", Desc="Scenario de test de nico", Level=5, Flags=0, PlayerCount=3, Language="en", LaunchDate=refTime - 800000, ScenarioType = 0,											NbRating=12, RateFun=8, RateDifficulty=2, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=15, Owner = "toto4", Title="Scenar de toto", Desc="Fight scenario", Level=5, Flags=0, PlayerCount=0, Language="fr", LaunchDate=refTime - 1000000, ScenarioType = 1,												NbRating=11, RateFun=8, RateDifficulty=56, RateAccessibility=78, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=16, Owner = "titi'", Title="Titi's scenario", Desc="Un peu de RP", Level=5, Flags=0, PlayerCount=4, Language="en", LaunchDate=refTime - 2000000, ScenarioType = 2,												NbRating=12, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=88, RateDirection=99})
table.insert(sampleList1,  { Id=17, Owner = "bob4", Title="Yubo's back", Desc="Chasse aux yubos", Level=5, Flags=0, PlayerCount=10, Language="fr", LaunchDate=refTime - 3000000, ScenarioType = 3,												NbRating=13, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=18, Owner = "bobette4", Title="Capriny hill", Desc="Une colline est envahie de capriny, nettoyer la", Level=5, Flags=0, PlayerCount=2, Language="de", LaunchDate=refTime - 4000000, ScenarioType = 4,			NbRating=14, RateFun=8, RateDifficulty=56, RateAccessibility=88, RateOriginality=78, RateDirection=99})
table.insert(sampleList1,  { Id=19, Owner = "nico4", Title="Nico test", Desc="Scenario de test de nico", Level=5, Flags=0, PlayerCount=3, Language="en", LaunchDate=refTime - 5000000, ScenarioType = 5,										NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})


local sampleList2 = 
{
}

table.insert(sampleList2,  { Id=0, Owner = "", Title="Scenar de toto", Desc="Fight scenario", Level=0, Flags=0, PlayerCount=0, Language="en", LaunchDate=refTime - 1000, ScenarioType = 0,				NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=1, Owner = "titi", Title="Titi's scenario", Desc="Un peu de RP", Level=0, Flags=1, PlayerCount=4,  Language="en", LaunchDate=refTime - 2000, ScenarioType = 1,			NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=2, Owner = "bob", Title="Yubo's back", Desc="Chasse aux yubos", Level=0, Flags=1, PlayerCount=10, Language="en", LaunchDate=refTime - 3000, ScenarioType = 2,			NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=3, Owner = "bobette", Title="Capriny hill", Desc="Une colline est envahie de capriny, nettoyer la", Level=1, Flags=1, PlayerCount=2, Language="fr", LaunchDate=refTime - 4000, ScenarioType = 3, NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=4, Owner = "nico", Title="Nico test", Desc="Scenario de test de nico", Level=1, Flags=2, PlayerCount=3, Language="fr", LaunchDate=refTime - 10000, ScenarioType = 4,	NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=5, Owner = "toto2", Title="Scenar de toto", Desc="Fight scenario", Level=1, Flags=2, PlayerCount=0, Language="de", LaunchDate=refTime - 20000, ScenarioType = 5,		NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=6, Owner = "titi2", Title="Titi's scenario", Desc="Un peu de RP", Level=1, Flags=3, PlayerCount=4, Language="it", LaunchDate=refTime - 40000, ScenarioType = 6,			NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})
table.insert(sampleList2,  { Id=7, Owner = "bob2", Title="Yubo's back", Desc="Chasse aux yubos", Level=2, Flags=3, PlayerCount=10, Language="cz", LaunchDate=refTime - 60000, ScenarioType = 0,			NbRating=18, RateFun=8, RateDifficulty=56, RateAccessibility=32, RateOriginality=78, RateDirection=99})

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
-- sorting by language
local function isBuiltInLang(lang)
	return RingAccessPoint.LangToTex[lang] ~= nil
end

local function lessLang(lhs, rhs)
	if isBuiltInLang(lhs) and isBuiltInLang(rhs) then
		return lhs < rhs
	else
		if isBuiltInLang(rhs) then return false end
		if isBuiltInLang(lhs) then return true end
		return lhs < rhs
	end
end


--*********************************
-- sorting by scenario type
local function lessScenarioType(lhs, rhs)
	return RingAccessPoint.ScenarioTypeToUtf8[lhs] < RingAccessPoint.ScenarioTypeToUtf8[rhs]
end



--*********************************
local function readableHour(time)
	if config.LanguageCode=="en" then
		return os.date("%I:%M %p", time)
	else
		return os.date("%H:%M", time)
	end
end

--*********************************
local function readableDay(time)
	if config.LanguageCode=="en" then
		return os.date("%m/%d/%Y", time)
	else
		return os.date("%d/%m/%Y", time)
	end
end

--*********************************
-- build a readable date
local function readableDate(time)
	if os.date("%d/%m/%Y") == os.date("%d/%m/%Y", time) then
		-- session started today
		return readableHour(time)
	else
		return "@{AAAF}" .. readableDay(time) .. " @{FFFF}" .. readableHour(time)
	end
end


-- init sort dir
table.insert(RingAccessPoint.SortDir, { Var="Flags", Up=false, Cmp = more })
table.insert(RingAccessPoint.SortDir, { Var="LaunchDate", Up=false, Cmp = less})
table.insert(RingAccessPoint.SortDir, { Var="Owner", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="Title", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="Desc", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="Level", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="PlayerCount", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="ScenarioType", Up=false, Cmp = lessScenarioType })
table.insert(RingAccessPoint.SortDir, { Var="Language", Up=false, Cmp = lessLang })
table.insert(RingAccessPoint.SortDir, { Var="AllowFreeTrial", Up=false, Cmp = more })

table.insert(RingAccessPoint.SortDir, { Var="RateFun", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="RateDifficulty", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="RateAccessibility", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="RateOriginality", Up=false, Cmp = less })
table.insert(RingAccessPoint.SortDir, { Var="RateDirection", Up=false, Cmp = less })

--***********************************************************************
function RingAccessPoint:getWindow()
	return getUI("ui:interface:ring_sessions")
end

--***********************************************************************
function RingAccessPoint:initScenarioTypes()
	for k = 0, 6 do		
		self.ScenarioTypeToUtf8[k] = i18n.get("uiRAP_ST_" .. tostring(k)):toUtf8()		
	end
end

--***********************************************************************
function RingAccessPoint:isInvited(flags)
	return flags == 2 or flags == 3
end

--***********************************************************************
function RingAccessPoint:isAllowedSession(allowFreeTrial)
	
	local freeTrial = isPlayerFreeTrial()

	return (not freeTrial or (freeTrial and allowFreeTrial~=0))
end

--***********************************************************************
function RingAccessPoint:isKicked(flags)
	return flags == 4
end

--***********************************************************************
function RingAccessPoint:getColumn(name)
	return getUI("ui:interface:ring_sessions:content:main:enclosing:columns:getw:column_group:" .. name .. ":values")
end


--***********************************************************************
function RingAccessPoint:getSelectList()
	return getUI("ui:interface:ring_sessions:content:main:enclosing:columns:getw:select")
	--return getUI("ui:interface:checkpass:content:enclosing:select")
end


local scratchUCStr = ucstring()

--***********************************************************************
function RingAccessPoint:newTemplate(name, cache)	
	local group
	if table.getn(cache) ~= 0 then
		group = cache[table.getn(cache)]
		table.remove(cache, table.getn(cache))
	else		
		group = createGroupInstance(name, "", {})	
	end
	return group
end

local lineFeed = ucstring("\n")
local lineEnd = ucstring(". ")

--***********************************************************************
-- build a new text group from utf8 text
function RingAccessPoint:newTextLabel(value)				
	local group = self:newTemplate("rap_text", self.TextCache)
	scratchUCStr:fromUtf8(value)
	scratchUCStr = findReplaceAll(scratchUCStr, lineFeed, lineEnd)
	group:find("t").uc_hardtext_single_line_format = scratchUCStr
	return group
end

--***********************************************************************
-- build a new text group from utf8 text
function RingAccessPoint:newCenteredTextLabel(value)				
	local group = self:newTemplate("rap_text_centered", self.CenteredTextCache)
	scratchUCStr:fromUtf8(value)
	group:find("t").uc_hardtext_single_line_format = scratchUCStr
	return group
end

--***********************************************************************
function RingAccessPoint:newNumberLabel(value)	
	local group = self:newTemplate("rap_number", self.NumberCache)
	group:find("t").uc_hardtext_single_line_format = tostring(value)
	return group
end

--***********************************************************************
function RingAccessPoint:newBitmap(texName, color)		
	if color == nil then color = "255 255 255 255" end
	local group = self:newTemplate("rap_bitmap", self.BitmapCache)	
	group.f.texture = texName
	group.f.color = color
	return group
end


--***********************************************************************
function RingAccessPoint:newBooleanLabel(value)	
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
function RingAccessPoint:newJauge(value)	

	local group = self:newTemplate("rap_rating", self.JaugeCache)

	local jaugeUI = group:find("jauge_bar")
	local levelUI = group:find("level_rating")

	local level, progress = RingPlayerInfo:getLevelRatingAndImprovementRate(value)

	jaugeUI.w = progress*group.f.w
	levelUI.texture = "r2ed_ring_rating_" .. level .. ".tga"

	return group
end

--***********************************************************************
function RingAccessPoint:addLine(line)
	self:getColumn("owner"):addChild(self:newTextLabel(line.Owner))
	self:getColumn("title"):addChild(self:newTextLabel(line.Title))
	
	local level = line.Level-1
	if level >= 0 and level <= 5 then
		self:getColumn("level"):addChild(self:newNumberLabel(i18n.get("uiRAP_Level" .. tostring(level)):toUtf8()))
	else
		self:getColumn("level"):addChild(self:newNumberLabel("?"))
	end
	self:getColumn("flags"):addChild(self:newBitmap(self.FlagsToTex[line.Flags].Bitmap, self.FlagsToTex[line.Flags].Color))
	self:getColumn("player_count"):addChild(self:newNumberLabel(tostring(line.PlayerCount)))
	if self.LangToTex[line.Language] ~= nil then
		self:getColumn("language"):addChild(self:newBitmap(self.LangToTex[line.Language]))
	elseif i18n.hasTranslation("uiR2ED" .. line.Language) then
		self:getColumn("language"):addChild(self:newCenteredTextLabel(i18n.get("uiR2ED" .. line.Language):toUtf8()))
	else
		self:getColumn("language"):addChild(self:newCenteredTextLabel(line.Language))
	end		
	if self.ScenarioTypeToUtf8[line.ScenarioType] == nil then
		self:getColumn("scenario_type"):addChild(self:newTextLabel("?"))
	else
		self:getColumn("scenario_type"):addChild(self:newTextLabel(self.ScenarioTypeToUtf8[line.ScenarioType]))
	end
	self:getColumn("launch_date"):addChild(self:newNumberLabel(readableDate(line.LaunchDate)))
	--
	--self:getColumn("author_rrp_rating"):addChild(self:newJauge(line.AuthorRating))
	--self:getColumn("owner_rrp_rating"):addChild(self:newJauge(line.OwnerRating))

	--
	--self:getColumn("scenario_rrp_rating"):addChild(self:newNumberLabel(line.ScenarioRRPTotal))
end

--***********************************************************************
function RingAccessPoint:putColumnInCache(column, cache)	
	local childrenCount = column.childrenNb
	for i = 0, childrenCount - 1 do
		local child = column:getChild(column.childrenNb - 1)
		table.insert(cache, child)
		column:detachChild(child)
	end
end


--***********************************************************************
function RingAccessPoint:putMixedColumnInCache(column, textCache, bitmapCache)	
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
function RingAccessPoint:putInCache()
	self.TextCache = {}
	self.CenteredTextCache = {}
	self.NumberCache = {}
	self.BitmapCache = {}
	self:putColumnInCache(self:getColumn("owner"), self.TextCache)
	self:putColumnInCache(self:getColumn("scenario_type"), self.TextCache)
	self:putColumnInCache(self:getColumn("title"), self.TextCache)
	self:putColumnInCache(self:getColumn("launch_date"), self.NumberCache)
	--
	self:putColumnInCache(self:getColumn("level"), self.NumberCache)
	self:putColumnInCache(self:getColumn("player_count"), self.NumberCache)
	--	
	self:putColumnInCache(self:getColumn("flags"), self.BitmapCache)	
	--
	self:putMixedColumnInCache(self:getColumn("language"), self.CenteredTextCache, self.BitmapCache)
	
	--
	--self:putColumnInCache(self:getColumn("author_rrp_rating"), self.JaugeCache)
	--self:putColumnInCache(self:getColumn("owner_rrp_rating"), self.JaugeCache)
	--self:putColumnInCache(self:getColumn("scenario_rrp_rating"), self.NumberCache)
end

--***********************************************************************
function RingAccessPoint:clear()	
	self:getColumn("owner"):clear()
	self:getColumn("scenario_type"):clear()
	self:getColumn("title"):clear()
	self:getColumn("launch_date"):clear()
	self:getColumn("level"):clear()
	self:getColumn("flags"):clear()
	self:getColumn("player_count"):clear()
	self:getColumn("language"):clear()

	--
	--self:getColumn("author_rrp_rating"):clear()
	--self:getColumn("owner_rrp_rating"):clear()
	--self:getColumn("scenario_rrp_rating"):clear()

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
function RingAccessPoint:testFill1()
	self:onSessionListReceived(sampleList1)
end

--***********************************************************************
function RingAccessPoint:testFill2()
	self:onSessionListReceived(sampleList2)
end

--***********************************************************************
function RingAccessPoint:onSessionListReceived(list)
	self.WaitingList = false
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.ListReceived = true
	self:fill(list)	
	--self:getWindow():find("refreshText").active = false
end	
--***********************************************************************
function RingAccessPoint:fill(list)				
	local enlargeButton = self:getWindow():find("enlarge")
	enlargeButton.frozen = false
	-- if the window is not active, then maybe an old msg -> ignore
	if not self:getWindow().active then return end	
	self:enableButtons(true)	
	--	
	local startTime = nltime.getPreciseLocalTime()	
	--
	self:getWindow():find("join").frozen = true
	self:getWindow():find("tell").frozen = true	
	--
	self:putInCache()
	--
	--	("***********************")
	--debugInfo("TextCache size =  " .. table.getn(self.TextCache))
	--debugInfo("CenteredTextCache size =  " .. table.getn(self.CenteredTextCache))
	--debugInfo("NumberCache size =  " .. table.getn(self.NumberCache))
	--debugInfo("BitmapCache size =  " .. table.getn(self.BitmapCache))	
	--self:clear()
	self.CurrList = list
	self.CurrActiveList = {}
	self:sort()	
	local selectList = self:getSelectList()
	selectList:clear()
	local count = 0
	local displayedCount = 0
	local lastSessionFound = false
	for k, v in pairs(self.CurrList) do
		count = count + 1
	    local active
	    if isBuiltInLang(v.Language) then
			active = self.LangFilter[v.Language]
		else
			active = self.LangFilter["misc"]
		end
		-- if player is invited, then always display
		if self:isInvited(v.Flags) then
			active = true
		end		
		if active then
			self:addLine(v)
			local newGroup = createGroupInstance("rap_select_line", selectList.id, { id=tostring(v.Id)})
			newGroup.active = active
			selectList:addChild(newGroup)		
			newGroup:find("but").pushed = (v.Id == self.SelectedSessionId)
			if v.Id == self.SelectedSessionId then
				lastSessionFound = true
			end
			table.insert(self.CurrActiveList, v)
			displayedCount = displayedCount + 1
		end
	end

	if not lastSessionFound then
		self.SelectedSessionId = nil
	end
	
	if displayedCount == 0 then				
		if count ~= 0 then 
			self:setErrorMessage(i18n.get("uiRAP_NoSessionForLangFilter"))
		else 
			self:setErrorMessage(i18n.get("uiRAP_NoSessionFound"))
			self:enableButtons(false)
		end
	else
		self:clearMessage()
	end
	local endTime = nltime.getPreciseLocalTime()
	self:updateJoinAndTellButtons()		
end


--***********************************************************************
function RingAccessPoint:setMessage(msg, color)		
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
function RingAccessPoint:clearMessage()
	--local errorTxt = self:getWindow():find("errorMsg")
	local errorTxt = self:getWindow():find("refreshText")
	errorTxt.active = false
end

--***********************************************************************
function RingAccessPoint:setErrorMessage(msg)
	self:setMessage(msg, "192 64 0 255")
end

--***********************************************************************
function RingAccessPoint:setInfoMessage(msg)
	self:setMessage(msg, "255 255 255 255")
end


--***********************************************************************
function RingAccessPoint:sort(list)
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
function RingAccessPoint:headerLeftClick(down, criterion)	
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
function RingAccessPoint:getSessionFromId(id)
	for k, v in pairs(self.CurrList) do
		if v.Id == id then return v end
	end
	return nil
end

--***********************************************************************
function RingAccessPoint:onLineLeftClick()

	self.SelectedSessionId = self.CurrActiveList[self:getSelectList():getElementIndex(getUICaller().parent) + 1].Id	
	local selectList = self:getSelectList()
	for k = 0, selectList.childrenNb - 1 do
		local but = selectList:getChild(k):find("but")
		if but == getUICaller() then
			but.pushed = true	
		else
			but.pushed = false
		end
	end 
	self:updateJoinAndTellButtons()	

	local session = self:getSessionFromId(self.SelectedSessionId)
	if not self:isAllowedSession(session.AllowFreeTrial) then

		local text = getUI("ui:interface:warning_free_trial:text")
		if text then 
			text.hardtext = i18n.get("uiRingWarningFreeTrial")
		end

		runAH(nil, "enter_modal", "group=ui:interface:warning_free_trial")
	end
end

--***********************************************************************
function RingAccessPoint:onLineSessionTooltip()	
	
	local activeLine = self.CurrActiveList[self:getSelectList():getElementIndex(getUICaller().parent) + 1]
	local contextHelpText
	if self:isInvited(activeLine.Flags) then
		contextHelpText = i18n.get("uiRAP_HowToJoin"):toUtf8()
	else
		contextHelpText = i18n.get("uiRAP_HowToBeInvited"):toUtf8()
	end

	local desc = activeLine.Desc
	if desc=="" then desc="-" end
	contextHelpText = contextHelpText .. "\n\n@{6F6F}" .. i18n.get("uiR2EDProp_LoadScenario_Description"):toUtf8() .. " : @{FFFF}" .. ucstring(desc):toUtf8() .. "\n\n"

	local rateFun =			"@{6F6F}" .. i18n.get("uiRAP_RateFun"):toUtf8() .. " : @{FFFF}"
	local rateDiff =		"@{6F6F}" .. i18n.get("uiRAP_RateDifficulty"):toUtf8() .. " : @{FFFF}"
	local rateAccess =		"@{6F6F}" .. i18n.get("uiRAP_RateAccessibility"):toUtf8() .. " : @{FFFF}"
	local rateOrig =		"@{6F6F}" .. i18n.get("uiRAP_RateOriginality"):toUtf8() .. " : @{FFFF}"
	local rateDirection =	"@{6F6F}" .. i18n.get("uiRAP_RateDirection"):toUtf8() .. " : @{FFFF}"
	
	if activeLine.NbRating>0 then
		rateFun =		rateFun .. tostring(math.min(100, activeLine.RateFun)) .. "/100"
		rateDiff =		rateDiff .. tostring(math.min(100, activeLine.RateDifficulty)) .. "/100"	
		rateAccess =	rateAccess .. tostring(math.min(100, activeLine.RateAccessibility)) .. "/100"	
		rateOrig =		rateOrig ..	tostring(math.min(100, activeLine.RateOriginality)) .. "/100"
		rateDirection =	rateDirection .. tostring(math.min(100, activeLine.RateDirection)) .. "/100"
	else
		rateFun =		rateFun .. i18n.get("uiRAP_NoRate"):toUtf8()
		rateDiff =		rateDiff .. i18n.get("uiRAP_NoRate"):toUtf8()	
		rateAccess =	rateAccess .. i18n.get("uiRAP_NoRate"):toUtf8()
		rateOrig =		rateOrig ..	i18n.get("uiRAP_NoRate"):toUtf8()
		rateDirection =	rateDirection .. i18n.get("uiRAP_NoRate"):toUtf8()
	end
		
--	contextHelpText = contextHelpText .. rateFun .. "\n"
--	contextHelpText = contextHelpText .. rateDiff .. "\n"
--	contextHelpText = contextHelpText .. rateAccess .. "\n"
--	contextHelpText = contextHelpText .. rateOrig .. "\n"
--	contextHelpText = contextHelpText .. rateDirection .. "\n"

	local uc_contextHelpText = ucstring()
	uc_contextHelpText:fromUtf8(contextHelpText)
	setContextHelpText(uc_contextHelpText)
end


--***********************************************************************
function RingAccessPoint:updateJoinAndTellButtons()
	
	if 	self.SelectedSessionId then
		local session = self:getSessionFromId(self.SelectedSessionId)
		self:getWindow():find("join").frozen = not self:isInvited(session.Flags) or not self:isAllowedSession(session.AllowFreeTrial)
		self:getWindow():find("tell").frozen = self:isKicked(session.Flags) or not self:isAllowedSession(session.AllowFreeTrial)
	else
		self:getWindow():find("join").frozen = true
		self:getWindow():find("tell").frozen = true
	end
end

--***********************************************************************
function RingAccessPoint:onLineRightClick()
	self:onLineLeftClick()	
	local menu = getUI("ui:interface:ring_session_menu")

	local session = self:getSessionFromId(self.SelectedSessionId)
	menu:find("join").grayed = not self:isInvited(session.Flags) or not self:isAllowedSession(session.AllowFreeTrial)
	menu:find("tell").grayed = self:isKicked(session.Flags) or not self:isAllowedSession(session.AllowFreeTrial)
	launchContextMenuInGame("ui:interface:ring_session_menu")
end


--***********************************************************************
function RingAccessPoint:onJoin()
	if not isFullyPatched() then
		runAH(nil, "leave_modal", "")
		self:getWindow().active = false
		bgdownloader:inGamePatchUncompleteWarning()
		return
	end	
	if self:isInvited(self:getSessionFromId(self.SelectedSessionId).Flags) then
		--debugInfo("join session with id = " .. self.SelectedSessionId)
		game.joinRingSession(self.SelectedSessionId)
		self:getWindow().active = false
	else
		-- can't join this session without invitation
		displaySystemInfo(i18n.get("uiRAP_NotInvited"), "BC")
	end
end

--***********************************************************************
-- called by C++ if session joining failed
function RingAccessPoint:onJoinFailed()
	messageBoxWithHelp(i18n.get("uiRAP_JoinFailed"))
end

--***********************************************************************
function RingAccessPoint:onTell()
	--debugInfo("tell to owner of session" .. self.SelectedSessionId)
	player = ucstring()		
	player:fromUtf8(self:getSessionFromId(self.SelectedSessionId).Owner)
	tell(player, i18n.get("uiRAP_AskForInvite"))	
	displaySystemInfo(concatUCString(i18n.get("uiRAP_TellSentTo"), player), "BC")	
end

--***********************************************************************
function RingAccessPoint:onLineLeftDblClick()
	local session = self:getSessionFromId(self.SelectedSessionId)

	if self:isKicked(session.Flags) then return end
	if not self:isAllowedSession(session.AllowFreeTrial) then 
		local text = getUI("ui:interface:warning_free_trial:text")
		if text then 
			text.hardtext = i18n.get("uiRingWarningFreeTrial")
		end
		runAH(nil, "enter_modal", "group=ui:interface:warning_free_trial")
		return 
	end

	if self:isInvited(session.Flags) then
		validMessageBox(i18n.get("uiRAP_JoinConfirm"), "lua", "RingAccessPoint:onJoin()", "", "", "ui:interface")	
	else
		-- default to a tell
		self:onTell()
	end
end

--***********************************************************************
function RingAccessPoint:updateLangFilterUI()
	self:getWindow():find("lang_en").pushed = self.LangFilter.en
	self:getWindow():find("lang_fr").pushed = self.LangFilter.fr
	self:getWindow():find("lang_de").pushed = self.LangFilter.de
	self:getWindow():find("lang_misc").pushed = self.LangFilter.misc
end

--***********************************************************************
function RingAccessPoint:toggleLangfilter(lang)
   self.LangFilter[lang] = not self.LangFilter[lang]
   self:updateLangFilterUI()
   self:fill(self.CurrList)
end

--***********************************************************************
function RingAccessPoint:refresh()
	self.PendingRefresh = true	
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.WaitingList = true
	--debugInfo("*refresh*")
end	

--***********************************************************************
-- Special : called when a tell tagged as 'R2_INVITE' has been received (called by C++)
-- Because the player is now invited in a session, force a refresh to display
-- the 'invite' icon
function RingAccessPoint:forceRefresh()
	self:refresh()
	self.LastRefreshQuerryTime = 0 -- make believe last refresh was *quite* long ago...
end	


--***********************************************************************
function RingAccessPoint:updatePendingRefresh()
	if self.PendingRefresh then
		local currTime = nltime.getLocalTime() / 1000
		if currTime - self.LastRefreshQuerryTime > self.MinRefreshPeriod and game.getRingSessionList then
			--debugInfo("doing actual querry")			
			self.LastRefreshQuerryTime = currTime
			self.PendingRefresh = false
			-- when you load an animation, lua state isn't initialized for a short time
			game.getRingSessionList()
		end
	end
end

--***********************************************************************
function RingAccessPoint:onShow()

	runAH(nil, 'context_ring_sessions', '')

	if(getUI("ui:interface:ring_sessions").active) then
		self:initScenarioTypes()
		setOnDraw(self:getWindow(), "RingAccessPoint:onDraw()")	
		self:updateLangFilterUI()	
		self:clear()
		self:enableButtons(false)	
		self:refresh()	
		local enlargeButton = self:getWindow():find("enlarge")
		enlargeButton.frozen = false
	end
end

local waitTextColor = CRGBA(255, 255, 255, 255)


function RingAccessPoint:connectError(errorTextId)
	if not self.WaitingList then return end	
	self:clear()
	self:setErrorMessage(i18n.get(errorTextId))	
	self.WaitingList = false
	self.PendingRefresh = false	
	self.LastRefreshTime = nltime.getLocalTime() / 1000 -- force to wait some more
	--self:getWindow():find("refreshText").active = false	
end	
--***********************************************************************
-- called by C++ if retrieving of sessions failed
function RingAccessPoint:onConnectionFailed()	
	self:connectError("uiRAP_ConnectionFailed")		
end

--***********************************************************************
-- called by C++ if retrieving of sessions failed
function RingAccessPoint:onDisconnection()
	self:connectError("uiRAP_Disconnection")	
end

--***********************************************************************
-- called by C++ if retrieving of sessions failed
function RingAccessPoint:onConnectionClosed()
	self:connectError("uiRAP_ConnectionClosed")	
end

--***********************************************************************
function RingAccessPoint:enableButtons(enabled)
	self:updateLangFilterUI()
	local win = self:getWindow()
	win:find("lang_en").frozen = not enabled
	win:find("lang_fr").frozen = not enabled
	win:find("lang_de").frozen = not enabled
	win:find("lang_misc").frozen = not enabled
	local alpha
	if enabled then alpha = 255 else alpha = 128 end
	win:find("en_bm").alpha = alpha
	win:find("fr_bm").alpha = alpha
	win:find("de_bm").alpha = alpha
	win:find("misc_bm").alpha = alpha
	self:getSelectList().active = enabled	
end

--***********************************************************************
function RingAccessPoint:show()
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
function RingAccessPoint:enlargeColumns()
	self:getWindow():find("header_line"):enlargeColumns(10)
	local enlargeButton = self:getWindow():find("enlarge")
	enlargeButton.frozen = true
end

--***********************************************************************
function RingAccessPoint:onResize()
	local enlargeButton = self:getWindow():find("enlarge")
	enlargeButton.frozen = false
end

--***********************************************************************
function RingAccessPoint:onDraw()	
	local timeInSec = nltime.getLocalTime() / 1000
	if self.WaitingList then
		if timeInSec - self.LastRefreshTime > self.WaitingPeriod then		
			self.WaitingList = false
			self.LastRefreshTime = nltime.getLocalTime() / 1000
			--self:getWindow():find("refreshText").active = false
		else		
			local waitText = i18n.get("uiRAP_WaitMsg" .. math.fmod(os.time(), 3))
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
		end
	end
	self:updatePendingRefresh()
end	

--***********************************************************************
function RingAccessPoint:newScenario()	
	if not isFullyPatched() then
		self:getWindow().active = false
		bgdownloader:inGamePatchUncompleteWarning()
		return
	end
	--self:getWindow().active = false
	--getUI("ui:interface:r2ed_scenario_control").active=true

	runAH(getUICaller(), 'open_scenario_control', '')
	if getUI("ui:interface:ring_scenario_loading_window").active then
		self:getWindow().active = false
	end
end

