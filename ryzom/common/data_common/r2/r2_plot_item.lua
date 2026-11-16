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
	Version			   = 1,						-- Version 1 : a plot item that named an item of the game has a plot item sheet (see updateVersion)
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
   return plotItemNamePrefix..this.Name
end

function plotItem.isNextSelectable(this)
	return true
end

---------------------------------------------------------------------------------------------------------
-- get select bar type
function plotItem.SelectBarType(this)
	return i18n.get("uiR2EDPlotItems")
end

---------------------------------------------------------------------------------------------------------
-- get first parent that is selectable in the select bar
function plotItem.getFirstSelectBarParent(this)
	return r2:getCurrentAct()
end

function plotItem.getSelectBarIcon(this)
	return "ICO_mission_purse.tga"
end

-- ////////////////////
-- // VERSION UPDATE //
-- ////////////////////

-- A plot item holds the number of its sheet (sheet_id.bin), and the version update works on the scenario
-- as plain data, in the client and outside of it: no sheet is there to ask. The plot item sheets are
-- known by their numbers here. They are the 185 sheets r2_plot_item_0.sitem to r2_plot_item_184.sitem,
-- the ones the scenario service accepts when a scenario starts (CRingAccess::init), and their numbers
-- have not changed since 2006. A number is 256 times the place of the sheet among the sheets of its type,
-- plus the type; the 185 sheets stand in a row there, in the order of their names (0, 1, 10, 100 ...).
-- Sheets of other types have numbers in between: the step of 256 is part of the test.
local firstPlotItemSheet = 8630062 -- r2_plot_item_0.sitem
local lastPlotItemSheet = 8677166 -- r2_plot_item_99.sitem

local function isPlotItemSheet(sheetId)
	return type(sheetId) == "number" and sheetId >= firstPlotItemSheet and sheetId <= lastPlotItemSheet
		and (sheetId - firstPlotItemSheet) % 256 == 0
end

-- Scenarios of the beta of 2006, and a few of early 2007, have plot items that name an item of the game
-- where a plot item sheet belongs: a piece of armor, a raw material, a tool. The scenario service does
-- not start such a scenario. The table below has the items found in those scenarios, by sheet number,
-- each with the plot item sheet that shows the icon of the item (Same; the boots have none: no plot item
-- sheet shows their icon) and a list of plot item sheets with icons of the same kind (Kind), the nearest
-- first, for a plot item that can not get the icon of its item:
--     [item] = { Same = plot item sheet, Kind = list }, -- item (its icon) > plot item sheet
-- The numbers are those of sheet_id.bin; the icons are those of the item sheets and of the plot item
-- sheets (the table of the sheet generator, sitem_plot_items.tsv in the leveldesign data).

-- a vest: clothes, the parts of an armor, the helmet and the armor tool, the glove, then the other pieces and the shields
local vestIcons =
{
	8630574, -- r2_plot_item_10.sitem (ico_clothes)
	8656686, -- r2_plot_item_26.sitem (ico_lining)
	8662062, -- r2_plot_item_45.sitem (ico_stuffing)
	8657710, -- r2_plot_item_3.sitem (ar_helmet)
	8649518, -- r2_plot_item_167.sitem (to_armor)
	8663086, -- r2_plot_item_49.sitem (mg_glove)
	8630062, -- r2_plot_item_0.sitem (ar_armpad)
	8654894, -- r2_plot_item_2.sitem (ar_hand)
	8660526, -- r2_plot_item_4.sitem (ar_pantabotte)
	8648750, -- r2_plot_item_164.sitem (sh_buckler)
	8649006, -- r2_plot_item_165.sitem (sh_large_shield)
}

-- sleeves: as for a vest, the lining first
local sleevesIcons =
{
	8656686, -- r2_plot_item_26.sitem (ico_lining)
	8630574, -- r2_plot_item_10.sitem (ico_clothes)
	8662062, -- r2_plot_item_45.sitem (ico_stuffing)
	8657710, -- r2_plot_item_3.sitem (ar_helmet)
	8649518, -- r2_plot_item_167.sitem (to_armor)
	8663086, -- r2_plot_item_49.sitem (mg_glove)
	8630318, -- r2_plot_item_1.sitem (ar_gilet)
	8654894, -- r2_plot_item_2.sitem (ar_hand)
	8660526, -- r2_plot_item_4.sitem (ar_pantabotte)
	8648750, -- r2_plot_item_164.sitem (sh_buckler)
	8649006, -- r2_plot_item_165.sitem (sh_large_shield)
}

