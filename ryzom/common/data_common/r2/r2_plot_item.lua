-- plot items. They are global to the scenario and can be given
-- by the DM to players

-- ////////////////////////////////
-- // PLOT ITEM CLASS DEFINITION //
-- ////////////////////////////////

local maxNumPlotItems = tonumber(getDefine("r2ed_max_num_plot_items"))
local maxNumPlotItemSheets = tonumber(getDefine("r2ed_max_num_plot_item_sheets"))

local plotItem = 
{
	BaseClass          = "BaseClass",
	Name			   = "PlotItem",
	DisplayerUI		   = "R2::CDisplayerLua",	-- name of the C++ class that displays this object in the ui
	DisplayerUIParams  = "plotItemDisplayer",	-- parameters passed to the ui displayer when it is created
   Menu="ui:interface:r2ed_base_menu",
	Prop = 
	{
		{ Name="SheetId", Type="Number", Visible=false },
		{ Name="Name", Type="String", MaxNumChar="32", MaxNumChar="32"},
		{ Name="Desc", Type="String" },
		{ Name="Comment", Type="String" },
	},
	-- rollout header : allows to view & change the item sheet
	PropertySheetHeader = 
	[[
		<ctrl type="sheet" id="item_sheet" 
		 onclick_r="" 
		 value="LOCAL:R2:CURR_PLOT_ITEM" 
		 dragable="false"
         use_quantity="false"
		 use_quality="false"		
		 posref="TL TL" x="0" 
		 onclick_l="lua"
		 params_l="r2.PlotItemsPanel:changeItem(r2:getSelectedInstance().IndexInParent)"
		/>
	]],
}

-- from base class
function plotItem.isGlobalObject(this)
	return true
end


local plotItemNamePrefix = i18n.get("uiR2EDPlotItemNamePrefix")


function plotItem.getDisplayName(this)
   r2.ScratchUCStr:fromUtf8(this.Name)
   return concatUCString(plotItemNamePrefix, r2.ScratchUCStr)
end

function plotItem.isNextSelectable(this)
	return true
end

---------------------------------------------------------------------------------------------------------
-- get select bar type
function plotItem.SelectBarType(this)
	return i18n.get("uiR2EDPlotItems"):toUtf8()
end

---------------------------------------------------------------------------------------------------------
-- get first parent that is selectable in the select bar
function plotItem.getFirstSelectBarParent(this)
	return r2:getCurrentAct()
end

function plotItem.getSelectBarIcon(this)
	return "ICO_mission_purse.tga"
end

r2.registerComponent(plotItem)



-- /////////////
-- // PRIVATE //
-- /////////////

local itemUCName = ucstring() -- keep a ucstring to avoid creation of string on the fly
local itemUCDesc = ucstring() -- keep a ucstring to avoid creation of string on the fly
local itemUCComment = ucstring() -- keep a ucstring to avoid creation of string on the fly

local plotItemSheetToDBPath = {}

local plotItemSheetNbRef = {} -- for each plot item sheet, store the number of references on it
							  -- each sheet must be use 0 or 1 time in the scenario
							  -- a ref count > 1 may happen during concurrent edition
							  -- in which case the user should be warned that some items are duplicated

local validItemColor = CRGBA(255, 255, 255)
local duplicatedItemColor = CRGBA(255, 0, 0)

-- /////////////////////////
-- // PLOT ITEM DISPLAYER //
-- /////////////////////////

-- Plot item displayer, shared between items
r2.PlotItemDisplayerCommon = {}



----------------------------------------------------------------------------
-- update name for tooltip display + availability
function r2.PlotItemDisplayerCommon:updateNameAndAvailability(instance, sheetId)
	if plotItemSheetNbRef[sheetId] == 0 then
		r2:setPlotItemInfos(sheetId, i18n.get("uiR2EDPlotItemDefaultName"), ucstring(), ucstring())			
		setDbProp(plotItemSheetToDBPath[sheetId], sheetId) -- available again
	elseif plotItemSheetNbRef[sheetId] > 1 then
		-- duplicated slot, may happen during concurrent edition (bad luck)
		r2:setPlotItemInfos(sheetId, i18n.get("uiR2EDDuplicatedPlotItemName"), ucstring(), ucstring())			
		setDbProp(plotItemSheetToDBPath[sheetId], 0) -- available again
		r2.PlotItemDisplayerCommon:touch() -- force to refresh the icon display
	else
		itemUCName:fromUtf8(instance.Name)
		itemUCDesc:fromUtf8(instance.Desc)
		itemUCComment:fromUtf8(instance.Comment)
		r2:setPlotItemInfos(sheetId, itemUCName, itemUCDesc, itemUCComment)		
		setDbProp(plotItemSheetToDBPath[sheetId], 0) -- available again
	end
end

