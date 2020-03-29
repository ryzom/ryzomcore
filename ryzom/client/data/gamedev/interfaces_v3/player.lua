-- In this file we define functions that serves for player windows

function getDbPropU(dbEntry)
	value = getDbProp(dbEntry)
	if (value < 0) then
		value = 4294967296+value
	end
	return value
end

if string.find(_VERSION, "Lua 5.0") then
	function math.fmod(a, b)
		return math.mod(a, b)
	end
end

------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

if (game.PVP == nil) then
	game.PVP = {};
	game.PVP.tagStartTimer = 0;
	game.PVP.flagStartTimer = 0;
	game.PVP.tagTimerStarted = false;
	game.PVP.flagTimerStarted = false;
end

if (game.BonusMalus == nil) then
	game.BonusMalus = {};
	game.BonusMalus.DeathPenaltyBefore = -1;
	game.BonusMalus.DeathPenaltyAfter = -1;
	game.BonusMalus.XPCatSlotBefore = -1;
	game.BonusMalus.XPCatSlotAfter = -1;
	game.BonusMalus.RingXPCatSlotBefore = -1;
	game.BonusMalus.RingXPCatSlotAfter = -1;
	game.BonusMalus.OutpostSlotBefore = -1;
	game.BonusMalus.OutpostSlotAfter = -1;
	game.BonusMalus.BonusAHList= {};
	game.BonusMalus.MalusAHList= {};
end


------------------------------------------------------------------------------------------------------------
-- Update player bars in function of what we wants to display (we can hide each one of the 3 bars : sap,stamina and focus)
function game:updatePlayerBars()

	local dispSap = getDbProp('UI:SAVE:PLAYER:DISP_SAP');
	local dispSta = getDbProp('UI:SAVE:PLAYER:DISP_STA');
	local dispFoc = getDbProp('UI:SAVE:PLAYER:DISP_FOC');
	
	local ui = getUI('ui:interface:player:content');

	-- active ui in function of what is displayed

	ui.b_sap.active = (dispSap == 1);
	ui.jsap.active = (dispSap == 1);

	ui.b_sta.active = (dispSta == 1);
	ui.jsta.active = (dispSta == 1);

	ui.b_foc.active = (dispFoc == 1);
	ui.jfoc.active = (dispFoc == 1);

	-- choose good y-position

	local totalBarDisp = dispSap + dispSta + dispFoc;
	if (totalBarDisp == 3) then

		ui.b_sap.y = -20;
		ui.b_sta.y = -35;
		ui.b_foc.y = -50;
		ui.current_action.y = -65;

	elseif (totalBarDisp == 2) then

		if (dispSap == 0) then
			ui.b_sta.y = -20;
			ui.b_foc.y = -35;
		end

		if (dispSta == 0) then
			ui.b_sap.y = -20;
			ui.b_foc.y = -35;
		end

		if (dispFoc == 0) then
			ui.b_sap.y = -20;
			ui.b_sta.y = -35;
		end

		ui.current_action.y = -50;

	elseif (totalBarDisp == 1) then

		ui.b_sta.y = -20;
		ui.b_foc.y = -20;
		ui.b_sta.y = -20;

		ui.current_action.y = -35;

	else
		ui.current_action.y = -20;
	end


end


------------------------------------------------------------------------------------------------------------
-- convert a boolean to a number 0 or 1
function booleanToNumber(thebool)
	if(thebool) then
		return 1;
	else
		return 0;
	end
end