-- gloves: the other glove first
local glovesIcons =
{
	8663086, -- r2_plot_item_49.sitem (mg_glove)
	8630574, -- r2_plot_item_10.sitem (ico_clothes)
	8656686, -- r2_plot_item_26.sitem (ico_lining)
	8662062, -- r2_plot_item_45.sitem (ico_stuffing)
	8657710, -- r2_plot_item_3.sitem (ar_helmet)
	8649518, -- r2_plot_item_167.sitem (to_armor)
	8630062, -- r2_plot_item_0.sitem (ar_armpad)
	8630318, -- r2_plot_item_1.sitem (ar_gilet)
	8660526, -- r2_plot_item_4.sitem (ar_pantabotte)
	8648750, -- r2_plot_item_164.sitem (sh_buckler)
	8649006, -- r2_plot_item_165.sitem (sh_large_shield)
}

-- pants: as for a vest, the stuffing first
local pantsIcons =
{
	8662062, -- r2_plot_item_45.sitem (ico_stuffing)
	8630574, -- r2_plot_item_10.sitem (ico_clothes)
	8656686, -- r2_plot_item_26.sitem (ico_lining)
	8657710, -- r2_plot_item_3.sitem (ar_helmet)
	8649518, -- r2_plot_item_167.sitem (to_armor)
	8663086, -- r2_plot_item_49.sitem (mg_glove)
	8630062, -- r2_plot_item_0.sitem (ar_armpad)
	8630318, -- r2_plot_item_1.sitem (ar_gilet)
	8654894, -- r2_plot_item_2.sitem (ar_hand)
	8648750, -- r2_plot_item_164.sitem (sh_buckler)
	8649006, -- r2_plot_item_165.sitem (sh_large_shield)
}

-- boots: the pants with their boots first, then the armor tool and the helmet
local bootsIcons =
{
	8660526, -- r2_plot_item_4.sitem (ar_pantabotte)
	8649518, -- r2_plot_item_167.sitem (to_armor)
	8657710, -- r2_plot_item_3.sitem (ar_helmet)
	8662062, -- r2_plot_item_45.sitem (ico_stuffing)
	8656686, -- r2_plot_item_26.sitem (ico_lining)
	8630574, -- r2_plot_item_10.sitem (ico_clothes)
	8663086, -- r2_plot_item_49.sitem (mg_glove)
	8630062, -- r2_plot_item_0.sitem (ar_armpad)
	8630318, -- r2_plot_item_1.sitem (ar_gilet)
	8654894, -- r2_plot_item_2.sitem (ar_hand)
	8648750, -- r2_plot_item_164.sitem (sh_buckler)
	8649006, -- r2_plot_item_165.sitem (sh_large_shield)
}

-- a raw material: the first raw materials of the list of icons, without the parts of buildings
local rawMaterialIcons =
{
	8664622, -- r2_plot_item_54.sitem (mp_amber)
	8664878, -- r2_plot_item_55.sitem (mp_bark)
	8667694, -- r2_plot_item_65.sitem (mp_beak)
	8667950, -- r2_plot_item_66.sitem (mp_blood)
	8668206, -- r2_plot_item_67.sitem (mp_bone)
	8668462, -- r2_plot_item_68.sitem (mp_bud)
	8669486, -- r2_plot_item_71.sitem (mp_cereal)
	8669742, -- r2_plot_item_72.sitem (mp_claw)
}

-- a tool: the tools of the other crafts
local toolIcons =
{
	8652590, -- r2_plot_item_178.sitem (to_range)
	8649262, -- r2_plot_item_166.sitem (to_ammo)
	8649518, -- r2_plot_item_167.sitem (to_armor)
	8651310, -- r2_plot_item_173.sitem (to_jewels)
	8650798, -- r2_plot_item_171.sitem (to_hammer)
	8651054, -- r2_plot_item_172.sitem (to_jewelry_hammer)
	8651566, -- r2_plot_item_174.sitem (to_leathercutter)
	8652078, -- r2_plot_item_176.sitem (to_needle)
	8654382, -- r2_plot_item_184.sitem (to_wrench)
}

