
-- In Translation file
-- Category : uiR2EdEasterEgg --
-- CreationFrom : uiR2EdEasterEggParameters


r2.Features.RandomChest = {}

local feature = r2.Features.RandomChest

feature.Name="RandomChest"

feature.Description="A feature that allows a NPC to take some item(s) from easter eggs"

feature.Components = {}



feature.Components.RandomChest =
	{
		BaseClass="LogicEntity",			
		Name="RandomChest",
		InEventUI = true,	
		Menu="ui:interface:r2ed_feature_menu",
	
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "randomChestDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = { "activation", "deactivation", "opened"},
	
		Conditions = { "is active", "is inactive" },
	
		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table", Visible = false},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="ItemNumber", Type="Number", Category="uiR2EDRollout_Items", WidgetStyle="EnumDropDown", 
			Enum={"1", "2", "3"},	},
			{Name="Item1Qty", Type="Number", Category="uiR2EDRollout_Items", Min="1", Max="50",DefaultValue="1", Visible= function(this) return this:displayRefId(1) end},
			{Name="Item1Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_Items", Visible= function(this) return this:displayRefId(1) end},
			{Name="Item1Weight", Type="Number", Category="uiR2EDRollout_Items", Min="1", Max="100", DefaultValue="33", Visible= function(this) return this:displayRefId(1) end},
			{Name="Item2Qty", Type="Number", Category="uiR2EDRollout_Items", Min="0", Max="50",DefaultValue="0", Visible= function(this) return this:displayRefId(2) end},
			{Name="Item2Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_Items", Visible= function(this) return this:displayRefId(2) end},
			{Name="Item2Weight", Type="Number", Category="uiR2EDRollout_Items", Min="1", Max="100", DefaultValue="33", Visible= function(this) return this:displayRefId(2) end},
			{Name="Item3Qty", Type="Number", Category="uiR2EDRollout_Items", Min="0",Max="50", DefaultValue="0", Visible= function(this) return this:displayRefId(3) end},
			{Name="Item3Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_Items", Visible= function(this) return this:displayRefId(3) end},
			{Name="Item3Weight", Type="Number", Category="uiR2EDRollout_Items", Min="1", Max="100", DefaultValue="33", Visible= function(this) return this:displayRefId(3) end},
			{Name="Active", Type="Number", WidgetStyle="Boolean", Min="0",  DefaultValue="1"},
		},

		--	
		-- from base class
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,
		--
		-- from base class			
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
			r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
			for k, component in specPairs(this.Components) do
				component:appendInstancesByType(destTable, kind)
			end
		end,

		--
		-- from base class
		getSelectBarSons = function(this)
			return Components
		end,

		--
		-- from base class		
		canHaveSelectBarSons = function(this)
			return false;
		end,
		
		--
		-- Called when running EditMode to create locally the feature without sending anything into the act (speed purpose).
		--
		onPostCreate = function(this)
			--this:createGhostComponents()
			if this.User.DisplayProp and this.User.DisplayProp == 1 then
				r2:setSelectedInstanceId(this.InstanceId)				
				r2:showProperties(this)		
				this.User.DisplayProp = nil
			end
		end,

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)	
			local eggId = this:getEggId(context)
		end,

		translate = function(this, context)
			local entity = this

			r2.Translator.translateAiGroup(this, context)

			local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
			
			if (this.Active == 1)
			then
				local eggId = this:getEggId(context)
				local initialAction = this:createActionActivateRandomChest(context)
				r2.Translator.translateAiGroupInitialState(this, context, initialAction)
			end
			r2.Translator.translateFeatureActivation(this, context)
		end

	}

-------------------------------------------------------------------------------------------------------------------------

local component = feature.Components.RandomChest

function component:displayRefId(index)
	local nbItems = self.ItemNumber + 1
	if index <= nbItems then
		return true
	end
	return false
end


local randomChestDisplayerTable = clone(r2:propertySheetDisplayer())

