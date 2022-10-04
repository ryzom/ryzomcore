--------------------------------------------------------------------------------------------
-- PROPERTY SHEETS & FORMS
-- ======================
-- This file contains code to generate xml of generic property sheets & forms that are used to change
-- properties of editor objects / tables
-- NB : Currenlty there can be only a single property sheet displayed at a time


-- To speed up reset of editor, we keep a cache to avoid to rebuild ui for class
-- whose properties have not changed
-- this cache is kept between 2 reset of the editor
-- the cache is a simple table : key = class name & value = property of the class
-- NB : we don't use the r2 table because it is reseted each time !!
if r2ClassDescCache == nil then
	-- first build of the table
	r2ClassDescCache = {}
	debugInfo(colorTag(255, 0, 255) .. "Init of property sheet cache")
end
-- same thing for forms
if r2FormsCache == nil then
	-- first build of the table
	r2FormsCache = {}
	debugInfo(colorTag(255, 0, 255) .. "Init of forms cache")
end

function resetR2UICache()
	r2FormsCache = nil
	r2ClassDescCache = nil
end


r2.DefaultPropertySheetTitleClampSize = -28

local eventArgs = {} -- static table here to avoid costly allocs with the ellipsis ...
local emptyArgs = {}
-- build an edit box for an arbitrary type
function r2:buildEditBox(prop, textRef, entryType, multiLine, maxNumChars, onChangeAction, onFocusLostAction)		
	local result =
	[[<instance template="edit_box_widget" text_y="-1" posref="ML ML" sizeref="w" w="-8" fontsize="14" x="4" reset_focus_on_hide="true"
		 child_resize_hmargin="4"
		 max_historic="0"
		 y="-2"		 
		 negative_filter = '"{}[]'
		 prompt="" enter_loose_focus="true" 
		 color="255 255 255 255"
		 continuous_text_update="true"
		 bg_texture="grey_40.tga"
		 onchange="lua" onchange_params="getUICaller():setupDisplayText(); getUICaller():find('edit_text'):updateCoords(); getUICaller():getEnclosingContainer().Env.updateSize()"
		 onenter="lua" on_focus_lost="lua"]] ..
	[[ id=               ]] .. strifyXml(prop.Name) ..
	[[ text_ref   =      ]] .. strifyXml(textRef)   ..
	[[ entry_type =      ]] .. strifyXml(entryType) ..
	[[ multi_line =      ]] .. strifyXml(multiLine) ..
	[[ max_num_chars =   ]] .. strifyXml(maxNumChars) ..
	[[ params = ]] .. strifyXml(onChangeAction) ..
	[[ on_focus_lost_params = ]] .. strifyXml(onFocusLostAction) ..	
	"/>"
	return result
end


-- build an edit box for an arbitrary type
--function r2:buildComboEditBox(prop, textRef, entryType, multiLine, maxNumChars, onChangeAction)
--	return
--	[[
--		<instance template="edit_box_widget" text_y="-1" posref="ML ML" sizeref="w" w="-32" fontsize="14" x="4" reset_focus_on_hide="true"
--		 child_resize_hmargin="4"
--		 max_historic="0"
--		 y="-2"		 
--		 prompt="" enter_loose_focus="true" 
--		 color="255 255 255 255"
--		 continuous_text_update="true"
--		 bg_texture="grey_40.tga"
--		 onchange="lua" onchange_params="getUICaller():setupDisplayText(); getUICaller():find('edit_text'):updateCoords(); getUICaller():getEnclosingContainer().Env.updateSize()"
--		 onenter="lua" on_focus_lost="lua"]] ..
--	[[ id=               ]] .. strifyXml(prop.Name) ..
--	[[ text_ref   =      ]] .. strifyXml(textRef)   ..
--	[[ entry_type =      ]] .. strifyXml(entryType) ..
--	[[ multi_line =      ]] .. strifyXml(multiLine) ..
--	[[ max_num_chars =   ]] .. strifyXml(maxNumChars) ..
--	[[ params = ]] .. strifyXml(onChangeAction) ..
--	[[ on_focus_lost_params = ]] .. strifyXml(onChangeAction) ..
--	[[ /> 
--
--
--
--	]]
--
--end


-- widget styles, key is the type, value is a table containing the widget factory (each wdget being identified by its 'WidgetStyle')
r2.WidgetStyles =
{	
}



--//////////////////////
--//  STRINGS WIDGETS //
--//////////////////////

-- Build widget factory for 'Strings'
--
-- Each key gives the name of the widget style. Each value is a function that returns the xml code for the widget
-- that must edit the 'prop' value.
-- Moreover, a "setter" function" must be returned, that will update the content of the widget from external modification
--
-- The widget may also return a table in place of the setter function, if it is of type 'refid', and wants to handle modification of its target
-- In this case, the table must have the following format :
-- local refIdTable = {}
-- -- Each function below takes 3 parameters :
-- -- widget : pointer to the ui widget
-- -- value  : current value of the property that is displayed by the widget
-- -- prop   : definition of the property displayed by this widget (taken from the definition of the class that contain that property)
--
-- function refIdTable:onSet(widget, value, prop) .. end				     -- handles modification of the reference 
-- function refIdTable:onTargetCreated(widget, value, prop) .. end			 -- handles creation of the targetted object
-- function refIdTable:onTargetErased(widget, value, prop) .. end			 -- handles deletion of the targetted object
-- function refIdTable:onTargetPreHrcMove(widget, value, prop) .. end	     -- targeted object is about to move in the object hierarchy
-- function refIdTable:onTargetPostHrcMove(widget, value, prop) .. end	     -- targeted object has moved in the object hierarchy
-- function refIdTable:onTargetAttrModified(widget, value, prop, targetAttr, targetIndexinArray) .. end   -- handles modifications of the deleted object
-- 

function r2.returnOk(param)
	return true
end

function r2.refuseEmptyString(param)
	assert(param)
	assert(type(param) == "string")
	if string.len(param) == 0 then
		local ucStringMsg = i18n.get("uiR2EdNoEmptyString")	
		displaySystemInfo(ucStringMsg, "BC")
		return false
	end	
	return true
end

r2.WidgetStyles.String = 
{
	-- default string edition, using an edit box
	Default = function(prop, className)
	
		local function setter(widget, prop, value)
			local ok = true
			if prop.ValidationFun  then
				ok = r2.assert(loadstring('return '.. prop.ValidationFun))()(value)
			end
			if ok then
				local ucValue = ucstring()
				--debugInfo("value = " .. tostring(value))
				ucValue:fromUtf8(value)
				widget.eb.uc_input_string = ucValue
				widget.eb.Env.CurrString = ucValue			
				--debugInfo("setting" .. widget.eb.id .. " to " .. tostring(value))
			else
				widget.eb.uc_input_string = widget.eb.Env.CurrString
				widget.eb.input_string = widget.eb.uc_input_string:toUtf8()
			end
		end

		local validation = ""
		
		if prop.ValidationFun then
			validation = string.format([[	if not %s(utf8Value) then editBox.uc_input_string = editBox.Env.CurrString; return end]], prop.ValidationFun)
		end
			
		local onChangeAction = 
			string.format(
			[[
				local editBox = getUICaller()
				if editBox.input_string == editBox.Env.CurrString then
					return
				end

				local utf8Value = editBox.uc_input_string:toUtf8()
				%s
				editBox.Env.CurrString = editBox.input_string		
				r2:requestSetObjectProperty('%s', utf8Value)
			]], validation, prop.Name)

		local onFocusLostAction = onChangeAction
		if prop.ValidateOnEnter then
			onChangeAction = onChangeAction .. "r2:validateForm(r2.CurrentForm)"
		end
		return r2:buildEditBox(prop, "TL TL", defaulting(prop.EntryType, "text"), true, defaulting(prop.MaxNumChar, 256), onChangeAction, onFocusLostAction),
		       setter,
			   nil
	end,
	-- string property displayed as a static text (not editable)
	StaticText = function(prop, className)
		--debugInfo("Building static text")
		local function setter(widget, prop, value)
			--debugInfo("value = " .. tostring(value))
			local ucValue = ucstring()
			ucValue:fromUtf8(value)
			widget.uc_hardtext = ucValue
		end
		local widgetXml = 
		string.format([[ <view type="text" id="%s" color="192 192 192 255" posparent="parent" active="true" posref="ML ML" x="4" y="-2"  global_color="true" fontsize="12" shadow="true" hardtext="toto" auto_clamp="true"/> ]], prop.Name)
		return widgetXml, setter, nil		
	end,

	StaticTextMultiline = function(prop, className)
		--debugInfo("Building static text")
		local function setter(widget, prop, value)
			--debugInfo("value = " .. tostring(value))
			local ucValue = ucstring()
			ucValue:fromUtf8(value)
			widget.uc_hardtext = ucValue
		end
		local widgetXml = 
		string.format([[ <view type="text" id="%s" color="192 192 192 255" posparent="parent" active="true" posref="ML ML" x="4" y="-2"  global_color="true" fontsize="12" shadow="true" hardtext="toto" multi_line="true" auto_clamp="true"/> ]], prop.Name)
		return widgetXml, setter, nil		
	end
}



------------------------------------------------------------------------------------------------------------
-- helper : build the name of a property tooltip : looks in the bas class until a translation is found
local function buildPropTooltipName(className, propName)
	assert(className)
	assert(propName)
	local tt = "uiR2EdPropertyToolTip_" .. className .. "_" .. propName
	if i18n.hasTranslation(tt) then 
		--debugInfo("### Translation found for " .. propName .. " in " .. className)
		return tt, true 
	end

	-- this Form is not a Class (its a  form not a property sheet) so don't search into its parents
	if r2.Classes[className] == nil then
		return tt, false
	end

	local parentClassName = r2.Classes[className].BaseClass
	if  parentClassName ~= "" and parentClassName~=nil then
		local parentClass = r2.Classes[parentClassName]
		if parentClass.NameToProp[propName] ~= nil then
			--debugInfo("### Translation not found for " .. propName .. " in " .. className .. ".Looking in parent class " .. parentClassName)
			local translation, found = buildPropTooltipName(parentClassName, propName)
			if found then return translation, true end			
		end
	end
	return tt, false -- not found, a 'NotExist' string will be displayed
end

--/////////////////////
--//  REF ID WIDGETS //
--/////////////////////

------------------------------------------------------------------------------------------------------------
--///////////////////////////
--// DEFAULT REF ID PICKER //
--///////////////////////////

-- handle updates of the "RefId" widget
-- this table is common to all the ref id widgets
local refIdDefaultEventHandler ={}
--
function refIdDefaultEventHandler:onSet(widget, prop, value)	
	--debugInfo("set received")
	self:update(widget, value, r2:getInstanceFromId(value))
end
--
function refIdDefaultEventHandler:onTargetCreated(widget, prop, value)
	self:update(widget, value, r2:getInstanceFromId(value))
end
--
function refIdDefaultEventHandler:onTargetErased(widget, prop, value)
	self:update(widget, value, nil)
end
--
function refIdDefaultEventHandler:onTargetAttrModified(widget, prop, value, targetAttr, targetIndexInArray)	
    -- we are only interested by a change of the instance name
	if targetAttr == "Name" then
		self:update(widget, value, r2:getInstanceFromId(value))
	end
