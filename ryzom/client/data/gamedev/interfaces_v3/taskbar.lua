
------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game = {}
end

------------------------------------------------------------------------------------------------------------
--
function game:getMilkoTooltipWithKey(prop, tooltip, tooltip_pushed, name, param)
	local tt

	-- Check if button is toggled and choose the good tooltip
	if (prop ~= '' and tooltip_pushed ~= '') then
		local db = getDbProp(prop)
		if (db == 1) then
			tt = tooltip_pushed
		else
			tt = tooltip
		end
	else
		tt = tooltip;
	end

	-- Get key shortcut
	local text = i18n.get(tt)
	local key = runExpr('getKey(\'' .. name .. '\',\'' .. param .. '\',1)')

	if (key ~= nil and key  ~= '') then
		key = ' @{2F2F}(' .. key .. ')'
		text = concatUCString(text, key)
	end

	setContextHelpText(text)
end

function game:taskbarDisableTooltip(ui)
	local uiGroup = getUI(ui)
	disableContextHelpForControl(uiGroup)
end

function game:activeMilkoKey(key, status)
	local ui = getUI("ui:interface:milko_pad:content:mode5:window"..tostring(key))
	if ui then
		ui.active = status
	end
end

function game:resizeMilkoPad()
	if getDbProp("UI:SAVE:SKIP_TUTORIAL") == 1 then
		removeOnDbChange(getUI("ui:interface"), "@UI:SAVE:MK_MODE")
		game:activeMilkoKey(1, true)
		game:activeMilkoKey(2, true)
		game:activeMilkoKey(3, true)
		game:activeMilkoKey(4, true)
		game:activeMilkoKey(5, true)
		game:activeMilkoKey(6, true)
		game:activeMilkoKey(7, true)
		game:activeMilkoKey(8, true)
		game:activeMilkoKey(9, true)
		getUI("ui:interface:milko_pad:content:mode_button").active = true
		setDbProp("UI:SAVE:MK_MODE", 1)
		setDbProp("UI:SAVE:MK_MODE", 5)
		getUI("ui:interface:milko_pad:content:mode5").x = 16
		return
	end

	local mk_mode = getDbProp("UI:SAVE:MK_MODE")
	if mk_mode == 5 then
		local ui = getUI("ui:interface:milko_pad:content:mode_button")
		local offset = 40
		if ui and ui.active then
			offset = 72
			getUI("ui:interface:milko_pad:content:mode5").x = 16
		else
			getUI("ui:interface:milko_pad:content:mode5").x = 0
		end
		local total = -1
		for i=1,9 do
			ui = getUI("ui:interface:milko_pad:content:mode5:window"..tostring(i))
			if ui and ui.active then
				total = total + 1
			end
		end
		ui = getUI("ui:interface:milko_pad")
		local last_w = ui.w
		ui.w = offset + (24*total)
		ui.x = ui.x + last_w - ui.w
	end
end

function game:updateMilkoKey(key, status)
	game:activeMilkoKey(key, status)
	game:resizeMilkoPad()
end


-- VERSION --
RYZOM_TASKBAR_VERSION = 10469