----------------------------------------------------------------------------
-- add a reference on a sheet id for a r2 plot item
function r2.PlotItemDisplayerCommon:addRef(sheetId)
	assert(plotItemSheetNbRef[sheetId] >= 0)
	plotItemSheetNbRef[sheetId] = plotItemSheetNbRef[sheetId] + 1
end

----------------------------------------------------------------------------
-- remove a reference on a sheet id for a r2 plot item
-- add a reference on a sheet id
function r2.PlotItemDisplayerCommon:decRef(sheetId)
	assert(plotItemSheetNbRef[sheetId] > 0)
	plotItemSheetNbRef[sheetId] = plotItemSheetNbRef[sheetId] - 1
end


----------------------------------------------------------------------------
-- show the property window and allows to edit the plot item name
function r2.PlotItemDisplayerCommon:editPlotItemName(instance)
	r2:setSelectedInstanceId(instance.InstanceId)
	r2:showProperties(instance)	
	local propWindow = r2.CurrentPropertyWindow
	-- tmp : quick & dirty access to the widget ...
	if propWindow.active then
     propWindow:blink(1)
		local editBox = propWindow:find("Name"):find("eb")
		if editBox then
			setCaptureKeyboard(editBox)
			editBox:setSelectionAll()
		end
	end	
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:onPostCreate(instance)
	self:touch()	
	setDbProp(plotItemSheetToDBPath[instance.SheetId], 0)  -- not available for other plot items
	-- if created by this client, prompt to enter the name
	if instance.User.SelfCreate then			
		instance.User.SelfCreate = nil
		self:editPlotItemName(instance)		
	end
	--
	if instance.User.AddedMsg then
		instance.User.AddedMsg = nil
		displaySystemInfo(i18n.get("uiR2EDPlotItemAdded"), "BC")      
      r2.PlotItemsPanel:pop()
	end
	--	
    instance.User.OldSheetId = instance.SheetId	
	self:addRef(instance.SheetId)
	self:updateNameAndAvailability(instance, instance.SheetId)
	if instance == r2:getSelectedInstance() then
		self:updatePropWindow(instance)
	end
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:onErase(instance)   
	self:touch()
	self:decRef(instance.SheetId)	
	self:updateNameAndAvailability(instance, instance.SheetId)	
	if instance == r2.PlotItemsPanel.ItemSelectCaller then
		-- discard item selection window if it was opened
		getUI("ui:interface:r2ed_choose_plot_item").active = false		
		self.ItemSelectCaller = nil
	end
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:onSelect(instance, selected)
   self:touch()
   if selected then
      self:updatePropWindow(instance)
   end
   getUI("ui:interface:r2ed_scenario"):find("delete_plot_item").frozen = not selected
   getUI("ui:interface:r2ed_scenario"):find("plot_item_properties").frozen = not selected   
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:updatePropWindow(instance)
	-- change the icon displayed in the property sheet	
	setDbProp("LOCAL:R2:CURR_PLOT_ITEM:SHEET", instance.SheetId)      
	-- get the property window, maybe not shown yet ...
	local propWindow = getUI(r2:getDefaultPropertySheetUIPath("PlotItem"))
	-- update color of the sheet in the property window
	if propWindow then
	 self:updateSheetColor(propWindow:find("item_sheet"), instance.SheetId)
	end
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:onAttrModified(instance, attributeName, indexInArray)
   self:touch()   
   if attributeName == "SheetId" then
	  self:decRef(instance.User.OldSheetId)
	  self:updateNameAndAvailability(instance, instance.User.OldSheetId)
	  self:addRef(instance.SheetId)
	  instance.User.OldSheetId = instance.SheetId
	  self:updateNameAndAvailability(instance, instance.SheetId)
	  if instance == r2:getSelectedInstance() then
		self:updatePropWindow(instance)
	  end
   end
   if attributeName == "Name" or attributeName == "Desc" or attributeName == "Comment" then
		self:updateNameAndAvailability(instance, instance.SheetId)
	end
end


----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:touch()
   -- update will be done in main loop only
   r2.UIMainLoop.PlotItemsModified = true
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:updateSheetColor(slot, sheetId)
	if plotItemSheetNbRef[sheetId] and plotItemSheetNbRef[sheetId] > 1 then
		slot.color = duplicatedItemColor
	  else
		slot.color = validItemColor
	end
end