local plotItemSheetOfItem =
{
	[8386350] = { Same = 8673582, Kind = rawMaterialIcons }, -- system_mp_choice.sitem (mp_generic) > r2_plot_item_86.sitem
	[8386606] = { Same = 8673582, Kind = rawMaterialIcons }, -- system_mp_excellent.sitem (mp_generic) > r2_plot_item_86.sitem
	[8386862] = { Same = 8673582, Kind = rawMaterialIcons }, -- system_mp_extra.sitem (mp_generic) > r2_plot_item_86.sitem
	[8387374] = { Same = 8673582, Kind = rawMaterialIcons }, -- system_mp_supreme.sitem (mp_generic) > r2_plot_item_86.sitem
	[8448814] = { Kind = bootsIcons }, -- iccalb.sitem, light boots (ar_botte) > no plot item sheet shows it
	[8450094] = { Same = 8654894, Kind = glovesIcons }, -- iccalg.sitem, light gloves (ar_hand) > r2_plot_item_2.sitem
	[8451374] = { Same = 8660526, Kind = pantsIcons }, -- iccalp.sitem, light pants (ar_pantabotte) > r2_plot_item_4.sitem
	[8452654] = { Same = 8630062, Kind = sleevesIcons }, -- iccals.sitem, light sleeves (ar_armpad) > r2_plot_item_0.sitem
	[8453934] = { Same = 8630318, Kind = vestIcons }, -- iccalv.sitem, light vest (ar_gilet) > r2_plot_item_1.sitem
	[8455214] = { Kind = bootsIcons }, -- iccamb.sitem, medium boots (ar_botte) > no plot item sheet shows it
	[8456494] = { Same = 8654894, Kind = glovesIcons }, -- iccamg.sitem, medium gloves (ar_hand) > r2_plot_item_2.sitem
	[8457774] = { Same = 8660526, Kind = pantsIcons }, -- iccamp.sitem, medium pants (ar_pantabotte) > r2_plot_item_4.sitem
	[8459054] = { Same = 8630062, Kind = sleevesIcons }, -- iccams.sitem, medium sleeves (ar_armpad) > r2_plot_item_0.sitem
	[8460334] = { Same = 8630318, Kind = vestIcons }, -- iccamv.sitem, medium vest (ar_gilet) > r2_plot_item_1.sitem
	[8461614] = { Same = 8660526, Kind = pantsIcons }, -- iccacp.sitem, caster pants (ar_pantabotte) > r2_plot_item_4.sitem
	[8629550] = { Same = 8651822, Kind = toolIcons }, -- newbie_tool.sitem (to_melee) > r2_plot_item_175.sitem
}

