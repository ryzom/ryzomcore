-- In this file we define functions that serves for help windows

------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (help==nil) then
	help = {}
end

------------------------------------------------------------------------------------------------------------
--
function help:closeCSBrowserHeader()
	local ui = getUI('ui:interface:cs_browser')

	-- save size
	ui_cs_browser_h = ui.h
	ui_cs_browser_w = ui.w

	-- reduce window size
	ui.pop_min_h = 32
	ui.h = 0
	ui.w = 216
end

------------------------------------------------------------------------------------------------------------
--
function help:openCSBrowserHeader()
	local ui = getUI('ui:interface:cs_browser')
	ui.pop_min_h = 96

	-- set size from saved values
	if (ui_cs_browser_h ~= nil) then
		ui.h = ui_cs_browser_h
	end

	 if (ui_cs_browser_w ~= nil) then
		ui.w = ui_cs_browser_w
	end
end

function help:skipTutorial()
	WebQueue:push("https://app.ryzom.com/app_arcc/outgame_rpbg.php?action=skip")
	setDbProp("UI:SAVE:SKIP_WELCOME", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_SETS", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_INFO_PLAYER_JOURNAL", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_PLAYER", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_COMPASS", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_TARGET", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_MAIN_CHAT", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_INVENTORY", 1)
	setDbProp("UI:SAVE:TUTORIAL_ACTIVE_ENCYCLOPEDIA", 1)
	setDbProp("UI:SAVE:ISENABLED:AROUND_ME", 1)
	setDbProp("UI:SAVE:ISENABLED:REGION_CHAT", 1)
	setDbProp("UI:SAVE:ISENABLED:DYNAMIC_CHAT0", 1)
	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 1 then
		return
	end
	setDbProp("UI:SAVE:SKIP_TUTORIAL", 1)
	runAH(nil, "milko_menu_do_reset_interface", "")
	removeOnDbChange(getUI("ui:interface:milko_pad"), "@UI:VARIABLES:CURRENT_SERVER_TICK")
end

function help:displayWelcome()
	-- Hide not ready windows
	if not game:areInSilan() then
		return
	end

	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 1 then
		return
	end

	game:keepExpandCapWeb(true)

	if getDbProp("UI:SAVE:TUTORIAL_ACTIVE_MAIN_CHAT") == 0 then
		setDbProp("UI:SAVE:ISENABLED:AROUND_ME", 0)
		setDbProp("UI:SAVE:ISENABLED:REGION_CHAT", 0)
		setDbProp("UI:SAVE:ISENABLED:DYNAMIC_CHAT0", 0)
	end

	if getDbProp("UI:SAVE:TUTORIAL_ACTIVE_SETS") == 0 then
		getUI("ui:interface:gestionsets").active=false
	end

	if getDbProp("UI:SAVE:TUTORIAL_ACTIVE_INFO_PLAYER_JOURNAL") == 0 then
		getUI("ui:interface:info_player_journal").active=false
	end

	if getDbProp("UI:SAVE:TUTORIAL_ACTIVE_PLAYER") == 0 then
		getUI("ui:interface:player").active=false
	end

	if getDbProp("UI:SAVE:TUTORIAL_ACTIVE_TARGET") == 0 then
		getUI("ui:interface:target").active=false
	end


	getUI("ui:interface:milko_pad:content:mode_button").active=false
	setDbProp("UI:SAVE:MK_MODE", 5)
	game:activeMilkoKey(1, false)
	game:activeMilkoKey(2, false)
	game:activeMilkoKey(3, false)
	game:activeMilkoKey(4, getDbProp("UI:SAVE:TUTORIAL_ACTIVE_INVENTORY") == 1) -- bag
	game:activeMilkoKey(5, getDbProp("UI:SAVE:TUTORIAL_ACTIVE_ENCYCLOPEDIA") == 1) -- ency
	game:activeMilkoKey(6, true) -- mission
	game:activeMilkoKey(7, true) -- help
	game:activeMilkoKey(8, true) -- system
	game:activeMilkoKey(9, true) -- quit
	game:resizeMilkoPad()
	getUI("ui:interface:milko_pad").y = 5
end

function help:initWelcome()
	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 1 then return end

	local root = getUI("ui:interface")
	local wins = {"compass", "player", "target", "welcome_info", "encyclopedia", "milko_pad:content:mode_button"}

	for _, win in ipairs(wins) do
		local ui = getUI("ui:interface:"..win, false)
		if ui and ui.active and getDbProp("UI:SAVE:TUTORIAL_ACTIVE_"..win:upper()) == 0 then
			ui.active = false
			if win == "milko_pad:content:mode_button" then
				game:resizeMilkoPad()
			end
		end
	end
	addOnDbChange(getUI("ui:interface:milko_pad"), "@UI:VARIABLES:CURRENT_SERVER_TICK", "help:checkCapActive()")
end

function help:checkCapActive()
	local cap = getUI("ui:interface:cap")
	if cap.active == false then
		cap.active = true
	end
end

function help:continueLesson(id, url)
	getUI("ui:interface:ArkLessonWin"..id).active = true
	webig:checkUrl(url)
end

function help:checkSkipTutorial()
	local skip_tutorial = getDbProp("UI:SAVE:SKIP_TUTORIAL")
	if skip_tutorial == 0 then
		debug("Skip Tutorial")
		setDbProp("UI:SAVE:SKIP_TUTORIAL", 1)
		help:skipTutorial()
	end
end

function help:checkSkipWelcomeTutorial()
	getUI("ui:interface:player").active = true
	getUI("ui:interface:target").active = true

	local skip_welcome = getDbProp("UI:SAVE:SKIP_WELCOME")
	local skip_tutorial = getDbProp("UI:SAVE:SKIP_TUTORIAL")
	debug("Welcome : "..tostring(skip_welcome).." Tuto : "..tostring(skip_tutorial))
	if skip_welcome == 0 or skip_tutorial == 0 then
		debug("Skip welcome & tutorial")
		help:skipTutorial()
	else
		debug("Already skiped welcome & tutorial")
		function help:checkCapActive()
		end
	end
end

function help:updateRpbg(slot)
	local rpbg_key_file = io.open("save/rpbg_"..slot..".key", "rb")
	if rpbg_key_file then
		debug("Setup RP BG save/rpbg_"..slot..".key")
		rpbg_key = rpbg_key_file:read()
		rpbg_key_file:close()
		os.remove("save/rpbg_"..slot..".key")
		WebQueue:push("https://app.ryzom.com/app_arcc/outgame_rpbg.php?action=save&key="..rpbg_key)
	end
end

function help:checkTutorialMilkoPad()
	getUI("ui:interface:cap_popup").active = false
	removeOnDbChange(getUI("ui:interface"), "@UI:SAVE:MK_MODE")
	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 0 then
		addOnDbChange(getUI("ui:interface"), "@UI:SAVE:MK_MODE", "game:resizeMilkoPad()")
	end
end

-- VERSION --
RYZOM_HELP_VERSION = 335