----------------------------------------------------------------------------
function r2.PlotItemDisplayerCommon:updateAll()
	if r2.Mode ~= "Edit" then return end
	r2.PlotItemsPanel.Locked = true
	local window = getUI("ui:interface:r2ed_scenario")
	local groupListSheet = window:find("plot_items_list")   
	-- update db sheets for items display	
	local index = 0
	for k, v in specPairs(r2.Scenario.PlotItems) do		
		setDbProp("LOCAL:R2:PLOT_ITEMS:" .. tostring(index) ..":SHEET", v.SheetId)
		local slot = groupListSheet["item_" .. tostring(index)]
		slot.but.pushed = (v == r2:getSelectedInstance()) -- update selection      
      itemUCName:fromUtf8(v.Name)
	   self:updateSheetColor(slot.sheet, v.SheetId)
      slot.t.uc_hardtext = itemUCName
      slot.t.global_color = true
		slot.active=true
		index = index + 1
	end
	-- last slot is the "create new item" slot
	if index <= maxNumPlotItems - 1 then
		-- special sheet here for new item creation		
		--setDbProp("LOCAL:R2:PLOT_ITEMS:" .. tostring(index) ..":SHEET", getSheetId("r2_create_new_plot_item.sitem"))		
		setDbProp("LOCAL:R2:PLOT_ITEMS:" .. tostring(index) ..":SHEET", 0)
		local slot = groupListSheet["item_" .. tostring(index)]
		slot.but.pushed = false
		slot.active=true
		slot.t.uc_hardtext = i18n.get("uiR2EDCreateNewItem")
        slot.t.global_color = false
		slot.sheet.color = validItemColor
		window:find("new_plot_item").frozen = false
		index = index + 1
	else
		window:find("new_plot_item").frozen = true
	end
	-- remove remaining sheets	
	while index < maxNumPlotItems do		
		setDbProp("LOCAL:R2:PLOT_ITEMS:" .. tostring(index) ..":SHEET", 0)
		local slot = groupListSheet["item_" .. tostring(index)]
		slot.active=false		
		index = index + 1
	end
	r2.PlotItemsPanel.Locked = false
end


-- Displayer ctor, just return a ref on the shared displayer
-- Works because we don't store per displayer infos
function r2:plotItemDisplayer()
	return r2.PlotItemDisplayerCommon
end

-- ///////////////////////////
-- // PLOT ITEM UI HANDLING //
-- ///////////////////////////


-- UI part
r2.PlotItemsPanel = 
{
	Locked = false, -- flag to avoid recursive selection
	CreateMode = "", 	
	ItemSelectCaller = nil, -- last plot item that called the 'select item' window (or nil if a new item is created)
	TargetRefId =  -- if CreateMode is "CreateNewAndAffectToRefId", gives the name of the refid to which the item should be affected
	{
		Name="",
		ParentInstanceId = ""
	}
}


----------------------------------------------------------------------------
function r2.PlotItemsPanel:selectItem(index)
	if r2.PlotItemsPanel.Locked then return end -- this is a false selection 											    
	-- we can't use the sheet here, because concurrent edition may lead to duplicated sheets
	-- here, a case that wecan resolve only when the scenario is about to be launched...	
	if index > r2.Scenario.PlotItems.Size then		
		debugInfo("bad item index")
	end
	if index == r2.Scenario.PlotItems.Size then
		-- this is the creation slot
		r2:setSelectedInstanceId("")
		enableModalWindow(getUICaller(), "ui:interface:r2ed_choose_plot_item")
		getUICaller().parent.but.pushed= false
		self.CreateMode = "CreateNew"
		self.ItemSelectCaller = nil
	else
		-- a new item is selected for real
		r2:setSelectedInstanceId(r2.Scenario.PlotItems[index].InstanceId)		
		r2.PlotItemDisplayerCommon:touch()
	end
end

----------------------------------------------------------------------------
-- for plot item ref. widget (see r2.PlotItemsWidget) : pop the item creation
-- window, then affect the created widget to a refid (with name 'refIdName' in object with id 'targetInstanceId')
function r2.PlotItemsPanel:createNewItemAnAffectToRefId(instanceId, refIdName)
	if r2.Scenario.PlotItems.Size >= maxNumPlotItems then
		displaySystemInfo(i18n.get("uiR2EDNoMorePlotItems"), "BC")
      self:pop()
		return
	end
   enableModalWindow(getUICaller(), "ui:interface:r2ed_choose_plot_item")	
	self.CreateMode = "CreateNewAndAffectToRefId"	
	self.ItemSelectCaller = nil
	self.TargetRefId.Name = refIdName
	self.TargetRefId.ParentInstanceId = instanceId
end

----------------------------------------------------------------------------
function r2.PlotItemsPanel:changeItem(index)
   self:selectItem(index)	
   -- this is the creation slot
   if index == r2.Scenario.PlotItems.Size then	
      self.CreateMode = "CreateNew"	  
	  self.ItemSelectCaller = nil
   else
      self.CreateMode = "Modify"	  
	  self.ItemSelectCaller = r2.Scenario.PlotItems[index]	  
   end
   enableModalWindow(getUICaller(), "ui:interface:r2ed_choose_plot_item")      
   self.ChangeIndex = index   

   local window = getUI("ui:interface:r2ed_scenario")
	local groupListSheet = window:find("plot_items_list")   
   if index <= maxNumPlotItems - 1 then		
		local slot = groupListSheet["item_" .. tostring(index)]
		slot.but.pushed = false
   end