-- Version 1. A plot item that names an item of the table gets a plot item sheet, so that the scenario
-- starts. Each plot item sheet is the sheet of one plot item of a scenario, which is how a player's item
-- is known to be this plot item or that one: a sheet another plot item of the scenario has is not taken,
-- whether that one had it before or got it earlier in this update (the plot items are updated in the
-- order of their list). The choice, in this order:
--   * the plot item sheet with the icon of the item (Same);
--   * the first sheet of the list of the same kind (Kind) that no plot item has;
--   * the first of the 185 plot item sheets, in the order of their numbers, that no plot item has.
-- The author is told which plot item it is, and for a sheet that does not show the icon of the item, why,
-- and that the icon can be changed. A scenario has 32 plot items at most (r2ed_max_num_plot_items, and
-- the check of an upload): one of the 185 sheets is always free. Should none be, the plot item is left as
-- it is, like one whose sheet is not in the table: nothing is guessed for a sheet the table does not have.
-- Such a plot item stays what the scenario service refuses, and the author is told to choose an icon.
local function updatePlotItemSheet(this, update)
	local sheetId = this.SheetId
	if type(sheetId) ~= "number" or isPlotItemSheet(sheetId) then return end
	local name = tostring(defaulting(this.Name, ""))
	local choice = plotItemSheetOfItem[sheetId]
	if choice == nil then
		update:note("uiR2EDVersionNotePlotItemUnknown", name, string.format("%.0f", sheetId))
		return
	end
	-- the plot item sheets the other plot items of the scenario have, each with the name of the first
	-- plot item that has it
	local used = {}
	local plotItems = update:getParent(this)
	if type(plotItems) == "table" then
		for i = 1, table.getn(plotItems) do
			local other = plotItems[i]
			if type(other) == "table" and not rawequal(other, this) and isPlotItemSheet(other.SheetId)
				and used[other.SheetId] == nil then
				used[other.SheetId] = tostring(defaulting(other.Name, ""))
			end
		end
	end
	if choice.Same ~= nil and used[choice.Same] == nil then
		update:set(this, "SheetId", choice.Same)
		update:note("uiR2EDVersionNotePlotItemIcon", name)
		return
	end
	local newSheetId = nil
	for i = 1, table.getn(choice.Kind) do
		if newSheetId == nil and used[choice.Kind[i]] == nil then
			newSheetId = choice.Kind[i]
		end
	end
	local candidate = firstPlotItemSheet
	while newSheetId == nil and candidate <= lastPlotItemSheet do
		if used[candidate] == nil then
			newSheetId = candidate
		end
		candidate = candidate + 256
	end
	if newSheetId == nil then
		update:note("uiR2EDVersionNotePlotItemUnknown", name, string.format("%.0f", sheetId))
		return
	end
	update:set(this, "SheetId", newSheetId)
	if choice.Same ~= nil then
		update:note("uiR2EDVersionNotePlotItemIconTaken", name, used[choice.Same])
	else
		update:note("uiR2EDVersionNotePlotItemIconNone", name)
	end
end

function plotItem.updateVersion(this, scenarioValue, currentValue, update)
	local patchValue = scenarioValue
	if patchValue < 1 then
		updatePlotItemSheet(this, update)
		patchValue = 1
	end
	if patchValue == currentValue then return true end
	return false
end

r2.registerComponent(plotItem)



-- /////////////
-- // PRIVATE //
-- /////////////

-- The reference plot item sheets are the item sheets of the client that are named r2_plot_item*
-- (CEditor::initReferencePlotItems puts them in LOCAL:R2:REFERENCE_PLOT_ITEMS). r2.PlotItemsPanel:reinit
-- reads them when a scenario is created, before the plot items of that scenario are.
--
-- A plot item of a scenario can have a sheet that is not one of them: the scenario was made with other
-- game data (it holds the number of the sheet, which belongs to the sheet_id.bin it was made with), or
-- this client has no plot item sheets. Such an item is an item of the scenario like the others: it is
-- counted when it is created and released when it is erased. It has no slot in the list of the item
-- chooser to mark as taken, and the client has no icon for it; the log says so once for each sheet.

local plotItemSheetToDBPath = {} -- for each reference sheet, its slot in the list of the item chooser

local plotItemSheetNbRef = {} -- for each plot item sheet, store the number of references on it
							  -- (0 for a reference sheet no plot item has; no entry for any other sheet
							  -- until a plot item has it)
							  -- each sheet must be use 0 or 1 time in the scenario
							  -- a ref count > 1 may happen during concurrent edition
							  -- in which case the user should be warned that some items are duplicated

local nbReferenceSheets = 0 -- how many reference sheets the client had when the scenario was created

local foreignSheetsTold = {} -- the sheets that are not reference sheets and that the log has named

-- The slot of a reference sheet in the list of the item chooser shows the sheet while no plot item has
-- it, and is empty while one has. A sheet that is not a reference sheet has no slot.
local function setSheetAvailable(sheetId, available)
	local dbPath = plotItemSheetToDBPath[sheetId]
	if dbPath ~= nil then
		if available then
			setDbProp(dbPath, sheetId)
		else
			setDbProp(dbPath, 0)
		end
	end
