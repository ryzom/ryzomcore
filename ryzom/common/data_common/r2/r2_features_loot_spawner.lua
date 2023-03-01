-- In Translation file
-- Category : uiR2EdLootSpawner --
-- CreationFrom : uiR2EdLootSpawnerParameters


r2.Features.LootSpawnerFeature = {}

local feature = r2.Features.LootSpawnerFeature

feature.Name="LootSpawnerFeature"

feature.Description="Spawns an easterEgg containing several user items after the death of a chosen number of npc"

feature.Components = {}

feature.Components.LootSpawner =
	{
		BaseClass="LogicEntity",			
		Name="LootSpawner",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "lootSpawnerDisplayer",

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
		--Category="uiR2EDRollout_Npc",
		--Category="uiR2EDRollout_SpawningEgg",
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible= false},
			{Name="Components", Type="Table"},
			{Name= "Ghosts", Type = "Table", Visible = false },
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name= "Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="TriggerValue", Type="Number", Min="0", Default="0", Translation="uiR2EDProp_TriggerValue"},
			{Name="NpcNumber", Type="Number", Category="uiR2EDRollout_Npcs", WidgetStyle="EnumDropDown", 
			Enum={"1", "2", "3", "4", "5"}, DefaultValue="5"},
			{Name="Npc1Id", Type="RefId", Category="uiR2EDRollout_Npcs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Npc1Id", Visible= function(this) return this:displayRefId(1) end},
			{Name="Npc2Id", Type="RefId", Category="uiR2EDRollout_Npcs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Npc2Id", Visible= function(this) return this:displayRefId(2) end},
			{Name="Npc3Id", Type="RefId", Category="uiR2EDRollout_Npcs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Npc3Id", Visible= function(this) return this:displayRefId(3) end},
			{Name="Npc4Id", Type="RefId", Category="uiR2EDRollout_Npcs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Npc4Id", Visible= function(this) return this:displayRefId(4) end},
			{Name="Npc5Id", Type="RefId", Category="uiR2EDRollout_Npcs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", Translation="uiR2EDProp_Npc5Id", Visible= function(this) return this:displayRefId(5) end},
			{Name="EasterEggId", Type="RefId", PickFunction="r2:canPickEasterEgg", SetRefIdFunction="r2:setEasterEggRefIdTarget", Translation="uiR2EDProp_EasterEgg"}
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
		end,

		translate = function(this, context)
			r2.Translator.translateAiGroup(this, context)
			r2.Translator.translateFeatureActivation(this, context)
		end
	}

-------------------------------------------------------------------------------------------------------------------------


local component = feature.Components.LootSpawner  

function component:displayRefId(index)
	local nbNPCs = self.NpcNumber + 1
	if index <= nbNPCs then
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

	--local  rtEggGrp = r2.Translator.getRtGroup(context, component.EasterEggId)
	local eggInstance = r2:getInstanceFromId(component.EasterEggId)
	assert(eggInstance)

	if (action.Action.Type == "spawnEntity") then	
		if eggInstance then
			local actionTrigger = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)
			local actionSpawn = eggInstance:createActionActivateEasterEgg(context)
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

--------------------------------------------------------------------------------------------------------------------
local lootSpawnerDisplayerTable = clone(r2:propertySheetDisplayer())

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
		local attrName = "Npc" ..i.. "Id"
		if attrName ~= attributeName and this[attrName] == tmpInstance.InstanceId then
			--if not tmpInstance.User.SelfModified or tmpInstance.User.SelfModified == false then 
			--	messageBox("'"..tmpInstance.Name.."' has already been picked.") 
			--else
			--	tmpInstance.User.SelfModified = false
			--end
			return false
		end
		i = i + 1
	end

	return true
end



local oldOnAttrModified = lootSpawnerDisplayerTable.onAttrModified

function lootSpawnerDisplayerTable:onAttrModified(instance, attributeName)
	--if not instance.User.SelfModified then return end
	-- call base version
	oldOnAttrModified(self, instance, attributeName)
	
	if attributeName == "NpcNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbNPCs = instance.NpcNumber + 1
		local i = 1
		while i <= 5 do
			if i > nbNPCs then
				local name = "Npc"..tostring(i).."Id"
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
	local refIdName = refId:find("name")
	
	local instanceId = instance[attributeName]
	if instanceId == "" then
		refIdName.hardtext = "NONE"
		return
	end
	
	if checkPickedEntity(instance, instanceId, attributeName) then
		local tmpInstance = r2:getInstanceFromId(instanceId)
		refIdName.hardtext = tmpInstance.Name
	else
		r2.requestSetNode(instance.InstanceId, attributeName, "")
	end
	
