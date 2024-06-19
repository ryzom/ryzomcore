-- In this file we define functions that serves for interaction windows


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game == nil) then
	game={}
end

if (game.ui_props == nil) then
	game.ui_props = {}
end

if arkNpcShop == nil then
	arkNpcShop = {}
end

------------------------------------------------------------------------------------------------------------
--
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

------------------------------------------------------------------------------------------------------------
-- called when server send an invitaion we receive a text id containing the string to display (invitor name)
function game:onTeamInvation(textID)

	local ui = getUI('ui:interface:join_team_proposal')
	ui.content.inside.invitor_name.textid = textID
	ui.active = true
	setTopWindow(ui)
	ui:center()
	ui:blink(2)
end

------------------------------------------------------------------------------------------------------------
--
function game:teamInvitationAccept()

	local ui = getUI('ui:interface:join_team_proposal')
	ui.active = false
	sendMsgToServer('TEAM:JOIN')
end

------------------------------------------------------------------------------------------------------------
--
function game:teamInvitationRefuse()

	local ui = getUI('ui:interface:join_team_proposal')
	ui.active = false
	sendMsgToServer('TEAM:JOIN_PROPOSAL_DECLINE')
end

------------------------------------------------------------------------------------------------------------
-- send team invite from friendslist
function game:teamInvite(uiID)
	runAH(nil, 'talk', 'mode=0|text=/invite '.. getUI('ui:interface:' .. uiID).title)
end

------------------------------------------------------------------------------------------------------------
-- send team invite from guildwindow
function game:teamInviteFromGuild(uiID)
	runAH(nil, 'talk', 'mode=0|text=/invite ' .. getGuildMemberName(tonumber(uiID:split(":m")[2])))
end

------------------------------------------------------------------------------------------------------------
--Send Guild invite from guildwindow
function game:invToGuild()
	player = getUI('ui:interface:add_guild'):find('edit_text').hardtext:split(">")[2]
	if(player ~= '')then
		runAH(nil, 'talk', 'mode=0|text=/guildinvite ' .. player)
	end
	runAH(nil, 'leave_modal', '')
end

------------------------------------------------------------------------------------------------------------
--Check and active invite to guild button
function game:updateGLinvB()
	if(getUI('ui:interface:guild').active)then
		for v = 0, (getNbGuildMembers()-1) do
			local invB = getUI('ui:interface:guild:content:tab_guild_info:invite')
			if(getPlayerName() == getGuildMemberName(v))then
				--debugInfo(getGuildMemberName(v))
				if(getGuildMemberGrade(v) ~= 'Member')then
					if(invB.active == false)then
						invB.active = true
					end
				else
					invB.active = false
				end
			end
		end
	end
end

------------------------------------------------------------------------------------------------------------
--
function game:switchChatTab(dbEntry)
	local	db= 'UI:SAVE:ISENABLED:' .. dbEntry
	local	val= getDbProp(db)
	-- switch value
	if(val==0)	then
		setDbProp(db, 1)
	else
		setDbProp(db, 0)
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


game.updateRpItemsUrl = nil
game.wantedRpLeftItem = ""
game.wantedRpRightItem = ""
game.wantedRpTargets = {}
game.wantedRpPositions = {}

function game:addRequireRpItemsPosition(x, y, id)
	table.insert(game.wantedRpPositions, {x, y, id})
end

function game:addRequireRpItems(left, target, mode, id)
	game.wantedRpTargets[left..":"..target..":"..mode..":"..id] = id
end

game.usedRpLeftItem  = "_"
game.usedRpRightItem  = "_"