------------------------------------------------------------------------------------------------------------
-- Update player pvp tag
function game:pvpTagUpdateDisplay()
	local currentServerTick = getDbPropU('UI:VARIABLES:CURRENT_SERVER_TICK');
	local pvpServerTagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:ACTIVATION_TIME');
	local pvpServerFlagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:FLAG_PVP_TIME_LEFT');
	local uiPlayer= getUI('ui:interface:player:header_opened');

	-- get the current state
	local	pvpServerFlag= pvpServerFlagTimer > currentServerTick;
	local	pvpLocalTag= (getDbProp('UI:TEMP:PVP_FACTION:TAG_PVP') == 1);
	local	pvpServerTag= (getDbProp('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:TAG_PVP') == 1);
	local	pvpServerActivateTimerOn= pvpServerTagTimer > currentServerTick;

	-- deduce the display state according to the current state
	local	GREEN= 0;
	local	ORANGE= 1;
	local	RED= 2;
	local	buttonMode= GREEN;
	local	buttonPushed= false;
	local	buttonTimer= false;
	-- if the flag is activated, then must display PVP flag button and timer
	if (pvpServerFlag) then
		-- ** RED MODE
		buttonMode= RED;
		buttonPushed= false;
		buttonTimer= true;
	-- else must display correct mode according to the TAG state
	else
		-- There are 8 possibilities according to the combination of the 3 flags
		-- Here: TL= pvpLocalTag, TS= pvpServerTag, AS= pvpServerActivateTimerOn)
		--  TL  TS  AS  
		--				** GREEN MODE **
		--  0   0   0	-> Standard disabled PVP
		--  1   0   0	-> The user pressed the button but still no response from server
		--  1   1   1	-> The user pressed the button and got response from server. => GREEN icon with timer
		--  0   1   1	-> The user canceled the activation (server not acked yet the cancel). => default display 
		--				** ORANGE MODE **
		--  1   1   0	-> Standard enabled PVP
		--  0   1   0	-> The user pressed the button but still no response from server
		--  0   0   1	-> The user pressed the button and got response from server. => ORANGE icon with timer
		--  1   0   1	-> The user canceled the activation (server not acked yet the cancel). => default display 

		-- From this table, we can deduce the following rules

		-- buttonMode is GREEN when TS==AS
		if( pvpServerTag == pvpServerActivateTimerOn ) then
			buttonMode= GREEN;
		else
			buttonMode= ORANGE;
		end

		-- the button is pushed if (there is a timer and TL==TS), or (no timer and TL!=TS)
		if( pvpServerActivateTimerOn == (pvpLocalTag == pvpServerTag) ) then
			buttonPushed= true;
		else
			buttonPushed= false;
		end

		-- display a timer only if the timer is activated and server and local tag are equals
		if( pvpServerActivateTimerOn and pvpLocalTag == pvpServerTag ) then
			buttonTimer= true;
		else
			buttonTimer= false;
		end

	end

	-- setup the local display
	setDbProp("UI:TEMP:PVP_FACTION:DSP_MODE", buttonMode);
	setDbProp("UI:TEMP:PVP_FACTION:DSP_PUSHED", booleanToNumber(buttonPushed));
	setDbProp("UI:TEMP:PVP_FACTION:DSP_TIMER", booleanToNumber(buttonTimer));

	-- setup the timer bar
	if(buttonTimer) then
		local uiBar = uiPlayer.pvp_timer;
		local uiBarBg = uiPlayer.pvp_timer_bg;
		-- Flag Bar?
		if(buttonMode==RED) then
			-- display a reverse timer
			uiBar.w = uiBarBg.w * (pvpServerFlagTimer - currentServerTick) / (pvpServerFlagTimer - game.PVP.flagStartTimer); 
		else
			-- display a forward timer
			uiBar.w = uiBarBg.w * (currentServerTick - game.PVP.tagStartTimer) / (pvpServerTagTimer - game.PVP.tagStartTimer); 
		end
	end

	-- force update of the tooltip for any button (by disabling then reenabling)
	disableContextHelpForControl(uiPlayer.pvp_tag_button_0);
	disableContextHelpForControl(uiPlayer.pvp_tag_button_1);
	disableContextHelpForControl(uiPlayer.pvp_tag_button_2);
end

------------------------------------------------------------------------------------------------------------
-- Update player pvp tag
function game:pvpTag()
	local buttonStat = getDbProp('UI:TEMP:PVP_FACTION:TAG_PVP');
	if (buttonStat == 0) then
		setDbProp('UI:TEMP:PVP_FACTION:TAG_PVP',1);
	else
		setDbProp('UI:TEMP:PVP_FACTION:TAG_PVP',0);
	end
	sendMsgToServerPvpTag(buttonStat == 0);

	-- update display
	self:pvpTagUpdateDisplay();
end

------------------------------------------------------------------------------------------------------------
-- Update button due to server validation
function game:updatePvpTag()
	-- force copy to temp of Server tag
	local	pvpServerTag= (getDbProp('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:TAG_PVP') == 1);
	setDbProp('UI:TEMP:PVP_FACTION:TAG_PVP', booleanToNumber(pvpServerTag));

	-- launch timer DB if necessary
	local currentServerTick = getDbPropU('UI:VARIABLES:CURRENT_SERVER_TICK');
	local pvpServerTagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:ACTIVATION_TIME');
	local pvpServerFlagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:FLAG_PVP_TIME_LEFT');

	if(pvpServerTagTimer > currentServerTick) or (pvpServerFlagTimer > currentServerTick) then
		local ui = getUI('ui:interface:player');
		addOnDbChange(ui,'@UI:VARIABLES:CURRENT_SERVER_TICK', 'game:updatePvpTimer()');

		if(pvpServerTagTimer > currentServerTick and game.PVP.tagTimerStarted == false) then
			game.PVP.tagStartTimer = currentServerTick;
			game.PVP.tagTimerStarted = true;
		end
		if(pvpServerFlagTimer > currentServerTick and game.PVP.flagTimerStarted == false) then
			game.PVP.flagStartTimer = currentServerTick;
			game.PVP.flagTimerStarted = true;
		end
	end

	-- update display (after start timer reseted)
	self:pvpTagUpdateDisplay();
end

