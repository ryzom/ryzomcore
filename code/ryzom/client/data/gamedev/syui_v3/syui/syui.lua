-- written by Syphox
local M = {}

function M.exist()
    return true
end

local bit32 = {}
local logic_and = {
[0] = { [0] = 0, 0, 0, 0},
[1] = { [0] = 0, 1, 0, 1},
[2] = { [0] = 0, 0, 2, 2},
[3] = { [0] = 0, 1, 2, 3},
}

local function checkint32( name, argidx, x, level )
    local n = tonumber( x )
    if not n then
        error( string.format(
               "bad argument #%d to '%s' (number expected, got %s)",
               argidx, name, type( x )
              ), level + 1 )
    end
    return math.floor( n ) % 0x100000000
end

local function comb( name, args, nargs, s, t )
    for i = 1, nargs do
        args[i] = checkint32( name, i, args[i], 3 )
    end
    local pow = 1
    local ret = 0
    for b = 0, 31, 2 do
        local c = s
        for i = 1, nargs do
            c = t[c][args[i] % 4]
            args[i] = math.floor( args[i] / 4 )
        end
        ret = ret + c * pow
        pow = pow * 4
    end
    return ret
end
function bit32.btest( ... )
    return comb( 'btest', { ... }, select( '#', ... ), 3, logic_and ) ~= 0
end


local function targetIsInSameTeam()
    if(getDbProp('UI:VARIABLES:IS_TEAM_PRESENT')~=0)then
	    for i=0,7 do
	        local groupEntityUID = getDbProp('SERVER:GROUP:' .. tostring(i) ..':UID')
		    if(groupEntityUID == getDbProp('UI:VARIABLES:TARGET:UID'))then
				return true
			end
	    end
		return false
	end
end

local function targetIsInSameGuild()
    if(getDbProp('SERVER:GUILD:NAME')~=0)then
        local nbMember = getNbGuildMembers();
	    for i=0,(nbMember-1) do
		    if(getGuildMemberName(i) == getTargetName())then
                return true
		    end
        end
		return false
	end
end

local function targetIsInSameLeague()
    local targetLeague = getDbProp('SERVER:Entities:E' .. getTargetSlot() .. ':P25')
	local playerLeague = getDbProp('SERVER:Entities:E0:P25')
	if(targetLeague == playerLeague and playerLeague ~= 0)then
	    return true
	end
	return false
end

-- no way found yet
local function targetIsInSameOpFight()

end

-- 1=duel, 2=unk, 3=arena, 4=unk, 5=gvg (pr), 6=unk, 7=tagged(mara), 8=unk, 9=tp safezone, 10=safe zone related 
function M.checkPvPMode()
    local targetProp = getDbProp('SERVER:Entities:E' .. getTargetSlot() .. ':P23')
	local playerProp = getDbProp('SERVER:Entities:E0:P23')
	local pvp_mode = {1,3,5,7}
	for i=1,8 do
        if(bit32.btest(targetProp, 2^(i-1)) and bit32.btest(playerProp, 2^(i-1)))then
		    return true
		end
	end
	return false
end

function M.PvPLogo()
    local targetProp = getDbProp('SERVER:Entities:E' .. getTargetSlot() .. ':P23')
	for i=1,8 do
        if(bit32.btest(targetProp, 2^(i-1)))then
		    return true
		end
	end
	return false
end

function M.isEnemy()
    if(isTargetPlayer() and M.checkPvPMode())then
	    if(targetIsInSameGuild())then
		    return false
		end
		if(targetIsInSameTeam())then
		    return false
		end
		if(targetIsInSameLeague())then
		    return false
		end
		return true
	end
	return false
end

local function TJauge(val)
    local jvalue = getDbProp("UI:VARIABLES:BARS:TARGET:" .. val) / 1.27
	if(jvalue < 0)then
	    jvalue = 0
	end
	setDbProp("UI:VARIABLES:BARS:TARGET:" .. val .. "_PERCENT", math.floor(jvalue))
end

function M.UpdateJauge()
    local bars = {"HP", "SAP", "STA"}
	for k,v in pairs(bars) do
		TJauge(v)
	end
end

function M.teamInvite(uiID)
    runAH(nil, 'talk', 'mode=0|text=/invite '.. getUI('ui:interface:' .. uiID).title)
end

function M.updateFLinvB(uiID)
    if(uiID==nil)then
	    return
	end
    local tUI = getUI('ui:interface:' .. uiID .. ':header_closed:invite_button')
	if(getUI('ui:interface:' .. uiID .. ':header_closed:online').texture ~= 'w_online.tga')then
		if(tUI.texture == 'invt.tga')then
			tUI.texture = ''
		end
	else
		if(tUI.texture == '')then
			tUI.texture = 'invt.tga'
			tUI.x = -52
			tUI.y = 0
		end
    end	
