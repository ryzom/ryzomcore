-- In this file we define functions that serves for info player windows


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end


-- Index of the mission slected during the last session (read from  the config file)
game.PrevSessionMission = - 1


-- flag set to true when the in game db has been initialized
game.InGameDbInitialized = false

game.WebMissionLastDesc = {}

------------------------------------------------------------------------------------------------------------
-- CAP
------------------------------------------------------------------------------------------------------------
-- Define the default url listing all caps
game.CapUrl = nil
--
game.CapTitle = ""
game.CapDesc = ""
game.CapInfosUrl = nil
game.CapNextUrl = nil
game.capExpanded = false
game.CapInfos = ""
game.keepExpandStatus = false

function debug(text, type)
end

function p(text, type)
end


------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
-- MAGIC DAMAGE PROTECTIONS
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------
-- From DB, compute the Total Max Magic Absorption, and store in local DB
function game:computeMagicProtectTotalAbsorb()

	-- *** pre compute the table of DB props (if not done)
	if (self.JewelDBs == nil) then
		local defList= {
			'headdress',
			'earl',
			'earr',
			'necklace',
			'wristl',
			'wristr',
			'fingerl',
			'fingerr',
			'anklel',
			'ankler',
		};

		self.JewelDBs= {};
		for k,v in pairs(defList) do
			self.JewelDBs[k]= getDefine(v) .. ':INDEX_IN_BAG';
		end
	end

	-- *** for each jewel, add the magic protection
	local	protectFactor= getDbProp(formatUI('%player_protect_absorbfactor'));
	local	totalProtect= 0;
	for k,dbJewel in pairs(self.JewelDBs) do
		-- if the index in bag is not null (ie something present)
		local	indexInBag= getDbProp(dbJewel);
		if (indexInBag>0) then
			-- real index in bag is -1
			indexInBag= indexInBag-1;
			-- check the sheetId is correct
			local dbSheet= formatUI('%bag:#1:SHEET', indexInBag);
			if (getDbProp(dbSheet)~=0) then
				local protect= getDbProp(formatUI('%bag:#1:QUALITY', indexInBag));
				-- mul by factor, and add to total protection
				protect= math.floor(protect * protectFactor / 100);
				totalProtect= totalProtect + protect;
			end
		end
	end

	-- *** additionaly, must add the max skill of player (mul by factor)
	local	protect= 0;
	protect= math.max(protect, getBaseSkillValueMaxChildren(getSkillIdFromName('SF')) );
	protect= math.max(protect, getBaseSkillValueMaxChildren(getSkillIdFromName('SM')) );
	protect= math.max(protect, getBaseSkillValueMaxChildren(getSkillIdFromName('SC')) );
	protect= math.max(protect, getBaseSkillValueMaxChildren(getSkillIdFromName('SH')) );
	protect= math.floor(protect * protectFactor / 100);
	totalProtect= totalProtect + protect;


	-- *** store result
	setDbProp('UI:VARIABLES:TOTAL_MAGIC_ABSORB', totalProtect);
end


------------------------------------------------------------------------------------------------------------
-- From given DBs, compute the 'Magic protection' tooltip to display
function game:tooltipMagicProtect(dbVal, ttFormat)
	local	fmt= i18n.get(ttFormat);

	-- get the current and max value. minimize the value displayed
	local	val= getDbProp(dbVal);
	local	vMax= getDbProp(formatUI('%player_protect_maxratio'));
	val= math.min(val, vMax);

	-- replace value
	fmt= findReplaceAll(fmt, "%v", tostring(val) );

	-- replace max text
	if (val>=vMax) then
		fmt= findReplaceAll(fmt, "%max", i18n.get('uittProtect_MaxReached') );
	else
		fmt= findReplaceAll(fmt, "%max", "" );
	end

	-- set the tooltip in InterfaceManager
	setContextHelpText(fmt);
end


------------------------------------------------------------------------------------------------------------
function game:displayMagicProtect(dbVal)
	-- get values. minimize the value displayed
	local	val= getDbProp(dbVal);
	local	vMax= getDbProp(formatUI('%player_protect_maxratio'));
	val= math.min(val, vMax);

	-- get the ui text
	local	ui= getUICaller();
	local	uiText= ui.val;

	-- set the text (percentage)
	uiText.uc_hardtext= tostring(val) .. "%";

	-- set color and global color according to maximum reached or not
	if(val >= vMax) then
		uiText.color= '255 240 130 255';
		uiText.global_color= false;
	else
		uiText.color= '255 255 255 255';
		uiText.global_color= true;
	end
end


------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
-- MAGIC RESISTANCES
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------
-- Compute the current Base Resist (nb maybe negative)
function game:getMagicResistBaseLevel()
	-- its just the max beetween SF, SM and SH/2
	local	vSF= getBaseSkillValueMaxChildren(getSkillIdFromName('SF'));
	local	vSM= getBaseSkillValueMaxChildren(getSkillIdFromName('SM'));
	local	vSH= getBaseSkillValueMaxChildren(getSkillIdFromName('SH'));
	local val= math.max(vSF, vSM);
	val= math.max(val, math.floor(vSH/2));
	-- Yoyo: -25 is an EGS variable. Hardcoded for now
	val= val-25;
	return val;
end


------------------------------------------------------------------------------------------------------------
-- Compute the current Max Resist
function game:getMagicResistMaxLevel()
	local mlvl= 25 + self:getMagicResistBaseLevel() + getDbProp(formatUI('%player_resist_maxratio'));
	return math.max(0,mlvl);
end


------------------------------------------------------------------------------------------------------------
-- From given DBs, compute the 'Magic Resistance' tooltip to display
function game:tooltipMagicResist(dbVal, ttFormat)
	local	fmt= i18n.get(ttFormat);

	-- get the current and max value. minimize the value displayed
	local	val= getDbProp(dbVal);
	local	vMax= self:getMagicResistMaxLevel();
	val= math.min(val, vMax);

	-- replace value
	fmt= findReplaceAll(fmt, "%v", tostring(val) );

	-- replace max text
	if (val>=vMax) then
		fmt= findReplaceAll(fmt, "%max", i18n.get('uittResist_MaxReached') );
	else
		fmt= findReplaceAll(fmt, "%max", "" );
	end

	-- Print Chances to resist agst Elemental spells
	local	casterSpellLvl= self:getMagicResistBaseLevel();		-- choose the skill base level for reference, +25
	casterSpellLvl= casterSpellLvl+25;
	casterSpellLvl= math.max(casterSpellLvl, 0);
	fmt= findReplaceAll(fmt, "%eml", tostring(casterSpellLvl));
	fmt= findReplaceAll(fmt, "%emr", tostring(getMagicResistChance(true, casterSpellLvl, val)));

	-- Print Chances to resist against Afflliction spells
	fmt= findReplaceAll(fmt, "%aml", tostring(casterSpellLvl));
	fmt= findReplaceAll(fmt, "%amr", tostring(getMagicResistChance(false, casterSpellLvl, val)));


	-- set the tooltip in InterfaceManager
	setContextHelpText(fmt);
end


------------------------------------------------------------------------------------------------------------
function game:displayMagicResist(dbVal)
	-- get values. minimize the value displayed
	local	val= getDbProp(dbVal);
	local	vMax= self:getMagicResistMaxLevel();
	val= math.min(val, vMax);

	-- get the ui text
	local	ui= getUICaller();
	local	uiText= ui.val;

	-- set the text (final value)
	uiText.uc_hardtext= tostring(val);

	-- set color and global color according to maximum reached or not
	if(val >= vMax) then
		uiText.color= '255 240 130 255';
		uiText.global_color= false;
	else
		uiText.color= '255 255 255 255';
		uiText.global_color= true;
	end
end


------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
-- NPC WEB BROWSER
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------
function game:initNpcWebPageData()
	if(self.NpcWebPage==nil) then
		self.NpcWebPage= {};
		self.NpcWebPage.UrlTextId= 0;
		self.NpcWebPage.BrowseDone= false;
		self.NpcWebPage.WaitingDynStr = false
	end
end

-- TMP TMP for test
NicoMagicURL = ""
--function nicoTest()
-- runCommand("db", "LOCAL:TARGET:CONTEXT_MENU:WEB_PAGE_URL", 45678)
-- end

------------------------------------------------------------------------------------------------------------
function game:onDrawNpcWebPage()

	self:initNpcWebPageData();

	-- if the browse has already been done
	if(self.NpcWebPage.BrowseDone) then
		return;
	end

	-- browse when the url string id is ready
	if(self.NpcWebPage.UrlTextId ~=0) then
      local available
      if config.Local == 1 then
         available = (NicoMagicURL ~= "")
      else
         available = isDynStringAvailable(self.NpcWebPage.UrlTextId)
      end
		if(available) then
			local	ucUrl
			if config.Local == 1 then
				ucUrl = ucstring(NicoMagicURL) -- for test in local mode
			else
				ucUrl = getDynString(self.NpcWebPage.UrlTextId);
			end
			-- browse
			local	uiStr= getUIId(getUICaller());
			-- if the url
			local utf8Url = ucUrl:toUtf8()
			local isRing = string.find(utf8Url, "ring_access_point=1") ~= nil
			if isRing then
				getUI("ui:interface:npc_web_browser").active = false
				runAH(nil, "context_ring_sessions", "")
				return
			else
				local hideWindow = string.find(utf8Url, "_hideWindow=1") ~= nil
				if hideWindow then
					getUI("ui:interface:npc_web_browser").active = false
				end
				self.NpcWebPage.BrowseDone= true;
				browseNpcWebPage(uiStr, utf8Url, true, 10); -- 'true' is for 'add parameters' here. 10 is standard timeout
			end
			-- clear undo/redo, to cannot undo to the "please wait" page :)
			clearHtmlUndoRedo(uiStr);
			-- if this is a ring window, then only the refresh button to access to filter will be available
			local isRing = string.find(utf8Url, "ring_access_point=1") ~= nil
			local browser = getUI("ui:interface:npc_web_browser")
			browser:find("browse_redo").active = true
			browser:find("browse_undo").active = true
			browser:find("browse_refresh").active = true
		end
	end
