-- In this file we define functions that serves for bot_chat windows


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end


------------------------------------------------------------------------------------------------------------
-- called to construct guild flags background in the modal window
function game:bcCreateGuildInitFlags()

	local ui = getUICaller();

	for i = 0,14 do
		local uiBack = getUI(getUIId(ui) .. ':back' .. i);
		uiBack.image1.back = i+1;
		uiBack.image1.symbol = 0;
		uiBack.image1.color1 = runExpr('makeRGB(255,255,255)');
		uiBack.image1.color2 = runExpr('makeRGB(0,0,0)');
	end
end

------------------------------------------------------------------------------------------------------------
-- called when UI:TEMP:MISSION:MISSION_TYPE changed
-- trap some parts should be deprecated ... try to clean it up
function game:bcMissionsUpdate()

	local mt = getDbProp('UI:TEMP:MISSION:MISSION_TYPE');

	-- init bot_chat_missions title
	local title = 'uiBotChatMissions';
	if (mt == 3) then		title = 'uiBotChatZCCharges';
	elseif (mt == 4) then	title = 'uiBotChatBuilding';
	elseif (mt == 5) then	title = 'uiBotChatRMBuy';
	elseif (mt == 6) then	title = 'uiBotChatRMUpgrade';
	end
	local ui = getUI('ui:interface:bot_chat_missions');
	ui.title = title;

	-- init desc
	title = 'uiSelectMission';
	if (mt == 3) then		title = 'uiSelectZCCharge';
	elseif (mt == 4) then	title = 'uiSelectBuilding';
	elseif (mt == 5) then	title = 'uiSelectRMBuy';
	elseif (mt == 6) then	title = 'uiSelectRMUpgrade';
	end
	ui.header_opened.mission_title.hardtext = title;

	ui.header_opened.zc_duty.active = (mt == 3);
	ui.header_opened.xp_guild.active = ((mt == 5) or (mt == 6));

	-- init bot_chat_accept_mission title
	title = 'uiAcceptMission';
	if (mt == 3) then		title = 'uiAcceptZCCharge';
	elseif (mt == 4) then	title = 'uiAcceptBuilding';
	elseif (mt == 5) then	title = 'uiAcceptRMBuy';
	elseif (mt == 6) then	title = 'uiAcceptRMUpgrade';
	end
	ui = getUI('ui:interface:bot_chat_accept_mission');
	ui.title = title;
end