function game:updateRpItems()
	local left = getPlayerTag(6)
	local right = getPlayerTag(5)

	if getDbProp("LOCAL:INVENTORY:HAND:1:INDEX_IN_BAG") ~= 0 then
		left = "_"
	end

	if getDbProp("LOCAL:INVENTORY:HAND:0:INDEX_IN_BAG") ~= 0 then
		right = "_"
	end

	local right_no_variant = right
	for str in string.gmatch(right, "([a-zA-Z_.]*)[|0-9]*") do
		right_no_variant = str
	end

	if game.updateRpItemsUrl then
		if game.usedRpLeftItem ~= left or game.usedRpRightItem ~= right then
			game.usedRpLeftItem = left
			game.usedRpRightItem = right
			webig:openUrl(game.updateRpItemsUrl.."&left_hand="..left.."&right_hand="..right)
		end

		local target = tostring(getTargetSheet())
		local mode = ""
		if target ~= "" then
			mode = tostring(getTargetMode())
		end

		-- game:checkRpItemsPosition()
		local html = getUI("ui:interface:rpitems_actions"):find("html")
		for k, v in pairs(game.wantedRpTargets) do
			local a = html:find("action"..v)
			if a then
				if string.find(left..":"..target..":"..mode..":"..tostring(v), k) ~= nil
				    or string.find(left..":::"..tostring(v), k) ~= nil
				    or string.find(left..":"..target.."::"..tostring(v), k) ~= nil
    				or string.find(left..":"..target..":*:"..tostring(v), k) ~= nil
    				or string.find(right..":"..target..":"..mode..":"..tostring(v), k) ~= nil
    				or string.find(right..":::"..tostring(v), k) ~= nil
    				or string.find(right..":"..target.."::"..tostring(v), k) ~= nil
    				or string.find(right..":"..target..":*:"..tostring(v), k) ~= nil
    				or string.find(right_no_variant..":"..target..":"..mode..":"..tostring(v), k) ~= nil
    				or string.find(right_no_variant..":::"..tostring(v), k) ~= nil
    				or string.find(right_no_variant..":"..target.."::"..tostring(v), k) ~= nil
    				or string.find(right_no_variant..":"..target..":*:"..tostring(v), k) ~= nil
				then
					a:find("img").texture = "grey_0.tga"
					a:find("but").onclick_l = "lua"
					a:find("but").alpha = 255
					a:find("text").alpha = 255
				else
					a:find("img").texture = "r2ed_toolbar_lock_small.tga"
					a:find("but").onclick_l = ""
					a:find("but").alpha = 150
					a:find("text").alpha = 100
				end
			end
		end
	end
end

function game:checkRpItemsPosition()
	local x,y,z = getPlayerPos()
	local html = getUI("ui:interface:rpitems_actions"):find("html")
	for _, v in pairs(game.wantedRpPositions) do
		vx = v[1]
		vy = v[2]
		id = v[3]
		local a = html:find("action"..id)
		if a then
			if (vx-x)*(vx-x) + (vy-y)*(vy-y) <= 50 then
				a:find("but").onclick_l = "lua"
				a:find("img").texture = "grey_0.tga"
				a:find("but").alpha = 255
				a:find("text").alpha = 255
			else
				a:find("but").onclick_l = ""
				a:find("img").texture = "r2ed_toolbar_lock_small.tga"
				a:find("but").alpha = 150
				a:find("text").alpha = 100
			end
		end
	end
end

------------------------------------------------------------------------------------------------------------
-- This function is called when a new target is selected, it should update the 'consider' widget
-- Level of the creature
-- Is its level known (not too high ...)
-- Boss/Mini-bosses/Names colored ring

function game:updateTargetConsiderUI()
	game:updateRpItems(game.usedRpLeftItem, game.usedRpRightItem)
	local targetWindow = getUI("ui:interface:target")
	--
	local wgTargetSlotForce = targetWindow:find("slot_force")
	local wgTargetLevel = targetWindow:find("target_level")
	local wgImpossible  = targetWindow:find("impossible")
	local wgSlotRing    = targetWindow:find("slot_ring")
	local wgToolTip     = targetWindow:find("target_tooltip")
	local wgPvPTag      = targetWindow:find("pvp_tags")
	local wgHeader      = targetWindow:find("header_opened")
	local wgLock        = targetWindow:find("lock")
	local wginfos       = targetWindow:find("target_tinfos")

	wgTargetSlotForce.active = true
	wgTargetSlotForce.texture = "consider_bg.tga"
	wgImpossible.active = true
	wgTargetSlotForce.h = 16

	wginfos.active = false

	-- no selection ?
	if twGetTargetLevel() == -1 then
		wgLock.active = false
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


-- /luaScript getUI("ui:interface:target:header_opened:lock").active=true

	-- if the selection is a player, then both the local & targeted player must be in PVP mode for the level to be displayed
	if (twIsTargetPlayer()) then
		-- don't display anything except infos ...
		wginfos.active = true
		wgLock.active = false
		wgTargetSlotForce.active = false
		wgTargetLevel.active = false
		wgImpossible.active = false
		wgSlotRing.active  = false
		wgToolTip.tooltip = ""
		if twIsTargetInPVPMode() then
			wgPvPTag.active = true
			wgHeader.h = 56;
		end
		return
	else
		wgLock.active = false
		local level = getDbProp(getDefine("target_player_level"))

		if level == 2 then -- Locked by team of player
			wgLock.active = true
			wgLock.color = "50 250 250 255"
		else
			if level == 1 then -- Locked by another team
				wgLock.active = true
				wgLock.color = "250 50 50 255"
			end
		end
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
	elseif levelForce == 12 then
		wgImpossible.color = "255 255 255 255"
		wgTargetSlotForce.texture = "episode_step.tga"
		wgTargetSlotForce.color = "255 255 255 255"
		wgToolTip.tooltip = i18n.get("uittConsiderUnknownLevel")
		wgTargetLevel.active = false
		wgImpossible.y = -5
	elseif levelForce > 12 then
		wgImpossible.color = "255 255 255 255"
		wgTargetSlotForce.texture = "episode_step_ok.tga"
		wgTargetSlotForce.color = "255 255 255 255"
		wgToolTip.tooltip = i18n.get("uittConsiderSpecialLevel")
		wgTargetLevel.active = false
		wgImpossible.y = -5
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
	local ui = getUI('ui:interface:webig')

	-- save size
	ui_webig_browser_h = ui.h
	ui_webig_browser_w = ui.w

	-- reduce window size
	ui.pop_min_h = 32
	ui.h = 0
	ui.w = 150
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