end

------------------------------------------------------------------------------------------------------------
-- UNUSED???
function game:initNpcWebPage()
	local	ui= getUICaller();
	if(ui~=nil) then
		setOnDraw(ui, "game:onDrawNpcWebPage()");
	end
end

------------------------------------------------------------------------------------------------------------
function string:split(Pattern)
	local Results = {}
	local Start = 1
	local SplitStart, SplitEnd = string.find(self, Pattern, Start)
	while(SplitStart)do
		table.insert(Results, string.sub(self, Start, SplitStart-1))
		Start = SplitEnd+1
		SplitStart, SplitEnd = string.find(self, Pattern, Start)
	end
	table.insert(Results, string.sub(self, Start))
	return Results
end

function game:getOpenAppPageMessage()
	local ucUrl = getDynString(self.NpcWebPage.UrlTextId)
	local url = ucUrl:toUtf8()
	surl = url:split("&")
	for i=1,#surl do
		if surl[i]:sub(1, 12) == "open_message" then
			return base64.decode(surl[i]:sub(14))
		end
	end
	return ""
end

function game:onDbChangeAppPage()
	if getDbProp("UI:VARIABLES:CURRENT_SERVER_TICK") > self.NpcWebPage.Timeout then
		local npcName = getTargetName()
		local message  = ucstring()
		local text  = ""
		if game.appNpcMessages[npcName] ~= nil then
			text = game.appNpcMessages[npcName]
		else
			text = game:getOpenAppPageMessage()
		end
		if text == "" then
			text = "w_magic_sep2.tga|"
		else
			text = findReplaceAll(text, "%s", npcName)
		end
		message:fromUtf8(game:parseLangText(text))
		displaySystemInfo(message, "ZON")
		removeOnDbChange(getUI("ui:interface:npc_web_browser"),"@UI:VARIABLES:CURRENT_SERVER_TICK")
	end
end

function game:startNpcWebPage()
	self:initNpcWebPageData();

	-- set the new page to explore.
	-- NB: must backup the Database, because don't want that the page change when clicking an other NPC
	if not self.NpcWebPage.WaitingDynStr then
		self.NpcWebPage.UrlTextId = getDbProp("LOCAL:TARGET:CONTEXT_MENU:WEB_PAGE_URL");
	end
	self.NpcWebPage.BrowseDone = false;

	available = isDynStringAvailable(self.NpcWebPage.UrlTextId)
	if available then
		if self.NpcWebPage.WaitingDynStr then
			self.NpcWebPage.WaitingDynStr = false
			removeOnDbChange(getUI("ui:interface:npc_web_browser"),"@UI:VARIABLES:CURRENT_SERVER_TICK")
		end
		local ucUrl = getDynString(self.NpcWebPage.UrlTextId)
		local utf8Url = ucUrl:toUtf8()

		if utf8Url:sub(1, 4) == "http" then
			runAH(nil, "browse", "name=ui:interface:npc_web_browser:content:html|url=release_wk.html|localize=1");
			clearHtmlUndoRedo("ui:interface:npc_web_browser:content:html");
			local ui= getUI("ui:interface:npc_web_browser");
			if(ui~=nil) then
				ui.active= true;
			end
			ui:find("browse_redo").active = false
			ui:find("browse_undo").active = false
			ui:find("browse_refresh").active = false
		else
			setTargetAsInterlocutor()
			self.NpcWebPage.Timeout = getDbProp("UI:VARIABLES:CURRENT_SERVER_TICK")+7
			addOnDbChange(getUI("ui:interface:npc_web_browser"),"@UI:VARIABLES:CURRENT_SERVER_TICK", "game:onDbChangeAppPage()")

			-- App url, need sign it with server
			runCommand("a", "openTargetApp", utf8Url)
		end
	else
		self.NpcWebPage.WaitingDynStr = true
		addOnDbChange(getUI("ui:interface:npc_web_browser"),"@UI:VARIABLES:CURRENT_SERVER_TICK", "game:startNpcWebPage()")
	end
end

------------------------------------------------------------------------------------------------------------
--
function game:closeNpcWebBrowserHeader()
	local ui = getUI('ui:interface:npc_web_browser');

	-- save size
	ui_npc_web_browser_h = ui.h;
	ui_npc_web_browser_w = ui.w;

	-- reduce window size
	ui.pop_min_h = 32;
	ui.h = 0;
	ui.w = 150;
end

------------------------------------------------------------------------------------------------------------
--
function game:openNpcWebBrowserHeader()
	local ui = getUI('ui:interface:npc_web_browser');
	ui.pop_min_h = 96;

	-- set size from saved values
	if (ui_npc_web_browser_h ~= nil) then
		ui.h = ui_npc_web_browser_h;
	end

	 if (ui_npc_web_browser_w ~= nil) then
		ui.w = ui_npc_web_browser_w;
	end
end


------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
-- FAME
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------
function game:initFameTribe()
	local	ui = getUICaller();

	local	firstIdx = getFirstTribeFameIndex();
	local	firstDBIdx = getFameDBIndex(firstIdx);
	local	nbFame = getNbTribeFameIndex();

	for c = 1,nbFame do
		local lineIdx = getFameDBIndex(firstIdx+c-1) - firstDBIdx;
		local uiLine = getUI(getUIId(ui) .. ':list:line' .. lineIdx);
		uiLine.t.hardtext = 'uiFame_' .. getFameName(firstIdx+c-1);
	end

end

------------------------------------------------------------------------------------------------------------
function game:updateFameBar(path)
	local	ui = getUICaller();
	local	thresholdKOS = getDbProp('SERVER:FAME:THRESHOLD_KOS');
	local	thresholdTrade = getDbProp('SERVER:FAME:THRESHOLD_TRADE');
	local	fameValue = getDbProp(path .. ':VALUE');
	local	fameMax = getDbProp(path .. ':THRESHOLD');

	-- known/unknown fame
	local	fameVisible = fameValue ~= -128
	if fameVisible then
		-- show unmodified value stored in #path:VALUE
		ui.t.hardtext = fameValue
	else
		ui.t.hardtext = "?"
	end
	-- show/hide fame bar components
	ui.m.active = fameVisible
	ui.p0.active = fameVisible
	ui.p1.active = fameVisible
	ui.p2.active = fameVisible
	ui.p3.active = fameVisible
	ui.p4.active = fameVisible
	ui.bar3d.active = fameVisible

	if (thresholdKOS < -100) then thresholdKOS = -100; end
	if (thresholdKOS > 100) then thresholdKOS = 100; end
	if (thresholdTrade < -100) then thresholdTrade = -100; end
	if (thresholdTrade > 100) then thresholdTrade = 100; end
	if (fameValue < -100) then fameValue = -100; end
	if (fameValue > 100) then fameValue = 100; end
	if (fameMax < -100) then fameMax = -100; end
	if (fameMax > 100) then fameMax = 100; end

	if (thresholdKOS > thresholdTrade) then thresholdKOS = thresholdTrade; end
	if (fameValue > fameMax) then fameValue = fameMax; end

	local	uiPart0 = ui.p0;
	local	uiPart1 = ui.p1;
	local	uiPart2 = ui.p2;
	local	uiPart3 = ui.p3;
	local	uiPart4 = ui.p4;
	local	uiBar3d = ui.bar3d;
	-- part 4 is there to fill in the hole

	local barW = ui.w - 4;
	local barX = 2;

	local bar3dStart= barX + barW/2;
	local bar3dLimit= bar3dStart;

	-- init part 0 of the bar
	uiPart0.x = barX;
	ui.ttp0.x = barX;
	ui.ttp0.w = barW * (thresholdKOS + 100) / 200;
	if (fameValue >= -100) and (fameValue <= thresholdKOS) then
		uiPart0.color = '80 0 0 255';
		uiPart0.w = barW * (fameValue + 100) / 200;

		uiPart4.color = '255 0 0 255';
		uiPart4.x = uiPart0.x + uiPart0.w;
		uiPart4.w = barW * (thresholdKOS - fameValue) / 200;

		bar3dLimit= uiPart4.x;
	else
		uiPart0.color= '80 0 0 255';
		uiPart0.w = barW * (thresholdKOS + 100) / 200;
	end

	-- init part 1 of the bar
	local	part1X= barX + barW * (thresholdKOS + 100) / 200;
	local	part1TotalW= barW * (thresholdTrade - thresholdKOS) / 200;
	uiPart1.x = part1X;
	ui.ttp1.x = part1X;
	ui.ttp1.w = part1TotalW;
	if (fameValue >= thresholdKOS) and (fameValue <= thresholdTrade) then
		uiPart1.color = '80 40 0 255';
		uiPart1.w = barW * (fameValue - thresholdKOS) / 200;

		uiPart4.color = '255 127 0 255';
		uiPart4.x = uiPart1.x + uiPart1.w;
		uiPart4.w = barW * (thresholdTrade - fameValue) / 200;

		bar3dLimit= uiPart4.x;
	else
		if (fameValue < thresholdKOS) then
			uiPart1.color = '255 127 0 255';
		else
			uiPart1.color = '80 40 0 255';
		end
		uiPart1.w = part1TotalW;
	end

	-- init part 2 of the bar
	local	part2X= barX + barW * (thresholdTrade + 100) / 200;
	local	part2TotalW= barW * (0 - thresholdTrade) / 200;
	uiPart2.x = part2X;
	ui.ttp2.x = part2X;
	ui.ttp2.w = part2TotalW;
	if (fameValue >= thresholdTrade) and (fameValue <= 0) then
		uiPart2.color = '80 80 0 255';
		uiPart2.w = barW * (fameValue - thresholdTrade) / 200;

		uiPart4.color = '255 255 0 255';
		uiPart4.x = uiPart2.x + uiPart2.w;
		uiPart4.w = barW * (0 - fameValue) / 200;

		bar3dLimit= uiPart4.x;
	else
		if (fameValue < thresholdTrade) then
			uiPart2.color = '255 255 0 255';
		else
			uiPart2.color = '80 80 0 255';
		end
		uiPart2.w = part2TotalW;
	end

	-- init part 3 of the bar
	local	part3X= barX + barW * (0 + 100) / 200;
	local	part3TotalW= barW * (100 - 0) / 200;
	uiPart3.x = part3X;
	ui.ttp3.x = part3X;
	ui.ttp3.w = part3TotalW;
	if (fameValue >= 0) and (fameValue <= 100) then
		uiPart3.color = '0 255 0 255';
		uiPart3.w = barW * (fameValue - 0) / 200;

		uiPart4.color = '0 80 0 255';
		uiPart4.x = uiPart3.x + uiPart3.w;
		uiPart4.w = barW * (100 - fameValue) / 200;

		bar3dLimit= uiPart4.x;
	else
		uiPart3.color = '0 80 0 255';
		uiPart3.w = part3TotalW;
	end

	-- init max limit
	local	uiMaxLimit = ui.m;
	uiMaxLimit.x = barX + barW * (fameMax + 100) / 200;

	-- init bar3d
	if (bar3dStart < bar3dLimit) then
		uiBar3d.x= bar3dStart;
		uiBar3d.w= bar3dLimit-bar3dStart;
	else
		uiBar3d.x= bar3dLimit;
		uiBar3d.w= bar3dStart - bar3dLimit;
	end