end

function M.invToGuild(ply)
    ply = getUI('ui:interface:add_guild'):find('edit_text').hardtext:split(">")[2]
    if(ply ~= '')then
        runAH(nil, 'talk', 'mode=0|text=/guildinvite ' .. ply)
	end
	runAH(nil, 'leave_modal', '')
end

function M.teamInviteFromGuild(uiID)
	runAH(nil, 'talk', 'mode=0|text=/invite ' .. getGuildMemberName(tonumber(uiID:split(":m")[2])))
end

local tGuild = 'ui:interface:guild:content:tab_guild:list_member:guild_members'
function M.updateGLinvB()
	if(getUI('ui:interface:guild').active)then
	    -- if get #id from member template, it creates too many instances and game will crash. bad coded from ryzom dev
		for v = 0, (getNbGuildMembers()-1) do
		    local uiTexture = getUI(tGuild .. ":" .. tGuild .. ":m" .. v .. ':online')
            local tUI = getUI(tGuild .. ":" .. tGuild .. ":m" .. v .. ':invite_button')
			if(getUI("ui:interface:player").title ~= getUI(tGuild .. ":" .. tGuild .. ":m" .. v .. ":name").hardtext)then
	            if(uiTexture.texture ~= 'w_online.tga')then
		            if(tUI.texture == 'invt.tga')then
			            tUI.texture = ''
		            end
	            else
		            if(tUI.texture == '')then
			            tUI.texture = 'invt.tga'
					    tUI.x = -22
					    tUI.y = 0
					end
		        end
			else
			    -- fix the invite button in guild tab, because it sometimes disappear
				-- the button appear for the player with higher grade than member
	            local invB = getUI('ui:interface:guild:content:tab_guild_i:invite')
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

function M.updateMemberCount()
    -- fix the new guild tab
    local mcount = getUI('ui:interface:guild:content:tab_guild_i:member_count')
	if(getUI('ui:interface:guild').active)then
	    if(tonumber(mcount.hardtext) ~= getNbGuildMembers())then
	        mcount.hardtext = getNbGuildMembers()
		end
	end
end

function M.updateFPS()
    local fpsUI = getUI('ui:interface:compass:frame:fps')
	if(fpsUI==nil)then return end;
	local fps = getDbProp('UI:VARIABLES:FPS')
	fpsUI.hardtext = fps;
	local colRGB = '255 81 81 255'
	if(fps >= 30)then
	    colRGB = '155 255 81 255'
	elseif(fps >= 20)then
	    colRGB = '249 255 81 255'
	end
	fpsUI.color = colRGB
end

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

function M.sysinfo(txt, mtd)
    if(mtd==nil)then mtd='SYS' end;
    displaySystemInfo(ucstring(tostring(txt)), mtd);
end

function M.showTargetPercent()
	local targetUI = getUI('ui:interface:target')
	local playerUI = getUI('ui:interface:player')
	local contentUI = targetUI:find("content")
	local clawImg = targetUI:find("slot_claw")
	local targetTitle = targetUI:find("target_title")
	local playerTitle = playerUI:find("player_title")
	local wgTargetConside = targetUI:find("conside")
	local wgTargetLevel = targetUI:find("target_level")
	targetTitle.color = "255 255 255 255"
	playerTitle.color = targetTitle.color
	
	if (isTargetUser() or 
	   (isTargetPlayer() and
	    not M.isEnemy()))then
	    wgTargetConside.active = false
		contentUI.h = 55
		contentUI.y = -24
		clawImg.active = false
	else
	    if(not isTargetPlayer() and 
		   not M.checkPvPMode() and 
		   getTargetLevel() > 0)then
	        if(clawImg.texture ~= "claw.tga")then
		        clawImg.texture = "claw.tga"
			    clawImg.x = 7
                clawImg.y = -6
			end
		end
		if(targetUI.title == "")then
	        wgTargetConside.active = false
		    clawImg.active = false
		end
		contentUI.h = 18
	    contentUI.y = -26
	end
	
	if(getTargetLevelForce() > 0 and 
	   not isTargetPlayer())then
	    if(getTargetLevel() > 0)then
	        --fix campfire, do not show claw
			if(tostring(getTargetSheet())~="object_campfire_28_b.creature")then
			    wgTargetConside.active = true
		        clawImg.active = true
    	    end
		end
	end
	
	if(getTargetLevel() <= 0 or isTargetNPC())then
	    wgTargetConside.active = false
		wgTargetLevel.active = false
		wgTargetLevel.hardtext = ""
		clawImg.active = false
		wgTargetConside.texture = "blank_n.tga"
	end
	
	if((isTargetUser() or isTargetPlayer()) and M.PvPLogo())then
	    if(clawImg.texture ~= "pvp.tga")then
		    clawImg.texture = "pvp.tga"
			clawImg.x = 0
            clawImg.y = 5
		end
		clawImg.active = true
		wgTargetConside.active = false
	end