end

----------------------------------------------------------------------------
function r2.PlotItemsPanel:rightClickItem(index)
	if r2.PlotItemsPanel.Locked then return end -- this is a false selection 											    
	-- we can't use the sheet here, because concurrent edition may lead to duplicated sheets
	-- here, a case that wecan resolve only when the scenario is about to be launched...	
	if index > r2.Scenario.PlotItems.Size then		
		debugInfo("bad item index")
	end
	if index == r2.Scenario.PlotItems.Size then
		-- this is the creation slot -> no-op
		return
	else
		-- a new item is selected for real
		r2:setSelectedInstanceId(r2.Scenario.PlotItems[index].InstanceId)
      r2:displayContextMenu()
	end
end

----------------------------------------------------------------------------
-- private
function r2.PlotItemsPanel:createNewItem(sheetDbPath)	
   local sheetId = getDbProp(sheetDbPath .. ":SHEET")   
   if self.CreateMode == "CreateNew" then
	  local newItem = self:requestNewItem(sheetId)      
      -- since created from this client, add a cookie to pop the property window when the item is created for real
      r2:setCookie(newItem.InstanceId, "SelfCreate", true)		  
   elseif self.CreateMode == "CreateNewAndAffectToRefId" then
		r2.requestNewAction(i18n.get("uiR2EDCreatePlotItemAction"))
		local newItem = self:requestNewItem(sheetId)
		r2:setCookie(newItem.InstanceId, "AddedMsg", true)	
		r2.requestSetNode(self.TargetRefId.ParentInstanceId, self.TargetRefId.Name, newItem.InstanceId)
   else
	  r2.requestNewAction(i18n.get("uiR2EDChangeIconAction"))
      r2.requestSetNode(r2.Scenario.PlotItems[self.ChangeIndex].InstanceId, "SheetId", sheetId)
   end
end

----------------------------------------------------------------------------
-- private: create a new item in the plot item list
function r2.PlotItemsPanel:requestNewItem(sheetId)	
	r2.requestNewAction(i18n.get("uiR2EDCreateNewPlotItemAction"))
	local newItem = r2.newComponent("PlotItem")
	newItem.SheetId = sheetId
	newItem.Name = i18n.get("uiR2EDPlotItemDefaultName"):toUtf8()
	newItem.Desc = ""
	newItem.Comment = ""
	r2.requestInsertNode(r2.Scenario.InstanceId, "PlotItems", -1, "", newItem)
	return newItem
end

----------------------------------------------------------------------------
function r2.PlotItemsPanel:reinit()	
	-- map sheetid to the good slot in the db
	plotItemSheetToDBPath = {}
	plotItemSheetNbRef = {}
	--local window = getUI("ui:interface:r2ed_scenario")
	--local groupListSheet = window:find("plot_items_list")   
	for k = 0, maxNumPlotItemSheets - 1 do
		local refDbPath = "LOCAL:R2:REFERENCE_PLOT_ITEMS:" .. tostring(k) ..":SHEET"
		local availableDbPath = "LOCAL:R2:AVAILABLE_PLOT_ITEMS:" .. tostring(k) ..":SHEET"
		local sheetId = getDbProp(refDbPath)
		local defaultPlotItemName = i18n.get("uiR2EDPlotItemDefaultName")
		if sheetId ~= 0 then
			plotItemSheetToDBPath[sheetId] = availableDbPath
			plotItemSheetNbRef[sheetId] = 0
			r2:setPlotItemInfos(sheetId, defaultPlotItemName, ucstring(), ucstring())			
			setDbProp(availableDbPath, getDbProp(refDbPath))
		end
		--local slot = groupListSheet["item_" .. tostring(k)]
		--slot.active = false	
	end
	-- empty all slots
	for k = 0, maxNumPlotItems - 1 do		
		setDbProp("LOCAL:R2:PLOT_ITEMS:" .. tostring(k) .. ":SHEET", 0)
	end    				
	--
    getUI("ui:interface:r2ed_scenario"):find("delete_plot_item").frozen = true
    getUI("ui:interface:r2ed_scenario"):find("plot_item_properties").frozen = true
end

----------------------------------------------------------------------------
-- pop the plot item tab
function r2.PlotItemsPanel:pop()
   local scenarioWnd = getUI("ui:interface:r2ed_scenario")
   if not scenarioWnd.active or scenarioWnd:find("scenario_tabs").selection ~= 3 then
      scenarioWnd.active = 1
      scenarioWnd:blink(1)
      scenarioWnd:find("scenario_tabs").selection = 3
   end
end