end

------------------------------------------------------------------------------------------------------------
function game:updateFameBarTT(path)
	local	fameMax = getDbProp(path .. ':THRESHOLD');

	local text = i18n.get('uittFameMaxPossible');
	text = findReplaceAll(text, '%famemax', tostring(fameMax));
	setContextHelpText(text);
end

------------------------------------------------------------------------------------------------------------
function game:getPvpEffects()
	local uiGroup= getUICaller();
	local n = 59-1;
	local i;
	local hasBonus = false;
	local hasMalus = false;

	local text = ''
	local textBonus = '';
	local textMalus = '';
	local fmt;

	-- check every malus and bonus
	for i=0, n do
		local path = formatUI('SERVER:PVP_EFFECTS:#1', i);
		local id = getDbProp(path .. ':ID');
		if (id ~= 0) then
			local isBonus = getDbProp(path .. ':ISBONUS');
			local param = getDbProp(path .. ':PARAM');
			if (isBonus == 1) then
				hasBonus = true;
				fmt = i18n.get('uiPvPEffect_' .. getRegionByAlias(id) .. '_Bonus');
				fmt = replacePvpEffectParam(fmt, param);
				if (textBonus ~= '') then
					textBonus = concatUCString(textBonus, '\n\n');
				end
				textBonus = concatUCString(textBonus, fmt);
			else
				hasMalus = true;
				fmt = i18n.get('uiPvPEffect_' .. getRegionByAlias(id) .. '_Malus');
				fmt = replacePvpEffectParam(fmt, param);
				if (textMalus ~= '') then
					textMalus = concatUCString(textMalus, '\n\n');
				end
				textMalus = concatUCString(textMalus, fmt);
			end;
		end
	end

	if (hasBonus) then
		uiGroup.pvpEffectsBonusMalusInfo.uc_hardtext_format 	= i18n.get('uiPvpEffectBonus');
		uiGroup.pvpEffectsBonusMalus.uc_hardtext_format 		= textBonus;
	elseif (hasMalus) then
		uiGroup.pvpEffectsBonusMalusInfo.uc_hardtext_format 	= i18n.get('uiPvpEffectMalus');
		uiGroup.pvpEffectsBonusMalus.uc_hardtext_format 		= textMalus;
	else
		uiGroup.pvpEffectsBonusMalusInfo.uc_hardtext_format 	= '';
		uiGroup.pvpEffectsBonusMalus.uc_hardtext_format 		= '';
	end

end

------------------------------------------------------------------------------------------------------------
function game:getFactionName(id)
	if (id == self.TPVPClan.Kami) then
		return i18n.get('uiFameKami');
	elseif (id == self.TPVPClan.Karavan) then
		return i18n.get('uiFameKaravan');
	elseif (id == self.TPVPClan.Fyros) then
		return i18n.get('uiFameFyros');
	elseif (id == self.TPVPClan.Matis) then
		return i18n.get('uiFameMatis');
	elseif (id == self.TPVPClan.Tryker) then
		return i18n.get('uiFameTryker');
	elseif (id == self.TPVPClan.Zorai) then
		return i18n.get('uiFameZorai');
	else
		return i18n.get('Unknown');
	end
end

------------------------------------------------------------------------------------------------------------
function game:getAllegiancePoints()
	local path 			= 'SERVER:PVP_EFFECTS:PVP_FACTION_POINTS';
	local civ 			= getDbProp(path .. ':CIV');
	local civPoints 	= getDbProp(path .. ':CIV_POINTS');
	local cult 			= getDbProp(path .. ':CULT');
	local cultPoints 	= getDbProp(path .. ':CULT_POINTS');

	local text;
	local uiGroup= getUICaller();

	-- civ allegiance
	if (civ == self.TPVPClan.None or civ == self.TPVPClan.Neutral) then
		text = i18n.get('uiPvpFameNoCivAllegiance');
	else
		text = i18n.get('uiPvpFameAllegiancePoints');
		text = findReplaceAll(text, '%faction', self:getFactionName(civ));
		text = findReplaceAll(text, '%points', tostring(civPoints));
	end
	uiGroup.civ_allegiance_pts.uc_hardtext_format = text;

	-- cult allegiance
	if (cult == self.TPVPClan.None or cult == self.TPVPClan.Neutral) then
		text = i18n.get('uiPvpFameNoCultAllegiance');
	else
		text = i18n.get('uiPvpFameAllegiancePoints');
		text = findReplaceAll(text, '%faction', self:getFactionName(cult));
		text = findReplaceAll(text, '%points', tostring(cultPoints));
	end
	uiGroup.cult_allegiance_pts.uc_hardtext_format = text;
end

------------------------------------------------------------------------------------------------------------
function game:updateAllegiance(path, uiText)
	local	alleg = getDbProp(path);

	local text = i18n.get('uiFameAllegiance' .. tostring(alleg) );
	getUICaller()[uiText].uc_hardtext= text;
end

------------------------------------------------------------------------------------------------------------
function game:fameAllegianceTooltipCiv()
	local	enum= getDbProp('SERVER:FAME:CIV_ALLEGIANCE');
	-- set the tooltip in InterfaceManager
	setContextHelpText( i18n.get('uittFameAllegianceCiv' .. tostring(enum)) );
end

------------------------------------------------------------------------------------------------------------
function game:fameAllegianceTooltipCult()
	local	enum= getDbProp('SERVER:FAME:CULT_ALLEGIANCE');
	-- set the tooltip in InterfaceManager
	setContextHelpText( i18n.get('uittFameAllegianceCult' .. tostring(enum)) );
end

------------------------------------------------------------------------------------------------------------
function game:fameAllegianceTooltipCivGuild()
	local	enum= getDbProp('SERVER:GUILD:FAME:CIV_ALLEGIANCE');
	-- set the tooltip in InterfaceManager
	setContextHelpText( i18n.get('uittFameAllegianceCivGuild' .. tostring(enum)) );
end

------------------------------------------------------------------------------------------------------------
function game:fameAllegianceTooltipCultGuild()
	local	enum= getDbProp('SERVER:GUILD:FAME:CULT_ALLEGIANCE');
	-- set the tooltip in InterfaceManager
	setContextHelpText( i18n.get('uittFameAllegianceCultGuild' .. tostring(enum)) );
end

------------------------------------------------------------------------------------------------------------
--
function game:tooltipDeltaValue(base, max)
	-- Calculate delta
	local val = max - base;

	local text;
	if (val == 0) then
		text = concatUCString('@{FFFF}', tostring(max));
	else
		if (val > 0) then
			-- bonus
			text = concatUCString('@{FFFF}', tostring(max));
			text = concatUCString(text, ' (');
			text = concatUCString(text, tostring(base));
			text = concatUCString(text, '@{0F0F}');
			text = concatUCString(text, ' + ');
			text = concatUCString(text, tostring(val));
			text = concatUCString(text, '@{FFFF}');
			text = concatUCString(text, ')');
		else
			-- malus
			text = concatUCString('@{FFFF}', tostring(max));
			text = concatUCString(text, ' (');
			text = concatUCString(text, tostring(base));
			text = concatUCString(text, '@{E42F}');
			text = concatUCString(text, ' - ');
			text = concatUCString(text, tostring(math.abs(val)));
			text = concatUCString(text, '@{FFFF}');
			text = concatUCString(text, ')');
		end
	end

	return text;