function randomChestDisplayerTable:onAttrModified(instance, attributeName)
	if attributeName == "ItemNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbRefId = instance.ItemNumber + 1
		local i = 1
		while i <= 3 do
			if i > nbRefId then
				local name = "Item"..tostring(i).."Id"
				local qty = "Item"..tostring(i).."Qty"
				local weight = "Item"..tostring(i).."Weight"
				r2.requestSetNode(instance.InstanceId, name, r2.RefId(""))
				r2.requestSetNode(instance.InstanceId, qty, 0)
				r2.requestSetNode(instance.InstanceId, weight, 0)
			end
			i = i + 1
		end
		propertySheet.Env.updatePropVisibility()
		return
	end
	r2:propertySheetDisplayer():onAttrModified(instance, attributeName)
end

function randomChestDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function r2:randomChestDisplayer()	
	return randomChestDisplayerTable  -- returned shared displayer to avoid wasting memory
end

--
-- Create the logic actions relative to the feature via the translator.
--

function getPlotItemIdByInstance(missionItem)
	if not missionItem then return "" end
	local container = r2.Scenario.PlotItems
	local k, v = next(container)
	local id = 0
	while k do
		if tostring(v.InstanceId) == tostring(missionItem.InstanceId)  then return id end
		id = id + 1		
	k, v = next(container, k)
	end

	return ""
end


function component:getItems()

	local str =  ""
	
	local id = 1
	while id <= 3 do
		local item = self["Item"..tostring(id).."Id"]
		--r2.print("ITEM= '" ..item.."'")
		if item and item ~= "" then
		
			local qt = tonumber(self["Item"..tostring(id).."Qty"])			

			local plotItem = r2:getInstanceFromId(item)
			if plotItem then
				local plotItemId = getPlotItemIdByInstance(plotItem)
				if str ~= "" then str =  str ..";" end

				str = str .. tostring(plotItemId)..":"..qt
			end
		end
		id = id + 1
	end
	return str
end

function component:computeItemWeight(index)
	local total = tonumber(self.Item1Weight) + tonumber(self.Item2Weight) + tonumber(self.Item3Weight)
	if total == 0 then return -1 end
	local coeff = 100.0 / total

	local weight = tonumber(self["Item"..tostring(index).."Weight"]) * coeff
	return weight
end

function component:createActionActivateRandomChest(context)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	local eggId = self:getEggId(context)
	local pos = r2.getWorldPos(self)
	local x = pos.x
	local y = pos.y
	local z = r2:snapZToGround(x, y)
	
	local item1Weight	= self:computeItemWeight(1)
	local item1Id		= tostring(getPlotItemIdByInstance(r2:getInstanceFromId(self.Item1Id)))
	local item1Qty		= tonumber(self.Item1Qty)

	local item2Weight	= self:computeItemWeight(2)
	local item2Id		= tostring(getPlotItemIdByInstance(r2:getInstanceFromId(self.Item2Id)))
	local item2Qty		= tonumber(self.Item2Qty)

	local item3Weight	= self:computeItemWeight(3)
	local item3Id		= tostring(getPlotItemIdByInstance(r2:getInstanceFromId(self.Item3Id)))
	local item3Qty		= tonumber(self.Item3Qty)
	
	if item1Weight == -1 or item2Weight == -1 or item3Weight == -1 then
		debugInfo("Random chest: cannot divide by 0!")
		return
	end

	local randomChestActivation = r2.Translator.createAction("random_chest_activate", rtGrp.Id, eggId, r2:getActId(context.Act), x, y, z, 
						item1Weight, item1Id, item1Qty, 
						item2Weight, item2Id, item2Qty, 
						item3Weight, item3Id, item3Qty,
						self.Name)
	local setValue = r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1)

	local rtAction = r2.Translator.createAction("multi_actions", {randomChestActivation, setValue})
	return rtAction
end
	
function component:createActionDeactivateEasterEgg(context)
	local eggId = self:getEggId(context)

	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	local actionDeactivate = r2.Translator.createAction("easter_egg_deactivate", rtGrp.Id, eggId, r2:getActId(context.Act))
	local actionSetValue = r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0)

	return r2.Translator.createAction("multi_actions", {actionDeactivate, actionSetValue})
end

