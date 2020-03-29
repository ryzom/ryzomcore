
r2.Features.BossSpawnerFeature = {}

local feature = r2.Features.BossSpawnerFeature

feature.Name="BossSpawnerFeature"

feature.Description="Spawns a boss after the death of a chosen number of kitin workers"

feature.Components = {}

feature.Components.BossSpawner =
	{
		BaseClass="LogicEntity",			
		Name="BossSpawner",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "bossSpawnerDisplayer",

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
			{Name= "Name", Type="String", MaxNumChar="32"},
			{Name= "Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="TriggerValue", Type="Number", Min="0", DefaultValue="0", Translation="uiR2EDProp_TriggerValue"},
			{Name="BossId", Type="RefId", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_BossId"},
			{Name="GuardNumber", Type="Number", Category="uiR2EDRollout_Guards", WidgetStyle="EnumDropDown", 
			Enum={"1", "2", "3", "4", "5"}, DefaultValue="5"},
			{Name="Guard1Id", Type="RefId", Category="uiR2EDRollout_Guards", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Guard1Id", Visible= function(this) return this:displayRefId(1) end},
			{Name="Guard2Id", Type="RefId", Category="uiR2EDRollout_Guards", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Guard2Id", Visible= function(this) return this:displayRefId(2) end},
			{Name="Guard3Id", Type="RefId", Category="uiR2EDRollout_Guards", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Guard3Id", Visible= function(this) return this:displayRefId(3) end},
			{Name="Guard4Id", Type="RefId", Category="uiR2EDRollout_Guards", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Guard4Id", Visible= function(this) return this:displayRefId(4) end},
			{Name="Guard5Id", Type="RefId", Category="uiR2EDRollout_Guards", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Guard5Id", Visible= function(this) return this:displayRefId(5) end},
			
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

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
			--r2.Translator.pretranslateDefaultFeature(this, context)
		end,

		translate = function(this, context)
			r2.Translator.translateAiGroup(this, context)
			
			r2.Translator.translateFeatureActivation(this, context)
			--r2.Translator.translateDefaultFeature(this, context)
		end
	}

-------------------------------------------------------------------------------------------------------------------------




local component = feature.Components.BossSpawner  

function component:getMaxTriggerValue()
	return tostring(this.GuardNumber)
end

function component:displayRefId(index)
	local nbGuards = self.GuardNumber + 1
	if index <= nbGuards then
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

	local  rtBossGrp = r2.Translator.getRtGroup(context, component.BossId)

	if (action.Action.Type == "spawnEntity") then	
		if rtBossGrp then
			local actionTrigger = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)
			local actionSpawn = r2.Translator.createAction("spawn", rtBossGrp.Id)
			local retAction = r2.Translator.createAction("condition_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				r2.Translator.createAction("multi_actions", {actionTrigger, actionSpawn})
			)
			return retAction, retAction
		end
		return nil, nil		
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
local bossSpawnerDisplayerTable = clone(r2:propertySheetDisplayer())

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
		local attrName = "Guard" ..i.. "Id"
		if attrName ~= attributeName and this[attrName] == tmpInstance.InstanceId then
			return false
		end
		i = i + 1
	end
	if attributeName ~= "BossId" and tmpInstance.InstanceId == this.BossId then
		return false
	end
	return true
end


local oldOnAttrModified = bossSpawnerDisplayerTable.onAttrModified
function bossSpawnerDisplayerTable:onAttrModified(instance, attributeName)
	-- call base version
    oldOnAttrModified(self, instance, attributeName)
	
	if attributeName == "GuardNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbGuards = instance.GuardNumber + 1
		local i = 1
		while i <= 5 do
			if i > nbGuards then
				local name = "Guard"..tostring(i).."Id"
				local refId = propertySheet:find(name)
				local refIdName = refId:find("name")
				refIdName.hardtext = "NONE"
				r2.requestSetNode(instance.InstanceId, name, "")
			end
			i = i + 1
		end
		if instance.TriggerValue >= nbGuards then
			messageBox("Trigger value was exceeding the number of picked guards and was reset to 0")
			displaySystemInfo(i18n.get("uiR2EdInvalidTriggerValue"), "BC")
			r2.requestSetNode(instance.InstanceId, "TriggerValue", 0)
		end
		propertySheet.Env.updatePropVisibility()
		return
	end
	
	
	--check if the trigger value doesn't exceed the number of picked guards (or groups)
	--if it does, reset the value to 0
	if attributeName == "TriggerValue" then
		local propertySheet = r2:getPropertySheet(instance)
		local i = 1
		local nbPickedGuards = 0

		while i <= 5 do
			local guardId = "Guard"..tostring(i).."Id"
			local refId = instance[guardId]
			r2.print("refId = " ..tostring(refId))
			if refId ~= "" then
				nbPickedGuards = nbPickedGuards + 1
			end
			i = i + 1
		end

		if instance.TriggerValue ~= 0 and tonumber(instance.TriggerValue) >= nbPickedGuards then
			r2.requestSetNode(instance.InstanceId, attributeName, 0)
			displaySystemInfo(i18n.get("uiR2EdInvalidTriggerValue"), "BC")
			messageBox("The trigger value shouldn't exceed the number of picked guards (or picked groups)")
		end

		return 
	end
	
	if string.find(attributeName, "Id") == nil or attributeName == "InstanceId" then return end 

	local propertySheet = r2:getPropertySheet(instance)
	local refId = propertySheet:find(attributeName)
	if refId == nil then return end
	local refIdName = refId:find("name")
	
	local instanceId = instance[attributeName]
	if instanceId == "" then
	
		local i = 1
		local nbPickedGuards = 0
		
		--when clearing a refId, check if the trigger value is still valid
		while i <= 5 do
			local guardId = "Guard"..tostring(i).."Id"
			local refId = instance[guardId]

			if refId ~= "" then
				nbPickedGuards = nbPickedGuards + 1
			end
			i = i + 1
		end

		if instance.TriggerValue ~= 0 and tonumber(instance.TriggerValue) >= nbPickedGuards then
			r2.requestSetNode(instance.InstanceId, "TriggerValue", 0)
			displaySystemInfo(i18n.get("uiR2EdInvalidTriggerValue"), "BC")
			messageBox("The trigger value shouldn't exceed the number of picked guards (or picked groups)")
		end


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


function bossSpawnerDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end


local function reattributeIdOnHrcMove(bSpawner, group, targetAttr)
	local propertySheet = r2:getPropertySheet(bSpawner)
	local refId = propertySheet:find(targetAttr)
	local refIdName = refId:find("name")
	
	r2.requestSetNode(bSpawner.InstanceId, targetAttr, group.InstanceId)
	refIdName.hardtext = group.Name
	bSpawner.User.onHrcMove = true

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




function r2:bossSpawnerDisplayer()	
	return bossSpawnerDisplayerTable  -- returned shared displayer to avoid wasting memory
end
--------------------------------------------------------------------------------------------------------------------


component.createGhostComponents= function(this, act)

	local comp = this
	local boss = r2:getInstanceFromId(comp.BossId)
	
	if boss == nil then
		debugInfo("BossSpawner: Can't spawn a nil boss. You have to pick one.")
		--assert(boss)
		return
	end
	
	local counter = r2.newComponent("Counter")
	assert(counter)

	counter.Base = "palette.entities.botobjects.milestone"
	counter.Name = "Guards Counter"
	counter.Position.x = comp.Position.x
	counter.Position.y = comp.Position.y
	counter.Position.z = 0

	
	
	local nbGuard = 0
	local eventType = "" --depends on the instance type (groupe or npc)
	local eventName = ""
	
	-- Add to each guard a 'OnDeath EventHandler' which decrements the counter 
	for id = 1, 5 do
		local propertyName = "Guard"..tonumber(id).."Id"
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
				--eventHandler.Event.Type = "death"
				eventHandler.Event.Type = eventType
				eventHandler.Event.Value = ""
				--eventHandler.Name = "On Death"
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
		r2.print("BossSpawner: No guard has been picked.")
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
	
		action.Entity =  r2.RefId(comp.InstanceId) --r2.RefId(boss.InstanceId)
		action.Action.Type = "spawnEntity"
		action.Action.Value = ""					
		
		table.insert(eventHandler.Actions, action)

		table.insert(counter.Behavior.Actions, eventHandler)
	
	end
	
	r2.requestInsertGhostNode(comp.InstanceId, "Components", -1, "", counter)
	--r2.requestInsertGhostNode(r2:getCurrentAct().InstanceId, "Features", -1, "", counter)
	
	if boss:isKindOf("NpcGrpFeature") then
		local instanceId = boss.Components[0].InstanceId
		--debugInfo("setting autospawn for: " ..instanceId)
		r2.requestSetGhostNode(instanceId, "AutoSpawn", 0)
	else
		r2.requestSetGhostNode(boss.InstanceId, "AutoSpawn", 0)
	end

end

component.createComponent = function(x, y)
	
	local comp = r2.newComponent("BossSpawner")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdBossSpawner")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	comp.TriggerValue = 0

	comp.GuardNumber = 0

	comp._Seed = os.time() 

	comp.Guard1Id = r2.RefId("")
	comp.Guard2Id = r2.RefId("")
	comp.Guard3Id = r2.RefId("")
	comp.Guard4Id = r2.RefId("")
	comp.Guard5Id = r2.RefId("")
	comp.BossId = r2.RefId("")

	return comp
end


component.create = function()	

	if not r2:checkAiQuota() then return end

	local function paramsOk(resultTable)

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		--debugInfo("Show again: "..tostring(showAgain))
		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		
		if showAgain == 1 then 
			r2.setDisplayInfo("BossSpawnerForm", 0)
		else r2.setDisplayInfo("BossSpawnerForm", 1) end
		
		local component = feature.Components.BossSpawner.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'BossSpawnerFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'BossSpawnerFeature' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("BossSpawner") == 1 then 
			r2.displayFeatureHelp("BossSpawner")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewBossSpawnerAction"))
		local component = feature.Components.BossSpawner.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'BossSpawnerFeature' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureBossSpawner")
	end

--
-- Registers the feature creation form used as a creation menu in the editor
--



function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdBossSpawner")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "BossSpawner")
end


function component:getLogicTranslations()
	local logicTranslations = {}
	r2.Translator.addActivationToTranslations(logicTranslations)
	return logicTranslations
end


r2.Features["BossSpawnerFeature"] =  feature