end

------------------------------------------------------------------------------------------------------------
--
function game:tooltipScore(dbBase, dbMax, ttFormat)
	-- Get DB values
	local base = getDbProp(dbBase);
	local max = getDbProp(dbMax);

	-- Tooltip text
	local fmt = i18n.get(ttFormat);
	local text = self:tooltipDeltaValue(base, max);
	fmt = findReplaceAll(fmt, "%n", text );

	-- Set tooltip
	setContextHelpText(fmt);
end

------------------------------------------------------------------------------------------------------------
--
function game:tooltipScoreEP(dbBase, dbMax, ttFormat, dbLvl, dbMod)
	-- Defender level
	local defLvl= getDbProp(formatUI(dbLvl));
	defLvl = math.max(0, defLvl);

	-- Attacker level
	local attLvl = getBaseSkillValueMaxChildren(getSkillIdFromName('SF'));

	-- Get DB values
	local base = getDbProp(dbBase);
	local max = getDbProp(dbMax);
	local chance = getDodgeParryChance(attLvl, defLvl);
	local mod = getDbProp(dbMod);
	local maxChance = chance + mod;

	-- Tooltip text
	local fmt = i18n.get(ttFormat);
	local text = self:tooltipDeltaValue(base, max);
	local textChance = self:tooltipDeltaValue(chance, maxChance);
	fmt = findReplaceAll(fmt, "%n", text );
	fmt = findReplaceAll(fmt, "%l", tostring(attLvl));
	fmt = findReplaceAll(fmt, "%p", textChance);

	-- Set tooltip
	setContextHelpText(fmt);
end




-------------------------------------------------------------------------------------------------------------
----------------------------------       RING STATS       ---------------------------------------------------
-------------------------------------------------------------------------------------------------------------

RingPlayerInfo =
{
	WaitingInfo = false,
	LastRefreshTime = 0,
	InfoReceived = false,
	PendingRefresh = false,
	WaitingPeriod = 15,
	RefreshPeriod = 10,
	MinRefreshPeriod = 4,
	LastRefreshQuerryTime = 0,
}

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:getWindow()
	local ui = getUI("ui:interface:info_player_skills")
	assert(ui)
	return ui
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:initRingStatPlayer()

	setOnDraw(self:getWindow(), "RingPlayerInfo:onRingRatingPlayerDraw()")
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:onRingRatingPlayerDraw()

	local timeInSec = nltime.getLocalTime() / 1000
	if self.WaitingInfo then
		if timeInSec - self.LastRefreshTime > self.WaitingPeriod then
			self.WaitingInfo = false
			self.LastRefreshTime = nltime.getLocalTime() / 1000
		else
			if not self.InfoReceived then
				--debugInfo("No received info")
			end
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

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:updatePendingRefresh()

	if self.PendingRefresh then
		local currTime = nltime.getLocalTime() / 1000
		if currTime - self.LastRefreshQuerryTime > self.MinRefreshPeriod and game.getRingStats then
			self.LastRefreshQuerryTime = currTime
			self.PendingRefresh = false
			game.getRingStats()
		end
	end
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:onRingStatsPlayerReceving(ringPoints)

	self.WaitingInfo = false
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.InfoReceived = true

	self:fill(ringPoints)
end

function RingPlayerInfo:fill(ringPoints)

	self.InfoReceived = false

	local ui = self:getWindow()

	-- author Rating
--	local authorRR = ui:find("author_ring_rating")
--	local jaugeUI = authorRR:find("jauge_bar")
--	local levelUI = authorRR:find("level_rating")
--	local tooltipUI = authorRR:find("tt")
--	local level, progress = self:getLevelRatingAndImprovementRate(ringPoints.AuthorRating)
--	jaugeUI.w = progress*156
--	local texturename = ""
--	if level~=0 then texturename = "r2ed_ring_rating_" .. level .. ".tga" end
--	levelUI.texture = texturename
--	tooltipUI.tooltip = self:tooltipRingRating(level, progress, "uiR2EDAuthorRingRatingTooltip")

	-- AM Rating
--	local AMRR = ui:find("am_ring_rating")
--	jaugeUI = AMRR:find("jauge_bar")
--	levelUI = AMRR:find("level_rating")
--	tooltipUI = AMRR:find("tt")
--	level, progress = self:getLevelRatingAndImprovementRate(ringPoints.AMRating)
--	jaugeUI.w = progress*156
--	texturename = ""
--	if level~=0 then texturename = "r2ed_ring_rating_" .. level .. ".tga" end
--	levelUI.texture = texturename
--	tooltipUI.tooltip = self:tooltipRingRating(level, progress, "uiR2EDAMRingRatingTooltip")

	-- masterless Rating
--	local masterlessRR = ui:find("masterless_ring_rating")
--	jaugeUI = masterlessRR:find("jauge_bar")
--	levelUI = masterlessRR:find("level_rating")
--	tooltipUI = masterlessRR:find("tt")
--	level, progress = self:getLevelRatingAndImprovementRate(ringPoints.MasterlessRating)
--	jaugeUI.w = progress*156
--	texturename = ""
--	if level~=0 then texturename = "r2ed_ring_rating_" .. level .. ".tga" end
--	levelUI.texture = texturename
--	tooltipUI.tooltip = self:tooltipRingRating(level, progress, "uiR2EDMasterlessRingRatingTooltip")

	-- ecosystem Points
	local ecosystems = {"Basic", "Desert", "Subtropic", "Forest", "Jungle", "PrimeRoot"}
	for k, eco in pairs(ecosystems) do
		local ecoVal = tostring(ringPoints[eco.."RingPoints"])
		local ecoValMax = tostring(ringPoints["Max" .. eco.."RingPoints"])
		local ecoUI = ui:find(string.lower(eco))
		local maxUI = ecoUI:find("max")
		local valUI = ecoUI:find("val")
		tooltipUI = ecoUI:find("tt")
		maxUI.hardtext = ecoValMax
		valUI.hardtext = ecoVal
		tooltipUI.tooltip = self:tooltipEcosystemPoints(ecoVal, ecoValMax, "uiR2ED" .. eco .. "PointsTooltip")
	end
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:refresh()

	self.PendingRefresh = true
	self.LastRefreshTime = nltime.getLocalTime() / 1000
	self.WaitingInfo = true
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:tooltipEcosystemPoints(rp, maxRp, ttFormat)

	-- Tooltip text
	local fmt = i18n.get(ttFormat);
	fmt = findReplaceAll(fmt, "%n", rp );
	fmt = findReplaceAll(fmt, "%p", maxRp );

	-- Set tooltip
	return fmt;
end


------------------------------------------------------------------------------------------------------------
function RingPlayerInfo:updateRRPSLevel(dbVal, tooltip)


	-- get values. minimize the value displayed
	local	val= getDbProp(dbVal);

	-- get the ui text
	local	ui= getUICaller();
	local	uiText= ui.val;

	-- set the text
	uiText.uc_hardtext= tostring(val)

	self:tooltipRRPs(dbVal, tooltip)
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:tooltipRRPs(dbBase, ttFormat)

	local val = getDbProp(dbBase);

	-- Tooltip text
	local fmt = i18n.get(ttFormat);
	local text = tostring(val)
	fmt = findReplaceAll(fmt, "%n", text );

	-- Set tooltip
	setContextHelpText(fmt);
end


--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:tooltipRingRating(level, progress, ttFormat)

	-- Tooltip text
	local fmt = i18n.get(ttFormat);
	progress = math.floor(progress*100)
	fmt = findReplaceAll(fmt, "%n", tostring(level));
	fmt = findReplaceAll(fmt, "%p", tostring(progress));
	-- Set tooltip
	return fmt;
end

--------------------------------------------------------------------------------------------------------------
--
function RingPlayerInfo:getLevelRatingAndImprovementRate(val)

	val = val / 100
	local level = 0
	local maxRatingInLevel = 0
	local minRatingInLevel = 0

	for i=0,9 do
		level = i
		minRatingInLevel = maxRatingInLevel
		maxRatingInLevel = maxRatingInLevel + math.pow(4, i+1)
		if maxRatingInLevel>val then break end
	end

	local progress = (val-minRatingInLevel)/(maxRatingInLevel-minRatingInLevel)

	return level, progress
end

--------------------------------------------------------------------------------------------------------------
--
function game:updateOrganization(path, uiOrgText, uiStatusText, uiPointsText)

	local org = getDbProp(path.."1:VALUE")
	getUICaller()[uiOrgText].uc_hardtext =  i18n.get('uiOrganization_' .. org)

	local status = getDbProp(path.."2:VALUE")
	getUICaller()[uiStatusText].uc_hardtext= status

	local points = getDbProp(path.."3:VALUE")
	getUICaller()[uiPointsText].uc_hardtext= points

end

------------------------------------------------------------------------------------------------------------
function game:organizationTooltip()
	-- set the tooltip in InterfaceManager
	setContextHelpText( i18n.get('uittOrganization') );
end


--------------------------------------------------------------------------------------------------------------
function game:popMissionList()
	local menu = getUI("ui:interface:mission_cb_menu")
	enableModalWindow(getUICaller(), "ui:interface:mission_cb_menu")
	self:updateMissionMenuSize()
end



--------------------------------------------------------------------------------------------------------------
function game:getGroupMissionFirstIndex()
	return tonumber(getDefine("ipj_nb_mission"))
end

