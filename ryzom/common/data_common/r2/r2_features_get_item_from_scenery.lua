
r2.Features.GetItemFromSceneryObjectFeature = {}

local feature = r2.Features.GetItemFromSceneryObjectFeature

feature.Name="GetItemFromSceneryObjectFeature"

feature.Description=""

feature.Components = {}

feature.Components.GetItemFromSceneryObject =
	{
		--PropertySheetHeader = r2.getHelpButtonHeader("GetItemFromSceneryObject") ,
		BaseClass="LogicEntity",			
		Name="GetItemFromSceneryObject",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "getItemFromSceneryObjectDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {"activation", "deactivation", "trigger"},

		Conditions = { "is active", "is inactive", "has triggered", "has not triggered" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="SceneryObject", Type="RefId", PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget"},

			{Name="ItemNumber", Type="Number", Category="uiR2EDRollout_ItemsToGive", WidgetStyle="EnumDropDown", 
			Enum={"1", "2", "3"}, DefaultValue="3"	},
			{Name="Item1Qty", Type="Number", Category="uiR2EDRollout_ItemsToGive", Min="0", DefaultValue="1", Visible= function(this) return this:displayRefId(1) end},
			{Name="Item1Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToGive", Visible= function(this) return this:displayRefId(1) end},


			{Name="Item2Qty", Type="Number", Category="uiR2EDRollout_ItemsToGive", Min="0", DefaultValue="0", Visible= function(this) return this:displayRefId(2) end},
			{Name="Item2Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToGive", Visible= function(this) return this:displayRefId(2) end},

			{Name="Item3Qty", Type="Number", Category="uiR2EDRollout_ItemsToGive", Min="0", DefaultValue="0", Visible= function(this) return this:displayRefId(3) end},
			{Name="Item3Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToGive", Visible= function(this) return this:displayRefId(3) end},

			
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay"},

			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Repeatable", Type="Number", WidgetStyle="Boolean", DefaultValue="0"}
			
		},
		

		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,
			
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
			r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
			for k, component in specPairs(this.Components) do
				component:appendInstancesByType(destTable, kind)
			end
		end,

		getSelectBarSons = function(this)
			return Components
		end,
		
		canHaveSelectBarSons = function(this)
			return false;
		end,

		onPostCreate = function(this)
			--this:createGhostComponents()
			if this.User.DisplayProp and this.User.DisplayProp == 1 then
				r2:setSelectedInstanceId(this.InstanceId)				
				r2:showProperties(this)		
				this.User.DisplayProp = nil
			end
		end,

		--pretranslate = function(this, context)
		--	r2.Translator.createAiGroup(this, context)
		--end,

	}


local component = feature.Components.GetItemFromSceneryObject

function component:displayRefId(index)
	local nbItems = self.ItemNumber + 1
	if index <= nbItems then
		return true
	end
	return false
end

function component:pretranslate(context)
	if context.InteractingSceneryObjects then
		if self.SceneryObject and self.SceneryObject ~= "" then
			local scObj = r2:getInstanceFromId(self.SceneryObject)
			if scObj then
				--assert(scObj)
				table.insert(context.InteractingSceneryObjects, scObj.InstanceId)
			end
		end
	end
	r2.Translator.createAiGroup(self, context)
end


local getItemFromSceneryObjectDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = getItemFromSceneryObjectDisplayerTable.onAttrModified

function getItemFromSceneryObjectDisplayerTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(self, instance, attributeName)
	r2:propertySheetDisplayer():onAttrModified(instance, attributeName)
	if attributeName == "ItemNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbRefId = instance.ItemNumber + 1
		local i = 1
		while i <= 3 do
			if i > nbRefId then
				local name = "Item"..tostring(i).."Id"
				local qty = "Item"..tostring(i).."Qty"
				r2.requestSetNode(instance.InstanceId, name, r2.RefId(""))
				r2.requestSetNode(instance.InstanceId, qty, 0)
			end
			i = i + 1
		end
		propertySheet.Env.updatePropVisibility()
		return
	end

end

function getItemFromSceneryObjectDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function r2:getItemFromSceneryObjectDisplayer()	
	return getItemFromSceneryObjectDisplayerTable  -- returned shared displayer to avoid wasting memory
end


function component:getItems()
	local str =  ""
	
	local id = 1
	while id <= 3 do
		local item = self["Item"..tostring(id).."Id"]
		local item2 = self.Item1Id
		if (item) then
		
			local qt = tonumber(self["Item"..tostring(id).."Qty"])			

			local plotItem = r2:getInstanceFromId(item)
			if plotItem then
				local plotItemSheetId = plotItem.SheetId
				if str ~= "" then str =  str ..";" end
				local name = r2.getSheetIdName(plotItemSheetId)
				str = str .. tostring(name)..":"..qt
			end
		end
		id = id + 1
	end
	return str
end

function component:textAdapter(text)
	local str =  ""
	local items = {}
	local qts = {}

	local id = 1
	while id <= 3 do
		local item = self["Item"..tostring(id).."Id"]
		local item2 = self.Item1Id
		if (item) then
		
			local qt = tonumber(self["Item"..tostring(id).."Qty"])			
			qts[id] = qt

			local plotItem = r2:getInstanceFromId(item)
			if plotItem then
				items[id] = plotItem.Name
			else
				items[id] = ""
			end
		end
		id = id + 1
	end
	local str = text
	str=string.gsub (str, "<qt1>", tostring(qts[1]))
	str=string.gsub (str, "<qt2>", tostring(qts[2]))
	str=string.gsub (str, "<qt3>", tostring(qts[3]))
	str=string.gsub (str, "<item1>", tostring(items[1]))
	str=string.gsub (str, "<item2>", tostring(items[2]))
	str=string.gsub (str, "<item3>", tostring(items[3]))

	local sceneryObjectName = ""
	if self.SceneryObject == nil then return end
	local sceneryObject = r2:getInstanceFromId(self.SceneryObject)
	if sceneryObject then
		sceneryObjectName = sceneryObject.Name
	end
	str=string.gsub(str, "<object_name>", tostring(sceneryObjectName))
	return str