------------------------------------------------------------------------------------------------------------
-- 
function game:updatePvpTimer()

	-- update display
	self:pvpTagUpdateDisplay();

	-- try to stop
	local currentServerTick = getDbPropU('UI:VARIABLES:CURRENT_SERVER_TICK');
	local pvpServerTagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:ACTIVATION_TIME');
	local pvpServerFlagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:FLAG_PVP_TIME_LEFT');

	-- Manage Tag Timer display
	if(pvpServerTagTimer <= currentServerTick) then
		game.PVP.tagTimerStarted = false;
	end

	-- Manage Flag Timer display
	if(pvpServerFlagTimer <= currentServerTick) then
		game.PVP.flagTimerStarted = false;
	end

	-- if both off, stop the db update
	if(game.PVP.tagTimerStarted == false) and (game.PVP.flagTimerStarted == false) then
		removeOnDbChange(getUI('ui:interface:player'),'@UI:VARIABLES:CURRENT_SERVER_TICK');
	end
end

------------------------------------------------------------------------------------------------------------
-- 
function game:formatTime(temps)
	
	local hours = math.floor(temps/(10*60*60));
	local minutes = math.floor((temps - (hours*10*60*60)) / (10*60));
	local seconds = math.floor((temps - (hours*10*60*60) - (minutes*10*60)) / 10);

	local fmt = i18n.get('uittPvPTime');
	fmt = findReplaceAll(fmt, '%h', tostring(hours));
	fmt = findReplaceAll(fmt, '%m', tostring(minutes));
	fmt = findReplaceAll(fmt, '%s', tostring(seconds));
	return fmt;
end

------------------------------------------------------------------------------------------------------------
-- 
function game:playerTTPvp()

	-- The tooltip to display depends on the current display state
	local buttonMode= getDbProp("UI:TEMP:PVP_FACTION:DSP_MODE");
	local buttonPushed= (getDbProp("UI:TEMP:PVP_FACTION:DSP_PUSHED")==1);
	local buttonTimer= (getDbProp("UI:TEMP:PVP_FACTION:DSP_TIMER")==1);
	local	text;

	-- Flag mode?
	if(buttonMode==2) then
		local pvpServerFlagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:FLAG_PVP_TIME_LEFT');
		local currentServerTick = getDbPropU('UI:VARIABLES:CURRENT_SERVER_TICK');
		local tempsString = game:formatTime( pvpServerFlagTimer - currentServerTick ); 
		text = i18n.get('uittPvPModeFlag');
		text = findReplaceAll(text, '%temps', tempsString); 
	-- Tag mode
	else
		-- base text
		if(buttonMode==0 and not(buttonPushed)) then
			text = i18n.get('uittPvPModeTagOff');
		elseif(buttonMode==0 and buttonPushed) then
			text = i18n.get('uittPvPModeTagOffChange');
		elseif(buttonMode==1 and not(buttonPushed)) then
			text = i18n.get('uittPvPModeTagOn');
		elseif(buttonMode==1 and buttonPushed) then
			text = i18n.get('uittPvPModeTagOnChange');
		else
			text = ucstring();
		end
		-- timer
		if(buttonTimer) then
			local pvpServerTagTimer = getDbPropU('SERVER:CHARACTER_INFO:PVP_FACTION_TAG:ACTIVATION_TIME');
			local currentServerTick = getDbPropU('UI:VARIABLES:CURRENT_SERVER_TICK');
			local tempsString = game:formatTime( pvpServerTagTimer - currentServerTick ); 
			local timeFmt= i18n.get('uittPvPTagTimer');
			timeFmt= findReplaceAll(timeFmt, '%temps', tempsString);
			text= concatUCString(text, timeFmt);
		end
	end

	-- set the text
	setContextHelpText(text);
end



-- ***************************************************************************
-- ***************************************************************************
-- BONUS MALUS
-- ***************************************************************************
-- ***************************************************************************


------------------------------------------------------------------------------------------------------------
function game:bonusMalusActiveText(ui, slot, state)
	local uiTextGroup= ui["text" .. tostring(slot) ];
	if(uiTextGroup) then
		uiTextGroup.active= state;
	end
end

------------------------------------------------------------------------------------------------------------
function game:bonusMalusSetText(ui, slot, fmt)
	local uiTextGroup= ui["text" .. tostring(slot) ];
	if(uiTextGroup) then
		uiTextGroup.shade0.uc_hardtext_format= fmt;
		uiTextGroup.shade1.uc_hardtext_format= fmt;
		uiTextGroup.shade2.uc_hardtext_format= fmt;
		uiTextGroup.shade3.uc_hardtext_format= fmt;
		uiTextGroup.text.uc_hardtext_format= fmt;
		uiTextGroup.text2.uc_hardtext_format= fmt;
	end
end

------------------------------------------------------------------------------------------------------------
-- From given DB vals, compute the 'Xp Bonus' text info
function game:updateXpCatQuantity(textSlot, ui)
	-- get the ui text to fill
	if(ui==nil) then
		ui= getUICaller();
	end

	-- format the text
	local	fmt= "x@{FF6F}" .. tostring( getDbProp("SERVER:CHARACTER_INFO:XP_CATALYSER:Count") );
	
	self:bonusMalusSetText(ui, textSlot, fmt);
