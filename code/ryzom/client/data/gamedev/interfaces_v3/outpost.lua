-- In this file we define functions that serves for Outposts windows


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

-- init The Outpost Object
if (game.Outpost==nil) then
	game.Outpost= {};
	game.Outpost.LastTimeZoneUpdate= 0;
end

-- init constants, from XML defines
if (game.OutpostEnums==nil) then
	game.OutpostEnums= {};
	-- force values to be stored as int
	game.OutpostEnums.UnknownOutpostState= tonumber( getDefine('outpost_status_UnknownOutpostState') );
	game.OutpostEnums.Peace= tonumber( getDefine('outpost_status_Peace') );
	game.OutpostEnums.WarDeclaration= tonumber( getDefine('outpost_status_WarDeclaration') );
	game.OutpostEnums.AttackBefore= tonumber( getDefine('outpost_status_AttackBefore') );
	game.OutpostEnums.AttackRound= tonumber( getDefine('outpost_status_AttackRound') );
	game.OutpostEnums.AttackAfter= tonumber( getDefine('outpost_status_AttackAfter') );
	game.OutpostEnums.DefenseBefore= tonumber( getDefine('outpost_status_DefenseBefore') );
	game.OutpostEnums.DefenseRound= tonumber( getDefine('outpost_status_DefenseRound') );
	game.OutpostEnums.DefenseAfter= tonumber( getDefine('outpost_status_DefenseAfter') );
end


-- ***************************************************************************
-- ***************************************************************************
-- OUTPOST COMMON
-- ***************************************************************************
-- ***************************************************************************


------------------------------------------------------------------------------------------------------------
function game:outpostSetTimeZone(tz)
	setDbProp('UI:SAVE:OUTPOST:TIME_ZONE', tz);
end

------------------------------------------------------------------------------------------------------------
function game:outpostDisplayTimeZone(uiLocal)
	local tz= getDbProp('UI:SAVE:OUTPOST:TIME_ZONE');
	local uiGroup= getUICaller();
	uiGroup[uiLocal].uc_hardtext= 'GMT ' .. string.format('%+d', tz);
end

------------------------------------------------------------------------------------------------------------

function game:outpostAdjustHour(uiLocal, prop)
	local uiGroup = getUICaller();
	local tz = getDbProp('UI:SAVE:OUTPOST:TIME_ZONE');
	local h = runExpr(prop);
	
	-- add time zone and clamp hour
	h = math.fmod(h + tz + 24, 24);
	uiGroup[uiLocal].uc_hardtext = string.format('%02d:00', h);
end

------------------------------------------------------------------------------------------------------------
function game:outpostUpdateTimeZone()
	local curTick = getDbProp('UI:VARIABLES:CURRENT_SERVER_TICK');
	setDbProp('UI:TEMP:OUTPOST:TIME_ZONE_NEXT_UPDATE', curTick + 50);
	game.Outpost.LastTimeZoneUpdate = curTick;
	runAH(nil,'outpost_update_time_zone_auto','');
end

------------------------------------------------------------------------------------------------------------
function game:outpostIsStatusWar(status)
	-- return true only if the status is a War
	if( status == self.OutpostEnums.UnknownOutpostState or
		status == self.OutpostEnums.Peace or
		status == self.OutpostEnums.WarDeclaration )then
		return false;
	else
		return true;
	end
end

------------------------------------------------------------------------------------------------------------
function game:outpostIsStatusWarRound(status)
	-- return true only if the status is an attack period
	if( status == self.OutpostEnums.AttackRound or
		status == self.OutpostEnums.DefenseRound ) then
		return true;
	else
		return false;
	end
end

------------------------------------------------------------------------------------------------------------
function game:outpostBCOpenStateWindow()
	-- Open the State Window from the BotChat. server msg
	runAH(nil, 'outpost_select_from_bc', '');

	-- Open the window
	runAH(nil, 'show', 'outpost_selected');
end

