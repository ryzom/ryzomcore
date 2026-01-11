---------------------------------------------------------------------------------------------
-- UI for act management by the DM (animation mode)



-- class for runtime acts ui
r2.ui.AnimActs = 
{
	ChosenActIndex = -1, -- the last act that was chosen in the act popup menu
}

------------
-- PUBLIC --
------------

----------------------------------------------------------------------------------------------
-- Called by the ui when the user clicks on the 'acts' button in animation mode
-- This will pop a menu to choose between available acts
function r2.ui.AnimActs:popMenu()
	local menuName = "ui:interface:r2ed_anim_acts_menu"	
	local menu = getUI(menuName)
	local rootMenu = menu:getRootMenu()	
	rootMenu:reset()
	local acts = self:getActTable()
	--
	if not acts then return end
	for k = 2, table.getn(acts) do
      if acts[k].Name == "" then
         r2.ScratchUCStr = i18n.get("uiR2EDUnamedAct")
      else
         r2.ScratchUCStr:fromUtf8(acts[k].Name)
      end
		rootMenu:addLine(r2.ScratchUCStr, "lua", "r2.ui.AnimActs:chooseAct(" .. tostring(k) .. ")", "")
	end
	launchContextMenuInGame(menuName)
	local but = getUI("ui:interface:r2ed_testbar"):find("r2ed_anim_acts")
	but:updateCoords()
	menu.x = but.x_real
	menu.y = but.y_real + but.w_real
	rootMenu.x = 0
	rootMenu.y = 0
	menu:updateCoords()
end

----------------------------------------------------------------------------------------------
-- called by the acts context menu when one act has been chosen
function r2.ui.AnimActs:chooseAct(index)

	-- see if current act differs in location or season

	--  acts[1] <==> permanent content
	if index == 1 then return end
	self.ChosenActIndex =  index - 1 

	local acts = self:getActTable()


	local currentActIndex = r2.getCurrentActIndex() + 1
	
	if acts[index].LocationId   ~= acts[currentActIndex].LocationId  then
		validMessageBox(i18n.get("uiR2EDActChangeWarning"), "lua", "r2.ui.AnimActs:validateActChange()", "", "", "ui:interface")
		return
	end

	self:validateActChange()
-- 	debugInfo(tostring(acts[index].Island))
-- 	debugInfo(tostring(r2:getCurrentIslandName()))
-- 	debugInfo(tostring(acts[index].Season))
-- 	debugInfo(tostring(getServerSeason()))
-- 	local targetSeason
--    	 if acts[index].Season == 0 then
-- 		targetSeason = getAutoSeason()
-- 	else
-- 		targetSeason = acts[index].Season
-- 	end
-- 
-- 	if acts[index].Island ~= r2:getCurrentIslandName() or
-- 		targetSeason ~= computeCurrSeason() then
-- 		-- warn the user that changing to this act will cause a teleport
-- 		validMessageBox(i18n.get("uiR2EDActChangeWarning"), "lua", "r2.ui.AnimActs:validateActChange()", "", "", "ui:interface")
-- 		return
-- 	end
-- 	self:validateActChange()
end

----------------------------------------------------------------------------------------------
function r2.ui.AnimActs:validateActChange()
	r2.requestStartAct(self.ChosenActIndex)	
end

-------------
-- PRIVATE --
-------------

-- private : Place holder for local mode : list of acts
--
---- Season enum is as follow :
-- 0 -> auto, computed locally from the current day (or not received from server yet)
-- 1 -> server force spring 
-- 2 -> server force summer
-- 3 -> server force autumn
-- 4 -> server force winter
local dummyActsList =
{
	{ Name = "Act 1", Island = "uiR2_Lakes01", Season=0  },
	{ Name = "Act 2", Island = "uiR2_Desert01", Season=1 },
	{ Name = "Act 3", Island = "uiR2_Lakes10",  Season=2 },
	{ Name = "Act 4", Island = "uiR2_Lakes11",  Season=3 },
	{ Name = "Act 5", Island = "uiR2_Lakes12",  Season=4 }
}

----------------------------------------------------------------------------------------------
function r2.ui.AnimActs:getActTable()
--	if false then -- config.Local == 1 then
--		return dummyActsList
--	else
--		return r2.AnimGlobals.Acts
--	end
	return r2.AnimGlobals.Acts
end