end

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
	if plotItemSheetToDBPath[sheetId] == nil then
		-- Not a reference sheet: the client has no item of this sheet to name (r2:setPlotItemInfos takes
		-- the plot item sheets of the client and raises for any other number), and the item chooser has
		-- no slot for it. The item keeps its name in the scenario.
		return
	end
	if plotItemSheetNbRef[sheetId] == 0 then
		r2:setPlotItemInfos(sheetId, i18n.get("uiR2EDPlotItemDefaultName"), "", "")
		setSheetAvailable(sheetId, true) -- available again
	elseif plotItemSheetNbRef[sheetId] > 1 then
		-- duplicated slot, may happen during concurrent edition (bad luck)
		r2:setPlotItemInfos(sheetId, i18n.get("uiR2EDDuplicatedPlotItemName"), "", "")
		setSheetAvailable(sheetId, false)
		r2.PlotItemDisplayerCommon:touch() -- force to refresh the icon display
	else
		r2:setPlotItemInfos(sheetId, instance.Name, instance.Desc, instance.Comment)
		setSheetAvailable(sheetId, false)
	end
end

----------------------------------------------------------------------------
-- add a reference on a sheet id for a r2 plot item
function r2.PlotItemDisplayerCommon:addRef(sheetId)
	if plotItemSheetNbRef[sheetId] == nil then
		-- not a reference sheet (see the top of this part): counted from here on
		plotItemSheetNbRef[sheetId] = 0
		if not foreignSheetsTold[sheetId] then
			foreignSheetsTold[sheetId] = true
			debugWarning("ASSET ERROR: a plot item of the scenario has the sheet " .. tostring(sheetId)
				.. ", which is not one of the " .. tostring(nbReferenceSheets) .. " plot item sheets of this client"
				.. " (the item sheets named r2_plot_item*). The item is kept, without an icon and without a place"
				.. " in the item chooser. The scenario was made with other game data, or the plot item sheets"
				.. " are missing from the data of this client: choose another icon for the item, or add the sheets.")
		end
	end
	assert(plotItemSheetNbRef[sheetId] >= 0)
	plotItemSheetNbRef[sheetId] = plotItemSheetNbRef[sheetId] + 1
end

----------------------------------------------------------------------------
-- remove a reference on a sheet id for a r2 plot item
-- add a reference on a sheet id
function r2.PlotItemDisplayerCommon:decRef(sheetId)
	-- every plot item is counted when it is created (onPostCreate) and when its sheet changes
	-- (onAttrModified): a sheet that is released has a count
	local nbRef = plotItemSheetNbRef[sheetId]
	assert(nbRef ~= nil and nbRef > 0, "the plot item sheet " .. tostring(sheetId) .. " is released and was not counted")
	plotItemSheetNbRef[sheetId] = nbRef - 1
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
	-- The count first: the item is counted once here and released once in onErase, whatever the steps
	-- below come to
	instance.User.OldSheetId = instance.SheetId
	self:addRef(instance.SheetId)
	setSheetAvailable(instance.SheetId, false)  -- not available for other plot items
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
		-- The counts first, the old sheet released and the new one taken in one go: what is shown for the
		-- two sheets comes after, and cannot leave the item counted for a sheet it no longer has
		local oldSheetId = instance.User.OldSheetId
		self:decRef(oldSheetId)
		self:addRef(instance.SheetId)
		instance.User.OldSheetId = instance.SheetId
		self:updateNameAndAvailability(instance, oldSheetId)
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
	   self:updateSheetColor(slot.sheet, v.SheetId)
      slot.t.text = v.Name
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
		slot.t.text = i18n.get("uiR2EDCreateNewItem")
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
	newItem.Name = i18n.get("uiR2EDPlotItemDefaultName")
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
	nbReferenceSheets = 0
	foreignSheetsTold = {}
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
			nbReferenceSheets = nbReferenceSheets + 1
			r2:setPlotItemInfos(sheetId, defaultPlotItemName, "", "")
			setDbProp(availableDbPath, getDbProp(refDbPath))
		end
		--local slot = groupListSheet["item_" .. tostring(k)]
		--slot.active = false	
	end
	if nbReferenceSheets == 0 then
		debugWarning("ASSET ERROR: this client has no plot item sheet (no item sheet named r2_plot_item*):"
			.. " the item chooser of the Ring editor is empty and no plot item can be made. The sheets come with"
			.. " the game data (sitem sheets and sheet_id.bin).")
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