--------------------------------------------------------------------------------------------------------------
function game:getMissionDbPath(missionIndex)
	local numMissions = game:getGroupMissionFirstIndex()
	if missionIndex >= numMissions then -- group mission ?
		return "SERVER:GROUP:MISSIONS:" .. tostring(missionIndex - numMissions)
	else
		return "SERVER:MISSIONS:" .. tostring(missionIndex)
	end
end

--------------------------------------------------------------------------------------------------------------
function game:getCurrMissionIndex()
	local result = getDbProp("UI:SAVE:MISSION_SELECTED")
	return result
end

function game:getCurrGroupMissionIndex()
	return getDbProp("UI:SAVE:MISSION_SELECTED") - tonumber(getDefine("ipj_nb_mission"))
end


--------------------------------------------------------------------------------------------------------------
function game:updateCurrMissionComboBox()
	local numMissions = tonumber(getDefine("ipj_nb_mission"))
	local missionFound = false
	local cb = getUI("ui:interface:info_player_journal:content:mission_combo")
	local missionList = getUI("ui:interface:info_player_journal:content:mission_list")
	for i = 0, numMissions - 1 do
		if getDbProp("SERVER:MISSIONS:" .. i .. ":TITLE") ~= 0
		   or getDbProp("SERVER:GROUP:MISSIONS:" .. i .. ":TITLE") ~= 0 then
			missionFound = true
			break
		end
	end
	if not missionFound then
		cb.arrow.active = false
		cb.mission_ico.active = false
		cb.mission_title.active = false
		cb.select.active = false
		cb.no_selected_mission.active = false
		cb.no_available_mission.active = true
		missionList.no_selected_mission.active = false
		missionList.no_available_mission.active = true
		return
	end
	cb.no_available_mission.active = false
	missionList.no_available_mission.active = false
	cb.arrow.active = true
	cb.select.active = true
	local currMission = self:getCurrMissionIndex()

	local dbPath = self:getMissionDbPath(currMission)
	--
	local selected = (currMission ~= -1)
	if selected then
		cb.mission_title.textid_dblink = dbPath .. ":TITLE"
		selected = (tile ~= 0)
	end
	cb.mission_ico.active = selected
	cb.mission_title.active = selected
	cb.no_selected_mission.active = not selected
	missionList.no_selected_mission.active = not selected
	if selected then
		if getDbProp(dbPath .. ":FINISHED") == 0 then
			cb.mission_ico.texture = runExpr("getMissionSmallIcon(" .. tostring(getDbProp(dbPath .. ":ICON") .. ")"))
		elseif getDbProp(dbPath .. ":FINISHED") == 1 then
			cb.mission_ico.texture = "Small_Task_Done.tga"
		else
			cb.mission_ico.texture = "Small_Task_Failed.tga"
		end
	end
end

--------------------------------------------------------------------------------------------------------------
function game:onMissionSelected(index)
	disableModalWindow()
	self:updateCurrMissionComboBox()
	game.WebMissionLastDesc = {}
	setOnDraw(getMissionWindow(), "game:ensureWebMissionVisibility()")
end

--------------------------------------------------------------------------------------------------------------
function game:onGroupMissionSelected(index)
	disableModalWindow()
	self:updateCurrMissionComboBox()
end

--------------------------------------------------------------------------------------------------------------
function game:onMissionDBIndexChanged()
	local missionIndex = self:getCurrMissionIndex()
	if missionIndex < 0 then return end
	-- if selection was made from the list, update the other list
	if missionIndex >= self:getGroupMissionFirstIndex() then
		local groupMissionIndex = missionIndex - self:getGroupMissionFirstIndex()
		getUI("ui:interface:info_player_journal:content:mission_list:b_group_title" .. tostring(groupMissionIndex)).pushed = true
		getUI("ui:interface:mission_cb_menu:mission_list:b_group_title" .. tostring(groupMissionIndex)).pushed = true
	else
		getUI("ui:interface:info_player_journal:content:mission_list:b_title" .. tostring(missionIndex)).pushed = true
		getUI("ui:interface:mission_cb_menu:mission_list:b_title" .. tostring(missionIndex)).pushed = true
	end
	game:updateMissionWindowLayout()
end

--------------------------------------------------------------------------------------------------------------
function game:onMissionTitleChanged(index)
	-- if title is not nil then a new mission has been added -> if db initilization is over, then selected this new mission
	if getDbProp(self:getMissionDbPath(index) .. ":TITLE") ~= 0 then
		if game.InGameDbInitialized or config.Local then
			self:setCurrentMission(index)
		end
	else
		self:updateCurrMissionComboBox()
		self:updateMissionMenuSize()
	end
end
--------------------------------------------------------------------------------------------------------------
function game:onGroupMissionTitleChanged(index)
	if getDbProp(self:getMissionDbPath(index + 15) .. ":TITLE") ~= 0 then
		if game.InGameDbInitialized or config.Local then
			self:setCurrentMission(index + 15)
		end
	else
		self:updateCurrMissionComboBox()
		self:updateMissionMenuSize()
	end
end

--------------------------------------------------------------------------------------------------------------
function game:updateMissionMenuSize()
	local parentCB = getUI("ui:interface:info_player_journal:content:mission_combo")
	local menu = getUI("ui:interface:mission_cb_menu")

	if not menu.active then
		return
	end

	local maxNumMissions = 2 * self:getGroupMissionFirstIndex()
	local missionCount = 0
	for k = 0, maxNumMissions - 1 do
		if getDbProp(self:getMissionDbPath(k) .. ":TITLE") ~= 0 then
			missionCount = missionCount + 1
		end
	end
	menu.h = 8 + missionCount * 18
	menu.y = 0
	menu:updateCoords()
	local y = parentCB.y_real - menu.h_real - 1
	if y < 0 then
		y = parentCB.y_real + parentCB.h_real + 1
	end
	local scrW
	local scrH
	scrW, scrH = getWindowSize()
	if y + menu.h > scrH then
		y = scrH - menu.h
	end
	menu.w = parentCB.w_real
	menu.y = y
	menu.x = parentCB.x_real
	menu.h = 8 + missionCount * 18
	menu:invalidateCoords()
end

function game:parseLangText(text)
	if text == nil then
		return ""
	end
	local final = ""
	local work = ""
	local translated = ""
	local stext = text:split("[[]")
	for k, v in pairs(stext) do
		if string.sub(v, 3, 3) == "]" then
			if string.sub(v, 1, 2):upper() == getClientCfg("LanguageCode"):upper() then
				work = ""
				translated = string.sub(v, 4)
			elseif string.sub(v, 1, 2):upper() == "WK" and translated == "" then
				work = string.sub(v, 4)
			end
		else
			if k > 1 then
				final = final .. "["
			end
			final = final .. v
		end
	end
	return final .. work .. translated
end

function game:onOverCap()
	local ui = getUI("ui:interface:cap")
	mx, my = getMousePos()
	-- Inside
	if mx > ui.x and mx < ui.x + ui.w and my < ui.y and my > ui.y - ui.h then
		if getCurrentWindowUnder() == ui then
			if game.keepExpandStatus or ui.opened == false then return end
			if game.capExpanded == false then
				game:expandCapWeb(true)
			end
		end
	else
	-- Outside
		if game.keepExpandStatus or ui.opened == false then return end
		if mx < ui.x or mx > ui.x + ui.w or my > ui.y or my < ui.y - ui.h then
			if game.capExpanded then
				game:expandCapWeb(false)
			end
		end
	end
end

function game:expandCapWeb(expand)
	local ui = getUI("ui:interface:cap")
	if expand then
		setTopWindow(ui)
		game.capExpanded = true
		ui.pop_min_h = game.ui_cap_expanded_h
		ui.pop_max_h = game.ui_cap_expanded_h + 1
		ui:find("header_opened").h = game.ui_cap_expanded_h - 16
	else
		game.capExpanded = false
		ui.pop_min_h = game.ui_cap_collapsed_h
		ui.pop_max_h = game.ui_cap_collapsed_h+1
		ui:find("header_opened").h = game.ui_cap_collapsed_h - 16
	end
end

function game:keepExpandCapWeb(force)
	local ui = getUI("ui:interface:cap")
	if force ~= nil then
		game.keepExpandStatus = not force
	end

	if game.keepExpandStatus == false then
		game.keepExpandStatus = true
		ui:find("right").texture = "w_win_lock.tga"
	else
		game.keepExpandStatus = false
		ui:find("right").texture = "grey_0.tga"
	end
end


function game:onCapResize()
	local ui = getUI("ui:interface:cap")
	local web = ui:find("html")
	web.w = ui.w - 18
end

game.ui_cap_collapsed_h = 65
game.ui_cap_expanded_h = 280
game.ui_cap_w = 292

function game:closeCapHeader()
	local ui = getUI("ui:interface:cap")

	-- save size
	game.ui_cap_h = ui.h
	game.ui_cap_w = ui.w

	-- reduce window size
	ui.pop_min_w = 292
	ui.pop_min_h = 50
	ui.pop_max_w = 292
	ui.pop_max_h = 50
	ui.h = 50
	ui.w = 280
	getUI("ui:interface:cap:header_closed").h = 34
end

function game:openCapHeader()
	local ui = getUI("ui:interface:cap")
	ui.pop_min_w = 292
	ui.pop_max_w = 800

	game:expandCapWeb(true)

	if (game.ui_cap_w ~= nil) then
		ui.w = game.ui_cap_w;
	end
end

function game:openFullCap()
	game.ui_cap_collapsed_h = 65
	getUI("ui:interface:cap").opened = true
	game:openCapHeader()
end