------------------------------------------------------------------------------------------------------------
function game:outpostDeclareWar()
	-- Send Msg to server
	runAH(nil, 'outpost_declare_war_start', '');

	-- wait a ack from server. Suppose not OK by default
	setDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_RECEIVED", 0);
	setDbProp("UI:TEMP:OUTPOST:DECLARE_WAR_ACK_OK", 0);

	-- Open the Declare War window
	runAH(nil, "show", "outpost_declare_war");
end

------------------------------------------------------------------------------------------------------------
function game:outpostDeclareWarValidate()
	-- Send Msg to server
	runAH(nil, 'outpost_declare_war_validate', '');

	-- close the Declare War window
	runAH(nil, "hide", "outpost_declare_war");
end

------------------------------------------------------------------------------------------------------------
-- some constants
game.Outpost.SelectSquadCapitalModal= 'ui:interface:squad_change_capital';
game.Outpost.SelectSquadCapitalEditBox= 'ui:interface:squad_change_capital:edit:eb';


------------------------------------------------------------------------------------------------------------
function game:outpostSelectSquadCapitalOpen()
	-- Retrieve the the edit box.
	local	uiEB= getUI(self.Outpost.SelectSquadCapitalEditBox);
	if(uiEB==nil) then
		return;
	end

	-- Retrieve the value in database
	local dbPath= formatUI('SERVER:GUILD:OUTPOST:O#1', math.max(0, getDbProp('UI:TEMP:OUTPOST:SELECTION')) );
	local val= getDbProp(dbPath .. ':SQUAD_CAPITAL');

	-- Open the modal
	runAH(getUICaller(), "enter_modal", "group=" .. game.Outpost.SelectSquadCapitalModal);

	-- Setup the default input string with it, and set focus
	uiEB.input_string= val;
	runAH(nil, "set_keyboard_focus", "target=" .. self.Outpost.SelectSquadCapitalEditBox .. "|select_all=true");
end

------------------------------------------------------------------------------------------------------------
function game:outpostSelectSquadCapitalConfirm()
	-- Retrieve the value in the edit box.
	local	uiEB= getUI(self.Outpost.SelectSquadCapitalEditBox);
	if(uiEB==nil) then
		return;
	end
	local	val= uiEB.input_string;
	if( tonumber(val) == nil ) then
		return;
	end

	-- Send a msg to server
	runAH(nil, "outpost_select_squad_capital", val);

	-- Close the modal
	runAH(nil, "leave_modal", "");
end

------------------------------------------------------------------------------------------------------------
function game:outpostToolTipTrainSquad(dbIndex)
	local halfMaxSquad= tonumber(getDefine("outpost_nb_max_squad_in_list"));

	-- compute the level at which the squad will spawn. 
	local	lvl;
	if(dbIndex < halfMaxSquad) then
		lvl= dbIndex*2 +1 ;		-- eg: 0 => 1. 1=> 3
	else
		lvl= (dbIndex-halfMaxSquad)*2 +2 ;	-- eg: 12 => 2
	end

	-- set the tooltip
	local text = i18n.get('uittOutpostSquadLvl');
	text = findReplaceAll(text, "%lvl", tostring(lvl));
	setContextHelpText(text);
end


-- ***************************************************************************
-- ***************************************************************************
-- OUTPOST INFO WINDOW
-- ***************************************************************************
-- ***************************************************************************


------------------------------------------------------------------------------------------------------------
function game:outpostInfoGetDbPath(uiGroup)
	if (uiGroup.Env.Type==0) then
		return 'SERVER:OUTPOST_SELECTED';
	else
		-- avoid problems, points to 0 if nothing selected
		return formatUI('SERVER:GUILD:OUTPOST:O#1', math.max(0, getDbProp('UI:TEMP:OUTPOST:SELECTION')) );
	end
end