end


------------------------------------------------------------------------------------------------------------
-- From given DB vals, compute the 'Ring Xp Bonus' text info
function game:updateRingXpCatQuantity(textSlot, ui)
	-- get the ui text to fill
	if(ui==nil) then
		ui= getUICaller();
	end

	-- format the text
	local	fmt= "x@{FF6F}" .. tostring( getDbProp("SERVER:CHARACTER_INFO:RING_XP_CATALYSER:Count") );
	
	self:bonusMalusSetText(ui, textSlot, fmt);
end


------------------------------------------------------------------------------------------------------------
function game:outpostUpdatePVPTimer(textSlot, ui)
	-- get the ui text to fill
	if(ui==nil) then
		ui= getUICaller();
	end

	-- Get the timer of interest (priority to player leaving the zone)
	local	endTimer= 0;
	local	endOfPvpTimer= getDbPropU('SERVER:CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP_TIME_END');
	if( endOfPvpTimer>0 ) then
		endTimer= endOfPvpTimer;
	else
		local	endOfRound= getDbProp('SERVER:CHARACTER_INFO:PVP_OUTPOST:ROUND_END_DATE');
		if( endOfRound>0 ) then
			endTimer= endOfRound;
		end
	end

	-- Use a text with a timer?
	if( endTimer>0 ) then
		-- compute the time that lefts in sec (suppose a smooth server tick is 1 ms)
		local curTick= getDbPropU('UI:VARIABLES:CURRENT_SERVER_TICK');
		local timeSec= (endTimer- curTick)/10;
		-- replace in str
		local	text= "@{FF6F}" .. runFct('secondsToTimeStringShort', timeSec);
		self:bonusMalusSetText(ui, textSlot, text);
	-- else Default display
	else
		self:bonusMalusSetText(ui, textSlot, "@{FF6F}on");
	end

end


------------------------------------------------------------------------------------------------------------
function game:deathPenaltyUpdateXPMalus()
end


------------------------------------------------------------------------------------------------------------
-- called when someone click on a bonus malus icon. redirect to correct action handler if any
function game:onLeftClickBonus()
	local	ui= getUICaller();
	local	id= getIndexInDB(ui);
	local	ah= self.BonusMalus.BonusAHList[id];
	if(ui and ah) then
		runAH(ui, ah, "");
	end
end

function game:onLeftClickMalus()
	local	ui= getUICaller();
	local	id= getIndexInDB(ui);
	local	ah= self.BonusMalus.MalusAHList[id];
	if(ui and ah) then
		runAH(ui, ah, "");
	end
end