end
--
function refIdDefaultEventHandler:update(widget, value, target)
	local text = widget:find("name")		
	if target then		
		local newName = ucstring()
		newName:fromUtf8(target.Name)
		text.uc_hardtext = newName
	else		
		text.uc_hardtext = i18n.get("uiR2EDNone")
	end
end
--
function refIdDefaultEventHandler:onTargetPreHrcMove(widget, prop, value)
	--debugInfo(string.format("displayer : pre hrc move : (%s)", prop.Name))
end
--
function refIdDefaultEventHandler:onTargetPostHrcMove(widget, prop, value)
	--debugInfo(string.format("displayer : post hrc move : (%s)", prop.Name))	
end


-- globals for the 'RefId' default widget
r2.currentRefIdWidgetParentInstance = nil
r2.currentRefIdWidgetFilter = nil
r2.currentRefIdWidgetAttrName = nil
r2.currentRefIdIndexInArray = nil
--
function r2:testCanPickRefIdWidgetTarget(instanceId)
	if instanceId == r2.currentRefIdWidgetParentInstance.InstanceId then return false end	
	return  r2:getInstanceFromId(instanceId):isKindOf(r2.currentRefIdWidgetFilter)
end
--
function r2:setRefIdWidgetTarget(instanceId)	
	--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, instanceId)
	r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
	--debugInfo("requestSetNode")
end

function r2:canPickTaskComponent(instanceId)
	local taskClasses = {}
	table.insert(taskClasses, "GiveItem")
	table.insert(taskClasses, "RequestItem")
	table.insert(taskClasses, "TalkTo")
	table.insert(taskClasses, "VisitZone")
	table.insert(taskClasses, "TargetMob")
	table.insert(taskClasses, "KillNpc")
	table.insert(taskClasses, "HuntTask")
	table.insert(taskClasses, "DeliveryTask")
	table.insert(taskClasses, "GetItemFromSceneryObjectTaskStep")
	table.insert(taskClasses, "SceneryObjectInteractionTaskStep")


	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then
		return true
	end

	--check act?

	local tmpInstance = r2:getInstanceFromId(instanceId)
	assert(tmpInstance)
	local class = tmpInstance.Class
	local k, v = next(taskClasses, nil)
	while k do
		if class == v then
			return true
		end
		k, v = next(taskClasses, k)
	end
	return false
end

function r2:setTaskComponentTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		if r2:canPickTaskComponent(instanceId) then
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end