function game:resizeCap(value)
	local ui = getUI("ui:interface:cap")
	game.ui_cap_expanded_h = value
	if game.capExpanded and getUI("ui:interface:cap").opened then
		ui.pop_min_h = game.ui_cap_expanded_h
		ui.pop_max_h = game.ui_cap_expanded_h+1
		ui.h = game.ui_cap_expanded_h
		getUI("ui:interface:cap:header_opened").h = ui.h-16
	end
end

function game:updateCapTooltip()
	local real_tooltip = game:parseLangText(mission_real_tooltip)
	getUI("ui:interface:cap:header_opened:cap_group:cap_ctrl").tooltip = getUCtf8("@{FB0F}"..game.CapTitle.."\n@{FFFF}"..game.CapDesc..real_tooltip)
	getUI("ui:interface:cap:header_closed:cap_group:cap_ctrl").tooltip = getUCtf8("@{FB0F}"..game.CapTitle.."\n@{FFFF}"..game.CapDesc..real_tooltip)
end

function game:setCapTitle(text)
	game.CapTitle = game:parseLangText(text)
	getUI("ui:interface:cap:header_opened:cap_group:cap_title").hardtext = game.CapTitle
	game:updateCapTooltip()
end

function game:setCapDesc(text)
	game.CapDesc = game:parseLangText(text)
	getUI("ui:interface:cap:header_opened:cap_group:cap_desc").hardtext = game.CapDesc
	getUI("ui:interface:cap:header_closed:cap_group:cap_desc").hardtext = game.CapDesc
	game:updateCapTooltip()
end

function game:setCapIcon(icon)
	getUI("ui:interface:cap:header_opened:cap_group:cap_icon").texture = icon
	getUI("ui:interface:cap:header_closed:cap_group:cap_icon").texture = icon
end

function game:setCapInfos(infos)
	getUI("ui:interface:cap:header_opened:cap_group:infos").uc_hardtext_format=getUCtf8(infos)
	game.CapInfos = infos
end

function game:informNewCap()
	local resize = 70
	if getUI("ui:interface:cap:header_closed").active then
		getUI("ui:interface:cap:header_closed:cap_group:infos").uc_hardtext_format=getUCtf8("@{0AFF}Nouveau cap disponible")
		getUI("ui:interface:cap:header_closed").h = 70
		game.ui_cap_closed_h = 70
	else
		getUI("ui:interface:cap:header_opened:cap_group:infos").uc_hardtext_format=getUCtf8("@{0AFF}Nouveau cap disponible")
		if game.keepExpandStatus then
			return
		end
		getUI("ui:interface:cap:header_opened").h = 70
		resize = 84
	end
	local ui = getUI("ui:interface:cap")
	ui.pop_min_h = resize
	ui.pop_max_h = resize+1
end


function game:capOpenUrl(url)
	if url == nil then
		local ui = getUI("ui:interface:cap:header_opened:cap_group:html")
		if ui then
			ui:browse(game.CapUrl)
		end
	else
		webig:openUrlInBg(url)
	end
end


function game:setCapProgress(value, text)
    if value == nil then
		getUI("ui:interface:cap:header_opened:cap_group:cap_progress").active = false
		getUI("ui:interface:cap:header_closed:cap_group:cap_progress").active = false
	else
		getUI("ui:interface:cap:header_opened:cap_group:cap_progress").active = true
		getUI("ui:interface:cap:header_closed:cap_group:cap_progress").active = true
		if value >= 0 then
			if value > 100 then
				game:capOpenUrl(game.CapNextUrl)
			else
				setDbProp("UI:TEMP:CAP_PROGRESS", value)
			end
		end
	end

	if text ~= nil then
		getUI("ui:interface:cap:header_opened:cap_group:cap_infos").hardtext = text
		getUI("ui:interface:cap:header_closed:cap_group:cap_infos").hardtext = text
	end
end

function game:autoHideCapPopup()

	if game.autoHideCapTimer == 0 then
		alpha = nltime.getLocalTime() - game.autoHideCapStartTime
		if alpha >= 254*5 then
			setOnDraw(getUI("ui:interface:cap_popup"), "")
			getUI("ui:interface:cap_popup").active=false
		else
			getUI("ui:interface:cap_popup").alpha=255-math.floor(alpha/5)
		end
	else
		if game.autoHideCapStartTime + game.autoHideCapTimer < nltime.getLocalTime() then
			game.autoHideCapStartTime = nltime.getLocalTime()
			game.autoHideCapTimer = 0
		end
	end
end

function game:displayRpMessage(message, icon)
	if icon == nil then
		icon = "rpjob_roleplay.tga"
	end

	local htmlcode = [[
	<body style="font-style: italic; font-weight: bold; color: white; background-position: center; background-image: url(pretty_notif.tga)">
	<table width="100%" cellspacing="0" cellpadding="0">
	<tr>
		<td id="icon" align="center" width="750" valign="middle" height="30px"><img width="32px" src="]]..icon..[["/></td>
	</tr><tr>
	<tr>
		<td align="center">]]..message..[[</td>
	</tr>
	</table>
	</body>
	]]

	getUI("ui:interface:cap_popup:html"):renderHtml(htmlcode)
	setTopWindow(getUI("ui:interface:cap_popup"))
	getUI("ui:interface:cap_popup").alpha=255
	getUI("ui:interface:cap_popup").y = getUI("ui:interface").h-170
	getUI("ui:interface:cap_popup").x = math.floor(getUI("ui:interface").w / 2) - 400
	getUI("ui:interface:cap_popup").active = true
	game.autoHideCapStartTime = nltime.getLocalTime()
	game.autoHideCapTimer = 3000
	setOnDraw(getUI("ui:interface:cap_popup"), "game:autoHideCapPopup()")
end

function game:openInfosUrl()
	if game.CapInfosUrl == nil then
		game:openMissionsCatalog()
	elseif game.CapInfosUrl:sub(1, 1) == "#" then
		local infos = {}
		for info in string.gmatch(game.CapInfosUrl, "[^%s]+") do
			table.insert(infos, info)
		end
		if infos[1] == "#lesson" then
			openLesson(infos[2], infos[3], true)
		end
	elseif game.CapInfosUrl ~= "" then
		getUI("ui:interface:web_transactions_lessons"):find("html"):browse(game.CapInfosUrl)
	end
end

function game:setInfosUrl(url)
	game.CapInfosUrl = url
end

function game:setNextUrl(url)
	game.CapNextUrl = url
end

function setCap(id, element, a, b)
	if element == nil then
		game.CapId = id
		game:setInfosUrl(a)
		game:setNextUrl(b)
		return
	end

	if id ~= game.CapId then
		return
	end

	if element == "t" then
		game:setCapTitle(a)
	elseif element == "d" then
		game:setCapDesc(a)
	elseif element == "i" then
		game:setCapIcon(a)
	elseif element == "p" then
		game:setCapProgress(a, b)
	elseif element == "u" then
		game:setInfosUrl(a)
		game:setNextUrl(b)
	elseif element == "n" then
		game:setNextUrl(a)
	elseif element == "o" then
		game:setInfosUrl(a)
	elseif element == "r" then
		game:displayRpMessage(a, b)
	elseif element == "b" then
		broadcast(a, b)
	elseif element == "f" then
		getUI("ui:interface:cap"):blink(1)
	end
end


function game:openMissionsCatalog()
	getUI("ui:interface:web_transactions"):find("html"):browse("https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=8747&command=reset_all")
end


--------------------------------------------------------------------------------------------------------------
function game:onMissionFinished(index)
	self:updateCurrMissionComboBox()
	--self:updateMissionDescCloseButton(index)
end

--------------------------------------------------------------------------------------------------------------
function game:onGroupMissionFinished(index)
	self:updateCurrMissionComboBox()
	--self:updateMissionDescCloseButton(index + game:getGroupMissionFirstIndex())
end

--------------------------------------------------------------------------------------------------------------
function game:expandMissionList()
	local missionCB = getUI("ui:interface:info_player_journal:content:mission_combo")
	missionCB.active = not missionCB.active
	self:updateMissionWindowLayout()
end

--------------------------------------------------------------------------------------------------------------
function game:updateMissionWindowLayout()
	if not isInRingMode() then
		local base = "ui:interface:info_player_journal:content:"
		local missionCB = getUI(base.."mission_combo")
		local missionList = getUI(base.."mission_list")
		local fake = getUI(base.."fake")
		local desc = getUI(base.."desc")
		local separator = getUI(base.."separator")
		local expanded
		local win = getUI("ui:interface:info_player_journal")

		if missionCB.active then
			missionList.active = false
			separator.active = true
			expanded = 0
			win.pop_min_h = 165 - win.content_y_offset
			desc.max_h = 0
		else
			missionList.active = true
			separator.active = true
			expanded = 1
			win.pop_min_h = 200 - win.content_y_offset
			desc.max_h = 27
		end

		setDbProp("UI:SAVE:EXPAND_MISSION_LIST", expanded)
		win:invalidateCoords()
	end
end
--------------------------------------------------------------------------------------------------------------
function game:onMissionJournalOpened()
	local missionDesc = getUI("ui:interface:info_player_journal:content:desc")
	missionDesc.active = getDbProp("UI:SAVE:MISSION_SELECTED") ~= -1

	local expandList = getDbProp("UI:SAVE:EXPAND_MISSION_LIST")
	self:updateMissionJournalMode()

	if not isInRingMode() then
		local missionCB = getUI("ui:interface:info_player_journal:content:mission_combo")
		if expandList == 1 then
			missionCB.active = false
		else
			missionCB.active = true
		end
	end

	self:updateMissionJournalHeader()
	self:updateMissionJournalFixedEntry()
end