------------------------------------------------------------------------------------------------------------
-- update if needed the ActionHandler and text update from DB
function game:updateBonusMalusTextSetup()
	local	numLocalBonusMalus= getDefine("num_local_bonus_malus");
	local	uiBonus= getUI('ui:interface:bonus_malus:header_opened:bonus');
	local	uiMalus= getUI('ui:interface:bonus_malus:header_opened:malus');
	local	dbXpCat= "@SERVER:CHARACTER_INFO:XP_CATALYSER:Count";
	local	dbRingXpCat= "@SERVER:CHARACTER_INFO:RING_XP_CATALYSER:Count";
	local	dbOutpost= "@SERVER:CHARACTER_INFO:PVP_OUTPOST, @UI:VARIABLES:CURRENT_SERVER_TICK";
	local	dbDeathPenalty= "@SERVER:USER:DEATH_XP_MALUS";


	-- reset cache
	self.BonusMalus.DeathPenaltyBefore= self.BonusMalus.DeathPenaltyAfter;
	self.BonusMalus.XPCatSlotBefore= self.BonusMalus.XPCatSlotAfter;
	self.BonusMalus.RingXPCatSlotBefore= self.BonusMalus.RingXPCatSlotAfter;
	self.BonusMalus.OutpostSlotBefore= self.BonusMalus.OutpostSlotAfter;


	-- *** remove and hide any preceding
	for i= 0,numLocalBonusMalus-1 do
		-- reset AH
		self.BonusMalus.BonusAHList[i]= nil;
		self.BonusMalus.MalusAHList[i]= nil;
		-- hide text view
		self:bonusMalusActiveText(uiBonus, i, false);
		-- reset special tooltip
		setDbProp( formatUI('UI:VARIABLES:BONUS:#1:SPECIAL_TOOLTIP', i), game.TBonusMalusSpecialTT.None);
	end
	removeOnDbChange(uiBonus, dbXpCat);
	removeOnDbChange(uiBonus, dbRingXpCat);
	removeOnDbChange(uiBonus, dbOutpost);


	-- *** set new XPCat setup
	local	slot= self.BonusMalus.XPCatSlotAfter;
	if(slot~=-1) then
		-- set AH to use for this slot
		self.BonusMalus.BonusAHList[slot]= "xp_catalyser_stop_use";
		-- add DB change, and call now! else not updated
		addOnDbChange(uiBonus, dbXpCat, formatUI("game:updateXpCatQuantity(#1, nil)", slot) );
		self:updateXpCatQuantity(slot, uiBonus);
		-- show text
		self:bonusMalusActiveText(uiBonus, slot, true);
		-- set special tooltip (id==1 for xpcat)
		setDbProp( formatUI('UI:VARIABLES:BONUS:#1:SPECIAL_TOOLTIP', slot), game.TBonusMalusSpecialTT.XpCatalyser);
	end

	-- *** set new RingXPCat setup
	local	slot= self.BonusMalus.RingXPCatSlotAfter;
	if(slot~=-1) then
		-- set AH to use for this slot
		self.BonusMalus.BonusAHList[slot]= "ring_xp_catalyser_stop_use";
		-- add DB change, and call now! else not updated
		addOnDbChange(uiBonus, dbRingXpCat, formatUI("game:updateRingXpCatQuantity(#1, nil)", slot) );
		self:updateRingXpCatQuantity(slot, uiBonus);
		-- show text
		self:bonusMalusActiveText(uiBonus, slot, true);
		-- set special tooltip (id==1 for ringxpcat)
		setDbProp( formatUI('UI:VARIABLES:BONUS:#1:SPECIAL_TOOLTIP', slot), game.TBonusMalusSpecialTT.XpCatalyser);
	end


	-- *** set new Outpost setup
	local	slot= self.BonusMalus.OutpostSlotAfter;
	if(slot~=-1) then
		-- no AH
		-- add DB change, and call now! else not updated
		addOnDbChange(uiBonus, dbOutpost, formatUI("game:outpostUpdatePVPTimer(#1, nil)", slot) );
		self:outpostUpdatePVPTimer(slot, uiBonus);
		-- show text
		self:bonusMalusActiveText(uiBonus, slot, true);
		-- don't set the tooltip here, because redone after return
	end


	-- *** set new DeathPenalty setup
	local	slot= self.BonusMalus.DeathPenaltyAfter;
	if(slot~=-1) then
		-- no AH
		-- add DB change, and call now! else not updated
		addOnDbChange(uiMalus, dbDeathPenalty, formatUI("game:deathPenaltyUpdateXPMalus(#1, nil)", slot) );
		self:deathPenaltyUpdateXPMalus(slot, uiMalus);
		-- show text
		self:bonusMalusActiveText(uiMalus, slot, true);
		-- set special tooltip (id==1 for death penalty)
		setDbProp( formatUI('UI:VARIABLES:MALUS:#1:SPECIAL_TOOLTIP', slot), game.TBonusMalusSpecialTT.DeathPenalty);
	end

end

