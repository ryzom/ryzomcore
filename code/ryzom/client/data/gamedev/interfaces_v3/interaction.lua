-- In this file we define functions that serves for interaction windows


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

------------------------------------------------------------------------------------------------------------
-- called when server send an invitaion we receive a text id containing the string to display (invitor name)
function game:onTeamInvation(textID)
	
	local ui = getUI('ui:interface:join_team_proposal');
	ui.content.inside.invitor_name.textid = textID;
	ui.active = true;
	setTopWindow(ui);
	ui:center();
	ui:blink(2);
end

------------------------------------------------------------------------------------------------------------
-- 
function game:teamInvitationAccept()

	local ui = getUI('ui:interface:join_team_proposal');
	ui.active = false;
	sendMsgToServer('TEAM:JOIN');
end

------------------------------------------------------------------------------------------------------------
-- 
function game:teamInvitationRefuse()

	local ui = getUI('ui:interface:join_team_proposal');
	ui.active = false;
	sendMsgToServer('TEAM:JOIN_PROPOSAL_DECLINE');
end

------------------------------------------------------------------------------------------------------------
-- 
function game:switchChatTab(dbEntry)
	local	db= 'UI:SAVE:ISENABLED:' .. dbEntry;
	local	val= getDbProp(db);
	-- switch value
	if(val==0)	then
		setDbProp(db, 1);
	else
		setDbProp(db, 0);
	end
end

