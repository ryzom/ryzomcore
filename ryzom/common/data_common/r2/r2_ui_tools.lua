----------
-- TOOL --
----------
-- Manage display of current selected tool into the ui
------------------------------------------------------------------------------------------------------------


-- base Name for the tool bar in the ui
local toolBarPath		= "ui:interface:r2ed_toolbar:"
local geoToolPath		= "ui:interface:r2ed_palette:content:sbtree_geo:geo_features:enclosing:geo_feature_list:"
local featuresToolPath  = "ui:interface:r2ed_palette:content:sbtree_features:features:enclosing:feature_list:"


r2.ToolUI =
{
	-- ref to current hightlighted tool in the ui	
	ActiveToolUI = nil,

	-- For named tools, map the name to the ui path
	ToolNameToUIPath = 
	{
		selectMove					= toolBarPath     .. "r2ed_tool_select",
		selectRotate				= toolBarPath     .. "r2ed_tool_rotate",
		teleport					= toolBarPath     .. "r2ed_tool_teleport",
		drawRoad					= geoToolPath     .. "r2ed_tool_draw_road:tool",
		drawRegion					= geoToolPath     .. "r2ed_tool_draw_region:tool",
		createFeatureBanditCamp		= featuresToolPath .. "r2ed_create_feature_bandit_camp:tool",
		createFeatureTimer			= featuresToolPath .. "r2ed_create_feature_timer:tool",
		createFeatureTimeTrigger	= featuresToolPath .. "r2ed_create_feature_time_trigger:tool",
		createFeatureFauna			= featuresToolPath .. "r2ed_create_feature_fauna:tool",
		createFeatureBossSpawner	= featuresToolPath .. "r2ed_create_feature_boss_spawner:tool",
		createFeatureZoneTrigger	= featuresToolPath .. "r2ed_create_feature_zone_trigger:tool",
		createFeatureUserTrigger	= featuresToolPath .. "r2ed_create_user_trigger:tool",
		createFeatureEasterEgg		= featuresToolPath .."r2ed_create_easter:tool",
		createFeatureLootSpawner	= featuresToolPath .."r2ed_create_feature_loot_spawner:tool",
		createDialog				= featuresToolPath .."r2ed_create_dialog:tool",
		createFeatureGiveItem		= featuresToolPath .."r2ed_create_feature_give_item:tool",
		createFeatureRequestItem	= featuresToolPath .."r2ed_create_feature_request_item:tool",
		createFeatureTalkTo			= featuresToolPath .."r2ed_create_feature_talk_to:tool",
		createFeatureAmbush			= featuresToolPath.."r2ed_create_feature_ambush:tool",
		createFeatureTimedSpawner	= featuresToolPath.."r2ed_create_feature_timed_spawner:tool",
		createFeatureManHunt		= featuresToolPath.."r2ed_create_feature_man_hunt:tool",

		
	}
}

------------------------------------------------------------------------------------------------------------
-- PRIVATE : hightlight a tool button by a ref on the tool ui
function r2.ToolUI:highlightToolUI(tool, hightlighted)
   if not tool then return end
   tool:find("selected").active = hightlighted
   tool:find("unselected").active = not hightlighted
end

------------------------------------------------------------------------------------------------------------
-- Get reference to a tool in the ui by its name (for named tools)
function r2.ToolUI:getToolUIByName(toolName)
	if toolName == "" then return nil end
   -- get Name of the ui button from the tool Name
	local uiPath = self.ToolNameToUIPath[toolName]
	if uiPath == nil then
		debugWarning("Can't find ui for tool : " .. tostring(toolName))
		return nil
    end	
	return getUI(uiPath)
end

------------------------------------------------------------------------------------------------------------
-- Get the current highlighted tool
function r2.ToolUI:getActiveToolUI()
	if self.CurrentToolUI and not self.CurrentToolUI.isNil then 
		return self.CurrentToolUI
	else
		return nil
	end
end


------------------------------------------------------------------------------------------------------------
-- Set the current highlighted tool
function r2.ToolUI:setActiveToolUI(tool)
	if self:getActiveToolUI() then
		self:highlightToolUI(self:getActiveToolUI(), false)
	end
	self.CurrentToolUI = tool
	self:highlightToolUI(tool, true)
end


------------------------------------------------------------------------------------------------------------
-- This function will be called by the framework when it wants to highlight a tool in the 
-- ui. It doesn't change the actual tool in the editor, just the ui
function r2.ToolUI:setActiveToolUIByName(toolName)
    self:setActiveToolUI(self:getToolUIByName(toolName))
end

------------------------------------------------------------------------------------------------------------
function r2.ToolUI:updateTooltip(onClickL, paramsL)	
   onClickL = defaulting(onClickL, getUICaller().onclick_l)
   paramsL = defaulting(paramsL, getUICaller().params_l)
   local expr = string.format("getKey('%s', '%s')", onClickL, paramsL)
   local keyName = ucstring(runExpr(expr))
	if keyName == i18n.get("uiNotAssigned") then
		-- no associated key
		setContextHelpText(getUICaller().tooltip)
	else
		setContextHelpText(concatUCString(getUICaller().tooltip, "@{6F6F} (", keyName, ")"))
	end
end

----------------------------------------------------------------------------
-- Update the undo / redo buttons grayed states
function r2.ToolUI:updateUndoRedo()	
	local toolbar = getUI("ui:interface:r2ed_toolbar")	
	toolbar:find("r2ed_tool_undo").unselected.active = r2:canUndo()
	toolbar:find("r2ed_tool_undo").disabled.active = not r2:canUndo()
	toolbar:find("r2ed_tool_redo").unselected.active = r2:canRedo()
	toolbar:find("r2ed_tool_redo").disabled.active = not r2:canRedo()	
end


----------------------------------------------------------------------------
-- Update the toggle windows buttons
function r2.ToolUI:updateToggleWindowButtons()
	local windowsBar = getUI("ui:interface:r2ed_windows_bar")	
	local active = (r2.Mode == "Edit")
	windowsBar.active = active
	if active then
		windowsBar:find("r2ed_tool_map_window").selected.active = getUI("ui:interface:map").active
		windowsBar:find("r2ed_tool_map_window").unselected.active = not getUI("ui:interface:map").active
		windowsBar:find("r2ed_tool_scenario_window").selected.active = getUI("ui:interface:r2ed_scenario").active
		windowsBar:find("r2ed_tool_scenario_window").unselected.active = not getUI("ui:interface:r2ed_scenario").active
		windowsBar:find("r2ed_tool_palette_window").selected.active = getUI("ui:interface:r2ed_palette").active
		windowsBar:find("r2ed_tool_palette_window").unselected.active = not getUI("ui:interface:r2ed_palette").active
	end
end

----------------------------------------------------------------------------
-- Update the toggle windows buttons
function r2.ToolUI:updateToggleWindowDMButtons()
	local windowsDMBar = getUI("ui:interface:r2ed_windows_dm_bar")	
	windowsDMBar:find("r2ed_live").selected.active = getUI("ui:interface:r2ed_scenario_control").active
	windowsDMBar:find("r2ed_live").unselected.active = not getUI("ui:interface:r2ed_scenario_control").active
	windowsDMBar:find("player_control").selected.active = getUI("ui:interface:ring_chars_tracking").active
	windowsDMBar:find("player_control").unselected.active = not getUI("ui:interface:ring_chars_tracking").active
end