end	

function lootSpawnerDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end


local function reattributeIdOnHrcMove(lootSpawner, group, targetAttr)
	local propertySheet = r2:getPropertySheet(lootSpawner)
	local refId = propertySheet:find(targetAttr)
	local refIdName = refId:find("name")
	
	r2.requestSetNode(lootSpawner.InstanceId, targetAttr, group.InstanceId)
	refIdName.hardtext = group.Name

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




function r2:lootSpawnerDisplayer()	
	return lootSpawnerDisplayerTable  -- returned shared displayer to avoid wasting memory
end
--------------------------------------------------------------------------------------------------------------------



component.createGhostComponents= function(this, act)

	local comp = this
	local easterEgg= r2:getInstanceFromId(comp.EasterEggId)

	if easterEgg == nil then
		debugInfo("LootSpawner: Can't spawn a nil easteregg. You have to pick one.")
		--assert(easterEgg)
		return
	end

	local counter = r2.newComponent("Counter")
	assert(counter)
	counter.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	counter.Name = "Npc Counter"
	counter.Position.x = comp.Position.x
	counter.Position.y = comp.Position.y
	counter.Position.z = 0


	
	local nbNpc = 0
	
	for id = 1, 5 do
		local propertyName = "Npc"..tonumber(id).."Id"
		if comp[propertyName] ~= nil and comp[propertyName] ~= "" then
			local npcInstance = r2:getInstanceFromId(comp[propertyName])
			if npcInstance then
				
				if npcInstance:isKindOf("Npc") then
					eventType = "death"
					eventName = "On Death"
				elseif npcInstance:isKindOf("NpcGrpFeature") then
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
				
				if npcInstance:isKindOf("Npc") then
					r2.requestInsertGhostNode(npcInstance.Behavior.InstanceId, "Actions", -1, "", eventHandler)
				elseif npcInstance:isKindOf("NpcGrpFeature") then
					r2.requestInsertGhostNode(npcInstance.Components[0].Behavior.InstanceId, "Actions", -1, "", eventHandler)
				end
				--r2.requestInsertGhostNode(guardInstance.Behavior.InstanceId, "Actions", -1, "", eventHandler)
				
				nbNpc = nbNpc + 1
			end
		end
	end

	if nbNpc == 0 then
		debugInfo("LootSpawner: No npc has been picked.")
		return
	end

	counter.Value = tonumber(nbNpc)
	--counter.TriggerValue = comp._TriggerValue
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

	r2.requestSetGhostNode(easterEgg.InstanceId, "Active", 0)

end

component.createComponent = function(x, y, tvalue)
	
	local comp = r2.newComponent("LootSpawner")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDRollout_LootSpawner")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	comp.TriggerValue = 0

	comp._Seed = os.time() 

	comp.Npc1Id = r2.RefId("")
	comp.Npc2Id = r2.RefId("")
	comp.Npc3Id = r2.RefId("")
	comp.Npc4Id = r2.RefId("")
	comp.Npc5Id = r2.RefId("")

	comp.NpcNumber = 0


	local easterEgg = r2.Features["EasterEggFeature"].Components.EasterEgg.createComponent(comp.Position.x + 1, comp.Position.y + 1)
	easterEgg.Position.x, easterEgg.Position.y  = r2:findEmptyPlace(comp.Position.x + 1, comp.Position.y + 1)
	--easterEgg.Position.y = comp.Position.y + 1
	easterEgg.InheritPos = 0
	comp.EasterEggId = easterEgg.InstanceId
	table.insert(comp.Components, easterEgg)


	return comp
end


component.create = function()	
	if r2:getLeftQuota() <= 0 then
		r2:makeRoomMsg()
		return
	end
	local function paramsOk(resultTable)

		

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		if showAgain == 1 then 
			r2.setDisplayInfo("LootSpawnerForm", 0)
		else r2.setDisplayInfo("LootSpawnerForm", 1) end
	
		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.LootSpawner.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'LootSpawnerFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'LootSpawnerFeature' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("LootSpawner") == 1 then 
			r2.displayFeatureHelp("LootSpawner")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewLootSpawnerFeatureAction"))
		local component = feature.Components.LootSpawner.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'LootSpawnerFeature' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureLootSpawner")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EDRollout_RewardChest")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "RewardChest")
end


function component:getLogicTranslations()
	local logicTranslations = {}
	r2.Translator.addActivationToTranslations(logicTranslations)
	return logicTranslations
end


r2.Features["LootSpawnerFeature"] =  feature