end



function component:translate(context)
	r2.Translator.translateAiGroup(self, context)

	if self.SceneryObject == nil then return end
	local sceneryObject = r2:getInstanceFromId(self.SceneryObject)
	if not sceneryObject then return end

	local sceneryObjectGrp = r2.Translator.getRtGroup(context, sceneryObject.InstanceId)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	local items = self:getItems()
	
	-----------------
	--Contextual text
	do	
		if sceneryObject then
			local actionContextual = r2.Translator.createAction("give_item", rtGrp.Id, items, self:textAdapter(self.ContextualText))
			--local actionSetSelectStatus = r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1)
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
				 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0,  -- giver has been spoken to
					--r2.Translator.createAction("multi_actions", {actionContextual, actionSetSelectStatus}) )
					actionContextual))
			r2.Translator.translateAiGroupEvent("player_target_npc", sceneryObject, context, rtAction)
		end
	end
	
	-------------------
	--Trigger
	if sceneryObject then
		--local rtSceneryObjectGrp = r2.Translator.getRtGroup(context, sceneryObject.InstanceId)

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, --if active
		  r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0, --and if the scenery object has been selected once by the player
			r2.Translator.createAction("multi_actions", { 
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 6),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ) })
					))

		--r2.Translator.translateAiGroupEvent("player_target_npc", sceneryObject, context, rtAction)
		r2.Translator.translateAiGroupEvent("user_event_1", self, context, rtAction)
	end
	
	-------------------
	--Start of state
	
	do
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- the scenery object hasn't been selected yet
		local rtAction4 = r2.Translator.createAction("set_value", rtGrp.Id, "v3", 0 )
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,rtAction4 } )
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
	end

	
	do	
		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				})
			); 
		local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- Not Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			); 
				
		local rtAction = r2.Translator.createAction("multi_actions", {action2, action3})
		--Autodeactivate after having triggered?
		r2.Translator.translateAiGroupEvent("user_event_6", self, context, rtAction)
	end

	r2.Translator.translateFeatureActivation(self, context)
end


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	return r2.Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
end

component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	local prefix = ""
	
	if rtNpcGrp.Id and rtNpcGrp.Id ~= "" then
		prefix = r2:getNamespace() .. rtNpcGrp.Id.."."
	end

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	elseif condition.Condition.Type == "has triggered" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v3", 1 );
		return action1, action1
	elseif condition.Condition.Type == "has not triggered" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v3", 0 );
		return action1, action1
	else
		assert(nil)
	end
	return nil,nil
end

component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component =  this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if event.Event.Type == "trigger" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	end

	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local comp = r2.newComponent("GetItemFromSceneryObject")
	assert(comp)

	local contextualText = i18n.get("uiR2EdGetItemFromSceneryObject_ContextualText"):toUtf8()
	
	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_request_item")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDGetItemFromSceneryObject")):toUtf8()			
	comp.ItemNumber = 0
		
	comp.ContextualText = contextualText


	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
--	comp.ItemQty = 1

	comp._Seed = os.time() 

	return comp
end

component.create = function()	

	if not r2:checkAiQuota() then return end


	local function paramsOk(resultTable)

		r2.requestNewAction(i18n.get("uiR2EDNewGetItemFromSceneryObjectFeature"))

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		if showAgain == 1 then 
			r2.setDisplayInfo("GetItemFromSceneryObject", 0)
		else r2.setDisplayInfo("GetItemFromSceneryObject", 1) end

		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.GetItemFromSceneryObject.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'GetItemFromSceneryObject' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'GetItemFromSceneryObject' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("GetItemFromSceneryObject") == 1 then 
			r2.displayFeatureHelp("GetItemFromSceneryObject")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewGetItemFromSceneryObjectFeature"))
		local component = feature.Components.GetItemFromSceneryObject.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'GetItemFromSceneryObject' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_bot_request_item.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureGetItemFromSceneryObject")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdGetItemFromSceneryObject")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "GetItemFromSceneryObject")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]			= { menu=i18n.get( "uiR2AA0Activate"				):toUtf8(), 
										text=i18n.get( "uiR2AA1Activate"				):toUtf8()}, 
			["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"				):toUtf8(), 
										text=i18n.get( "uiR2AA1Deactivate"				):toUtf8()}, 
		},
		["Events"] = {	
			["activation"]			= { menu=i18n.get( "uiR2Event0Activation"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Activation"			):toUtf8()},
			["deactivation"]		= { menu=i18n.get( "uiR2Event0Deactivation"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Deactivation"			):toUtf8()},
			["trigger"]				= { menu=i18n.get( "uiR2Event0Trigger"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Trigger"			):toUtf8()},

		},
		["Conditions"] = {	
			["is active"]			= { menu=i18n.get( "uiR2Test0Active"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Active"				):toUtf8()},
			["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Inactive"				):toUtf8()},
			["has triggered"]		= { menu=i18n.get( "uiR2Test0Trigger"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Trigger"				):toUtf8()},
			["has not triggered"]		= { menu=i18n.get( "uiR2Test0NotTrigger"				):toUtf8(), 
										text=i18n.get( "uiR2Test1NotTrigger"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["GetItemFromSceneryObject"] =  feature