------------------------------------------------------------------------------------------------------------
-- Update Bonus malus local DB according to server DB
function game:updatePlayerBonusMalus()
	local numServerBonusMalus= tonumber(getDefine("num_server_bonus_malus"));
	local numLocalBonusMalus= tonumber(getDefine("num_local_bonus_malus"));
	local dbServerBonusBase= getDefine("bonus") .. ":" ;
	local dbServerMalusBase= getDefine("malus") .. ":" ;
	local dbLocalBonusBase= "UI:VARIABLES:BONUS:";
	local dbLocalMalusBase= "UI:VARIABLES:MALUS:";

	local	i;
	local	mustUpdateTextSetup= false;


	-- ***********************
	-- *** Insert Bonus
	-- ***********************
	local	destIndex= 0;
	local	mustShowBonus= false;

	-- *** Insert XPCatalyzer first
	local	xpcatCount= getDbProp("SERVER:CHARACTER_INFO:XP_CATALYSER:Count");
	if(xpcatCount~=0) then
		local	xpcatLevel= getDbProp("SERVER:CHARACTER_INFO:XP_CATALYSER:Level");
		-- Get the most appropriate icon
		local	iconLevel= 50;
		for i= 50,250,50 do
			if(i<=xpcatLevel) then
				iconLevel= i;
			end
		end
		-- Set the DB for this brick
		mustShowBonus= true;
		setDbProp(dbLocalBonusBase .. tostring(destIndex) .. ":SHEET", getSheetId('big_xpcat_' .. tostring(iconLevel) .. '.sbrick' ) );
		setDbProp(dbLocalBonusBase  .. tostring(destIndex) .. ":DISABLED", 0 );
		self.BonusMalus.XPCatSlotAfter = destIndex;
		destIndex= destIndex+1;
	else
		self.BonusMalus.XPCatSlotAfter = -1;
	end
	if(self.BonusMalus.XPCatSlotAfter ~= self.BonusMalus.XPCatSlotBefore) then
		mustUpdateTextSetup= true;
	end

	-- *** Then insert RingXPCatalyzer 
	local	ringxpcatCount= getDbProp("SERVER:CHARACTER_INFO:RING_XP_CATALYSER:Count");
	if(ringxpcatCount~=0) then
		local	ringxpcatLevel= getDbProp("SERVER:CHARACTER_INFO:RING_XP_CATALYSER:Level");
		-- Get the most appropriate icon
		local	iconLevel= 50;
		for i= 50,250,50 do
			if(i<=ringxpcatLevel) then
				iconLevel= i;
			end
		end
		-- Set the DB for this brick
		mustShowBonus= true;
		setDbProp(dbLocalBonusBase .. tostring(destIndex) .. ":SHEET", getSheetId('big_ring_xpcat_' .. tostring(iconLevel) .. '.sbrick' ) );
		setDbProp(dbLocalBonusBase  .. tostring(destIndex) .. ":DISABLED", 0 );
		self.BonusMalus.RingXPCatSlotAfter = destIndex;
		destIndex= destIndex+1;
	else
		self.BonusMalus.RingXPCatSlotAfter = -1;
	end
	if(self.BonusMalus.RingXPCatSlotAfter ~= self.BonusMalus.RingXPCatSlotBefore) then
		mustUpdateTextSetup= true;
	end


	-- *** Insert PVPOutpost
	local	pvpOutpostPresent= getDbProp("SERVER:CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP");
	local	pvpOutpostEndOfPVPFlag= 0;
	local	pvpOutpostEndOfRound= 0;
	if(pvpOutpostPresent~=0) then
		local	pvpOutpostLevel= 0;
		pvpOutpostEndOfPVPFlag= getDbPropU('SERVER:CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP_TIME_END');
		pvpOutpostEndOfRound= getDbPropU('SERVER:CHARACTER_INFO:PVP_OUTPOST:ROUND_END_DATE');
		-- set a level only if we have some round, and if the out timer is not set
		if(pvpOutpostEndOfRound~=0 and pvpOutpostEndOfPVPFlag==0) then
			pvpOutpostLevel= 1 + getDbProp('SERVER:CHARACTER_INFO:PVP_OUTPOST:ROUND_LVL_CUR');
		end

		-- Set the DB for this brick
		mustShowBonus= true;
		setDbProp(dbLocalBonusBase .. tostring(destIndex) .. ":SHEET", getSheetId('big_outpost_pvp_' .. tostring(pvpOutpostLevel) .. '.sbrick' ) );
		setDbProp(dbLocalBonusBase  .. tostring(destIndex) .. ":DISABLED", 0 );
		self.BonusMalus.OutpostSlotAfter = destIndex;
		destIndex= destIndex+1;
	else
		self.BonusMalus.OutpostSlotAfter = -1;
	end
	if(self.BonusMalus.OutpostSlotAfter ~= self.BonusMalus.OutpostSlotBefore) then
		mustUpdateTextSetup= true;
	end


	-- *** Insert standard Bonus
	for i=0,numServerBonusMalus-1 do
		-- get
		local	sheet= getDbProp(dbServerBonusBase .. tostring(i) .. ":SHEET" );
		local	disabled= getDbProp(dbServerBonusBase .. tostring(i) .. ":DISABLED" );
		if(sheet~=0) then
			mustShowBonus= true;
		end
		-- copy (to index shifted if needed)
		setDbProp(dbLocalBonusBase .. tostring(destIndex) .. ":SHEET", sheet );
		setDbProp(dbLocalBonusBase  .. tostring(destIndex) .. ":DISABLED", disabled );
		destIndex= destIndex+1;
	end
	if(mustShowBonus) then
		setDbProp("UI:VARIABLES:SHOW_BONUS", 1);
	else
		setDbProp("UI:VARIABLES:SHOW_BONUS", 0);
	end


	-- *** erase any remaining bonus
	while destIndex<numLocalBonusMalus do
		setDbProp(dbLocalBonusBase .. tostring(destIndex) .. ":SHEET", 0 );
		destIndex= destIndex + 1;
	end



	-- ***********************
	-- *** Insert Malus
	-- ***********************
	local	mustShowMalus= false;
	destIndex= 0;

	-- *** Insert Death Penalty first
	local	deathPenalty= getDbProp("SERVER:USER:DEATH_XP_MALUS");
	if(deathPenalty~=255 and deathPenalty~=0) then
		-- Set the DB for this brick
		mustShowMalus= true;
		setDbProp(dbLocalMalusBase .. tostring(destIndex) .. ":SHEET", getSheetId('death_penalty.sbrick' ) );
		setDbProp(dbLocalMalusBase  .. tostring(destIndex) .. ":DISABLED", 0 );
		self.BonusMalus.DeathPenaltyAfter = destIndex;
		destIndex= destIndex+1;
	else
		self.BonusMalus.DeathPenaltyAfter = -1;
	end
	if(self.BonusMalus.DeathPenaltyAfter ~= self.BonusMalus.DeathPenaltyBefore) then
		mustUpdateTextSetup= true;
	end

	-- *** insert standard malus
	for i=0,numServerBonusMalus-1 do
		-- get
		local	sheet= getDbProp(dbServerMalusBase .. tostring(i) .. ":SHEET" );
		local	disabled= getDbProp(dbServerMalusBase .. tostring(i) .. ":DISABLED" );
		if(sheet~=0) then
			mustShowMalus= true;
		end
		-- copy
		setDbProp(dbLocalMalusBase .. tostring(destIndex) .. ":SHEET", sheet );
		setDbProp(dbLocalMalusBase  .. tostring(destIndex) .. ":DISABLED", disabled );
		destIndex= destIndex+1;
	end
	if(mustShowMalus) then
		setDbProp("UI:VARIABLES:SHOW_MALUS", 1);
	else
		setDbProp("UI:VARIABLES:SHOW_MALUS", 0);
	end


	-- *** erase any remaining malus
	while destIndex<numLocalBonusMalus do
		setDbProp(dbLocalMalusBase .. tostring(destIndex) .. ":SHEET", 0 );
		destIndex= destIndex + 1;
	end


	
	-- ***********************
	-- *** update Text setup
	-- ***********************
	if(mustUpdateTextSetup) then
		game:updateBonusMalusTextSetup();
	end

	-- set special tooltip for outpost (id==2,3,4 for outpost)
	if(self.BonusMalus.OutpostSlotAfter ~= -1) then
		local	dbFmt= formatUI('UI:VARIABLES:BONUS:#1:SPECIAL_TOOLTIP', self.BonusMalus.OutpostSlotAfter);
		if(pvpOutpostEndOfPVPFlag ~= 0) then
			setDbProp(dbFmt, game.TBonusMalusSpecialTT.OutpostPVPOutOfZone);
		elseif(pvpOutpostEndOfRound ~= 0) then
			setDbProp(dbFmt, game.TBonusMalusSpecialTT.OutpostPVPInRound);
		else
			setDbProp(dbFmt, game.TBonusMalusSpecialTT.OutpostPVPOn);
		end
	end