function r2:canPickSceneryObject(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then 
		return true 
	end

	if r2.Translator.checkActForPicking(comp.InstanceId, instanceId) == false then 
		return false 
	end
	
	local tmpInstance = r2:getInstanceFromId(instanceId)

	if tmpInstance:isBotObject() then
		return true
	else
		return false
	end

end

function r2:setSceneryObjectTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		if r2:canPickSceneryObject(instanceId) then
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end

function r2:canPickCivilian(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then 
		return true 
	end

	if r2.Translator.checkActForPicking(comp.InstanceId, instanceId) == false then 
		return false 
	end
	
	local tmpInstance = r2:getInstanceFromId(instanceId)

	--if not (tmpInstance.Class == "NpcCustom" or tmpInstance.Class == "Npc") or tmpInstance:isBotObject()
	--or tmpInstance:isGrouped() then
	--	return false
	if tmpInstance:isGrouped() or string.find(tmpInstance.Base, "civil") == nil then
		return false
	else
		return true
	end

end

function r2:setCivilian(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		if r2:canPickCivilian(instanceId) then
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end


function r2:canPickTalkingNpc(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then 
		return true 
	end

	if r2.Translator.checkActForPicking(comp.InstanceId, instanceId) == false then 
		return false 
	end
	
	local tmpInstance = r2:getInstanceFromId(instanceId)

	if not (tmpInstance.Class == "NpcCustom" or tmpInstance.Class == "Npc") or tmpInstance:isBotObject()
	or tmpInstance:isGrouped() then
		return false
	else
		return true
	end

end

function r2:setTalkingNpc(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		if r2:canPickTalkingNpc(instanceId) then
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end


function r2:canPickNpcOrGroup(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then return true end
	local componentId = comp.InstanceId
	--if r2.Translator.checkActForPicking(componentId, instanceId) == false then 
	--	return false
	--end
	if instanceId == r2.currentRefIdWidgetParentInstance.InstanceId then return false
	else
		local tmpInstance = r2:getInstanceFromId(instanceId)
		
		if tmpInstance and not tmpInstance:isBotObject() then
			if tmpInstance:isKindOf("Npc") 
			or tmpInstance.ParentInstance:isKindOf("NpcGrpFeature")
			or tmpInstance:isKindOf("NpcGrpFeature") then
				return true
			else return false end
		end
		return false
	end

end

function r2:setNpcOrGroupRefIdTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		-- if the instance is a npc belonging to a npcgrp
		if not tmpInstance:isBotObject() and tmpInstance:isKindOf("Npc") and tmpInstance.ParentInstance:isKindOf("NpcGrpFeature") then
				--debugInfo("inserted: " ..tmpInstance.ParentInstance.Name)
				--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, tmpInstance.ParentInstance.InstanceId)
				r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, tmpInstance.ParentInstance.InstanceId)
		-- else if the instance is npc or a npcgrp
		elseif not tmpInstance:isBotObject() and tmpInstance:isKindOf("NpcGrpFeature") or tmpInstance:isKindOf("Npc") then
			--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, instanceId)
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end

function r2:canPickEasterEgg(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then return true end
	local componentId = comp.InstanceId
	if r2.Translator.checkActForPicking(componentId, instanceId) == false then 
		return false
	end
	if instanceId == r2.currentRefIdWidgetParentInstance.InstanceId then return false
	else
		local tmpInstance = r2:getInstanceFromId(instanceId)
		
		if tmpInstance then
			if tmpInstance:isKindOf("EasterEgg") then return true
			else return false end
		end
		return false
	end	
end

function r2:setEasterEggRefIdTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		-- if the instance is a npc belonging to a npcgrp
		if tmpInstance:isKindOf("EasterEgg") then
			--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, instanceId)
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end

function r2:canPickNotGroupedNpc(instanceId)
	--if instanceId == r2.currentRefIdWidgetParentInstance.InstanceId then return false
	--else
		if r2:getSelectedInstance() and instanceId ~= "" then
			local componentId = r2:getSelectedInstance().InstanceId
			if r2.Translator.checkActForPicking(componentId, instanceId) == false then 
				return false
			end
			local tmpInstance = r2:getInstanceFromId(instanceId)
		
			if tmpInstance then
				if not tmpInstance:isBotObject() and tmpInstance:isKindOf("Npc") and not tmpInstance.ParentInstance:isKindOf("NpcGrpFeature") then return true
				else return false end
			end
			return false
		end
		return true
	--end
end

function r2:setNotGroupedNpcRefIdTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		if not tmpInstance:isBotObject() and tmpInstance:isKindOf("Npc") and not tmpInstance.ParentInstance:isKindOf("NpcGrpFeature") then
			--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, instanceId)
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end

function r2:canPickZone(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then return true end
	local componentId = comp.InstanceId
	if r2.Translator.checkActForPicking(componentId, instanceId) == false then 
		return false
	end
	if instanceId == r2.currentRefIdWidgetParentInstance.InstanceId then return false
	else
		local tmpInstance = r2:getInstanceFromId(instanceId)
		if tmpInstance then
			if tmpInstance:isKindOf("Region") then return true
			else return false end
		end
		return false
	end
end

function r2:setZoneRefIdTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance and tmpInstance:isKindOf("Region") then 
		--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, instanceId)
		r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
	end
end

function r2:canPickDialog(instanceId)
	local comp = r2:getSelectedInstance()
	if not comp or instanceId == "" then return true end
	local componentId = comp.InstanceId
	if r2.Translator.checkActForPicking(componentId, instanceId) == false then 
		return false
	end
	if instanceId == r2.currentRefIdWidgetParentInstance.InstanceId then return false
	else
		local tmpInstance = r2:getInstanceFromId(instanceId)
		
		if tmpInstance then
			if tmpInstance:isKindOf("ChatSequence") then return true
			else return false end
		end
		return false
	end	
end

function r2:setDialogRefIdTarget(instanceId)
	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance then
		-- if the instance is a npc belonging to a npcgrp
		if tmpInstance:isKindOf("ChatSequence") then
			--r2.requestSetNode(r2.currentRefIdWidgetParentInstance.InstanceId, r2.currentRefIdWidgetAttrName, instanceId)
			r2.currentRefIdWidgetParentInstance:setRefIdValue(r2.currentRefIdWidgetAttrName, instanceId)
		end
	end
end
------------------------------------------------------------------------------------------------------------
--///////////////////////
--// PLOT ITEMS PICKER //
--///////////////////////

-- handle updates of the "RefId" widget for plot items
local maxNumPlotItems = tonumber(getDefine("r2ed_max_num_plot_items"))

local refIdPlotItemEventHandler = clone(refIdDefaultEventHandler)
--

function refIdPlotItemEventHandler:updateSheet(widget, targetPlotItem)     
   for k = 0, maxNumPlotItems - 1 do
      local dbPath = "LOCAL:R2:PLOT_ITEMS:" .. tostring(k)
      if getDbProp(dbPath .. ":SHEET") == targetPlotItem.SheetId then         
         widget.sheet.sheet = dbPath
         r2.ScratchUCStr:fromUtf8(targetPlotItem.Name)	
         widget.t.uc_hardtext = r2.ScratchUCStr
         return true
      end
   end   
   return false
end

function refIdPlotItemEventHandler:update(widget, value, target)
   local targetPlotItem = r2:getInstanceFromId(value)
   widget:find("edit_plot_item").frozen = (targetPlotItem == nil)
	-- target object must already have created its display in the database -> point the same object
   if targetPlotItem then   
      if self:updateSheet(widget, targetPlotItem) then   return end
      -- update may fail because update of the db for the plot items is done in the main loop
      -- and may not has been done yet, so force a refresh and retry
      r2.PlotItemDisplayerCommon:updateAll()
      if self:updateSheet(widget, targetPlotItem) then return end               
   end      
 
   widget.sheet.sheet = ""
   widget.t.uc_hardtext = i18n.get("uiR2EDChooseItem")   
end

-- ui handling for selection of plot items from the scenario
r2.PlotItemsWidget = 
{
	SelectionId = "",
	DestProp = "" -- the dest property to which new item selection must bedone
}

-- called when user click on the item sheet -> allows to choose a newplot item
function r2.PlotItemsWidget:changeItem(propName)
	if r2.Scenario.PlotItems.Size == 0 then
		enableModalWindow(getUICaller(), "ui:interface:r2ed_dm_gift_no_plot_items")
		return
	end
	enableModalWindow(getUICaller(), "ui:interface:r2ed_choose_property_sheet_plot_item")
	self.SelectionId = r2:getSelectedInstance().InstanceId
	self.DestProp = propName
end

-- called when user has chosen a new sheet to affect
function r2.PlotItemsWidget:validateItem(sheet)	
	local target = r2:getInstanceFromId(self.SelectionId)
	if not target then 
		return  -- maybe the target has been deleted by someone else between the first click and the choice ?
	end	
	if sheet == "UI:EMPTY" or sheet == "UI:DUMMY" then
		-- empty choice ...
		r2.requestSetNode(self.SelectionId, self.DestProp, "")
	else
		local sheetId = getDbProp(sheet .. ":SHEET")
		-- search plot item with the same sheet
		for k, v in specPairs(r2.Scenario.PlotItems) do
			if v.SheetId == sheetId then
				r2.requestNewAction(i18n.get("uiR2EDChangePlotItem"))
				r2.requestSetNode(self.SelectionId, self.DestProp, v.InstanceId) -- makeref to the new item
				return
			end
		end
		debugInfo("Plot item not found from its sheet")
	end
end

-- called when the user hit the 'new' button -> pop a dialog to create a new plot item
function r2.PlotItemsWidget:newPlotItem(propName)	
	r2.PlotItemsPanel:createNewItemAnAffectToRefId(r2:getSelectedInstance().InstanceId, propName)
end

-- called when the user hit the 'edit' button -> pop a dialog to change the name of the edited plot item
function r2.PlotItemsWidget:editPlotItem(propName)	
	r2.PlotItemDisplayerCommon:editPlotItemName(r2:getInstanceFromId(r2:getSelectedInstance()[propName]))	
end


-- build widget factory for 'RefId'
r2.WidgetStyles.RefId = 
{	
	Default = function(prop, className)
		local testFunction = ""
		local pickFunction = ""
		if not prop.PickFunction  or prop.PickFunction == ""  then
			testFunction = "r2:testCanPickRefIdWidgetTarget"
		else
			testFunction = prop.PickFunction
		end
		
		if not prop.SetRefIdFunction or prop.SetRefIdFunction == "" then
			pickFunction = "r2:setRefIdWidgetTarget"
		else
			pickFunction = prop.SetRefIdFunction
		end

		--- TestFunction

		local widgetXml =
		string.format(
		[[
			<group id="%s" child_resize_w="true" child_resize_h="true">
				<ctrl style="text_button_16" id="pick" posref="TL TL" color="255 255 255 255" col_over="255 255 255 255" col_pushed="255 255 255 255"
				 onclick_l="lua"
				 params_l="r2.currentRefIdWidgetParentInstance = r2:getPropertySheetTarget()
				  r2.currentRefIdWidgetAttrName = '%s'
				  r2.currentRefIdIndexInArray = -1
				  r2.currentRefIdWidgetFilter = '%s'
				  runAH(nil, 'r2ed_picker_lua', 'TestFunc=%s|PickFunc=%s')" 
				 hardtext="uiR2EDPick"/>
				 <ctrl style="text_button_16" id="clear" posref="TR TL" posparent="pick" color="255 255 255 255" col_over="255 255 255 255" col_pushed="255 255 255 255" 
				 onclick_l="lua" 
				 params_l="r2.requestSetNode(r2:getPropertySheetTarget().InstanceId, '%s', '')"
				 hardtext="uiR2EDClear"/>
				<view type="text" id="name" y="-2" color="192 192 192 255" posparent="clear" posref="MR ML" x="4" global_color="true" fontsize="12" shadow="true" hardtext="uiR2EDNone" auto_clamp="false"/>
			</group>
		]], prop.Name, prop.Name, select(prop.Filter, prop.Filter, "BaseClass"), testFunction, pickFunction, prop.Name
		)
		return widgetXml, refIdDefaultEventHandler, nil
	end,
	PlotItem = function(prop, className)
		widgetXml = 
		string.format(
		[[
			<instance template="r2ed_property_sheet_plot_item" id="%s" value=""/>
		]], prop.Name)
		return widgetXml, refIdPlotItemEventHandler, nil
	end,
}


-------------------------------------------------------------------------------------
-- Signal that a property of object being currently edited (in a property sheet or a form)
-- has been changed. This will update the object property with value 'value'. The appropriate net msg will be sent.
-- This function should be called by edition widgets.
function r2:requestSetObjectProperty(propName, value, isLocal)
	if getUICaller() == nil then
		debugInfo("<r2:requestSetObjectProperty> should be called by a widget")
		return
	end
	local container = getUICaller().parent:getEnclosingContainer()
	-- TODO nico : do distinction between forms and property sheet in a better way (with polymorphism)
	if container.Env.Form ~= nil then
		-- this is a form (update is done in local)
		local form =  container.Env.Form
		local prop = form.NameToProp[propName]		
		if prop.convertFromWidgetValue ~= nil then		
			-- if there's a conversion function then use it
			value = prop.convertFromWidgetValue(value)
		end
		container.Env.FormInstance[propName] = value
		container.Env.setter(propName, value)		
		-- if there's a 'onChange' function then call it
		if type(prop.onChange) == "function" then
			prop.onChange(container.Env.FormInstance)
		end
		-- visibility of some properties may depend on this one, so update them
		container.Env.updatePropVisibility()
	else		
		local target = r2:getPropertySheetTarget()
		local class =  r2:getClass(target)
		local prop = class.NameToProp[propName]				
		if target[propName] == value then
			return -- not really modified -> no op
		end		
		local ucActionName = r2:getPropertyTranslation(prop)		
		r2.requestNewAction(concatUCString(i18n.get("uiR2EDChangePropertyAction"), ucActionName, i18n.get("uiR2EDChangePropertyOf"), target:getDisplayName()))
		-- this is a property sheet
		

		-- if the instance is currently being erased, no use
		-- to send pending properties (may happen ifthe user being to enter a string
		-- an click on the erase button -> then the propertu sheet is closed and this function
		-- is fired)
		if target.User.Erased == true then
			return
		end
		
		if prop.convertFromWidgetValue ~= nil then		
			-- if there's a conversion function then use it
			value = prop.convertFromWidgetValue(value)
		end		
		--debugInfo(string.format("Setting node : prop name = %s, value = %s", tostring(propName), tostring(value)))
		
		if isLocal then
			r2.requestSetLocalNode(target.InstanceId, propName, value)			
		else
			r2.requestSetNode(target.InstanceId, propName, value)
		end	

		if prop.SecondRequestFunc and prop.SecondRequestFunc~="" then
			prop.SecondRequestFunc(value)
		end
	end
end

-------------------------------------------------------------------------------------
-- Signal that a property of object being currently edited (in a property sheet or a form)
-- has been changed. This will update the object property with value 'value'. should be called by edition widget
-- This is a local version of 'r2:requestSetObjectProperty' : no net msg is sent, value is modified locally.
-- Value is actually sent at commit time (when calling r2:requestCommitLocalObjectProperty)
function r2:requestSetLocalObjectProperty(propName, value)
	self:requestSetObjectProperty(propName, value, true)
end

-------------------------------------------------------------------------------------
-- Commit last modifications done on the object that is currently being edited (using r2:requestSetLocalObjectProperty)
-- A net msg is sent to update the object property on the server.
-- The local value is copied to the client value that was previously shadowed by a call to 'r2:requestSetLocalObjectProperty'
function r2:requestCommitLocalObjectProperty(propName, value)
	if getUICaller() == nil then
		debugInfo("<r2:requestSetObjectProperty> should be called by a widget")
		return
	end
	local container = getUICaller().parent:getEnclosingContainer()	
	if container.Env.Form == nil then				
		-- this is a property sheet
		local target = r2:getPropertySheetTarget()
		local class =  r2:getClass(target)
		local prop = class.NameToProp[propName]
		--debugInfo(string.format("Setting node : prop name = %s, value = %s", tostring(propName), tostring(value)))		
		r2.requestCommitLocalNode(target.InstanceId, propName)
		r2.requestEndAction()
	end
	-- no op for forms (no 'commit' notion)
end

-------------------------------------------------------------------------------------
-- Cancel last modifications done on the object that is currently being edited (using r2:requestSetLocalObjectProperty)
-- Object property is restored to its initial value (technically, it stop being 'shadowed' by the local value, so any
-- third party modification will be seen from here)
-- As a consequence no net msg is sent
function r2:requestRollbackLocalObjectProperty(propName, value)
	if getUICaller() == nil then
		debugInfo("<r2:requestSetObjectProperty> should be called by a widget")
		return
	end
	local container = getUICaller().parent:getEnclosingContainer()	
	if container.Env.Form ~= nil then				
		-- this is a form (update is done in local)
		local form =  container.Env.Form
		local prop = form.NameToProp[propName]
		if prop.convertFromWidgetValue ~= nil then		
			-- if there's a conversion function then use it
			value = prop.convertFromWidgetValue(value)
		end
		container.Env.FormInstance[propName] = value
		container.Env.setter(propName, value)
	else
		-- this is a property sheet
		local target = r2:getPropertySheetTarget()
		local class =  r2:getClass(target)
		local prop = class.NameToProp[propName]
		--debugInfo(string.format("Setting node : prop name = %s, value = %s", tostring(propName), tostring(value)))		
		r2.requestRollbackLocalNode(target.InstanceId, propName)		
	end
	-- no op for forms (no 'commit' notion)
end

-------------------------------------------------------------------------------------
-- get the object whose property sheet is currently displayed
-- should be used only with property sheets
function r2:getPropertySheetTarget()
   if not r2.CurrentPropertyWindow then return nil end
	-- can only be the selection for now ...	
	return r2.CurrentPropertyWindow.Env.TargetInstance	
end


------------------------------------------------------------------------------------------------------------
-- get name of ui for default property sheet from the class name
function r2:getDefaultPropertySheetUIPath(className)
	return "ui:interface:r2ed_property_sheet_" .. className
end

------------------------------------------------------------------------------------------------------------
-- get definition of property edited from its widget
function r2:getPropertyDefinition(propName, uiCaller)	
	-- useful infos are stored in the parent container
	assert(uiCaller)
	local container = uiCaller:getEnclosingContainer()
	if container.Env.Form ~= nil then				
		-- this is a form (update is done in local)				
		return container.Env.Form.NameToProp[propName]
	else		
		-- this is a property sheet
		local target = r2:getPropertySheetTarget()
		local class =  r2:getClass(target)
		return class.NameToProp[propName]
	end
end




------------------------------------------------------------------------------------------------------------
-- return pointer to a property sheet of an object from its class name
function r2:getPropertySheet(instance)	
   local class = instance:getClass()
	local uiPath = r2:evalProp(class.PropertySheetUIPath, instance, r2:getDefaultPropertySheetUIPath(class.Name))	
	return getUI(uiPath)	
end

------------------------------------------------------------------------------------------------------------
-- get a form from its name
function r2:getForm(name)
	assert(name) -- why is nam nil???
	return getUI("ui:interface:r2ed_form_" .. name)
end

--/////////////////////
--//  NUMBER WIDGETS //
--/////////////////////

-- widget styles for 'Numbers'
r2.WidgetStyles.Number = 
{
	--------------------------------------------------------------------------------------------------------------------
	Default = function(prop, className)
		local function setter(widget, prop, value)
			widget.eb.input_string = tostring(value)
			widget.eb.Env.CurrString = tostring(value)
		end
		local onChangeAction = 
		string.format(
		[[			
			local editBox = getUICaller()
			if editBox.input_string == editBox.Env.CurrString then
				return
			end
			editBox.Env.CurrString = editBox.input_string
			local newValue = tonumber(getUICaller().input_string)
			if newValue == nil then
				debugInfo('Invalid number value : ' .. getUICaller().input_string)
				return
			end
			local prop = r2:getPropertyDefinition('%s', getUICaller())
			assert(prop)
			local clamped = false						
			if prop.Min and newValue &lt; tonumber(prop.Min) then
				newValue = tonumber(prop.Min)
				clamped = true
			end			
			if prop.Max and newValue &gt; tonumber(prop.Max) then
				newValue = tonumber(prop.Max)
				clamped = true
			end			
			if clamped then				
				editBox.input_string = tostring(newValue)				
				editBox.Env.CurrString = tostring(newValue)				
			end			
			r2:requestSetObjectProperty('%s', newValue)						
		]], prop.Name, prop.Name)
		return r2:buildEditBox(prop, "TR TR", "integer", false, 16, onChangeAction, onChangeAction), setter, nil
	end,
	--------------------------------------------------------------------------------------------------------------------
	Boolean = function(prop, className)	
		local function setter(widget, prop, value)
			--debugInfo("setter : " .. tostring(value))
			-- widget is a pointer to the enclosing group
			if value == 1 then
				value = true
			else
				value = false
			end
			widget.butt.pushed = value
			--widget.text_true.active = value
			--widget.text_false.active = not value
		end
		local function buildCoverAllButton(prop)
			return [[ <ctrl type="button" button_type="push_button"
				id="b_]] .. prop.Name .. [["
				tx_normal="blank.tga" tx_pushed="blank.tga" tx_over="blank.tga" 
				 sizeref="wh"							 
				 scale="true"
				 posref="ML ML"
				color="0 0 0 0"
				col_pushed="0 0 0 0"
				col_over="0 0 0 0"
				onclick_l="lua"
				params_l="local value; if getUICaller().parent:getEnclosingContainer():find(']] .. prop.Name .. [[').butt.pushed then value = 0 else value = 1 end; r2:requestSetObjectProperty(']] .. prop.Name .. [[', value)"
				 /> 
			 ]]			
		end
--		local widgetXml = 
--		string.format([[
--			<group id="%s" posref="TL TL" child_resize_h="true" x="2" child_resize_hmargin="4" child_resize_w="true" child_resize_wmargin="4"> 
--				<ctrl type="button" id="butt" button_type="toggle_button" posref="ML ML" x="2" y="0"
--				tx_normal="w_slot_on.tga" tx_pushed="w_opacity_on.tga" tx_over="w_slot_on.tga"
--				color="255 255 255 255"	col_pushed="255 255 255 255" col_over="255 255 255 0" 
--				onclick_l="lua"
--				params_l="local value; if getUICaller().pushed == true then value = 1 else value = 0 end; r2:requestSetObjectProperty('%s', value)"
--				/>
--				<view type="text" id="text_true" posparent="butt" posref="MR ML" x="4" y="-3" active="false" global_color="true" fontsize="12" shadow="true" hardtext="uiR2EDTrue"/>
--				<view type="text" id="text_false" posparent="butt" posref="MR ML" x="4" y="-3" active="true" global_color="true" fontsize="12" shadow="true" hardtext="uiR2EDFalse"/>
--			</group> 
--		]], prop.Name, prop.Name)
		local widgetXml = 
		string.format([[
			<group id="%s" posref="TL TL" child_resize_h="true" x="2" child_resize_hmargin="4" sizeref="w"> ]]
				.. buildCoverAllButton(prop) 	.. [[
				<ctrl type="button" id="butt" button_type="toggle_button" posref="ML ML" x="0" y="0"				
				tx_normal="w_slot_on.tga" tx_pushed="w_opacity_on.tga" tx_over="w_slot_on.tga"
				color="255 255 255 255"	col_pushed="255 255 255 255" col_over="255 255 255 0" 
				onclick_l="lua"
				params_l="local value; if getUICaller().pushed == true then value = 1 else value = 0 end; r2:requestSetObjectProperty('%s', value)"
				/> 
			</group> 
		]], prop.Name, prop.Name)
		-- Caption is special here :
		-- It contains a button to avoid to have to click in the tiny checkbox, which is inconvenient		
		local iwidth0 = defaulting(prop.CaptionWidth, 35)
		local width0 = string.format("%d%%", iwidth0)
		local width1 = string.format("%d%%", 100 - iwidth0)
		local invertWidget = defaulting(prop.InvertWidget, false)

		if invertWidget then
			local tmp = width0
			width0 = width1
			width1 = tmp
		end

		local part0 = [[<TD width=]] .. strifyXml(width0) .. [[ ignore_max_width="true" ignore_min_width="true" bgcolor="80 80 80 127" height="0" align="left" valign="middle" id= "l_]] .. prop.Name .. [[" > ]]
		
		
		local tooltipTextId, tooltipTextIdFound = buildPropTooltipName(className, prop.Name)
		part0 = part0 .. [[<group id="caption_group" sizeref="w" child_resize_h="true" child_resize_hmargin="4" posref="ML ML"
							tooltip_parent="win"
							tooltip_posref="auto"								
							instant_help="true"
							tooltip=]] .. strifyXml(tooltipTextId) .. ">"
		part0 = part0 .. buildCoverAllButton(prop)

		local color = "255 255 255 255"
		local globalColor = "true"
		local hardText
		local found
		hardText, found = r2:getPropertyTranslationId(prop)
		if not found and config.R2EDExtendedDebug == 1 then
			color = "255 0 0 255"
			globalColor = "false"
			hardText = hardText .. " (NOT TRANSLATED)"
		end	

		part0 = part0 .. [[ <view type="text" y="-2" sizeref="w" over_extend_view_text="true" over_extend_parent_rect="true"]] .. 
						[[ id =        ]] .. strifyXml(prop.Name .. "_Caption") ..
						[[ hardtext =  ]] .. strifyXml(hardText) ..				
						[[ color =  ]] .. strifyXml(color) .. 
						[[ global_color=]] .. strifyXml(globalColor) .. [[ fontsize="12" shadow="true" auto_clamp="true"/> ]]
		part0 = part0 .. "</group>"
		part0 = part0 .. "</TD>"
		
		--dumpSplittedString(widgetXml)
		return widgetXml, setter, part0
	end,
	--------------------------------------------------------------------------------------------------------------------
	Slider = function(prop, className)		
		local function setter(widget, prop, value)
			if widget.c.value ~= nil then
				widget.c.value = value
			end
		end		
		--
		local widgetXml = 
		string.format([[
		<group posref="TL TL" x="4" y="3" sizeref="w" w="-4" child_resize_h="true" child_resize_hmargin="3" id= ]] .. strifyXml(prop.Name) .. ">" .. [[
			<view type="bitmap" id="bk" posref="BL BL" scale="true" y="4" sizeref="w" h="2" texture="W_line_hor2.tga" />
			<ctrl type="scroll" id="c" posparent="bk" posref="MM MM" x="0" y="-1" sizeref="w" h="12"
								vertical="false" align="L" tracksize="12" 
								cancelable="true"
								tx_topright="w_scroll_R.tga" tx_middle="w_scroll_M.tga" tx_bottomleft="w_scroll_L.tga"
								onscroll="lua"		   params="r2:requestSetLocalObjectProperty('%s', getUICaller().value)"
								onscrollend="lua"      end_params="r2:requestCommitLocalObjectProperty('%s')"
								onscrollcancel="lua"   cancel_params="r2:requestRollbackLocalObjectProperty('%s')"
			min="%s"
			max="%s"
			step_value="%s"
			/>

			<group id="bitmaps" active="%s" y="7" posparent="bk" posref="TL BL" sizeref="w" child_resize_h="true" >

				<view type="bitmap" id="l_t" posref="BL BL" x="0" y="0" texture="%s" 
				/>

				<view type="bitmap" id="m_t" posref="BM BM" x="0" y="0" texture="%s" 
				/>

				<view type="bitmap" id="r_t" posref="BR BR" x="0" y="0" texture="%s" 
				/>

			"</group>"

		"</group>"
		]], prop.Name, prop.Name, prop.Name, defaulting(prop.Min, 0), defaulting(prop.Max, 100), defaulting(prop.StepValue, 1),
		 defaulting(prop.ActiveBitmaps, 'false'), defaulting(prop.LeftBitmap, ""), defaulting(prop.MiddleBitmap, ""), defaulting(prop.RightBitmap, ""))		
		--
		--debugInfo(string.format("Creating slider widget, min = %s, max = %s", defaulting(prop.Min, 0), defaulting(prop.Min, 100)))
		return widgetXml, setter	
	end,
	--------------------------------------------------------------------------------------------------------------------
	EnumDropDown = function(prop, className)
		if type(prop.Enum) ~= "table" then
			debugInfo("Can't create enum combo box, the 'Enum' table is not found or of bad type")
			return ""
		end
		local function setter(widget, prop, value)
			if widget.selection ~= nil then
				widget.parent.Env.Locked = true
				widget.selection = value
				widget.parent.Env.Locked = false
			end
		end
		local result = 
		[[
		<group type="combo_box" sizeref="w" w="-2" x="2" y="0" child_resize_h="true" child_resize_hmargin="10" linked_to_db="false" posref="TL TL" id=]]
		.. strifyXml(prop.Name) .. 
		string.format([[ on_change="lua" on_change_params="if getUICaller().parent.Env.Locked ~= true then  r2:requestSetObjectProperty('%s', getUICaller().selection) end"]], prop.Name) ..
		">"
		result = result .. [[<instance template="combo_box_def1" />]]
		--  append enumerated values
		
		for k, v in pairs(prop.Enum) do				
			result = result .. [[<combo_text name=]] .. strifyXml(tostring(v)) .. [[ />]]
		end				
		result = result .. "</group>"				
		return result, setter
	end,
}


------------------------------------------------------------------------------------------------------------
-- build a widget from the definition of the property
function r2:buildPropWidget(prop, className)
	local widgetFactory = r2.WidgetStyles[prop.Type]
	if widgetFactory == nil then
		--debugInfo("Type '" .. tostring(prop.Type) .. "' not found. Widget not built")
		return nil
	end
	local widgetStyle = prop.WidgetStyle
	if widgetStyle == nil then
		widgetStyle = "Default"
	end
	if widgetFactory[widgetStyle] == nil then
		debugInfo("Widget style '" .. tostring(widgetStyle) .. "' not found for type '" .. tostring(prop.Type) ..
		          "', widget not built" )
		return nil
	end
	local result, setter, caption = widgetFactory[widgetStyle](prop, className)
	-- add common functionnality of setter
	if setter ~= nil then
		-- if there's a conversion function on set, then call it
		if prop.convertToWidgetValue ~= nil then			
			if type(setter) == "function" then
				local oldSetter = setter
				setter = function(widget, prop, value)
					oldSetter(widget, prop, prop.convertToWidgetValue(value))
				end
			else
				assert(type(setter) == "table")
				if (setter.onSet) then
					local oldSetter = setter.onSet
					function setter:onSet(widget, prop, value)
						self:oldSetter(widget, prop, prop.convertToWidgetValue(value))
					end
				end
			end
		end
	end
	return result, setter, caption
end

------------------------------------------------------------------------------------------------------------
-- create a table of properties
function r2:createPropertyXmlTable(props, className, posparent, posref, x, y, widgetEventHandlerTable)
	local result = "" -- the resulting xml			  
	-- add a new string to the resulting string
	local function add(value)
		result = result .. value
	end
	add([[<group type="table" sizeparent="parent"]] ..
	   [[ posparent= ]] .. strifyXml(posparent) ..
		[[ posref=    ]] .. strifyXml(posref) ..
		[[ x=         ]] .. strifyXml(x) ..
		[[ y=         ]] .. strifyXml(y) ..
		[[ id="prop_table" sizeref="w" width="100%" border="0" bgcolor="0 0 0 255"
		 cellspacing="1"
		 cellpadding="0"
		 continuous_update="true"
		>	
		]])
	
	for key, prop in pairs(props) do      
		local widgetXmlDesc, setter, captionXmlDesc = self:buildPropWidget(prop, className)
		if widgetXmlDesc ~= nil then
			add("<TR>")
			-- build the caption			
			local color = "255 255 255 255"
			local globalColor = "true"
			--if SeenNames[prop.Name] == nil then
			--	debugInfo(prop.Name)			
			--	SeenNames[prop.Name] = true
			--end

			local hardText
			local found
			hardText, found = r2:getPropertyTranslationId(prop)
			if not found and config.R2EDExtendedDebug == 1 then
				color = "255 0 0 255"
				globalColor = "false"
				hardText = hardText .. " (NOT TRANSLATED)"
			end	
			--			
			local iwidth0 = defaulting(prop.CaptionWidth, 35)
			local width0 = string.format("%d%%", iwidth0)
			local width1 = string.format("%d%%", 100 - iwidth0)
			local invertWidget = defaulting(prop.InvertWidget, false)
	
			if invertWidget then
				local tmp = width0
				width0 = width1
				width1 = tmp
			end
			
			local tooltipTextId, tooltipTextIdFound = buildPropTooltipName(className, prop.Name)
			--debugInfo(string.format("%60s %s", tooltipTextId, "	[@{F00F} FILL ME ! :" .. prop.Name .. "]"))

			local part0
			if not captionXmlDesc then
				part0 = [[<TD width=]] .. strifyXml(width0) .. [[ ignore_max_width="true" ignore_min_width="true" bgcolor="80 80 80 127" height="0" align="left" valign="middle" id= "l_]] .. prop.Name .. [[" > ]]

				part0 = part0 .. [[<group id="caption_group" sizeref="w" child_resize_h="true" posref="ML ML"
									tooltip_parent="win"
									tooltip_posref="auto"								
									instant_help="true"
									tooltip=]] .. strifyXml(tooltipTextId) .. ">"
				part0 = part0 .. [[ <view type="text" y="-2" sizeref="w" over_extend_view_text="true" over_extend_parent_rect="true"]] .. 
								[[ id =        ]] .. strifyXml(prop.Name .. "_Caption") ..
								[[ hardtext =  ]] .. strifyXml(hardText) ..				
								[[ color =  ]] .. strifyXml(color) .. 
								[[ global_color=]] .. strifyXml(globalColor) .. [[ fontsize="12" shadow="true" auto_clamp="true"/> ]]
				part0 = part0 .. "</group>"
				part0 = part0 .. "</TD>"
			else				
				part0 = captionXmlDesc
			end

			-- build the widget					
			local part1 = [[<TD width=]] .. strifyXml(width1) .. [[ ignore_max_width="true" ignore_min_width="true" bgcolor="64 64 64 127" height="0" align="left" valign="middle" id= "r_]] .. prop.Name .. [[" > ]]
			part1 = part1 .. [[<group id="widget_group" sizeref="w" child_resize_h="true" posref="ML ML"
							    tooltip_parent="win"
								tooltip_posref="auto"
								tooltip_posref_alt="TL TR"
								instant_help="true"
								tooltip=]] .. strifyXml(tooltipTextId) .. ">"			                 
			part1 = part1 .. widgetXmlDesc .. [[</group></TD>]]			

			if invertWidget then
				add(part1 .. part0)
			else
				add(part0 .. part1)
			end
			-- add the setter function in the table
			--debugInfo("inserting entry" .. prop.Name)
			if type(setter) == "function" then
				-- setter is a plain 'set' function
				-- => event handler is a simple table with a 'onSet' method ...
				widgetEventHandlerTable[prop.Name] = 
				{
					onSet= function(this, widget, prop, value)
						setter(widget, prop, value)
					end
				}					
			else
				-- debugInfo(prop.Name .. " : " .. type(setter))
			    -- setter is a complete object (a lua table) with event handling methods
				if type(setter) ~= "table" then 
               debugInfo(type(setter))
               inspect(setter)
               assert(0)
            end            
				widgetEventHandlerTable[prop.Name] = setter
			end			
			add("</TR>")
		end
	end	
	add([[</group>]])
	return result
end

------------------------------------------------------------------------------------------------------------
-- build xml for a rollout containing a table of properties
function r2:buildPropRolloutXml2(className, caption, id, posparent, posref, props, rolloutY, widgetEventHandlerTable, isForm)	
	local content = self:createPropertyXmlTable(props, className, "caption", "BL TL", 0, -4, widgetEventHandlerTable)
	-- todo : use something more clear than string.format !!!
	local global_color_over = "255 255 255 192"
	local params_l = "r2:openCloseRollout('prop_table')"
	if isForm then
		global_color_over = "127 127 127 127"
		params_l = ""
	end
	local result = string.format(
	[[ <group id ="%s" x = "0" y="%s" sizeref="w" child_resize_h="true" posparent="%s" posref="%s" w="0">
			<group id="caption" sizeref="w" w="0" child_resize_h="true" child_resize_hmargin="2" posref="TL TL">				
				<ctrl type="button" id="rollout_button" button_type="push_button" sizeref = "wh" posref="TL TL"				 
				 tx_normal="grey_40.tga" 
				 tx_pushed="grey_80.tga" 
				 tx_over="grey_60.tga"
				 scale="true"
				 tooltip=""				 
				 global_color="true"
				 global_color_normal="127 127 127 127"
				 global_color_over="%s"
				 global_color_pushed="false"
				 onclick_l="lua"
				 params_l="%s"
				/>	
		]], id, rolloutY, posparent, posref, global_color_over, params_l)
		

		if not isForm then
			result = result ..
			string.format([[
				<group id="open_indicator" child_resize_h="true" x="3" y="-1" child_resize_w="true" posref="ML ML" posparent="rollout_button">
					<view type="bitmap"  render_layer="2" id="opened" active="true" posref="TL TL" texture="rollout_opened.tga" color="255 255 255 255"/>
					<view type="bitmap"  render_layer="2" id="closed" active="false" posref="TL TL" texture="rollout_closed.tga" color="255 255 255 255"/>
				</group>
				<view type="text" render_layer="2" id="rollout_text" auto_clamp="true" posref="MR ML" x="2" y="-1" posparent="open_indicator" global_color="true" fontsize="12" shadow="false" hardtext="%s"
				 over_extend_view_text="true" over_extend_parent_rect="true"
				/>]], caption)
		else
			result = result ..
			string.format([[<view type="text" render_layer="2" id="rollout_text" auto_clamp="true" posref="ML ML" x="2" y="-1" posparent="parent" global_color="true" fontsize="12" shadow="false" hardtext="%s"
							over_extend_view_text="true" over_extend_parent_rect="true"
						/>]], caption)
		end
		
		result = result .. string.format(
			[[			<view type="bitmap"  render_layer="2" scale="true" id="top" sizeref="w" h="1" posref="TL TL" texture="blank.tga" color="0 0 0 255"/>				
						<view type="bitmap" render_layer="2" scale="true" id="left" sizeref="h" w="1" posref="TL TL" texture="blank.tga" color="0 0 0 255"/>
						<view type="bitmap" render_layer="2" scale="true" id="right" sizeref="h" w="1" posref="TR TR" texture="blank.tga" color="0 0 0 255"/>				
					</group>
					<view type="bitmap" scale="true" render_layer="2" id="bottom" posparent="caption" sizeref="w" h="1" y="0" posref="BL TL" texture="blank.tga" color="0 0 0 255"/>
					
					%s <!-- the content -->
				</group>
			]],content)	

	   return result
end

-- build xml for a rollout containing a table of properties
function r2:buildPropRolloutXml(caption, id, posparent, posref, props, className, rolloutY, widgetEventHandlerTable, isForm)	
	local content = self:createPropertyXmlTable(props, className, "caption", "BL TL", 0, -4, widgetEventHandlerTable)
	-- add the caption
	local result = string.format(
								 [[ <group id ="%s" x = "0" y="%s" sizeref="w" 
								     child_resize_h="true" posparent="%s" 
									 posref="%s" w="0"> ]], id, rolloutY, posparent, posref)

	local color = "255 255 255 255"
	local globalColor = "true"	
	--if SeenRolloutCaptions[caption] == nil then
	--	debugInfo(caption)			
	--	SeenRolloutCaptions[caption] = true
	--end	
	if not i18n.hasTranslation(caption) then			
		if config.R2EDExtendedDebug == 1 then
			color = "255 0 0 255"
			globalColor = "false"			
			caption = caption .. "(NOT TRANSLATED)"
		end
	end		

	-- add the rollout bar
	if not isForm then
		result = result .. 
				[[ <instance template="rollout_bar" caption=]] .. strifyXml(caption) .. 
				[[ color=]] .. strifyXml(color) ..
				[[ global_color=]] .. strifyXml(globalColor) ..
				[[ content_name="prop_table"/> ]]		
	else
		result = result .. 
				 [[ <instance template="form_bar" caption=]] .. strifyXml(caption) .. 
				 [[ color=]] .. strifyXml(color) ..
				 [[ global_color=]] .. strifyXml(globalColor) ..
		         [[ /> ]]
	end
	
	-- add the content
	result = result .. content

	-- close
	result = result .. "</group>"

	return result
end





----------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------
-- Generation of property sheet ui xml code from a class definition
-- this function will return 2 values :
-- xml code to generate the dialog to edit instance of the class
-- A registration function for that dialog, that must be called prior to use (and once the ui has been created from the xml code)
-- Note : Registration isn't done at creation in order to pack several class descriptions in 
-- a single xml script to speed up parsing

function r2:buildPropertySheetXml(class, className, id, title, isForm)
	
	-- The following table contains for each property a set method that allow to change it from the outside	
	-- these value is filled by each widget "ctor" function.
	-- In a similar way, each widget is given a setter function.	
	local widgetEventHandlerTable = {}
	-- 
	--local function dump()
	--	for key, value in pairs(widgetEventHandlerTable) do
	--			debugInfo('*')
	--			debugInfo(tostring(key, value))
	--	end
	--end
	--
	local result = "" -- the resulting xml			  
	-- add a new string to the resulting string
	local function add(value)
		result = result .. value
	end
		

	local rolloutsX = 12
	local rolloutsW = -14
	if isForm then
		rolloutsX = 0
		rolloutsW = -2
	end
	
	
	

	add(	
	[[		
		<group type="container" savable="false" title_delta_max_w="]] .. tostring(r2.DefaultPropertySheetTitleClampSize) .. [[" title="]] .. title .. [[" global_color="false" line_at_bottom="false"
		    active="false"
		    escapable="true"
			movable="true" opened="true" openable="false" header_color="UI:SAVE:WIN:COLORS:]] .. select(isForm, "R2_FORM", "R2_PROP_WINDOW") .. 
			[[" pop_min_h="32" pop_max_h="10000"
			on_active="lua"			
	]])
	---------------------------------
	if isForm then -- for forms, closing the form is equivalent to clicking on 'cancel'
	  add(' resizer="true" ')		
      local w = defaulting(class.Width, 500)      
      add(' pop_min_w=' .. strifyXml(w))
      add(' pop_max_w=' .. strifyXml(w))      
      add(' w=' .. strifyXml(w))
		local cancelCode = 
		[[	local form = getUICaller()
			if form.Env.Choice == nil then
				form.Env.Choice= 'Cancel'
				if form.Env.onCancel then
					form.Env.onCancel(form.Env.FormInstance, form)		
				end
			end
		]]
		add(string.format([[ on_deactive="lua" on_deactive_params="%s" on_escape="lua" on_escape_params="%s" ]], cancelCode, cancelCode))
		add([[on_enter="lua" on_enter_params="r2:validateForm(getUICaller())" ]])
		add([[on_alpha_settings_changed="lua:r2:onFormAlphaSettingsChanged()" ]])
		add([[on_active_params="if r2 ~= nil then
									local updateAll = getUICaller().Env.updateAll
									if updateAll then updateAll() end
									r2:restoreContAlphaSettings('R2_FORM')
							  end;" ]])		
	else
		add(' resizer="true" ')
		add('pop_min_w="150"')
		add(' pop_max_w="800" ')
		add(' w="250" ')
		add([[on_alpha_settings_changed="lua:r2:onPropertySheetAlphaSettingsChanged()" ]])
		add([[on_active_params="if r2 ~= nil then
									local updateAll = getUICaller().Env.updateAll
									if updateAll then updateAll() end
									r2:restoreContAlphaSettings('R2_PROP_SHEET')
							  end;" ]])
	end

	---------------------------------
	if not isForm then -- for property sheets, closing the sheet is equivalent to clicking on the 'toggle property window' button		
		add([[ on_deactive="lua" on_deactive_params="r2.PropertyWindowVisible = false" ]])
	end
	---------------------------------





	add([[id=]] .. strifyXml(id) .. [[
		>]])
	


	if className == "NpcCustom" then
		add(
		[[			
			<group id="header_opened" x="0" y="0" w="0" h="16" posref="TL TL"
			 group_onclick_r="active_menu"
		     group_params_r="menu=ui:interface:base_menu_with_color"
			>	
		
				<ctrl style="button_ok" id="help" active="true" x="-16" y="0" posref="MR MR" color="255 255 255 255"
				text_y="-2" onclick_l="lua" params_l="r2.displayFeatureHelp('Npc')" hardtext="uiR2EdHelp" fontsize="10" />
			</group>
		]])
	elseif r2.hasDisplayInfo(className) == 1 or string.find(className, "_Form") ~= nil then	
		debugInfo("Adding help header for "..className)
		local featureName = className
		if string.find(className, "_Form") ~= nil then
			local len = string.len(featureName)
			featureName = string.sub(featureName, 1, len - 5)	
		end

		add(
		[[			
			<group id="header_opened" x="0" y="0" w="0" h="16" posref="TL TL"
			 group_onclick_r="active_menu"
		     group_params_r="menu=ui:interface:base_menu_with_color"
			>	
		
				<ctrl style="button_ok" id="help" active="true" x="-16" y="0" posref="MR MR" color="255 255 255 255"
				text_y="-2" onclick_l="lua" params_l="r2.displayFeatureHelp(']]..featureName..[[')" hardtext="uiR2EdHelp" fontsize="10" />
			</group>
		]])
	else		
		add([[<group id="header_opened" x="0" y="0" w="430" h="16" posref="TL TL" 
			   group_onclick_r="active_menu"
			   group_params_r="menu=ui:interface:base_menu_with_color"
			  >		
			  </group>]])
	end
	
	
	add([[
		<group id="content" x="0" y="-4" sizeref="wh" w="0" h="0" posref="TL TL" >		
		<group id="enclosing" sizeref="wh" posref="TL TL" x="0" y="-4" w="0">
		
		<group id="rollouts" sizeref="w" w="]] .. rolloutsW .. [[" posref="TL TL" x="]] .. rolloutsX .. [[" y="0" max_h="-4" max_sizeref="h" child_resize_h="true"
		 child_resize_hmargin="4"
		>
		
	]])
	
	

	-- sort properties by category
	local categories = {}
	local numCategories = 0
	for key, prop in pairs(class.Prop) do
		if prop.Visible ~= false then
			local category = prop.Category
			if category == nil then
				category = "uiR2EDRollout_Default"
			end		
			if categories[category] == nil then
				--debugInfo("Adding new category " .. tostring(category))
				-- create a new table if no entries for that category of properties
				categories[category] = {}
				numCategories = numCategories + 1
			end
			table.insert(categories[category], prop)
		end		
	end

	local posparent = "parent"
	local posref= "TL TL"
	local rolloutY = -4

	-- if there's a xml property sheet header, use it
	if class.PropertySheetHeader then
		-- enclose the header in a group to keep good width

		add([[<group id="sheet_header" posref="TL TL" y="-4" sizeref="w" child_resize_h="true" child_resize_hmargin="0">]] ..
		    class.PropertySheetHeader ..
			[[</group>]]
		   )
		posref="BL TL"
		posparent="sheet_header"
	end

	-- if there's just a 'default' category, then don't create a rollout
	if numCategories == 1 and categories["uiR2EDRollout_Default"] ~= nil then				
		add(self:createPropertyXmlTable(categories["uiR2EDRollout_Default"], className, posparent, posref, 0, rolloutY, widgetEventHandlerTable))
		posparent="prop_table"
	else		
		-- add each rollout and its properties
		rolloutY = -2
		local categoryIndex = 0		
		for k, v in pairs(categories) do
			add(self:buildPropRolloutXml(k, tostring(categoryIndex), posparent, posref, v, className, rolloutY, widgetEventHandlerTable, isForm))
			posparent = tostring(categoryIndex)
			posref ="BL TL"
			rolloutY = -4
			categoryIndex = categoryIndex + 1			
		end
	end

	-- if the dialog is a form, then add 'ok' & 'cancel' button
	if isForm then
		add([[ <group id="okcancel" posparent="]] .. posparent .. [[" posref="BL TL" y="-4" sizeref="w" child_resize_h="true">
					<ctrl style="text_button_16" id="validate" posref="MM ML" x="12" wmargin="8" color="255 255 255 255" col_over="255 255 255 255" col_pushed="255 255 255 255" onclick_l="lua" params_l="r2:validateForm(getUICaller().parent:getEnclosingContainer())" hardtext="uiR2EDValidateForm"/>
					<ctrl style="text_button_16" id="cancel" posref="MM MR" x="8" wmargin="8" color="255 255 255 255" col_over="255 255 255 255" col_pushed="255 255 255 255" onclick_l="lua" params_l="r2:cancelForm(getUICaller().parent:getEnclosingContainer())" hardtext="uiR2EDCancelForm"/>
			   </group>
			]])
	end
	

	-- close the dialog
	add([[	</group> <!-- rollout --> ]])
	if not isForm then
		-- scroll bar for property sheet only
		add([[ <ctrl style="skin_scroll" id="scroll_bar" align="T" target="rollouts" y="4"/>]])
	end
	add([[
					</group>  <!-- enclosing -->
				</group> <!-- content -->
			</group>	
			<tree node=]] .. strifyXml(id) .. [[ >
			</tree>	
		]])
	
	
--///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
--///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		

	-- THE REGISTRATION FUNCTION, called after the ui has been created for real from the xml definition
	local function registrationFunction()		
		local propertySheet = getUI("ui:interface:" .. id) -- tmp : hardcoded name here
		local form = propertySheet -- alias, if the dialog is actually a form
		local propNameToWidget = {}		
		local propNameToLCaption= {}
		local propNameToRCaption= {}
		-- keep a pointer to each widget
		-- can only do it now, because this registration is called only once the ui has been created
		for k, prop in pairs(class.Prop) do
			propNameToWidget[prop.Name] = propertySheet:find(prop.Name)
			propNameToLCaption[prop.Name] = propertySheet:find("l_" .. prop.Name)
			propNameToRCaption[prop.Name] = propertySheet:find("r_" .. prop.Name)
		end
		-- Fucntion to retrieve a reference on the currently edited object
		local function getTargetInstance()
			if isForm then					
				return form.Env.FormInstance
			else
				return r2:getPropertySheetTarget()					
			end
		end		
		------------------------------------------------------		
		if propertySheet ~= nil then -- should not be nil if parsing succeeded
			-- create a handleEvent function and put it in the lua environment of the property sheet
			-- (in the C++ code, each group in the interface, such as a container, has a 'Env' lua table attached to it)
			function propertySheet.Env.handleEvent(eventName, attributeName, args)            
				-- TODO nico : arrays not handled yet
				if propertySheet.active == false then
					return	-- no update if dialog not visible (updateAll() is called when dialog is shown again)
				end				
				local targetInstance = getTargetInstance()
				local widgetEventHandler = widgetEventHandlerTable[attributeName] -- closure here : use locally defined widgetEventHandlerTable
				if widgetEventHandler == nil then
					return -- no display for that widget
				end
				local handlingMethod = widgetEventHandler[eventName]
				if handlingMethod == nil then
					-- no handler for this event, just return
					debugInfo("Event not handled for : " .. attributeName)
					inspect(getTargetInstance())
					return
				end
				local propWidget = propNameToWidget[attributeName]	 -- closure here : use locally defined propertySheet
																		 -- find the name of the widget by its id
				  if propWidget == nil then
					debugInfo("Can't retrieve widget associated with property '" .. attributeName .. "'")
					return
				  end		
				-- call the actual event handler with the widget as its first parameter											
				handlingMethod(widgetEventHandler, propWidget, class.NameToProp[attributeName], getTargetInstance()[attributeName], unpack(args))				
			end			
			local handleEventFunction = propertySheet.Env.handleEvent
			-- syntaxic sugar : 'setter' function for simple set operation
			function propertySheet.Env.setter(attributeName, value)
				table.clear(eventArgs)
				table.insert(eventArgs, value)
				handleEventFunction("onSet", attributeName, eventArgs)
			end
			local setter = propertySheet.Env.setter
			------------------------------------------------------------------------------------------------------------------
			-- this function is called when the property sheet is shown for the first time 
			-- in order to update its content
			local function updateAll()				
				--debugInfo("updateAll")
				local target = getTargetInstance()
				if not target then return end -- 'updateAll' will be triggered at init time when
                                              -- windows are positionned
				for k, prop in pairs(class.Prop) do
					-- update the 'visible' state of each property
					local active = true
					if type(prop.Visible) == "function" then
						active = prop.Visible(target)


						local oldActive = propNameToLCaption[prop.Name].active
						propNameToLCaption[prop.Name].active = active
						propNameToRCaption[prop.Name].active = active

						if oldActive ~= active then
							propNameToLCaption[prop.Name]:invalidateCoords()
							propNameToLCaption[prop.Name]:updateCoords()
						end
						
						
					end										
					setter(prop.Name, target[prop.Name]) -- retrieve value from object and assign to setter					
				end
				propertySheet:invalidateCoords()
				propertySheet:updateCoords()
				debugInfo("*")
			end	
							
			propertySheet.Env.updateAll = updateAll			
		end
   ------------------------------------------------------------------------------------------------------------------
		-- this function is called by forms or proprty sheets to update visible properties
		-- when visibility depends on other properties
		local function updatePropVisibility()				
			local target = getTargetInstance()				
			local modified = false
			for k, prop in pairs(class.Prop) do
				-- update the 'visible' state of each property
				local active = true               
				if type(prop.Visible) == "function" then
					local active = prop.Visible(target)
					if active ~= propNameToLCaption[prop.Name].active then
						modified = true
						propNameToLCaption[prop.Name].active = active
						propNameToRCaption[prop.Name].active = active
						propNameToRCaption[prop.Name]:invalidateContent()                  
					end
				end					
			end
         return modified
		end		
		------------------------------------------------------
		propertySheet.active = false
		-- if this is a form, then the dialog should be resized when text is entered (for multi line texts)		
		-- (else no-op for update coords)
		-- We should resize it by script, because by default, container impose size of their children
		if isForm then
			function propertySheet.Env.updateSize()
				local rollouts = propertySheet:find("rollouts")				
				local deltaH = 40				
				propertySheet:invalidateCoords()
				propertySheet:updateCoords()
				local newHReal = rollouts.h_real				
				-- must resize the parent
				local newH = newHReal + deltaH
				local yOffset = newH - propertySheet.h
				--propertySheet.h = newH
				propertySheet.y = propertySheet.y + yOffset / 2 
				propertySheet.pop_min_h = newH 
				propertySheet.pop_max_h = newH 
				propertySheet:invalidateCoords()
				propertySheet:updateCoords()				
			end
			propertySheet.Env.updatePropVisibility = function()
				local modified = updatePropVisibility() -- call local function defined above
				-- for forms, update size if content was modified
				if modified then 
					propertySheet.Env.updateSize()
				end
			end
		else
			propertySheet.Env.updatePropVisibility = updatePropVisibility -- call local function defined above
			function propertySheet.Env.updateSize()
				-- no op for property sheet (user can change the size of the window)
			end
		end
	end
		
	
	return result, registrationFunction
end

--///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
--///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		


------------------------------------------------------------------------------------------------------------
-- called when the user has clicked on a rollout
-- TODO nico : also used by the scenario window, so factor the code
function r2:setRolloutOpened(rollout, content, opened)
	if not rollout or not content then
		debugInfo("r2:setRolloutOpened : rollout or content is nil")
		return
	end
	content.active = opened	
	rollout:find("open_indicator").opened.active = opened
	rollout:find("open_indicator").closed.active = not opened
end


------------------------------------------------------------------------------------------------------------
function r2:buildAllPropertySheetsAndForms()
	-- TMP TMP
	--SeenNames = {}
	--SeenRolloutCaptions = {}
	local xmlScript = ""
	local registrationFunctions = {}
	--debugInfo('building property sheets')
	local mustReparse = false
	for k, class in pairs(r2.Classes) do
		-- only build if class require a property sheet
		--if class.BuildPropertySheet == true then			
		if true then
			-- to avoid costly parsing, see in the cache if the class was not already parsed
			local mustRebuild = true			
			local propSheetCacheInfo 
			if class.ClassMethods then
				propSheetCacheInfo = class.ClassMethods.getGenericPropertySheetCacheInfos(class)
				if r2ClassDescCache[class.Name] ~= nil then
					if isEqualIgnoreFunctions(r2ClassDescCache[class.Name], propSheetCacheInfo) then
						mustRebuild = false					
					else
						debugInfo("Class " .. class.Name .. " found in cache, but different, rebuilding")					
					end
				end
			end
			local result, registerFunc = r2:buildPropertySheetXml(class, class.Name, "r2ed_property_sheet_" .. class.Name, "uiR2EDProperties", false)
			table.insert(registrationFunctions, registerFunc)
			if mustRebuild then
				xmlScript = xmlScript .. result
				mustReparse = true
				r2ClassDescCache[class.Name] = propSheetCacheInfo -- update cache
				--debugInfo("Rebuilding property sheet")
			end			
		end
	end


	-- register feature Forms
	do
		local k, feature = next (r2.Features, nil)
		while  k do
			if feature.registerForms then
				feature.registerForms()
			end
			k, feature = next (r2.Features, k)
		end
	end
	
	-- register userComponentManager forms
	do
		if r2_core.UserComponentManager then
			debugInfo("Registering UserComponentManager forms...")
			r2_core.UserComponentManager.registerForms()
		end
	end
	
	--debugInfo('building forms')
	if r2.Forms ~= nil then
		for formName, form in pairs(r2.Forms) do
			local mustRebuild = true			
			if r2FormsCache[formName] ~= nil then
				if isEqualIgnoreFunctions(r2FormsCache[formName], form.Prop) then
					mustRebuild = false
				end
			end	
			local result, registerFunc = r2:buildPropertySheetXml(form, formName, "r2ed_form_" .. formName, "uiR2EDForm", true) 
			table.insert(registrationFunctions, registerFunc)
			if mustRebuild then
				xmlScript = xmlScript .. result
				mustReparse = true
				r2FormsCache[formName] =  form.Prop -- updating cache
				--debugInfo("Rebuilding property form")
			end
		end
	end
	--debugInfo('parsing')	
	if mustReparse then		
		debugInfo(colorTag(255, 0, 255) .. "Reparsing generated xml")
		local startTime = nltime.getLocalTime()
		-- TMP TMP
		--local f = io.open("testui.log", "w")
		--f:write(r2:encloseXmlScript(xmlScript))
		--f:flush()
		--f:close()
		parseInterfaceFromString(ucstring(r2:encloseXmlScript(xmlScript)):toUtf8())
		local endTime = nltime.getLocalTime()
		debugInfo(string.format("Reparsing generated xml took %f second", (endTime - startTime) / 1000))
	end
	-- do the registration
	for k, regFunc in pairs(registrationFunctions) do
		regFunc()
	end	
	-- center all windows at start		
	local function initPropertySheetPos(wnd)
		if wnd == nil then return end
		wnd.active = true
		r2:initDefaultPropertyWindowPosition(wnd) -- position definition in r2_ui_windows.lua
		wnd.active = false		
	end	
	--
	for k, class in pairs(r2.Classes) do
		-- only build if class require a property sheet
		if class.BuildPropertySheet == true then		
			--debugInfo('*3')
			local wnd = getUI(r2:getDefaultPropertySheetUIPath(class.Name))
			if wnd ~= nil then
			r2:adjustPropertySheetTitleClampSize(wnd)
				initPropertySheetPos(wnd)				
			end						
		end
	end	
	if r2.Forms ~= nil then
--		for formName, form in pairs(r2.Forms) do
			for formName, form in pairs(r2.Forms) do
				local wnd = r2:getForm(formName)
				if wnd then

					-- prevent update of window content here (no selection exists)
					local oldOnActiveParams = wnd.on_active_params
					local oldOnDeactiveParams = wnd.on_deactive_params
					wnd.on_active_params = ""
					wnd.on_deactive_params = ""
					wnd.active = true
					r2:adjustPropertySheetTitleClampSize(wnd)
					wnd.Env.updateSize()										
					wnd.active = false
					wnd.on_active_params = oldOnActiveParams
					wnd.on_deactive_params = oldOnDeactiveParams
					

					--wnd.active = true
					--wnd:invalidateCoords()
					--wnd:updateCoords()
					--local maxH = wnd:find("rollouts").h_real					
					--wnd.pop_max_h = maxH + 40
					--wnd.pop_min_h = maxH + 40
					--wnd:center()
					--wnd:invalidateCoords()
					--wnd:updateCoords()
					--wnd:center()
					-- force the y size (no scroll) by evaluating content size					
					--wnd.active = false
				end
			end
--		end
	end
		
	-- force to center all windows at start
	--debugInfo('*4')
	initPropertySheetPos(getUI("ui:interface:r2ed_property_sheet_no_properties"))
	initPropertySheetPos(getUI("ui:interface:r2ed_property_sheet_no_selection"))
	
	--debugInfo('*5')
end

------------------------------------------------------------------------------------------------------------
-- TMP
function r2:testPropertySheet()	
	-- tmp	
	local result, registrationFunction = r2:buildPropertySheetXml(r2.Classes.MapDescription, "r2ed_property_sheet")	
	-- parse & register
	parseInterfaceFromString(r2:encloseXmlScript(result)) -- parseInterfaceFromString :C++ function exported from the interface manager to parse an new interface
	                                 -- from its xml description	
	registrationFunction()	
	local propertySheet = getUI("ui:interface:r2ed_property_sheet")
	if propertySheet ~= nil then
		propertySheet.active = false
		propertySheet.active = true -- force an update
		propertySheet:center()
		updateAllLocalisedElements()
	end
end


------------------------------------------------------------------------------------------------------------
-- Displayer to update the content of the property sheet
local propertySheetDisplayerTable = {}

------------------------------------------------
function propertySheetDisplayerTable:onCreate(instance)	
end
------------------------------------------------
function propertySheetDisplayerTable:onErase(instance)		
end
------------------------------------------------
function propertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function propertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function propertySheetDisplayerTable:onFocus(instance, hasFocus)		
end
------------------------------------------------
function propertySheetDisplayerTable:onSelect(instance, isSelected)		
end

------------------------------------------------
-- handle an event on a property, possibly with additionnal parameters
function propertySheetDisplayerTable:handleAttrEvent(eventName, instance, attributeName, args)	
	-- TODO nico : arrays not handled yet
	local class = instance:getClass()
	if not class.BuildPropertySheet then
		return false
	end			
	local propertySheet = r2:getPropertySheet(instance)
	if propertySheet == nil or propertySheet.active == false then		
		return false
	end		
	--debugInfo("Property sheet test")
	--debugInfo(ct .. "Instance " .. instance.InstanceId .." has an attribute modified : " .. attributeName)
	-- call event handler into the ui				
	propertySheet.Env.handleEvent(eventName, attributeName, args)
	return true
end
------------------------------------------------
function propertySheetDisplayerTable:onAttrModified(instance, attributeName, indexInArray)	
	-- TODO nico : arrays not handled yet		
	if self.handleAttrEvent and self:handleAttrEvent("onSet", instance, attributeName, emptyArgs) then		
		-- special case for "Name" : title of the container depends on it
		-- update if the property sheet is visible
		local propertySheet = r2:getPropertySheet(instance)
			if propertySheet then
				if attributeName == "Name" and instance == r2:getSelectedInstance() then
					propertySheet.uc_title = concatUCString(i18n.get("uiRE2DPropertiesOf"), instance:getDisplayName())
				end
				propertySheet.Env.updatePropVisibility()
			end
	end	
end		
------------------------------------------------
function propertySheetDisplayerTable:onTargetInstanceCreated(instance, refIdName, refIdIndexInArray)
	-- TODO nico : arrays not handled yet	
	self:handleAttrEvent("onTargetCreated", instance, refIdName, emptyArgs)
end
------------------------------------------------
function propertySheetDisplayerTable:onTargetInstanceErased(instance, refIdName, refIdIndexInArray)	
	-- TODO nico : arrays not handled yet	
	self:handleAttrEvent("onTargetErased", instance, refIdName, emptyArgs)
end
------------------------------------------------
function propertySheetDisplayerTable:onTargetInstancePreHrcMove(instance, refIdName, refIdIndexInArray)
	-- TODO nico : arrays not handled yet	
	self:handleAttrEvent("onTargetPreHrcMove", instance, refIdName, emptyArgs)
end
------------------------------------------------
function propertySheetDisplayerTable:onTargetInstancePostHrcMove(instance, refIdName, refIdIndexInArray)	
	-- TODO nico : arrays not handled yet	
	self:handleAttrEvent("onTargetPostHrcMove", instance, refIdName, emptyArgs)
end

------------------------------------------------
function propertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
	-- TODO nico : arrays not handled yet	
	--debugInfo(tostring(refIdName))
	--debugInfo(tostring(refIdIndexInArray))
	--debugInfo(tostring(targetAttrName))
	--debugInfo(tostring(targetAttrIndexInArray))
	table.clear(eventArgs)
	table.insert(eventArgs, targetAttrName)
	table.insert(eventArgs, targetAttrIndexInArray)
	self:handleAttrEvent("onTargetAttrModified", instance, refIdName) -- additionnal parameters
end

function r2:propertySheetDisplayer()	
	return propertySheetDisplayerTable -- generic property displayer is shared by all instance
end


-- last coordinate of property window (for generic property window)
r2.PropertyWindowCoordsBackup = nil

-- see if a property sheet window coords must be backuped
function r2:getPropertySheetBackupFlag(wnd)		
	local result = true
	if not wnd then return false end
	local targetInstance = wnd.Env.TargetInstance
	if targetInstance then		
		local targetClass = r2:getClass(targetInstance)				
		if targetClass and not targetClass.isNil then			
			result =  defaulting(targetClass.BackupPropertySheetSize, true)
		end
	end	
	return result
end
 
------------------------------------------------------------------------------------------------------------
-- 'show properties' handler : display properties for selected instance
-- TODO nico : maybe would better fit inside 'r2_ui_windows.lua' ?
function r2:showProperties(instance)

	r2.PropertyWindowVisible = false
	-- alias on r2.PropertyWindowCoordsBackup
	local wndCoord = r2.PropertyWindowCoordsBackup
	-- hide previous window	
	if r2:getPropertySheetBackupFlag(r2.CurrentPropertyWindow) then				
		wndCoord = r2:backupWndCoords(r2.CurrentPropertyWindow)					
	end	
	if r2.CurrentPropertyWindow then
		r2.CurrentPropertyWindow.active = false
	end
	local newPropWindow
	if instance == nil then
		-- display the 'no selected instance window'			
		newPropWindow = getUI("ui:interface:r2ed_property_sheet_no_selection")
	else
		local class = instance:getClass()
		if class.BuildPropertySheet ~= true then
			newPropWindow = getUI("ui:interface:r2ed_property_sheet_no_properties")
		else				
			newPropWindow = r2:getPropertySheet(instance)
		end
	end
	r2.CurrentPropertyWindow = newPropWindow
	
	if newPropWindow ~= nil then
		if instance ~= nil then			
			newPropWindow.Env.TargetInstance = instance -- set the instance being edited			
		end
		newPropWindow.active = false
		newPropWindow:invalidateCoords()
		newPropWindow.active = true
		r2.PropertyWindowVisible = true
		-- see if window want to restore size from previous property sheet (for generic property windows)	
		if r2:getPropertySheetBackupFlag(r2.CurrentPropertyWindow) then
			if wndCoord ~= nil then
				newPropWindow.x = wndCoord.x
				newPropWindow.y = wndCoord.y
				newPropWindow.w = math.max(wndCoord.w, newPropWindow.w)
				newPropWindow.h = math.max(wndCoord.h, newPropWindow.h)
			end			
		end		
		if instance and instance:getClass().BuildPropertySheet then
			newPropWindow.uc_title = concatUCString(i18n.get("uiRE2DPropertiesOf"), instance:getDisplayName())
		end
	end	
	r2.CurrentPropertyWindow = newPropWindow

	return newPropWindow
end


r2.CurrentForm = nil

------------------------------------------------------------------------------------------------------------
-- display a form with the given init parameters, & call a function to notify when ok has been pressed
-- the callback is called with 
function r2:doForm(formName, initialTable, validateCallback, cancelCallback)	
	if r2.CurrentForm then r2:cancelForm(r2.CurrentForm) end
	local form = r2.Forms[formName]
	if form == nil then
		debugInfo("<r2:doForm> Can't retrieve form with name " .. tostring(formName))
		return
	end	
	if form.Prop == nil then
		debugInfo("<r2:doForm> no properties found for form with name " .. tostring(formName))		
		return
	end	
	local formUI = r2:getForm(formName)	
	if form.Prop == nil then
		debugInfo("<r2:doForm> can't find ui for form with name " .. tostring(formName))		
		return
	end
	-- fill all properties with their default values
	for k, prop in pairs(form.Prop) do
		if initialTable[prop.Name] == nil then
			if prop.Default ~= nil then				
				initialTable[prop.Name] = prop.Default
			else
				-- TODO nico : add a table here when more types are available
				if prop.Type == "String" then
					initialTable[prop.Name] = ""
				else
					initialTable[prop.Name] = 0
				end
			end
		end
	end	
	--	
	formUI.Env.Choice = nil -- no choice made yet
	formUI.Env.Form = form
	formUI.Env.FormInstance = initialTable
	formUI.Env.onValidate = validateCallback
	formUI.Env.onCancel = cancelCallback
	-- TMP for debug : directly call the callback
	--validateCallback(formUI.Env.FormInstance)
	
	formUI.active = true	
	formUI.Env.updateAll()
	formUI.Env.updateSize()
	formUI:updateCoords()
	formUI:center()	
	formUI:updateCoords()
	if form.Caption ~= nil then
		formUI.uc_title = i18n.get(form.Caption)
	else
		formUI.uc_title = i18n.get("uiR2EDForm")
	end
	r2.CurrentForm = formUI
	if type(form.onShow) == "function" then
		form.onShow(initialTable)
	end
	--runAH(nil, "enter_modal", "group=" .. formUI.id)	
end

-- called when the user hit the 'ok' boutton of a form
function r2:validateForm(form)	
	form.Env.Choice="Ok"
	form.active = false
	if form.Env.onValidate then
		form.Env.onValidate(form.Env.FormInstance, form)
	end
	
	--r2.CurrentForm = nil
end

-- called when the user hit the 'cancel' boutton of a form
function r2:cancelForm(form)	
	form.Env.Choice="Cancel"
	form.active = false
	r2.CurrentForm = nil
	if form.Env.onCancel then
		form.Env.onCancel(form.Env.FormInstance, form)		
	end

end

-- test if there's an help button in the window. If so, clamp more of the title
function r2:adjustPropertySheetTitleClampSize(wnd)
	if wnd.header_opened == nil then return end
	local helpBut = wnd.header_opened.help
	if helpBut ~= nil and helpBut.active then		
		helpBut:getViewText():updateCoords()
		helpBut:updateCoords()		
		if wnd.title_delta_max_w == r2.DefaultPropertySheetTitleClampSize then			
			wnd.title_delta_max_w = wnd.title_delta_max_w - helpBut.w_real - 4
		end
	end
end

-- test of forms
function r2:testForm()
	local function onOk(resultTable)
		debugInfo('ok was pressed')
		inspect(resultTable)
	end
	local function onCancel()
		debugInfo('cancel was pressed')		
	end
	r2:doForm("TestForm", {}, onOk, onCancel)
end

--function mysetDbProp(dbpath, value)
--	debugInfo("Setting "  .. dbpath .. " to " .. tostring(value))
--	setDbProp(dbpath, value)
--end

--function mygetDbProp(dbPath)
--	debugInfo("Getting"  .. dbPath .. " -> value is " .. tostring(getDbProp(dbPath)))
--	return getDbProp(dbPath)
--end




function r2:onContAlphaSettingsChanged(dbPath)
	--debugInfo("onContAlphaSettingsChanged " .. dbPath)
	--debugInfo('1')
	local cont = getUICaller()
	setDbProp("UI:SAVE:" .. dbPath ..":CONTAINER_ALPHA", cont.container_alpha)	
	setDbProp("UI:SAVE:" .. dbPath ..":CONTENT_ALPHA", cont.content_alpha)
	setDbProp("UI:SAVE:" .. dbPath ..":ROLLOVER_CONTENT_ALPHA", cont.rollover_content_alpha)
	setDbProp("UI:SAVE:" .. dbPath ..":ROLLOVER_CONTAINER_ALPHA", cont.rollover_container_alpha)
	setDbProp("UI:SAVE:" .. dbPath ..":USE_GLOBAL_ALPHA_SETTINGS", select(cont.use_global_alpha_settings, 1, 0))
	--debugInfo('2')
end

-- called by action handler attached to the container when alpha settings of a property sheets have been modified
function r2:onPropertySheetAlphaSettingsChanged()	
	--debugInfo("onPropertySheetAlphaSettingsChanged")
	r2:onContAlphaSettingsChanged("R2_PROP_SHEET")	
end

-- called by action handler attached to the container when alpha settings of a form have been modified
function r2:onFormAlphaSettingsChanged()	
	--debugInfo("onFormAlphaSettingsChanged")
	r2:onContAlphaSettingsChanged("R2_FORM")		
end

-- called by action handler attached to the container when it is opened to restore 
-- its alpha settings from the databse (this is because all prop / forms window share the same alpha settings)
function r2:restoreContAlphaSettings(dbPath)
	--debugInfo('restoreContAlphaSettings ' .. dbPath)
	local cont = getUICaller()
	local oldHandler = cont.on_alpha_settings_changed
	cont.on_alpha_settings_changed = ""
	cont.container_alpha = getDbProp("UI:SAVE:" .. dbPath ..":CONTAINER_ALPHA")
	cont.content_alpha = getDbProp("UI:SAVE:" .. dbPath ..":CONTENT_ALPHA")
	cont.rollover_content_alpha = getDbProp("UI:SAVE:" .. dbPath ..":ROLLOVER_CONTENT_ALPHA")
	cont.rollover_container_alpha = getDbProp("UI:SAVE:" .. dbPath ..":ROLLOVER_CONTAINER_ALPHA")
	local use_global_alpha_settings =  getDbProp("UI:SAVE:" .. dbPath ..":USE_GLOBAL_ALPHA_SETTINGS")	
	--debugInfo("use_global_alpha_settings = " .. tostring(use_global_alpha_settings))
	cont.use_global_alpha_settings = use_global_alpha_settings ~= 0
	cont.on_alpha_settings_changed = oldHandler
end

