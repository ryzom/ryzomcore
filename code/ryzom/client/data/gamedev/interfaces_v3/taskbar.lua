
------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

------------------------------------------------------------------------------------------------------------
-- 
function game:getMilkoTooltipWithKey(prop, tooltip, tooltip_pushed, name, param)
	local tt;
	
	-- Check if button is toggled and choose the good tooltip
	if (prop ~= '' and tooltip_pushed ~= '') then
		local db = getDbProp(prop);
		if (db == 1) then
			tt = tooltip_pushed;
		else
			tt = tooltip;
		end
	else
		tt = tooltip;
	end
	
	-- Get key shortcut
	local text = i18n.get(tt);
	local key = runExpr('getKey(\'' .. name .. '\',\'' .. param .. '\',1)');
	
	if (key ~= nil and key  ~= '') then
		key = ' @{2F2F}(' .. key .. ')';
		text = concatUCString(text, key);
	end
	
	setContextHelpText(text);
end

function game:taskbarDisableTooltip(ui)
	local uiGroup = getUI(ui);
	disableContextHelpForControl(uiGroup);
end