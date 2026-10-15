--------------------------------------------------------------------------------------
-- Managment of the user trigger menu in anim mode


-- the anim triggers class
r2.ui.AnimUserTriggers = 
{
}

------------
-- PUBLIC --
------------

--------------------------------------------------------------------------------------
-- Called when the user click on the triggers icon -> pop the menu with the trigger list
function r2.ui.AnimUserTriggers:popMenu()	
	local menuName = "ui:interface:r2ed_anim_triggers_menu"	
	local menu = getUI(menuName)
	local rootMenu = menu:getRootMenu()	
	rootMenu:reset()
	local triggers = self:getTriggerTable()	
	if not triggers then return end
	for k = 1, table.getn(triggers) do
		if triggers[k].Act == 0 or triggers[k].Act == r2.getCurrentActIndex() then
			r2.ScratchUCStr:fromUtf8(triggers[k].Name)
			rootMenu:addLine(r2.ScratchUCStr, "lua",
							 string.format("r2.ui.AnimUserTriggers:fireTrigger(%d, %d)", triggers[k].Act, triggers[k].Id), "")
		end
	end
	if rootMenu:getNumLine() == 0 then
		rootMenu:addLine(i18n.get("uiR2EDNoTriggersDefined"), "", "", "")
	end
	launchContextMenuInGame(menuName)
	local but = getUI("ui:interface:r2ed_testbar"):find("r2ed_anim_triggers")
	but:updateCoords()
	menu.x = but.x_real
	menu.y = but.y_real + but.w_real
	rootMenu.x = 0
	rootMenu.y = 0
	menu:updateCoords()
end

--------------------------------------------------------------------------------------
-- Called by the ui when the user has chosen a trigger in the menu
function r2.ui.AnimUserTriggers:fireTrigger(triggerAct, triggerId)
	r2.triggerUserTrigger(triggerAct, triggerId)
	local trig = self:findTrigger(triggerAct, triggerId)
	if trig then
		r2.ScratchUCStr:fromUtf8(trig.Name)
		displaySystemInfo(concatUCString(i18n.get("uiR2EDTriggering"), r2.ScratchUCStr), "BC")	
	end
end

-------------
-- PRIVATE --
-------------

-- private : dummy list of triggers -> for local client mode
local dummyUserTriggerList = 
{
	{ Name="Trigger1", Act=1, Id=1 },
	{ Name="Trigger2", Act=1, Id=2 },
	{ Name="Trigger3", Act=1, Id=3 }
}


--------------------------------------------------------------------------------------
-- Get the table describing all the user triggers
function r2.ui.AnimUserTriggers:getTriggerTable()
	if config.Local == 1 then
		return dummyUserTriggerList
	else
		return r2.AnimGlobals.UserTriggers
	end
end

--------------------------------------------------------------------------------------
-- Find a trigger from an act / id pair, returns it definition as a table
function r2.ui.AnimUserTriggers:findTrigger(act, id)
	local triggerTable = self:getTriggerTable()
	for k, v in pairs(triggerTable) do
		if v.Act == act and v.Id == id then return v end
	end
end