------------------------------------------------------------------------------------------------------------
function game:outpostInfoOnDraw()
	local uiGroup= getUICaller();
	
	-- get status
	local path= self:outpostInfoGetDbPath(uiGroup);
	local status= getDbProp(path .. ':STATUS');

	-- translate ticks to time
	local curTick= getDbProp('UI:VARIABLES:CURRENT_SERVER_TICK');
	local endTick= getDbProp( path .. ':STATE_END_DATE' );
	-- suppose a server tick is 100 ms
	local	timeSec= (endTick - curTick)/10;
	timeSec= math.max(timeSec, 0);

	-- readable form
	if (status == self.OutpostEnums.Peace) then
		uiGroup.Env.Timer.uc_hardtext= '';
	else
		uiGroup.Env.Timer.uc_hardtext= '(' .. runFct('secondsToTimeString', timeSec) .. ')';
	end
end

------------------------------------------------------------------------------------------------------------
function	game:outpostInfoOnDbChange()
	local uiGroup= getUICaller();

	-- get the selection
	local	path= self:outpostInfoGetDbPath(uiGroup);

	-- change path for icon and textid
	uiGroup.global_state.outpost_owner.guild.blason.sheet= path .. ':GUILD';
	uiGroup.global_state.outpost_owner.guild.name.textid_dblink= path .. ':GUILD:NAME';

	-- change path for attacker text id
	uiGroup.global_state.outpost_attacker.name.textid_dblink= path .. ':GUILD:NAME_ATT';
end

------------------------------------------------------------------------------------------------------------
function game:outpostInitInfo(type)
	local uiGroup= getUICaller();

	-- bkup Timer control
	uiGroup.Env.Timer= uiGroup.global_state.outpost_status_time;
	uiGroup.Env.Type= type;

	-- must change timer each frame
	setOnDraw(uiGroup, 'game:outpostInfoOnDraw()');

	-- For Type==1 (outpost from GuildInfo), must change the DB Path for the GuildIcon, and GuildName TextId
	if(type==1) then
		addOnDbChange(uiGroup,"@UI:TEMP:OUTPOST:SELECTION", 'game:outpostInfoOnDbChange()');
	end
end

------------------------------------------------------------------------------------------------------------
function game:outpostActiveDefenderHourButton()
	-- uiGroup here should be the 'war_schedule' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local path= self:outpostInfoGetDbPath(uiGroup.parent);
	local owned= getDbProp(path .. ':OWNED');
	local admin= getDbProp('SERVER:USER:OUTPOST_ADMIN');
	local status= getDbProp(path .. ':STATUS');

	-- status start with 3 for Actual War (2 for War Declaration). If the user has not the right to change the defense period
	-- if the player has no right to edit
	if( owned==0 or admin==0 or self:outpostIsStatusWar(status) ) then
		uiGroup.outpost_def_hour.frozen= true;
		uiGroup.outpost_def_hour.tooltip= i18n.get('uittOutpostDefHourCannotEdit');
	else
		uiGroup.outpost_def_hour.frozen= false;

		-- get defense hours in 0-23 range
		local timeRangeDef= getDbProp(path .. ':TIME_RANGE_DEF');
		local timeRangeDefWanted= getDbProp(path .. ':TIME_RANGE_DEF_WANTED');
		timeRangeDef= secondsSince1970ToHour( timeRangeDef );
		timeRangeDef= math.fmod(timeRangeDef+24, 24);
		timeRangeDefWanted= math.fmod(timeRangeDefWanted+24, 24);

		-- if time required is the one obtained, or if we are in peace
		if( timeRangeDef == timeRangeDefWanted or status<=game.OutpostEnums.Peace ) then
			uiGroup.outpost_def_hour.tooltip= i18n.get('uittOutpostDefHour');
		else
			uiGroup.outpost_def_hour.tooltip= i18n.get('uittOutpostDefHourError');
		end
	end

	-- Force the tooltip to be up to date (important if dbs are desync)
	disableContextHelpForControl(uiGroup.outpost_def_hour);
end