------------------------------------------------------------------------------------------------------------
--
function game:closeWindowHeader()
	local ui = getUICaller().parent;
	local id = ui.id;

	if game.ui_props[id] == nil then
		game.ui_props[id] = {}
	end

	-- save size
	game.ui_props[id].w = ui.w
	game.ui_props[id].h = ui.h
	game.ui_props[id].pop_min_h = ui.pop_min_h

	-- reduce window size
	ui.pop_min_h = 32
	ui.h = 0;
	ui.w = 150
end

------------------------------------------------------------------------------------------------------------
--
function game:openWindowHeader()
	local ui = getUICaller().parent;
	local id = ui.id;

	if ui ~= nil and game.ui_props[id] ~= nil then
		-- set size from saved values
		if game.ui_props[id].pop_min_h ~= nil then
			ui.pop_min_h = game.ui_props[id].pop_min_h
		end

		if game.ui_props[id].h ~= nil then
			ui.h = game.ui_props[id].h
		end

		 if ui_webig_browser_w ~= nil then
			ui.w = game.ui_props[id].w;
		end
	end
end


------------------------------------------------------------------------------------------------------------
function game:openGuildIsland(url_island)
	local nbMember = getNbGuildMembers();
	local params = "";
	for i = 0,(nbMember-1) do
		local memberGrade = getGuildMemberGrade(i);
		if (memberGrade == "Leader") or (memberGrade == "HighOfficer") then
			params = params .. string.lower(getGuildMemberName(i)) .. "=" .. memberGrade.."&";
		end
	end
	local x,y,z = getPlayerPos()
	params = params .. "&posx=" .. tostring(x) .. "&posy=" .. tostring(y) .. "&posz=" .. tostring(z)

	getUI("ui:interface:guild:content:tab_island:props:html"):browse(url_island.."params="..params);
	runAH(nil, "browse_home", "name=ui:interface:guild:content:tab_island:inv:html")
end

function game:openGuildSpecialBag()
	runAH(nil, "browse_home", "name=ui:interface:guild:content:tab_special_bag:inv:html")
end



------------------------------------------------------------------------------------------------------------
local SavedUrl = "";
function game:chatUrl(url)
	SavedUrl = url
	runAH(nil, "active_menu", "menu=ui:interface:chat_uri_action_menu")
end
function game:chatUrlCopy()
	runAH(nil, "copy_to_clipboard", SavedUrl)
end
function game:chatUrlBrowse()
	runAH(nil, "browse", "name=ui:interface:webig:content:html|url=" .. SavedUrl)
end

------------------------------------------------------------------------------------------------------------
--
if game.sDynChat == nil then game.sDynChat = {} end

-- called from onInGameDbInitialized
function game:openChannels()
	if getDbProp("UI:SAVE:CHAT:AUTO_CHANNEL") > 0 then
		local uc = readUserChannels()
		if uc then
			local index = 0
			for _ in pairs(uc) do
				index = index + 1
			end
			local channels = {}
			for i = 0, index-1 do
				local node = uc[tostring(i)]
				channels[tonumber(node.id)] = {
					rgba = node.rgba,
					name = node.name,
					passwd = node.passwd
				}
			end
			local t = {}

			for k in pairs(channels) do table.insert(t, k) end
			table.sort(t)
			-- sorted
			for _, id in ipairs(t) do
				local found = false
				for i = 0, getMaxDynChan()-1 do
					if getDbProp("UI:SAVE:ISENABLED:DYNAMIC_CHAT"..i) == 1 then
						local cname = getDbProp("SERVER:DYN_CHAT:CHANNEL"..i..":NAME")
						if isDynStringAvailable(cname) then
							local chan = getDynString(cname):toUtf8()
							-- already opened?
							if channels[id].name == chan then found = true end
						end
					end
				end
				if not found then
					self:connectUserChannel(channels[id].name.." "..channels[id].passwd)
					-- now restore colors
					if channels[id].rgba ~= '' then
						local i = 0
						local c = {}
						local rgba = {[0]="R", [1]="G", [2]="B", [3]="A"}
						for color in string.gmatch(channels[id].rgba, "%d+") do
							c[rgba[i]] = tonumber(color)
							i = i + 1
						end
						setDbRGBA("UI:SAVE:CHAT:COLORS:DYN:"..id, CRGBA(c.R, c.G, c.B, c.A))
					end
				end
			end
		end
	end
