
r2.Features.ManHuntFeature = {}

local feature = r2.Features.ManHuntFeature

feature.Name="ManHuntFeature"

feature.Description="Triggers when all selected NPCs die."



feature.Components = {}

feature.Components.ManHunt =
	{
		PropertySheetHeader = r2.getDisplayButtonHeader("r2.events:openEditor()", "uiR2EdEditEventsButton"),
		BaseClass="LogicEntity",			
		Name="ManHunt",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "manHuntDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = { "activation", "deactivation", "trigger"},

		Conditions = { "is active", "is inactive"},

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		--Category="uiR2EDRollout_Guards"
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table", Visible= false},
			{Name= "Ghosts", Type = "Table", Visible = false },
			{Name= "Name", Type="String"},
			{Name= "Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="TriggerValue", Type="Number", Min="0", Default="0", Translation="uiR2EDProp_TriggerValue"},
			{Name="MobNumber", Type="Number", Category="uiR2EDRollout_Mobs", WidgetStyle="EnumDropDown", Enum={"1", "2", "3", "4", "5"},
			},
			{Name="Mob1Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Translation="uiR2EdProp_Mob1Id", Visible= function(this) return this:displayRefId(1) end},
			{Name="Mob2Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Translation="uiR2EdProp_Mob2Id", Visible= function(this) return this:displayRefId(2) end},
			{Name="Mob3Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Translation="uiR2EdProp_Mob3Id", Visible= function(this) return this:displayRefId(3) end},
			{Name="Mob4Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Translation="uiR2EdProp_Mob4Id", Visible= function(this) return this:displayRefId(4) end},
			{Name="Mob5Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Translation="uiR2EdProp_Mob5Id", Visible= function(this) return this:displayRefId(5) end},
		},

		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
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

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
			--r2.Translator.pretranslateDefaultFeature(this, context)
		end,

		translate = function(this, context)
			r2.Translator.translateAiGroup(this, context)
			
			r2.Translator.translateFeatureActivation(this, context)
		end
	}

-------------------------------------------------------------------------------------------------------------------------


local component = feature.Components.ManHunt  

function component:displayRefId(index)
	local nbMobs = self.MobNumber + 1
	if index <= nbMobs then
		return true
	end
	return false
end

component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	if (action.Action.Type == "trigger") then	
			local retAction = r2.Translator.createAction("condition_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)
			)
			return retAction, retAction		
	end
	return r2.Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
end

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

	local component = this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if tostring(event.Event.Type) == "trigger" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	end

	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)

end

------------------------------------------------------------------------------------------------------------------
local manHuntDisplayerTable = clone(r2:propertySheetDisplayer())

--
-- If the message is received by a client that didn't request the modification, we must make sure this client 
-- doesn't modify the data because it has already been performed by the initial client. 
--
local function checkPickedEntity(this, instanceId, attributeName)
	if instanceId == "" then
		return false
	end
	local tmpInstance = r2:getInstanceFromId(instanceId)
	assert(tmpInstance)
	local i = 1
	while i < 6 do
		local attrName = "Mob" ..i.. "Id"
		if attrName ~= attributeName and this[attrName] == tmpInstance.InstanceId then
			return false
		end
		i = i + 1
	end
	return true
end


local oldOnAttrModified = manHuntDisplayerTable.onAttrModified
function manHuntDisplayerTable:onAttrModified(instance, attributeName)
	 oldOnAttrModified(self, instance, attributeName)

	if attributeName == "MobNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbMobs = instance.MobNumber + 1
		local i = 1
		while i <= 5 do
			if i > nbMobs then
				local name = "Mob"..tostring(i).."Id"
				local refId = propertySheet:find(name)
				local refIdName = refId:find("name")
				refIdName.hardtext = "NONE"
				r2.requestSetNode(instance.InstanceId, name, "")
			end
			i = i + 1
		end
		propertySheet.Env.updatePropVisibility()
		return
	end

	if string.find(attributeName, "Id") == nil or attributeName == "InstanceId" then return end 
	
	local propertySheet = r2:getPropertySheet(instance)
	local refId = propertySheet:find(attributeName)
	if refId == nil then return end
	local refIdName = refId:find("name")
	
	local instanceId = instance[attributeName]
	if instanceId == "" then
		refIdName.hardtext = "NONE"
		return
	end
	
	local inserted = checkPickedEntity(instance, instanceId, attributeName)
	if  inserted == true then
		local tmpInstance = r2:getInstanceFromId(instanceId)
		refIdName.hardtext = tmpInstance.Name
	else
		r2.requestSetNode(instance.InstanceId, attributeName, "")
	end
	instance.User.onHrcMove = false
end	

function manHuntDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end


local function reattributeIdOnHrcMove(mHunt, group, targetAttr)
	local propertySheet = r2:getPropertySheet(mHunt)
	local refId = propertySheet:find(targetAttr)
	local refIdName = refId:find("name")
	
	r2.requestSetNode(bSpawner.InstanceId, targetAttr, group.InstanceId)
	refIdName.hardtext = group.Name
	mHunt.User.onHrcMove = true

end


function component:onTargetInstancePostHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]

	local tmpInstance = r2:getInstanceFromId(targetId)
	
	assert(tmpInstance)
	if tmpInstance.User.SelfModified and tmpInstance.User.SelfModified == true then
		local group = tmpInstance.ParentInstance
		if group:isKindOf("NpcGrpFeature") then
			reattributeIdOnHrcMove(self, group, targetAttr)
		end
		tmpInstance.User.SelfModified = false		
	end
	
end




function r2:manHuntDisplayer()	
	return manHuntDisplayerTable  -- returned shared displayer to avoid wasting memory
end
--------------------------------------------------------------------------------------------------------------------


component.createGhostComponents= function(this, act)

	local comp = this
	
	local counter = r2.newComponent("Counter")
	assert(counter)

	counter.Base = "palette.entities.botobjects.milestone"
	counter.Name = "Mob Counter"
	counter.Position.x = comp.Position.x
	counter.Position.y = comp.Position.y
	counter.Position.z = 0

	
	
	local nbGuard = 0
	local eventType = "" --depends on the instance type (groupe or npc)
	local eventName = ""
	
	-- Add to each guard a 'OnDeath EventHandler' which decrements the counter 
	for id = 1, 5 do
		local propertyName = "Mob"..tonumber(id).."Id"
		if comp[propertyName] ~= nil and comp[propertyName] ~= "" then
			local guardInstance = r2:getInstanceFromId(comp[propertyName])
			if guardInstance then
				
				if guardInstance:isKindOf("Npc") then
					eventType = "death"
					eventName = "On Death"
				elseif guardInstance:isKindOf("NpcGrpFeature") then
					eventType = "group death"
					eventName = "On Group Death"
				end
					 
				local eventHandler = r2.newComponent("LogicEntityAction")
				eventHandler.Event.Type = eventType
				eventHandler.Event.Value = ""
				eventHandler.Event.Name = eventName
				
				local action = r2.newComponent("ActionStep")
				table.insert(eventHandler.Actions, action)
				action.Entity = r2.RefId(counter.InstanceId)
				action.Action.Type = "Decrement"
				action.Action.Value = ""
				
				if guardInstance:isKindOf("Npc") then
					r2.requestInsertGhostNode(guardInstance.Behavior.InstanceId, "Actions", -1, "", eventHandler)
				elseif guardInstance:isKindOf("NpcGrpFeature") then
					r2.requestInsertGhostNode(guardInstance.Components[0].Behavior.InstanceId, "Actions", -1, "", eventHandler)
				end
				--r2.requestInsertGhostNode(guardInstance.Behavior.InstanceId, "Actions", -1, "", eventHandler)
				
				nbGuard = nbGuard + 1
			end
		end
	end
	
	if nbGuard == 0 then
		debugInfo("Man Hunt: No mob has been picked.")
		return
	end

	counter.Value = tonumber(nbGuard)
	counter.TriggerValue = comp.TriggerValue

	do
		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = "On Trigger"
		eventHandler.Event.Value = ""
		eventHandler.Name = "On Trigger"
		
		local action = r2.newComponent("ActionStep")
	
		action.Entity =  r2.RefId(comp.InstanceId) 
		action.Action.Type = "trigger"
		action.Action.Value = ""					
		
		table.insert(eventHandler.Actions, action)

		table.insert(counter.Behavior.Actions, eventHandler)
	
	end
	
	r2.requestInsertGhostNode(comp.InstanceId, "Components", -1, "", counter)
	

end

component.createComponent = function(x, y)
	
	local comp = r2.newComponent("ManHunt")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdManHunt")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	comp.TriggerValue = 0

	comp._Seed = os.time() 

	comp.Guard1Id = r2.RefId("")
	comp.Guard2Id = r2.RefId("")
	comp.Guard3Id = r2.RefId("")
	comp.Guard4Id = r2.RefId("")
	comp.Guard5Id = r2.RefId("")

	return comp
end


component.create = function()	
	if not r2:checkAiQuota() then return end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'ManHuntFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'ManHuntFeature' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("ManHunt") == 1 then 
			r2.displayFeatureHelp("ManHunt")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewManHuntAction"))
		local component = feature.Components.ManHunt.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'ManHuntFeature' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureManHunt")
	end

--
-- Registers the feature creation form used as a creation menu in the editor
--



function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdManHunt")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "ManHunt")
end


function component:getLogicTranslations()
	local logicTranslations = {}
	r2.Translator.addActivationToTranslations(logicTranslations)
	return logicTranslations
end


r2.Features["ManHuntFeature"] =  feature

