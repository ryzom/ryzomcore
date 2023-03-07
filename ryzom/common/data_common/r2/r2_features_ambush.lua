
r2.Features.Ambush = {}

local feature = r2.Features.Ambush 

feature.Name="Ambush"


feature.Description=""

feature.Components = {}

local classAmbushVersion = 1

feature.Components.Ambush =
	{
		--PropertySheetHeader = r2.getDisplayButtonHeader("r2.events:openEditor()", "uiR2EdEditEventsButton"),
		BaseClass="LogicEntity",			
		Name="Ambush",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		Version=classAmbushVersion ,
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "ambushDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},
		ApplicableActions = {"activate", "deactivate", "trigger"},
		Events = {"activation", "deactivation", "trigger"},
		Conditions = {"is active", "is inactive"},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="MobNumber", Type="Number", Category="uiR2EDRollout_Mobs", WidgetStyle="EnumDropDown", Enum={"1", "2", "3", "4", "5"},
			},
			{Name="Mob1Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Visible= function(this) return this:displayRefId(1) end},
			{Name="Mob2Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			 Visible= function(this) return this:displayRefId(2) end},
			{Name="Mob3Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			Visible= function(this) return this:displayRefId(3) end},
			{Name="Mob4Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			 Visible= function(this) return this:displayRefId(4) end},
			{Name="Mob5Id", Type="RefId", Category="uiR2EDRollout_Mobs",PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget", 
			 Visible= function(this) return this:displayRefId(5) end},
			{Name="TriggerOn", Type="Number", WidgetStyle="EnumDropDown",
			Enum={"Leaves the zone", "Enters the zone"},
			},
			{Name="Components", Type="Table"},
		
		},
		-----------------------------------------------------------------------------------------------		
		-- from base class
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class			
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
			r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
			for k, component in specPairs(this.Components) do
				component:appendInstancesByType(destTable, kind)
			end
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class
		getSelectBarSons = function(this)
			return Components
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class		
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
		end,

		updateVersion = function(this, scenarioValue, currentValue )
			
		end,		


	}

local component = feature.Components.Ambush  

function component:displayRefId(index)
	local nbMobs = self.MobNumber + 1
	if index <= nbMobs then
		return true
	end
	return false
end
------------------------------------------------------------------------------------------------------------------
local ambushDisplayerTable = clone(r2:propertySheetDisplayer())

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



function ambushDisplayerTable:onAttrModified(instance, attributeName)
	
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

function ambushDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end


local function reattributeIdOnHrcMove(ambush, group, targetAttr)
	local propertySheet = r2:getPropertySheet(ambush)
	local refId = propertySheet:find(targetAttr)
	local refIdName = refId:find("name")
	
	r2.requestSetNode(ambush.InstanceId, targetAttr, group.InstanceId)
	refIdName.hardtext = group.Name
	ambush.User.onHrcMove = true

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


function r2:ambushDisplayer()	
	return ambushDisplayerTable  -- returned shared displayer to avoid wasting memory
end
--------------------------------------------------------------------------------------------------------------------



component.getLogicAction = function(entity, context, action)
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if (action.Action.Type == "trigger") then	
		local i = 1
		local spawnActions = {}
		while i <= 5 do
			local attrName = "Mob"..i.."Id"
			if component[attrName] ~= "" then
				local rtMobGrp = r2.Translator.getRtGroup(context, component[attrName])
				local actionSpawn = r2.Translator.createAction("spawn", rtMobGrp.Id)
				table.insert(spawnActions, actionSpawn)
			end
			i = i + 1
		end		
		
		if table.getn(spawnActions) ~= 0 then
			local actionTrigger = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)
			table.insert(spawnActions, actionTrigger)
			local retAction = r2.Translator.createAction("condition_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				r2.Translator.createAction("multi_actions", spawnActions)
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

component.createGhostComponents= function(this, act)
	
	local comp = this
	local nbMob = 0
	
	for id = 1, 5 do
		local propertyName = "Mob"..id.."Id"
		if comp[propertyName] ~= nil and comp[propertyName] ~= "" then
			local mob = r2:getInstanceFromId(comp[propertyName])
			if mob then
				nbMob = nbMob + 1
				if mob:isKindOf("NpcGrpFeature") then
					local instanceId = mob.Components[0].InstanceId
					r2.requestSetGhostNode(instanceId, "AutoSpawn", 0)
				else
					r2.requestSetGhostNode(mob.InstanceId, "AutoSpawn", 0)
				end
			end
		end
	end
	
	if nbMob == 0 then
		return
	end
	local zoneTrigger = r2:getInstanceFromId(comp._ZoneId)
	assert(zoneTrigger)

	do
		local type = "On Player Left"
		if comp.TriggerOn == 1 then
			type = "On Player Arrived"
		end

		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = type
		eventHandler.Event.Value = ""
		eventHandler.Name = type
		
		local action = r2.newComponent("ActionStep")
	
		action.Entity =  r2.RefId(comp.InstanceId) 
		action.Action.Type = "trigger"
		action.Action.Value = ""					
		
		table.insert(eventHandler.Actions, action)
		
		local behaviorId = zoneTrigger.Behavior.InstanceId
		assert(behaviorId)
		r2.requestInsertGhostNode(behaviorId, "Actions", -1, "", eventHandler)
	
	end

	do
		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = "activation"
		eventHandler.Event.Value = ""
		eventHandler.Name = "activation"
		
		local action = r2.newComponent("ActionStep")
	
		action.Entity =  r2.RefId(zoneTrigger.InstanceId) 
		action.Action.Type = "activate"
		action.Action.Value = ""					
		
		table.insert(eventHandler.Actions, action)
		
		local behaviorId = this.Behavior.InstanceId
		assert(behaviorId)
		r2.requestInsertGhostNode(behaviorId, "Actions", -1, "", eventHandler)	
	end
	do
		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = "deactivation"
		eventHandler.Event.Value = ""
		eventHandler.Name = "deactivation"
		
		local action = r2.newComponent("ActionStep")
	
		action.Entity =  r2.RefId(zoneTrigger.InstanceId) 
		action.Action.Type = "deactivate"
		action.Action.Value = ""					
		
		table.insert(eventHandler.Actions, action)
		
		local behaviorId = this.Behavior.InstanceId
		assert(behaviorId)
		r2.requestInsertGhostNode(behaviorId, "Actions", -1, "", eventHandler)	
	end
	
	

end


component.createComponent = function(x, y)
	
	local comp = r2.newComponent("Ambush")
	assert(comp)
	assert(comp.Position)
	
	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdAmbush"))	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	local zoneTrigger = r2.Features["ZoneTrigger"].Components.ZoneTrigger.createComponent(x + 3, y + 3)
	zoneTrigger.Name = comp.Name.." "..i18n.get("uiR2EDZoneTrigger")--r2:genInstanceName(i18n.get("uiR2EdZoneTrigger"))
	zoneTrigger.InheritPos = 0
	zoneTrigger.Deletable = false
	table.insert(comp.Components, zoneTrigger)
	comp._ZoneId = zoneTrigger.InstanceId

	return comp
end

component.create = function()	
	if not r2:checkAiQuota() then return end

	local function posOk(x, y, z)
		debugInfo("Validate creation of an Ambush.")
		if r2.mustDisplayInfo("Ambush") == 1 then 
			r2.displayFeatureHelp("Ambush")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewAmbushFeatureAction"))
		local component = feature.Components.Ambush.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end

	local function posCancel()	end

	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature") 
	r2:choosePos(creature, posOk, posCancel, "createFeatureAmbush")
end

-----------------------------------------
--- register the curent Feature to menu

function component:getLogicTranslations()
	-- register trad
	local logicTranslations = {
		["ApplicableActions"] = {	
			["activate"]		= { menu=i18n.get( "uiR2AA0Activate"		), 
									text=i18n.get( "uiR2AA1Activate"		)}, 
			["deactivate"]		= { menu=i18n.get( "uiR2AA0Deactivate"		), 
									text=i18n.get( "uiR2AA1Deactivate"		)}, 
			["trigger"]			= { menu=i18n.get( "uiR2AA0Trigger"			), 
									text=i18n.get( "uiR2AA1Trigger"			)},
		},
		["Events"] = {
			["trigger"]			= { menu=i18n.get( "uiR2Event0Trigger"		), 
									text=i18n.get( "uiR2Event1Trigger"		)},
			["activation"]		= { menu=i18n.get( "uiR2Event0Activation"	), 
									text=i18n.get( "uiR2Event1Activation"	)},
			["deactivation"]	= { menu=i18n.get( "uiR2Event0Deactivation"	), 
									text=i18n.get( "uiR2Event1Deactivation"	)},
		},
		["Conditions"] = {
			["is active"]		= { menu=i18n.get( "uiR2Test0Active"		),
									text=i18n.get( "uiR2Test1Active"		)},
			["is inactive"]		= { menu=i18n.get( "uiR2Test0Inactive"		),
									text=i18n.get( "uiR2Test1Inactive"		)}
		}
	}

	return logicTranslations
end

r2.Features["Ambush"] =  feature