------------------------------------------------------------------------------------------------------------
-- 
function game:updateEmoteMenu(prop, tooltip, tooltip_pushed, name, param)
	for i=0,9 do
		-- Get key shortcut
		local text = i18n.get('uiTalkMemMsg0' .. i);
		local key = runExpr( "getKey('talk_message','0" .. i .. "',1)" );
		
		if (key ~= nil and key  ~= '') then
			key = ' @{T25}@{2F2F}(' .. key .. ')';
			text = concatUCString(text, key);
		end

		-- if we don't do the full getUI, it doesn't work (don't understand why)
		local	uiQC= getUI("ui:interface:user_chat_emote_menu:quick_chat:" .. "qc" .. i);
		uiQC.uc_hardtext_format= text;
	end
	
end

------------------------------------------------------------------------------------------------------------
-- 
if (ui_free_chat_h == nil) then
	ui_free_chat_h = {}
end

if (ui_free_chat_w == nil) then
	ui_free_chat_w = {}
end

------------------------------------------------------------------------------------------------------------
-- 
function game:closeTellHeader(uiID)
	local ui = getUI('ui:interface:' .. uiID);
	
	-- save size
	ui_free_chat_h[uiID] = ui.h;
	ui_free_chat_w[uiID] = ui.w;
	
	-- reduce window size
	ui.pop_min_h = 32;
	ui.h = 0;
	ui.w = 216;
end

------------------------------------------------------------------------------------------------------------
-- 
function game:openTellHeader(uiID)
	local ui = getUI('ui:interface:' .. uiID);
	ui.pop_min_h = 96;

	-- set size from saved values
	if (ui_free_chat_h[uiID] ~= nil) then
		ui.h = ui_free_chat_h[uiID];
	end
	
	 if (ui_free_chat_w[uiID] ~= nil) then
		ui.w = ui_free_chat_w[uiID];
	end

	-- set Header Color to normal values (when a tell is closed and the telled player say someone, header change to "UI:SAVE:WIN:COLORS:INFOS")
	ui:setHeaderColor('UI:SAVE:WIN:COLORS:COM');
end


--/////////////////////////
--// TARGET WINDOW SETUP //
--/////////////////////////

-- local functions for tests
local function levelToForceRegion(level)
	if level < 20 then
		return 1
	elseif level >= 250 then
		return 6
	else
		return math.floor(level / 50) + 2
	end	
end 

local function levelToLevelForce(level)
	return math.floor(math.fmod(level, 50) * 5 / 50) + 1	
end 



-- tmp var for tests in local mode
local twPlayerLevel = 10
local twTargetLevel = 19
local twTargetForceRegion = levelToForceRegion(twTargetLevel)
local twTargetLevelForce  = levelToLevelForce(twTargetLevel)
local twTargetPlayer = false
local twPlayerInPVPMode = false
local twTargetInPVPMode = false


-----------------------------------
local function twGetPlayerLevel()
	if config.Local == 1 then
		return twPlayerLevel
	else
		return getPlayerLevel()
	end
end

-----------------------------------
local function twGetTargetLevel()
	if config.Local == 1 then
		return twTargetLevel
	else
		return getTargetLevel()
	end
end

-----------------------------------
local function twGetTargetForceRegion()
	if config.Local == 1 then
		return twTargetForceRegion
	else
		return getTargetForceRegion()
	end
end

-----------------------------------
local function twGetTargetLevelForce()
	if config.Local == 1 then
		return twTargetLevelForce
	else
		return getTargetLevelForce()
	end
end

-----------------------------------
local function twIsTargetPlayer()
	if config.Local == 1 then
		return 	twTargetPlayer
	else 
		return isTargetPlayer()
	end
end

-----------------------------------
local function twIsPlayerInPVPMode()
	if config.Local == 1 then
		return 	twPlayerInPVPMode
	else 
		return isPlayerInPVPMode()
	end
end

-----------------------------------
local function twIsTargetInPVPMode()
	if config.Local == 1 then
		return 	twTargetInPVPMode
	else 
		return isTargetInPVPMode()
	end
end



------------------------------------------------------------------------------------------------------------
-- This function is called when a new target is selected, it should update the 'consider' widget
-- Level of the creature
-- Is its level known (not too high ...)
-- Boss/Mini-bosses/Names colored ring
function game:updateTargetConsiderUI()
	--debugInfo("Updating consider widget")


	local targetWindow = getUI("ui:interface:target")	
	-- 	
	local wgTargetSlotForce = targetWindow:find("slot_force")
	local wgTargetLevel = targetWindow:find("target_level")
	local wgImpossible  = targetWindow:find("impossible")
	local wgSlotRing    = targetWindow:find("slot_ring")
	local wgToolTip     = targetWindow:find("target_tooltip")
	local wgPvPTag     = targetWindow:find("pvp_tags")
	local wgHeader     = targetWindow:find("header_opened")
		
	wgTargetSlotForce.active = true
	wgImpossible.active = true

	-- no selection ?
	if twGetTargetLevel() == -1 then
		wgTargetSlotForce.active = false
		wgTargetLevel.active = false
		wgImpossible.active = false
		wgSlotRing.active  = false	
		if (isTargetUser() and twIsPlayerInPVPMode()) then
			wgToolTip.tooltip = ""
			wgPvPTag.active = true
			wgHeader.h = 56;
		else
			wgPvPTag.active = false
			wgHeader.h = 34;
			wgToolTip.tooltip = i18n.get("uittConsiderTargetNoSelection")
		end
		return
	end

	local pvpMode = false
	wgPvPTag.active = false
	wgHeader.h = 34;

	-- if the selection is a player, then both the local & targeted player must be in PVP mode for the level to be displayed
	if (twIsTargetPlayer()) then
		-- don't display anything ...
		wgTargetSlotForce.active = false
		wgTargetLevel.active = false
		wgImpossible.active = false
		wgSlotRing.active  = false
		wgToolTip.tooltip = ""
		if twIsTargetInPVPMode() then
			debugInfo("target in pvp")
			wgPvPTag.active = true
			wgHeader.h = 56;
		end
		return
	end

	-- depending on the number of people in the group, set the max diff for visibility between player level
	-- & creature level (x 10 per member)
	local maxDiffLevel = 10
	if not pvpMode then
		-- exception there : when "pvping", don't relate the levelof the target to the level of the group, but to thelocal
		-- player only
		for gm = 0, 7 do
			if getDbProp("SERVER:GROUP:" .. tostring(gm) .. ":PRESENT") ~= 0 then
				maxDiffLevel = maxDiffLevel + 10
			end
		end
	end

	--debugInfo("Max diff level= " .. tostring(maxDiffLevel))

	local impossible = (twGetTargetLevel() - twGetPlayerLevel() > maxDiffLevel)

	wgSlotRing.active = false
	
	if impossible then
		-- targeted object is too hard too beat, display a skull
		wgTargetLevel.active = false
		wgImpossible.y = -5
		wgImpossible.color = "255 50 50 255"
	else
		-- player can see the level of the targeted creature		
		wgTargetLevel.active = true
		wgImpossible.y = 6
		wgTargetLevel.hardtext = tostring(twGetTargetLevel())
		wgImpossible.color = "255 255 255 255"
		wgTargetLevel.color = getDefine("region_force_" .. tostring(levelToForceRegion(twGetTargetLevel())))
	end

	-- based on the 'level force', set a colored ring around the level
	local levelForce = twGetTargetLevelForce()
	wgTargetSlotForce.color = getDefine("region_force_" .. tostring(levelToForceRegion(twGetTargetLevel())))

	wgImpossible.texture = getDefine("force_level_" .. tostring(levelForce))
	wgImpossible.active = true
	if levelForce < 6 then 
		wgToolTip.tooltip = i18n.get("uittConsiderTargetLevel")
	elseif levelForce == 6 then
		-- Named creature
		wgImpossible.color = "117 132 126 255"
		wgSlotRing.color = "117 132 126 255"
		wgTargetSlotForce.color = "117 132 126 255"
		wgSlotRing.texture = "consider_ring.tga"
		wgToolTip.tooltip = i18n.get("uittConsiderNamedOrMiniBoss")
	elseif levelForce == 7 then
		-- Boss
		wgImpossible.color = "156 98 65 255"
		wgSlotRing.color = "156 98 65 255"
		wgTargetSlotForce.color = "156 98 65 255"
		wgSlotRing.texture = "consider_ring.tga"
		wgToolTip.tooltip = i18n.get("uittConsiderNamedOrMiniBoss")
	elseif levelForce == 8 then
		-- Mini-Boss
		wgImpossible.color = "213 212 9 255"
		wgSlotRing.texture = "consider_ring.tga"
		wgSlotRing.color = "213 212 9 255"
		if isTargetNPC() then
			wgTargetSlotForce.color = "255 255 255 255"
			wgToolTip.tooltip = i18n.get("uittConsiderBossNpc")
		else
			wgTargetSlotForce.color = "213 212 9 255"
			wgToolTip.tooltip = i18n.get("uittConsiderBoss")
		end
	end

	if impossible then
		wgToolTip.tooltip = concatUCString(wgToolTip.tooltip, ucstring("\n"), i18n.get("uittConsiderUnknownLevel"))
	end

end

----------------------
-- MISC local tests function
-- no selection
function twTest0()
	twTargetLevel = -1
	twTargetPlayer = false
	game:updateTargetConsiderUI()
end
-- selection, not impossible
function twTest1()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 15
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = levelToLevelForce(twTargetLevel)
	game:updateTargetConsiderUI()
end
-- selection, not impossible (limit)
function twTest2()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 20
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = levelToLevelForce(twTargetLevel)
	game:updateTargetConsiderUI()
end
-- selection, impossible
function twTest3()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 21
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = levelToLevelForce(twTargetLevel)
	game:updateTargetConsiderUI()
end
------ NAMED
------
-- selection, not impossible, named
function twTest4()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 15
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 6
	game:updateTargetConsiderUI()
end
-- selection, not impossible (limit), named
function twTest5()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 20
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 6
	game:updateTargetConsiderUI()
end
-- selection, impossible, named
function twTest6()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 21
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 6
	game:updateTargetConsiderUI()
end
------ BOSS
------
-- selection, not impossible, boss
function twTest7()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 15
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 7
	game:updateTargetConsiderUI()
end
-- selection, not impossible (limit), boss
function twTest8()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 20
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 7
	game:updateTargetConsiderUI()
end
-- selection, impossible, boss
function twTest9()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 21
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 7
	game:updateTargetConsiderUI()
end
------ MINI-BOSS
------
-- selection, not impossible, boss
function twTest10()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 15
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 8
	game:updateTargetConsiderUI()
end
-- selection, not impossible (limit), boss
function twTest11()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 20
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 8
	game:updateTargetConsiderUI()
end
-- selection, impossible, boss
function twTest12()
	twTargetPlayer = false
	twPlayerLevel = 10
	twTargetLevel = 21
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = 8
	game:updateTargetConsiderUI()
end

------ PLAYER SELECTION
------ 2 players, no pvp
function twTest13()
	twTargetPlayer = true
	twPlayerInPVPMode = false
	twTargetInPVPMode = false

	twPlayerLevel = 10
	twTargetLevel = 15
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = levelToLevelForce(twTargetLevel)
	game:updateTargetConsiderUI()
end

function twTest14()
	twTargetPlayer = true
	twPlayerInPVPMode = true
	twTargetInPVPMode = true

	twPlayerLevel = 10
	twTargetLevel = 15
	twTargetForceRegion = levelToForceRegion(twTargetLevel)
	twTargetLevelForce  = levelToLevelForce(twTargetLevel)
	game:updateTargetConsiderUI()
end


------ 2 players, pvp

-- groups
function twGroup(groupSize)
	for gm = 0, 7 do
		if gm < groupSize then
			setDbProp("SERVER:GROUP:" .. tostring(gm) .. ":PRESENT", 1)
		else
			setDbProp("SERVER:GROUP:" .. tostring(gm) .. ":PRESENT", 0)
		end
	end	
end

------------------------------------------------------------------------------------------------------------
-- 
function game:closeWebIGBrowserHeader()
	local ui = getUI('ui:interface:webig');
	
	-- save size
	ui_webig_browser_h = ui.h;
	ui_webig_browser_w = ui.w;
	
	-- reduce window size
	ui.pop_min_h = 32;
	ui.h = 0;
	ui.w = 150;
end

------------------------------------------------------------------------------------------------------------
-- 
function game:openWebIGBrowserHeader()
	local ui = getUI('ui:interface:webig');
	ui.pop_min_h = 96;

	-- set size from saved values
	if (ui_webig_browser_h ~= nil) then
		ui.h = ui_webig_browser_h;
	end
	
	 if (ui_webig_browser_w ~= nil) then
		ui.w = ui_webig_browser_w;
	end
end