end

function M.newConsider()
    local targetWindow = getUI("ui:interface:target")
	local clawImg = targetWindow:find("slot_claw")
    local targetTitle = targetWindow:find("target_title")	
	local wgTargetLevel = targetWindow:find("target_level")
	local wgTargetConside = targetWindow:find("conside")
	local wgImpossible  = targetWindow:find("impossible")
	local wgSlotRing    = targetWindow:find("slot_ring")
	local wgToolTip     = targetWindow:find("target_tooltip")
    local pvpMode = false

	local maxDiffLevel = 10
	if not pvpMode then
		for gm = 0, 7 do
			if getDbProp("SERVER:GROUP:" .. tostring(gm) .. ":PRESENT") ~= 0 then
				maxDiffLevel = maxDiffLevel + 10
			end
		end
	end

	local impossible = (getTargetLevel() - getPlayerLevel() > maxDiffLevel)

	wgSlotRing.active = false
	wgTargetConside.active = false
	wgImpossible.active = false
	wgTargetConside.texture = "blank_n.tga"
	
	if impossible then
		-- targeted object is too hard too beat, display a skull
		wgTargetLevel.active = false
		wgTargetConside.active = false
		wgImpossible.texture = "skull_imp.tga"
		clawImg.active = false
		wgImpossible.active = true
		wgImpossible.color = "255 255 255 255"
		--wgImpossible.color = "255 50 50 255"
		wgTargetLevel.hardtext = ""
	else
	    --fix campfire, do not show, claw, lvl and consider
	    if(tostring(getTargetSheet())=="object_campfire_28_b.creature")then
	        wgTargetConside.active = false
		    wgTargetLevel.active = false
		    wgTargetLevel.hardtext = ""
		    clawImg.active = false
		    wgTargetConside.texture = "blank_n.tga"
			return
	    end
		-- player can see the level of the targeted creature, but not from NPC's
        if(not isTargetNPC())then		
		    wgTargetLevel.active = true
		    wgTargetConside.active = true
		    wgImpossible.active = false
		    wgImpossible.texture = "blank_n.tga"

		    wgTargetLevel.hardtext = tostring(getTargetLevel())
		    wgTargetLevel.color = "255 255 255 255"
		    wgImpossible.color = "255 255 255 255"
	    end
		
		local image={ 'b1', 'b2', 'b3', 'b4', 'b5', 'g1', 'g2', 'g3', 'g4', 'g5', 'ge1', 'ge2', 'ge3', 'ge4', 'ge5', 'r1', 'r2', 'r3', 'r4', 'r5', 'l1', 'l2', 'l3', 'l4', 'l5', 'l5', 'l5' }
		
		if(getTargetLevel()<10)then
		    wgTargetConside.texture = 'consider_gr.tga'
		end
		
		for k,v in pairs(image) do
		    if(getTargetLevel()>=tonumber(k .. 0))then
		        wgTargetConside.texture = 'consider_' .. v .. '.tga'
			end
		end
    end
	
	-- based on the 'level force', set a colored ring around the level
	local levelForce = getTargetLevelForce()

	wgImpossible.active = true
	if levelForce < 6 then 
		wgToolTip.tooltip = i18n.get("uittConsiderTargetLevel")
	elseif levelForce == 6 then
		-- Named creature
		wgImpossible.color = "255 255 255 255"
		--wgImpossible.color = "191 225 254 255"
		wgImpossible.texture = "skull_imp.tga"
		wgImpossible.active = true
		wgToolTip.tooltip = i18n.get("uittConsiderNamedOrMiniBoss")
		wgTargetLevel.hardtext = ""
	elseif levelForce == 7 then
		-- Boss
		wgImpossible.color = "255 255 255 255"
		--wgImpossible.color = "222 191 254 255"
		wgImpossible.texture = "skull_imp.tga"
		wgImpossible.active = true
		wgToolTip.tooltip = i18n.get("uittConsiderNamedOrMiniBoss")
		wgTargetLevel.hardtext = ""
	elseif levelForce == 8 then
		-- Mini-Boss
		wgImpossible.color = "255 255 255 255"
		--wgImpossible.color = "254 191 191 255"
		wgImpossible.texture = "skull_imp.tga"
		wgImpossible.active = true
		wgTargetLevel.hardtext = ""
		wgTargetConside.active = false
		if isTargetNPC() then
			wgToolTip.tooltip = i18n.get("uittConsiderBossNpc")
		else
			wgToolTip.tooltip = i18n.get("uittConsiderBoss")
		end
	end

	if impossible then
		wgToolTip.tooltip = concatUCString(wgToolTip.tooltip, ucstring("\n"), i18n.get("uittConsiderUnknownLevel"))
	end
end

return M