end

-- store channel detail before it open
function game:connectUserChannel(args)
	local argv = {}
	for w in string.gmatch(args, "%S+") do
		table.insert(argv, w)
	end
	if #argv > 0 then
		local params = argv[1]
		if #argv == 2 then
			for _, ch in pairs(self.sDynChat) do
				if ch[argv[1]] then ch[argv[1]] = nil end
			end
			if argv[2] ~= '*' and argv[2] ~= '***' then
				table.insert(self.sDynChat, {[argv[1]]=argv[2]})
			end
			params = params.." "..argv[2]
		end
		runAH(nil, "talk", "mode=0|text=/a connectUserChannel "..params)
	end
end

-- save user created channels
function game:saveChannel(verbose)
	if verbose == nil then
		verbose = false
	end
	local channels = {}
	for i = 0, getMaxDynChan()-1 do
		if getDbProp("UI:SAVE:ISENABLED:DYNAMIC_CHAT"..i) == 1 then
			local cname = getDbProp("SERVER:DYN_CHAT:CHANNEL"..i..":NAME")
			if isDynStringAvailable(cname) then
				local chan = getDynString(cname):toUtf8()
				local found = false
				-- avoid empty cvar case
				if getClientCfgVar("ChannelIgnoreFilter") then
					for _, k in pairs(getClientCfgVar("ChannelIgnoreFilter")) do
						if k == chan then found = true end
					end
					if not found then
						-- store current colors
						local cRGBA = getDbRGBA("UI:SAVE:CHAT:COLORS:DYN:"..i)
						local password = ''
						-- include private channels
						for _, k in pairs(game.sDynChat) do
							if k[chan] then password = k[chan] end
						end
						channels[tostring(i)] = {
						    rgba = cRGBA,
						    name = chan,
						    passwd = password
						}
					end
				end
			end
		end
	end
	saveUserChannels(channels, verbose)
end

------------------------------------------------------------------------------------------------------------
--
function game:chatWelcomeMsg(input)
	local msg
	local name
	if not input then
		input = getUICaller().params_r
		if input then
			input = input:match("ED:([^_]+)"):lower()
	    end
	end
	local chat = input
	local temp = "UI:TEMP:ONCHAT:"
	if game.InGameDbInitialized then
		-- is input chat a dynamic channel?
		if type(input) == "number" then
			local id = getDbProp("SERVER:DYN_CHAT:CHANNEL"..input..":NAME")
			if isDynStringAvailable(id) then
				name = getDynString(id):toUtf8()
				-- variable for this session
				if getDbProp(temp..name) == 0 then
					-- faction, nation and organization
					for k, v in pairs({
						kami = i18n.get("uiFameAllegiance2"),
						karavan = i18n.get("uiFameAllegiance3"),
						fyros = i18n.get("uiFameAllegiance4"),
						matis = i18n.get("uiFameAllegiance5"),
						tryker = i18n.get("uiFameAllegiance6"),
						zorai = i18n.get("uiFameAllegiance7"),
						marauder = i18n.get("uiFameMarauders"),
						ranger = i18n.get("uiOrganization_7")
					}) do
						if name == v:toUtf8() then
							msg = i18n.get("uiWelcome_"..k)
							name = v:toUtf8()
						end
					end
				end
				-- chat_group_filter sParam
				chat = "dyn_chat"..input
			end
		else
			-- around, region and universe
			if getDbProp(temp..input) == 0 then
				msg = i18n.get("uiWelcome_"..input)
				name = input
			end
		end
		if msg then
			displayChatMessage(tostring(msg), input)
			-- save for this session
			addDbProp(temp..name, 1)
		end
	end
	runAH(getUICaller(), "chat_group_filter", chat)
end


function game:TalkWithNpc(bullying)
	setTargetAsInterlocutor()

	if bullying == 1 then
		runCommand("a", "openTargetUrl", "1")
	else
		runCommand("a", "openTargetUrl")
	end
end

-- VERSION --
RYZOM_INTERACTION_VERSION = 335