--------------------------------------------------------------------------------------------------------------
function game:updateMissionJournalHeader()
	local win = getUI("ui:interface:info_player_journal")
	local headerActive = getDbProp("UI:SAVE:MISSION_JOURNAL_HEADER_ACTIVE") ~= 0
	win.header_active = headerActive
	win.right_button_enabled = headerActive
	if headerActive then
		win.uc_title_opened = i18n.get("uiJournalTitle")
		win.content_y_offset = 0
	else
		win.uc_title_opened = ucstring("")
		win.content_y_offset = win.header_opened.h_real + 3
	end
end


--------------------------------------------------------------------------------------------------------------
function game:updateMissionJournalFixedEntry()
	-- update fixed entry text
	self:updateMissionWindowLayout()
end

--------------------------------------------------------------------------------------------------------------
function game:setCurrentMission(index)
	mw = getMissionWindow()
	mw.active = game.InGameDbInitialized
	if index < self:getGroupMissionFirstIndex() then
		runAH(nil, "proc", "mission_proc_title|" .. tostring(index))
	else
		runAH(nil, "proc", "group_mission_proc_title|" .. tostring(index - self:getGroupMissionFirstIndex()))
	end
end

--------------------------------------------------------------------------------------------------------------
function game:onMissionComboWheelUp()
	local currMissionIndex = self:getCurrMissionIndex()
	while currMissionIndex > 0 do
		currMissionIndex = currMissionIndex - 1
		if getDbProp(self:getMissionDbPath(currMissionIndex) .. ":TITLE") ~= 0 then
			self:setCurrentMission(currMissionIndex)
			return
		end
	end
end

--------------------------------------------------------------------------------------------------------------
function game:onMissionComboWheelDown()
	local currMissionIndex = self:getCurrMissionIndex()
	local maxNumMission = 2 * self:getGroupMissionFirstIndex()
	while currMissionIndex < (maxNumMission - 1) do
		currMissionIndex = currMissionIndex + 1
		if getDbProp(self:getMissionDbPath(currMissionIndex) .. ":TITLE") ~= 0 then
			self:setCurrentMission(currMissionIndex)
			return
		end
	end
end



--------------------------------------------------------------------------------------------------------------
function game:toggleMissionJournalCaption()
	local dbPath = "UI:SAVE:MISSION_JOURNAL_HEADER_ACTIVE"
	setDbProp(dbPath, 1 - getDbProp(dbPath))
	local win = getUI("ui:interface:info_player_journal")
	self:updateMissionJournalHeader()
	self:updateMissionWindowLayout()
end

--------------------------------------------------------------------------------------------------------------
-- handler called by C++ to tell that the main loop is about to begin
function game:onMainLoopBegin()
	--getUI("ui:interface:db_loading").active=true
	game.InGameDbInitialized = false
	game.setupWebigWithDBInitialized = false
	game.webigInitialized = false
	game.PrevSessionMission = getDbProp("UI:VARIABLES:MISSION_SELECTED_PREV_SESSION")

	debug("MainLoop")
end

function game:checkRpItemsUpdate()
	if game.initializedRpItems == 0 then
		if nltime.getLocalTime() > game.initializedRpItems + 10 then
			game.initializedRpItems = nltime.getLocalTime()
			game.usedRpRightItem = "xx"
			game:updateRpItems()
		end
	else
		setOnDraw(getUI("ui:interface:rpitems_actions"), "")
	end

end