end



-- ***************************************************************************
-- ***************************************************************************
-- CURRENT ACTION
-- ***************************************************************************
-- ***************************************************************************


------------------------------------------------------------------------------------------------------------
function game:updateCurrentActionPosition()
	local uiMemory= getUI("ui:interface:gestionsets");
	local uiAction= getUI("ui:interface:current_action");
	local uiMain= getUI("ui:interface");
	if(uiAction and uiMain and uiMemory and uiMemory.active) then

		-- NB: must use harcoded 182 and 40 size for the window, because may not be active at this time

		-- refresh the x position
		uiAction.x= uiMemory.x_real + uiMemory.w_real/2 - 182/2;

		-- setup the y position according to position of the memory bar
		local distBelow= uiMemory.y_real;
		local distAbove= uiMain.h - (uiMemory.y_real + uiMemory.h_real);
		if(distBelow < distAbove) then
			uiAction.y= uiMemory.y_real + uiMemory.h_real + 40;
		else
			uiAction.y= uiMemory.y_real;
		end

	end
end


LastTooltipPhrase = nil

------------------------------------------------------------------------------------------------------------
-- tool function used by game:updatePhraseTooltip
function game:setPhraseTooltipCarac(ttWin, name, value, textValue)
	local icon = ttWin:find(name)
	local text = ttWin:find(name .. "_text")
	if value == 0 then
		icon.active = false
		text.active = false
	else
		icon.active = true
		text.active = true
		if textValue ~= nil then 			
			text.uc_hardtext = textValue
		else
			text.hardtext = tostring(value)			
		end
	end
end


function game:timeInSecondsToReadableTime(regenTime)			
	local seconds = math.fmod(regenTime, 60)	
	local minutes = math.fmod(math.floor(regenTime / 60), 60)	
	local hours = math.floor(regenTime / 3600)	
	local result = ""
	if seconds > 0 then result = concatUCString(tostring(seconds), i18n.get("uittSecondsShort"))	end
	if minutes > 0 then result = concatUCString(tostring(minutes), i18n.get("uittMinutesShort"), result) end	
	if hours > 0 then result = concatUCString(tostring(hours), i18n.get("uittHoursShort"), result) end	
	return result	
end

------------------------------------------------------------------------------------------------------------
-- display the time left for a power / auras in its tooltip
function game:setPhraseTooltipPowerRegenTime(ttWin, regenTimeInTicks)
	local text = ttWin:find("regen_time")	
	if regenTimeInTicks == 0 then		
		text.active = false
	else				
		text.active = true		
		text.uc_hardtext_single_line_format = concatUCString(i18n.get("uittRegenTime"), game:timeInSecondsToReadableTime(math.floor((regenTimeInTicks + 9) * 0.1)))
		text:invalidateCoords()
		ttWin:invalidateCoords()
	end
end


local EmptyUCString = ucstring()

