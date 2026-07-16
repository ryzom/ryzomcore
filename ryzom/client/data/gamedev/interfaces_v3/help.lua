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
	setDbProp("UI:SAVE:SKIP_TUTORIAL", 1)
end

function help:displayWelcome()
end

function help:initWelcome()
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

function help:checkTutorialMilkoPad()
	getUI("ui:interface:cap_popup").active = false
	removeOnDbChange(getUI("ui:interface"), "@UI:SAVE:MK_MODE")
	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 0 then
		addOnDbChange(getUI("ui:interface"), "@UI:SAVE:MK_MODE", "game:resizeMilkoPad()")
	end
end


-- VERSION --
FILE_HELP_VERSION = 182