------------------------------------------------------------------------------------------------------------
function game:outpostActiveAttackerHourButton()
	-- uiGroup here should be the 'war_schedule' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local canEdit= getDbProp('UI:TEMP:OUTPOST:DECLARE_WAR_ACK_OK');

	-- status start with 3 for Actual War (2 for War Declaration). If the user has not the right to change the defense period
	-- if the player has no right to edit
	if( canEdit==0 ) then
		uiGroup.outpost_att_hour.frozen= true;
		uiGroup.outpost_att_hour.tooltip= i18n.get('uittOutpostAttHour');
	else
		uiGroup.outpost_att_hour.frozen= false;

		-- get attack hours in 0-23 range
		local timeRangeAtt= getDbProp('UI:TEMP:OUTPOST:DECLARE_WAR_ACK_TIME_RANGE_ATT');
		local timeRangeAttWanted= getDbProp('UI:TEMP:OUTPOST:DECLARE_WAR_ATTACK_PERIOD');
		timeRangeAtt= secondsSince1970ToHour( timeRangeAtt );
		timeRangeAtt= math.fmod(timeRangeAtt+24, 24);
		timeRangeAttWanted= math.fmod(timeRangeAttWanted+24, 24);

		-- if time required is the one obtained
		if( timeRangeAtt == timeRangeAttWanted ) then
			uiGroup.outpost_att_hour.tooltip= i18n.get('uittOutpostAttHour');
		else
			uiGroup.outpost_att_hour.tooltip= i18n.get('uittOutpostAttHourError');
		end
	end

	-- Force the tooltip to be up to date. important here because 
	-- DECLARE_WAR_ACK_TIME_RANGE_ATT(server) is received after DECLARE_WAR_ATTACK_PERIOD(local)
	disableContextHelpForControl(uiGroup.outpost_att_hour);
end

------------------------------------------------------------------------------------------------------------
function game:outpostIsGuildInvolved()
	local found = 0;
	local sheetCur;
	-- try to get sheet id from bot object
	local sheetSel = getDbProp('SERVER:OUTPOST_SELECTED:SHEET');
	
	if (sheetSel == 0) then
		-- try to get sheet id from outpost manager
		local ind = getDbProp('UI:TEMP:OUTPOST:SELECTION');
		if (ind == -1) then
			return 0;
		end
		sheetSel = getDbProp(formatUI('SERVER:GUILD:OUTPOST:O#1', ind) .. ':SHEET');
	end
	
	-- check every outpost owned or challenged by the guild
	local i;
	for i = 0, 15 do
		sheetCur = getDbProp(formatUI('SERVER:GUILD:OUTPOST:O#1', i) .. ':SHEET');
		if (sheetCur == sheetSel) then
			found = 1;
		end
	end
	
	return found;
	
end