component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	
	if (action.Action.Type == "activate") then
		
		local retAction  = component:createActionActivateRandomChest(context)
		return retAction, retAction
	elseif (action.Action.Type == "deactivate") then
		local retAction  = component:createActionDeactivateEasterEgg(context)
		return retAction, retAction
	end

	local firstAction, lastAction = nil, nil
	return firstAction, lastAction
end

--
-- Checks the conditions defined for this feature
--
component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	return r2.Translator.getFeatureActivationCondition(condition, rtNpcGrp)
end


component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component =  this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local eventType = tostring(event.Event.Type)
	
	local eventHandler, lastCondition = nil, nil

	if eventType == "opened" then 
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 2)
	elseif eventType == "activation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 4)
	elseif eventType == "deactivation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 5)
 	end
	return eventHandler, firstCondition, lastCondition
end


-- feature part


--
-- Creates an instance of the feature with attributes retrieved from the creation form
--
function component.createComponent(x, y)
	
	local comp = r2.newComponent("RandomChest")
	assert(comp)

	comp.Base = "palette.entities.botobjects.chest_wisdom_std_sel"
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdRandomChest")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	comp.ItemQty = 1

	comp._Seed = os.time() 

	return comp
end


function component:getEggId(context)
	assert(context)
	local n = 0

	if context.EasterEggUniqId == nil then
		context.EasterEggUniqId = {}		
		context.EasterEggMaxId = 0
	end

	local rtNpcGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	if context.EasterEggUniqId[rtNpcGrp.Id] == nil then
		local n = context.EasterEggMaxId + 1
		context.EasterEggUniqId[rtNpcGrp.Id] = n
		context.EasterEggMaxId = n
	end
	
	return context.EasterEggUniqId[rtNpcGrp.Id]

end



-- from ihm
-- Displays the creation form of the feature and calls CreateComponent with the user input values
--
function component.create()	

	if not r2:checkAiQuota() then return end



	local function paramsOk(resultTable)

		

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		if showAgain == 1 then 
			r2.setDisplayInfo("EasterEggForm", 0)
		else r2.setDisplayInfo("EasterEggForm", 1) end

		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		local comp = component.createComponent( x, y)
		r2:setCookie(comp.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", comp)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'RandomChest' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'RandomChest' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("RandomChest") == 1 then 
			r2.displayFeatureHelp("RandomChest")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewRandomChestAction"))
		local comp = component.createComponent( x, y)
		r2:setCookie(comp.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", comp)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'RandomChest' position")
	end	
	r2:choosePos("object_chest_wisdom_std_sel.creature", posOk, posCancel, "createFeatureEasterEgg")
end

-----------------------------------------
--- register the current Feature to menu


----------------------------------------------------------------------------
-- add a line to the event menu
function component:getLogicTranslations()

	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]		= { menu=i18n.get( "uiR2AA0Spawn"		):toUtf8(),
									text=i18n.get( "uiR2AA1Spawn"		):toUtf8()},
			["deactivate"]		= { menu=i18n.get( "uiR2AA0Despawn"		):toUtf8(),
									text=i18n.get( "uiR2AA1Despawn"		):toUtf8()},
		},
		["Events"] = {	
			["activation"]		= { menu=i18n.get( "uiR2Event0Spawn"		):toUtf8(), 
									text=i18n.get( "uiR2Event1Spawn"		):toUtf8()},
			["deactivation"]	= { menu=i18n.get( "uiR2Event0Despawn"		):toUtf8(), 
									text=i18n.get( "uiR2Event1Despawn"		):toUtf8()},			
			["opened"]			= { menu=i18n.get( "uiR2Event0ChestOpened"	):toUtf8(),
									text=i18n.get( "uiR2Event1ChestOpened"	):toUtf8()},
		},
		["Conditions"] = {
			["is active"]		= { menu=i18n.get( "uiR2Test0Spawned"		):toUtf8(),
									text=i18n.get( "uiR2Test1Spawned"		):toUtf8()},
			["is inactive"]		= { menu=i18n.get( "uiR2Test0Despawned"		):toUtf8(),
									text=i18n.get( "uiR2Test1Despawned"		):toUtf8()}
		}
	}
	return logicTranslations
end

r2.Features["RandomChest"] =  feature