--------------------------------------------------------------------------------------------------------------
-- handler called by C++ to tell that all initial value have been set in the db
function game:onInGameDbInitialized()
	--getUI("ui:interface:db_loading").active=false
	game.InGameDbInitialized = true
	debug("IG DB initialized")
	-- Add waiters to guild chests
	for i=0, 19 do
		addOnDbChange(getUI("ui:interface:inv_guild"), "@SERVER:GUILD:CHEST:"..tostring(i)..":NAME", "updateChestList()")
	end
	addOnDbChange(getUI("ui:interface:inv_guild"), "@SERVER:GUILD:CHEST:0:BULK_MAX", "updateChestList(true)")

	game.initializedRpItems = nltime.getLocalTime()
	game.usedRpRightItem = "xx"
	game:updateRpItems()
	setOnDraw(getUI("ui:interface:rpitems_actions"), "game:checkRpItemsUpdate()")

	-- if the journal is opened, force an update for the fixed entry text
	-- (says if we're in start island, paying account ...) need DB flags like
	-- IS_NEWBIE & IS_TRIAL to be received
	game:updateMissionJournalFixedEntry()
	-- If a mission was previously selected, restore it
	if game.PrevSessionMission ~= -1  then
		self:setCurrentMission(game.PrevSessionMission)
	end

	game:setInfoPlayerCharacterRace()

	game:openChannels()

	runAH(nil, "proc", "init_3d_preview")
	runAH(nil, "sort_tribefame", "")

	if getDbProp("UI:SAVE:AUTO_LIGHT") == 1 then
		runAH(nil, "light_on", "")
	end
end

function game:onWebIgReady()
	-- Call init webig
	debug("Webig ready")
	game.webigInitialized = true
	setOnDraw(getUI("ui:interface:encyclopedia"), "ArkMissionCatalog:startResize()")
	getUI("ui:interface:webig:content:html"):browse("home")
	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 0 then
		addOnDbChange(getUI("ui:interface"), "@UI:SAVE:MK_MODE", "game:resizeMilkoPad()")
		help:initWelcome()
	end

	game.setupWebigWithDBInitialized = true
	help:displayWelcome()
	game:onCapResize()
	local cap = getUI("ui:interface:cap")
	setOnDraw(cap, "game:onOverCap()")
	ArkLessons:init()

end

--------------------------------------------------------------------------------------------------------------
-- handler called by C++ at the start of a far TP (log to char selection or far tp)
function game:onFarTpStart()
	debugInfo("game:onFarTpStart()")
	--game:deinitWebIgApps()
	--
	if getDbProp("UI:SAVE:CHAT:SAVE_CHANNEL") > 0 then
		game:saveChannel()
	end

	artefact:onClose()
end

--------------------------------------------------------------------------------------------------------------
-- handler called by C++ after characer reselection or the end of a far TP
function game:onFarTpEnd()
	debugInfo("game:onFarTpEnd()")
	--game:preInitWebIgApps()
end

--------------------------------------------------------------------------------------------------------------
-- handler called by C++ at the end of the main loop
function game:onMainLoopEnd()
	game.InGameDbInitialized = false
	game:updateMissionJournalFixedEntry()
	--
	if getDbProp("UI:SAVE:CHAT:SAVE_CHANNEL") > 0 then
		game:saveChannel()
	end

	artefact:onClose()
end

--------------------------------------------------------------------------------------------------------------
-- ring journal on / off
function game:setMissionJournalRingMode(isRing)
	local journal = getUI("ui:interface:info_player_journal")
	if isRing then
		journal.content.expand_mission_list.active = false
		journal.content.mission_combo.active = false
		journal.content.active = true
		journal.content.mission_list.active = false
		journal.content.sv.active = false
		journal.content.fake.active = false
		journal.content.separator.active = false
		journal.content.separator_bis.active = false
		journal.content.desc.active = false
		journal.content.sv_desc.active = false
		journal.no_available_missions.active = true
	else
		journal.content.expand_mission_list.active = true
		journal.no_available_missions.active = false;
		journal.content.active = true;
		--journal.content.mission_list.active = true;
		journal.content.sv.active = true;
		journal.content.fake.active = true;
		journal.content.separator.active = true;
		journal.content.desc.active = getDbProp("UI:SAVE:MISSION_SELECTED") ~= -1;
		journal.content.sv_desc.active = true
	end
end

--------------------------------------------------------------------------------------------------------------
-- update mission journal depending on wether we're in the ring or not
function game:updateMissionJournalMode()
	--local isRing = r2~=nil and r2.Mode~=nil and r2.Mode=='r2ed_anim_test'
	game:setMissionJournalRingMode(isInRingMode())
end



local remainingMissionTextIDs = {}



function getMissionWindow()
	return getUI("ui:interface:info_player_journal")
end

--------------------------------------------------------------------------------------------------------------
-- This is called when a new step is added to the current mission. If so, make sure that the step
-- is visible in the listbox
function game:onNewMissionStepAdded(stepIndex)
	local missionWnd = getMissionWindow()
	local missionIndex = getDbProp("UI:SAVE:MISSION_SELECTED")
	local dbPath

	if missionIndex < 0 then
		return
	end

	-- debugInfo("New Step")
	if missionIndex < 15 then
		dbPath = "SERVER:MISSIONS:" .. tostring(missionIndex) .. ":GOALS:" .. tostring(stepIndex) .. ":TEXT"
	else
		dbPath = "SERVER:GROUP:MISSIONS:" .. tostring(missionIndex - 15) .. ":GOALS:" .. tostring(stepIndex) .. ":TEXT"
	end
	local stringID = getDbProp(dbPath)
	if stringID ~= 0 then
		-- debugInfo(tostring(stringID))
		table.insert(remainingMissionTextIDs, stringID)
		setOnDraw(missionWnd, "game:ensureLastMissionStepVisibility0()")
	else
	end
	game.WebMissionLastDesc = {}
end

function game:ensureLastMissionStepVisibility0()

	local missing = false
	for k, v in pairs(remainingMissionTextIDs) do
		if not isDynStringAvailable(v) then
			missing = true
			break
		end
	end
	local missionWnd = getMissionWindow()
	if not missing then
		remainingMissionTextIDs = {}
		-- delay real update to newt frame
		setOnDraw(missionWnd, "game:ensureLastMissionStepVisibility1()")
	else
		-- for debug : dump the list of remaining "dyn string"
		--local stringList = "{"
		--for k, v in remainingMissionTextIDs do
		--	if not isDynStringAvailable(v) then
		--		stringList = stringList .. " " .. tostring(v)
		--	end
		--end
		--stringList = stringList .. "}"
	end
end

function game:ensureLastMissionStepVisibility1()
	local missionWnd = getMissionWindow()
	local missionIndex = getDbProp("UI:SAVE:MISSION_SELECTED")
	local scrollBar = missionWnd:find("sv_desc")
	--scrollBar.trackPos = 20000 -- move upward
	--scrollBar:updateCoords()
	--setOnDraw(missionWnd, "")
	local descWnd = missionWnd:find("desc")
	local maxNumSteps = getDefine("ipj_nb_goal")
	local topStep
	for  stepIndex = 0, maxNumSteps -1 do
		local currStep = descWnd["step" .. tostring(stepIndex)]
		if currStep.active then
			topStep = currStep
		end
	end

	if topStep == nil then
		return
	end

	scrollBar:ensureVisible(topStep, "M", "M")

	--local wantedY = topStep.h_real / 2 - (descWnd.y_real - topStep.y_real)
	--local wantedY = descWnd.y_real + descWnd.h_real - topStep.y_real
	--local offsetY = wantedY - descWnd.max_h_real / 2
	--if offsetY < 0 then offsetY = 0 end
	--descWnd.ofsy = offsetY
	--descWnd:invalidateCoords()
	--descWnd:updateCoords()

	game.WebMissionLastDesc = {}
	setOnDraw(missionWnd, "game:ensureWebMissionVisibility()")

end

function game:ensureWebMissionVisibility()
	local missionWnd = getMissionWindow()
	local missionIndex = getDbProp("UI:SAVE:MISSION_SELECTED")
	local scrollBar = missionWnd:find("sv_desc")
	local descWnd = missionWnd:find("desc")
	local maxNumSteps = getDefine("ipj_nb_goal")
	local topStep
	local haveWeb = false
	for stepIndex = 0, maxNumSteps -1 do
		local currStep = descWnd["step" .. tostring(stepIndex)]
		if missionIndex < 15 then
			dbPath = "SERVER:MISSIONS:" .. tostring(missionIndex) .. ":GOALS:" .. tostring(stepIndex) .. ":TEXT"
			local stringID = getDbProp(dbPath)
			local uctext = getDynString(stringID)
			local text = uctext:toUtf8()
			if text ~= "" and game.WebMissionLastDesc[stepIndex] ~= text then
				game.WebMissionLastDesc[stepIndex] = text
				if string.sub(text, 1, 4) == "@WEB" then
					text = string.sub(text, 6)
					haveWeb = true
					local win = getUI(descWnd.id..":web:html")
					for web, final_text in string.gmatch(text, "(.*)\n@@@\n(.*)") do
						win:renderHtml(web)
						currStep.hardtext = final_text
						break
					end
				else
					currStep.uc_hardtext = uctext
				end
			end
		end

		if (game.WebMissionLastDesc[stepIndex] ~= nil) and (string.sub(game.WebMissionLastDesc[stepIndex], 1, 4) == "@WEB") then
			haveWeb = true
		end
	end

	if haveWeb then
		getUI(descWnd.id..":web").h = 56
	else
		getUI(descWnd.id..":web").h = 0
	end

	local base = "ui:interface:info_player_journal:content:"
	local fake = getUI(base.."fake")
	local missionCB = getUI(base.."mission_combo")
	local missionL = getUI(base.."mission_list")
	local desc = getUI(base.."desc")
	local win = getUI("ui:interface:info_player_journal")
	win.pop_min_h = 200 - win.content_y_offset

	local missionsH = 24
	local maxNumMissions = 2 * self:getGroupMissionFirstIndex()
	for k = 0, maxNumMissions - 1 do
		if getDbProp(self:getMissionDbPath(k) .. ":TITLE") ~= 0 then
		missionsH = missionsH + 17
		end
	end

	if missionCB.active then
		fake.h = 24
		win.pop_min_h = 224 + getUI(descWnd.id..":web").h
	else
		fake.h = missionL.h
		win.pop_min_h =  missionsH + 200 + getUI(descWnd.id..":web").h
	end
end

--------------------------------------------------------------------------------------------------------------
-- This handler is triggered when a new mission has been added. In this case, we select the mission automatically
function game:onNewMissionAdded(missionIndex)
	setOnDraw(missionWnd, "game:ensureWebMissionVisibility()")
	debugInfo("Mission " .. missionIndex .. " has been added")
	game.WebMissionLastDesc = {}
end

--------------------------------------------------------------------------------------------------------------
-- RPJOBS

function game:addRpJob(jobtype, id, value, rpjobs)
	local base_path = "ui:interface:info_player_skills:content:rpjobs:rpjob_"..jobtype.."_"..id..":rpjob_"..jobtype.."_infos_"..id

	local group = getUI("ui:interface:info_player_skills:content:rpjobs:rpjob_"..jobtype.."_"..id)

	if (value == nil) then
		group.active = false
	else
		local name = "rpjob_" .. string.format("%03d", value)
		local sitem = name..".sitem"
		if (rpjobs[sitem] == nil) then
			group.active = false
		else
			group.active = true

			local echelon_value = rpjobs[sitem][1]
			local quantity = rpjobs[sitem][2]

			local maxlevel = (echelon_value*6)-30

			if (quantity > maxlevel) then
				quantity = maxlevel
			end

			local base = getUI(base_path..":t")
			base.hardtext = i18n.get(name):toUtf8()
			local ui = getUI(base_path..":icon")
			ui.texture = name..".tga"
			local echelon = getUI(base_path..":echelon_value")
			echelon.hardtext = tostring(echelon_value/10)
			local bar = getUI(base_path..":bar3d:level")
			local t = getUI(base_path..":bar3d:t")
			if (echelon_value >= 60) then
				bar.color = "255 0 0 255"
				bar.w = "368"
				t.hardtext = i18n.get("uiRpjobMaxLevel"):toUtf8()
				t.color = "255 255 0 255"
			else
				bar.color = tostring(math.floor((105*quantity)/maxlevel)).." "..tostring(100+math.floor((155*quantity)/maxlevel)).." "..tostring(math.floor((105*quantity)/maxlevel)).." 255"
				bar.w = tostring((368*quantity)/maxlevel)
				t.hardtext = tostring(quantity).." / "..tostring(maxlevel)
				t.color = tostring(255*math.floor(3*(maxlevel-quantity)/maxlevel)).." "..tostring(255*math.floor(3*(maxlevel-quantity)/maxlevel)).." "..tostring(255*math.floor(3*(maxlevel-quantity)/maxlevel)).." 255"
			end
		end
	end
end


function game:getRPJobs()
	rpjobs_advanced = {}
	rpjobs_elementary = {}
	rpjobs_roleplay = {}
	rpjobs = {}

	for i = 0, 499, 1 do
		local sheet =  getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":SHEET")
		if (sheet ~= 0) then
			local name = getSheetName(sheet)
			if (string.sub(name, 0, 6) == "rpjob_") then
				local quality =  getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":QUALITY")
				local quantity =  getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":QUANTITY")

				if (name == "rpjob_advanced.sitem") then
					table.insert(rpjobs_advanced, quality)
				else
					if (name == "rpjob_elementary.sitem") then
						table.insert(rpjobs_elementary, quality)
					else
						if (name == "rpjob_roleplay.sitem") then
							table.insert(rpjobs_roleplay, quality)
						else
							if rpjobs[name] == nil then
								rpjobs[name] = {quality, quantity}
							else
								if rpjobs[name][1] < quality then
									rpjobs[name] = {quality, quantity}
								end
							end
						end
					end
				end
			end
		end
	end

	for id=1,2,1 do
		game:addRpJob("advanced", id, rpjobs_advanced[id], rpjobs)
	end

	for id=1,3,1 do
		game:addRpJob("elementary", id, rpjobs_elementary[id], rpjobs)
	end


end

--------------------------------------------------------------------------------------------------------------
function game:setInfoPlayerCharacterRace()
	getUI("ui:interface:info_player_skills:content:basics_skills:character_race_name").uc_hardtext = i18n.get("io"..getUserRace())
end

function game:arkTitlesAddClassics()
	local current_title = getUI("ui:interface:player:header_opened:player_title").uc_hardtext
	runAH(nil, "title_init_combobox", "")
	local cb = getUI("ui:interface:info_player_skills:content:webinfos:title:player_title")
	local ui = getUI("ui:interface:encyclopedia:content:htmlC")
	local html = [[<body style="background-color: #0009">]]

	local titles = {}
	for i=0,cb:getNumTexts()-1 do
		table.insert(titles, tostring(cb:getText(i)))
	end
	table.sort(titles)
	for i,title in ipairs(titles) do
		local current = "_blue"
		if tostring(current_title) == tostring(title) then
			current = ""
		end

		html = html .. [[<div class="ryzom-ui-grouptemplate" id="div_ark_title" style="template:title_template;id:div_ark_title:display_title;icon:ico_amber_ball]]..current..[[.tga;text:]]
		html = html .. title .. [[;titleid:]] .. title
		html = html .. [[;color:255 255 255 255;enable:50;tooltip:"></div>]]
	end
	html = html .. [[<br/><br/><br/></body>]]
	ui:renderHtml(html)
end



-- VERSION --
RYZOM_INFO_PLAYER_VERSION = 335