------------------------------------------------------------------------------------------------------------
function game:outpostGetStatusInfo(statusExpr, dbIndex, isTooltip)
	local uiGroup = getUICaller();
	local status = runExpr(statusExpr);
	local uittOutpost;
	local path;
	
	-- get outpost
	if (isTooltip == 'no') then
		path = self:outpostInfoGetDbPath(uiGroup.parent);
	else
		path = formatUI('SERVER:GUILD:OUTPOST:O#1', math.max(0, dbIndex));
	end
	
	-- Peace
	if (status == self.OutpostEnums.Peace) then
		uittOutpost = i18n.get('uittOutpostPeace');
		
	-- WarDeclaration
	elseif (status == self.OutpostEnums.WarDeclaration) then
		if (isTooltip == 'yes' or self:outpostIsGuildInvolved() == 1) then
			local myGuild = getDbProp('SERVER:GUILD:NAME');
			local outpostGuild = getDbProp('SERVER:OUTPOST_SELECTED:GUILD:NAME');
			local outpostTribe = getDbProp('SERVER:OUTPOST_SELECTED:GUILD:TRIBE');
			-- bot object flag : check 'owned' param to determine who are the attacker and defender
			if (outpostTribe == 0 and outpostGuild == 0) then
				local owned = getDbProp(path .. ':OWNED');
				if (owned == 1) then
					uittOutpost = i18n.get('uittOutpostWarDeclarationMyGuildDefend');
				else
					uittOutpost = i18n.get('uittOutpostWarDeclarationMyGuildAttack');
				end
			-- outpost manager : check guild name to determine who are the attacker and defender
			else
				if (myGuild == outpostGuild) then
					uittOutpost = i18n.get('uittOutpostWarDeclarationMyGuildDefend');
				else
					uittOutpost = i18n.get('uittOutpostWarDeclarationMyGuildAttack');
				end
			end
		else
			uittOutpost = i18n.get('uittOutpostWarDeclaration');
		end
		
	-- AttackBefore
	elseif (status == self.OutpostEnums.AttackBefore) then
		uittOutpost = i18n.get('uittOutpostAttackBefore');
	
	-- AttackRound
	elseif (status == self.OutpostEnums.AttackRound) then
		uittOutpost = i18n.get('uittOutpostAttackRound');
		
	-- AttackAfter
	elseif (status == self.OutpostEnums.AttackAfter) then
		local lvlMax = getDbProp(path .. ':ROUND_LVL_THRESHOLD');
		local lvlAtt = getDbProp(path .. ':ROUND_LVL_MAX_ATT');
		if (lvlAtt > lvlMax) then
			uittOutpost = i18n.get('uittOutpostAttackAfterWin');
		else
			uittOutpost = i18n.get('uittOutpostAttackAfterLoose');
		end
		
	-- DefenseBefore
	elseif (status == self.OutpostEnums.DefenseBefore) then
		uittOutpost = i18n.get('uittOutpostDefenseBefore');
	
	-- DefenseRound
	elseif (status == self.OutpostEnums.DefenseRound) then
		uittOutpost = i18n.get('uittOutpostDefenseRound');
		
	-- DefenseAfter
	elseif (status == self.OutpostEnums.DefenseAfter) then
		uittOutpost = i18n.get('uittOutpostDefenseAfter');
		
	-- default
	else
		uittOutpost = i18n.get('uittOutpostPeace');
	end
	
	return uittOutpost;
	
end

------------------------------------------------------------------------------------------------------------
function game:outpostDisplayStatusInfo(statusExpr, id)
	local uiGroup = getUICaller();
	local text = self:outpostGetStatusInfo(statusExpr, -1, 'no');
	uiGroup[id].uc_hardtext_format = text;
end

------------------------------------------------------------------------------------------------------------
function game:outpostDisplayStatusInfoTooltip(statusExpr, dbIndex)
	local text = self:outpostGetStatusInfo(statusExpr, dbIndex, 'yes');
	setContextHelpText(text);
end

------------------------------------------------------------------------------------------------------------
function game:outpostUpdateBuildingSheet(buildingIndex)
	local ui= getUICaller();
	local outpostIndex= getDbProp('UI:TEMP:OUTPOST:SELECTION');
	if (outpostIndex<0) then
		outpostIndex= 0;
	end
	local	db= 'SERVER:GUILD:OUTPOST:O' .. tostring(outpostIndex) .. ':BUILDINGS:' .. tostring(buildingIndex) ;
	ui.building_sheet.sheet= db;
end


-- ***************************************************************************
-- ***************************************************************************
-- OUTPOST INFO: ROUND STATE DISPLAY
-- ***************************************************************************
-- ***************************************************************************


------------------------------------------------------------------------------------------------------------
-- ROUND_LVL_THRESHOLD
function game:outpostChangeRoundLvlThreshold()
	-- uiGroup here should be the 'round_state' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local path= self:outpostInfoGetDbPath(uiGroup.parent);
	local val= getDbProp(path .. ':ROUND_LVL_THRESHOLD');

	-- setup text
	uiGroup.outpost_lvl_thre.uc_hardtext= tostring(val);
end


------------------------------------------------------------------------------------------------------------
-- ROUND_LVL_MAX_ATT
function game:outpostChangeRoundLvlMaxAtt()
	-- uiGroup here should be the 'round_state' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local path= self:outpostInfoGetDbPath(uiGroup.parent);
	local val= getDbProp(path .. ':ROUND_LVL_MAX_ATT');
	local status= getDbProp(path .. ':STATUS');

	-- setup text (only relevant when attack period has begun)
	if(status>=self.OutpostEnums.AttackRound) then
		uiGroup.outpost_lvl_max_att.uc_hardtext= tostring(val);
	else
		uiGroup.outpost_lvl_max_att.uc_hardtext= '- ';
	end
