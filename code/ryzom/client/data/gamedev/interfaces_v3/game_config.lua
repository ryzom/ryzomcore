-- In this file we define functions that serves for game config windows


------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

-- init the temp space for color picker
if (game.ColorPicker == nil) then
	game.ColorPicker = {};
	game.ColorPicker.r = 255;
	game.ColorPicker.g = 255;
	game.ColorPicker.b = 255;
	game.ColorPicker.button = nil;
	game.ColorPicker.pal = 0;
end

------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
-- WIDGET TO CHOOSE A COLOR 
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------
-- called when we click a button to choose a color from
function game:mcwChooseColor(title, pal)

	local ui = getUICaller();

	-- get the color of the button and store it in temporary db location	
	local rgba = ui.col_normal_rgba;
	game.ColorPicker.r = rgba.R;
	game.ColorPicker.g = rgba.G;
	game.ColorPicker.b = rgba.B;
	game.ColorPicker.button = ui;
	game.ColorPicker.pal = pal;
	
	-- setup the color picker window
	local uiModalWin = getUI('ui:interface:define_mcw_color');
	if (game.ColorPicker.pal == 0) then
		uiModalWin.pick.r = rgba.R;
		uiModalWin.pick.g = rgba.G;
		uiModalWin.pick.b = rgba.B;
		uiModalWin.pick.active = true;
		uiModalWin.pick2.active = false;
	else
		uiModalWin.pick2.r = rgba.R;
		uiModalWin.pick2.g = rgba.G;
		uiModalWin.pick2.b = rgba.B;
		uiModalWin.pick.active = false;
		uiModalWin.pick2.active = true;
	end
	uiModalWin.text.hardtext = title;

	game.mcwOnColorChanged();

	-- launch the color picker modal window
	runAH(ui, 'push_modal', 'group=ui:interface:define_mcw_color');
end

------------------------------------------------------------------------------------------------------------
-- called when we have finished choosing the color in the modal window and the chosen color is valid
function game:mcwValidateColor()

	if (game.ColorPicker.button == nil) then 
		return; 
	end
	game.ColorPicker.button.col_normal = game.ColorPicker.r .. ' ' .. game.ColorPicker.g .. ' ' .. game.ColorPicker.b;
	--debugInfo(game.ColorPicker.button.col_normal)
	game.ColorPicker.button.col_over = game.ColorPicker.button.col_normal;
	game.ColorPicker.button.col_pushed = game.ColorPicker.button.col_normal;
	runAH(game.ColorPicker.button, 'ddx_color', '');
	runAH(game.ColorPicker.button, 'leave_modal', '');
end

------------------------------------------------------------------------------------------------------------
-- called when the color changed
function game:mcwOnColorChanged()

	local uiPath = 'ui:interface:define_mcw_color:';
	if (game.ColorPicker.pal == 0) then
		uiPath = uiPath .. 'pick';
	else
		uiPath = uiPath .. 'pick2';
	end
	local ui = getUI(uiPath);
	game.ColorPicker.r = ui.r;
	game.ColorPicker.g = ui.g;
	game.ColorPicker.b = ui.b;

	local uiModalWin = getUI('ui:interface:define_mcw_color');
	uiModalWin.color = game.ColorPicker.r .. ' ' .. game.ColorPicker.g .. ' ' .. game.ColorPicker.b;
	uiModalWin.text.color = uiModalWin.color;
end

------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
-- CONFIG WINDOW
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------
-- hide all the pages of the config window
function game:configHideAll()

	local	uiList = { 
		'explanation', 'general', 'landscape', 'fx', 'char', 'hud', 'language', 'alpha_colors',
		'chat_colors', 'entity_colors', 'in_scene_user', 'in_scene_friend', 'in_scene_enemy', 
		'in_scene_chat_messages', 'win_colors', 'win_colors_r2', 'mouse', 'keyb', 'sound', 'landmark_colors', 'help' 
	};

	for k,v in pairs(uiList) do
		local uiGrp = getUI('ui:interface:game_config:content:' .. v);
		uiGrp.active = false;
	end
end

------------------------------------------------------------------------------------------------------------
-- show one of the multiple pages in the config window
function game:configShowOne(strUIToShow)

	game:configHideAll();
	-- special case : if the display tab was shown, update the aspect ratio if needed
	local generalGrp = getUI('ui:interface:game_config:content:general');
	local uiGrp = getUI('ui:interface:game_config:content:' .. strUIToShow);	
	-- Removed the following code to solve RT n°14720
	-- The 'game_config_change_screen_ratio_custom' action handler 
	-- should only be called if the user changed the apect ratio himself 
	-- (else the edit box containing the aspect ratio may not have been initiliazed here)
	--if uiGrp ~= generalGrp then		
	--	runAH(nil, 'game_config_change_screen_ratio_custom', '')
	--end
	uiGrp.active = true;
end

------------------------------------------------------------------------------------------------------------
-- 
function game:configInit()

	-- init language
	local lang = getClientCfg('LanguageCode');
	local langNb = 0;

	if (lang == 'de') then
		langNb = 2;
	elseif (lang == 'fr') then
		langNb = 1;
	end

	-- force observers call
	setDbProp('UI:TEMP:LANGUAGE', -1);
	setDbProp('UI:TEMP:LANGUAGE', langNb);

	runAH(nil, 'game_config_init', '');

	local r2WinOn = false
	if r2 ~= nil then
		if r2.Mode ~= "r2ed_anim_test" then
			r2WinOn = true
		end
	end
	local win = getUI("ui:interface:game_config")

	local dy = -24

	local function winActive(name, active)
		local win = win:find(name)
		win.active = active
		if not active then
			win.y = 0
		else
			win.y = dy
			dy = -4
		end
	end

	winActive("wc_r2_palette", r2WinOn)
	winActive("wc_r2_scenario", r2WinOn)
	winActive("wc_r2_prop_window", r2WinOn)
	winActive("wc_r2_form", r2WinOn)
	winActive("wc_r2_custom_look", r2WinOn)
	winActive("wc_r2_dialogs", r2WinOn)
	winActive("wc_r2_events", r2WinOn)
	winActive("wc_r2_activities", r2WinOn)
	winActive("wc_r2_feature_help", r2WinOn)	
	winActive("wc_r2_connect", not r2WinOn)	
	winActive("wc_r2_session_browser", not r2WinOn)	
	winActive("wc_r2_scenario_control", true)	
	winActive("wc_r2_player_tracking", r2WinOn)
end
