
r2.Features.HiddenChest = {}

local feature = r2.Features.HiddenChest 

feature.Name="HiddenChest"


feature.Description=""

feature.Components = {}

local classHiddenChestVersion = 1

feature.Components.HiddenChest =
	{
		--PropertySheetHeader = r2.getDisplayButtonHeader("r2.events:openEditor()", "uiR2EdEditEventsButton"),
		BaseClass="EasterEgg",			
		Name="HiddenChest",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		Version=classHiddenChestVersion ,

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
			{Name="TriggerOn", Type="Number", WidgetStyle="EnumDropDown", Translation="uiR2EdProp_TriggerOn",
			Enum={"Leaves the zone", "Enters the zone"},
			},
		},
		
		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
			local eggId = this:getEggId(context)
		end,

		translate = function(this, context)
			local eggId = this:getEggId(context)
			r2.Translator.translateAiGroup(this, context)
			r2.Translator.translateFeatureActivation(this, context)
		end,

		updateVersion = function(this, scenarioValue, currentValue )
			
		end,		


	}

local component = feature.Components.HiddenChest  


component.getLogicAction = function(entity, context, action)
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if action.Action.Type == "trigger" then
		local actionTrigger = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)
		local actionSpawn = component:createActionActivateEasterEgg(context)
		local retAction = r2.Translator.createAction("condition_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
			r2.Translator.createAction("multi_actions", {actionTrigger, actionSpawn})
		)
		return retAction, retAction
	elseif (action.Action.Type == "activate") then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 1)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 4)
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		assert(retAction)
		return retAction, retAction
	elseif (action.Action.Type == "deactivate") then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 0)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 5)
		--local action3 = component:createActionDeactivateEasterEgg(context)
		--local retAction = r2.Translator.createAction("multi_actions", {action1, action2, action3})
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		assert(retAction)
		return retAction, retAction
	end
	return nil, nil
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
	
	local comp = r2.newComponent("HiddenChest")
	assert(comp)
	assert(comp.Position)
	
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdHiddenChest")):toUtf8()
	comp.Base = "palette.entities.botobjects.chest_wisdom_std_sel"	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	comp.CompareClass = true
	
	comp.ItemNumber = 0

	local zoneTrigger = r2.Features["ZoneTrigger"].Components.ZoneTrigger.createComponent(x + 3, y + 3)
	zoneTrigger.Name = comp.Name.." "..i18n.get("uiR2EdZoneTrigger"):toUtf8()--r2:genInstanceName(i18n.get("uiR2EdZoneTrigger")):toUtf8()
	zoneTrigger.InheritPos = 0
	zoneTrigger.Deletable = false
	table.insert(comp.Components, zoneTrigger)
	comp._ZoneId = zoneTrigger.InstanceId

	return comp
end

component.create = function()	
	if not r2:checkAiQuota() then return end

	local function posOk(x, y, z)
		debugInfo("Validate creation of an HiddenChest.")
		if r2.mustDisplayInfo("HiddenChest") == 1 then 
			r2.displayFeatureHelp("HiddenChest")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewHiddenChestFeatureAction"))
		local component = feature.Components.HiddenChest.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end

	local function posCancel()	end

	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature") 
	r2:choosePos(creature, posOk, posCancel, "createFeatureHiddenChest")
end


component.initLogicEntitiesInstancesMenu = function(this, subMenu, calledFunction)
			
	local entitiesTable = r2.Scenario:getAllInstancesByType(this.Name)
	for key, entity in pairs(entitiesTable) do
		local uc_name = ucstring()
		uc_name:fromUtf8(entity.Name)
		subMenu:addLine(uc_name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
	end

	if table.getn(entitiesTable)==0 then
		subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
	end
end
-----------------------------------------
--- register the curent Feature to menu

function component:getLogicTranslations()
	-- register trad
	local logicTranslations = {
		["ApplicableActions"] = {	
			["activate"]		= { menu=i18n.get( "uiR2AA0Activate"		):toUtf8(),
									text=i18n.get( "uiR2AA1Activate"		):toUtf8()},
			["deactivate"]		= { menu=i18n.get( "uiR2AA0Deactivate"		):toUtf8(),
									text=i18n.get( "uiR2AA1Deactivate"		):toUtf8()},
			["trigger"]				= { menu=i18n.get( "uiR2AA0Trigger"			):toUtf8(), 
										text=i18n.get( "uiR2AA1Trigger"			):toUtf8()},
		},
		["Events"] = {
			["activation"]		= { menu=i18n.get( "uiR2Event0Activation"		):toUtf8(), 
									text=i18n.get( "uiR2Event1Activation"		):toUtf8()},
			["deactivation"]	= { menu=i18n.get( "uiR2Event0Deactivation"		):toUtf8(), 
									text=i18n.get( "uiR2Event1Deactivation"		):toUtf8()},			
			["trigger"]				= { menu=i18n.get( "uiR2Event0Trigger"		):toUtf8(), 
										text=i18n.get( "uiR2Event1Trigger"		):toUtf8()},
		},
		["Conditions"] = {
			["is active"]		= { menu=i18n.get( "uiR2Test0Active"		):toUtf8(),
									text=i18n.get( "uiR2Test1Active"		):toUtf8()},
			["is inactive"]		= { menu=i18n.get( "uiR2Test0Inactive"		):toUtf8(),
									text=i18n.get( "uiR2Test1Inactive"		):toUtf8()}
		}
	}

	return logicTranslations
end

r2.Features["HiddenChest"] =  feature