------------------------------------------------------------------------------------------------------------
-- called by C++ code when the tooltip of a phrase is about to be displayed
function game:updatePhraseTooltip(phrase)	
	LastTooltipPhrase = phrase
	local ttWin = getUI("ui:interface:action_context_help")
	local text = phrase:getName()
	
	if not text or text == EmptyUCString then
		text = ucstring("")
	end

	local desc = phrase:getDesc()
	if desc and desc ~= EmptyUCString then
		local str = tostring(desc)
		local charFound = false
		for k = 1, string.len(str) do
			if string.byte(str, k) ~= 32 then
				charFound = true
				break
			end
		end
		if charFound then
			text = concatUCString(text, "\n@{CCCF}", desc)
		end
	else
		text = concatUCString(text, "@{CCCF}")
	end
	-- IMPORTANT : the following getters on 'phrase' take in account the 'total action malus' for the timebeing 	
	self:setPhraseTooltipCarac(ttWin, "hp_cost",	phrase:getHpCost())
	self:setPhraseTooltipCarac(ttWin, "sta_cost",	phrase:getStaCost())
	self:setPhraseTooltipCarac(ttWin, "sap_cost",	phrase:getSapCost())	
	self:setPhraseTooltipCarac(ttWin, "focus_cost", phrase:getFocusCost())	
	self:setPhraseTooltipCarac(ttWin, "cast_time",  phrase:getCastTime(), concatUCString(string.format("%.1f", phrase:getCastTime()), i18n.get("uittSeconds")))
	local castRange = phrase:getCastRange()
	if not phrase:isMagicPhrase() then
		castRange = 0
	end
	self:setPhraseTooltipCarac(ttWin, "cast_range", castRange, concatUCString(tostring(castRange), i18n.get("uittMeters")))
	-- if the phrase is a power / aura, then we may want to display its regen time in the tooltip
	if phrase:isPowerPhrase() then
		setOnDraw(ttWin, "game:updatePowerPhraseTooltip()")
	else
		setOnDraw(ttWin, "")
	end
	--
	local successRateText = ttWin:find("success_rate")
	local successRate = phrase:getSuccessRate()
	if successRate == 0 then
		successRateText.active = false
	else
		successRateText.active = true
		successRateText.uc_hardtext_single_line_format = concatUCString(i18n.get("uittSuccessRate"), tostring(successRate), " %")
	end

	local disableTimeText = ttWin:find("disable_time")	
	if phrase:isPowerPhrase() then
		local disableTime = phrase:getPowerDisableTime()
		if disableTime == 0 then
			disableTimeText.active = false
		else
			disableTimeText.active = true
			disableTimeText.uc_hardtext_single_line_format = concatUCString(i18n.get("uittDisableTime"), game:timeInSecondsToReadableTime(disableTime / 10))
		end
	else
		disableTimeText.active = false
	end
	game:updatePowerPhraseTooltip()
	updateTooltipCoords()	
	return text
end

------------------------------------------------------------------------------------------------------------
-- called at each frame when a power/aura tooltip is displayed,in order to update the regen countdown
function game:updatePowerPhraseTooltip()
	local ttWin = getUI("ui:interface:action_context_help")
	local leftRegenTime = 0
	if LastTooltipPhrase:isPowerPhrase() then
		leftRegenTime = LastTooltipPhrase:getTotalRegenTime() - LastTooltipPhrase:getRegenTime()
	end	
	if leftRegenTime < 0 then
		leftRegenTime = 0
	end
	self:setPhraseTooltipPowerRegenTime(ttWin, leftRegenTime)
	updateTooltipCoords()
end


-- ***************************************************************************
-- ***************************************************************************
-- CURRENT BUFF ITEM
-- ***************************************************************************
-- ***************************************************************************

------------------------------------------------------------------------------------------------------------
-- called by C++ code when the tooltip of a buff item is about to be displayed
function game:updateBuffItemTooltip(buffItem)	
	local ttWin = getUI("ui:interface:buff_item_context_help")	
	local text = buffItem:getName()	

	self:setPhraseTooltipCarac(ttWin, "hp_buff",	buffItem:getHpBuff())		
	self:setPhraseTooltipCarac(ttWin, "sta_buff",	buffItem:getStaBuff())		
	self:setPhraseTooltipCarac(ttWin, "sap_buff",	buffItem:getSapBuff())	
	self:setPhraseTooltipCarac(ttWin, "focus_buff", buffItem:getFocusBuff())	

	updateTooltipCoords()	
	return text
end

-- ***************************************************************************
-- ***************************************************************************
-- CURRENT CRYSTALLIZED SPELL
-- ***************************************************************************
-- ***************************************************************************

------------------------------------------------------------------------------------------------------------
-- called by C++ code when the tooltip of a cristallized spell is about to be displayed
function game:updateCrystallizedSpellTooltip(crystallizedSpell)	
	local ttWin = getUI("ui:interface:crystallized_spell_context_help")	
	local text = crystallizedSpell:getName()	

	crystallizedSpell:buildCrystallizedSpellListBrick()

	updateTooltipCoords()	
	return text
end