end


------------------------------------------------------------------------------------------------------------
-- ROUND_LVL_MAX_DEF
function game:outpostChangeRoundLvlMaxDef()
	-- uiGroup here should be the 'round_state' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local path= self:outpostInfoGetDbPath(uiGroup.parent);
	local val= getDbProp(path .. ':ROUND_LVL_MAX_DEF');
	local status= getDbProp(path .. ':STATUS');

	-- setup text (only relevant in War)
	if(status>=self.OutpostEnums.DefenseRound) then
		uiGroup.outpost_lvl_max_def.uc_hardtext= tostring(val);
	else
		uiGroup.outpost_lvl_max_def.uc_hardtext= '- ';
	end
end


------------------------------------------------------------------------------------------------------------
-- ROUND_LVL_CUR
function game:outpostChangeRoundLvlCur()
	-- uiGroup here should be the 'round_state' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local path= self:outpostInfoGetDbPath(uiGroup.parent);
	local val= getDbProp(path .. ':ROUND_LVL_CUR');
	local status= getDbProp(path .. ':STATUS');
	-- add 1, because server send a 0 based value
	val= val+1;

	-- setup text (only in a Attack/Defense Round)
	if(self:outpostIsStatusWarRound(status)) then
		uiGroup.outpost_lvl_cur.uc_hardtext= tostring(val);
	else
		uiGroup.outpost_lvl_cur.uc_hardtext= '- ';
	end
end


------------------------------------------------------------------------------------------------------------
-- ROUND_ID_CUR / ROUND_ID_MAX
function game:outpostChangeRoundId()
	-- uiGroup here should be the 'round_state' uigroup (not the whole parent infoGroup)
	local uiGroup= getUICaller();

	-- get values
	local path= self:outpostInfoGetDbPath(uiGroup.parent);
	local val= getDbProp(path .. ':ROUND_ID_CUR');
	local maxRound= getDbProp(path .. ':ROUND_ID_MAX');
	local status= getDbProp(path .. ':STATUS');
	-- add 1 because server sends a 0 based value
	val= val+1;

	-- setup text (only in a Attack/Defense Round)
	if(self:outpostIsStatusWarRound(status)) then
		uiGroup.outpost_round_cur.uc_hardtext= tostring(val) .. ' / ' .. tostring(maxRound);
	else
		uiGroup.outpost_round_cur.uc_hardtext= '- ';
	end
end


-- ***************************************************************************
-- ***************************************************************************
-- OUTPOST PVP INFO / PROPOSAL
-- ***************************************************************************
-- ***************************************************************************


------------------------------------------------------------------------------------------------------------
function game:outpostPvpJoin(choice)
	-- send msg server
	runAH(nil, 'outpost_pvp_join', choice);

	-- hide the window
	runAH(nil, 'hide', 'join_pvp_outpost_proposal');
end


------------------------------------------------------------------------------------------------------------
function game:outpostPvpJoinTimerOnDraw()
	local uiGroup= getUICaller();

	-- Compute time left
	local curTick= getDbProp('UI:VARIABLES:CURRENT_SERVER_TICK');
	local endTick= getDbProp('UI:TEMP:OUTPOST:PVP_PROPOSAL_TICK_END');
	local timeSec= (endTick - curTick)/10;
	timeSec= math.max(timeSec, 0);
	timeSec= math.floor(timeSec+0.5);

	-- replace in str
	local	text= i18n.get('uiOutpostJoinPVPTimer');
	text= findReplaceAll(text, "%time", tostring(timeSec));
	uiGroup.Env.Timer.uc_hardtext_format= text;

end


------------------------------------------------------------------------------------------------------------
function game:outpostInitPvpJoinTimer()
	local uiGroup= getUICaller();

	-- bkup Timer viewtext
	uiGroup.Env.Timer= uiGroup.text;

	-- must change timer each frame
	setOnDraw(uiGroup, 'game:outpostPvpJoinTimerOnDraw()');
end

