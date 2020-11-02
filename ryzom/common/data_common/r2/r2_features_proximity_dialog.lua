
r2.Features.ProximityDialog = {}

local feature = r2.Features.ProximityDialog 

feature.Name="ProximityDialog"


feature.Description=""

feature.Components = {}

local classProximityDialogVersion = 1

feature.Components.ProximityDialog =
	{
		PropertySheetHeader = r2.getDisplayButtonHeader("r2.dialogs:openEditor()", "uiR2EdEditDialogButton"),
		BaseClass="ChatSequence",			
		Name="ProximityDialog",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		Version=classProximityDialogVersion ,
		
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},
		ApplicableActions = {"activate", "deactivate", "trigger", "starts dialog", "stops dialog", "starts chat"},
		Events = {"activation", "deactivation", "trigger", "end of chat", "end of dialog", "start of chat", "start of dialog"},
		Conditions = {"is active", "is inactive", "is in dialog", "is not in dialog", "is in chat"},
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
		
		updateVersion = function(this, scenarioValue, currentValue )
			
		end,		
	}

local component = feature.Components.ProximityDialog  

function component:translate(context)
	r2.Classes["ChatSequence"].translate(self, context)

	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	
	--start of state
	do
		--use zone trigger prop Cyclic as Repeatable for this component
		local zTrigger = r2:getInstanceFromId(self._zoneTriggerId)
		assert(zTrigger)
		local cyclic = tonumber(zTrigger.Cyclic)

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "Repeatable", cyclic) 
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", 0)
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,} )
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
	end

	--trigger
	do
		local actionStartDialog = r2.Translator.createAction("dialog_starts", rtGrp.Id)
		local actionDeactivate = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Repeatable", 0,
			r2.Translator.createAction("user_event_trigger", rtGrp.Id, 8))
		local actionSetV3 = r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1)

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,
			r2.Translator.createAction("multi_actions", {actionStartDialog, actionDeactivate, actionSetV3}))

		r2.Translator.translateAiGroupEvent("user_event_6", self, context, rtAction)
	end

	--deactivation
	do
		local rtAction = r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 )
		
		r2.Translator.translateAiGroupEvent("user_event_8", self, context, rtAction)
	end

end

function component.createGhostComponents(this, act)
	local zoneTrigger = r2:getInstanceFromId(this._zoneTriggerId)
	assert(zoneTrigger)
	do
		local type = "On Player Left"
		if this.TriggerOn == 1 then
			type = "On Player Arrived"
		end

		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = type
		eventHandler.Event.Value = ""
		eventHandler.Name = type
		
		local action = r2.newComponent("ActionStep")
	
		action.Entity =  r2.RefId(this.InstanceId) 
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

function onComponentCreated(this)
	local x = this.Position.x + 3
	local y = this.Position.y + 3
	local zoneTrigger = r2.Features["ZoneTrigger"].Components.ZoneTrigger.createComponent(x, y)
	assert(zoneTrigger)
	zoneTrigger.Name = this.Name.." "..i18n.get("uiR2EdZoneTrigger")
	zoneTrigger.InheritPos = 0
	zoneTrigger.Deletable = false
	table.insert(this.SubComponents, zoneTrigger)
	this._zoneTriggerId = zoneTrigger.InstanceId
end

component.create = function()	
	if r2:getLeftQuota() <= 0 then 
		r2:makeRoomMsg()
		return
	end

	local function createComponent(x, y)
		r2.requestNewAction(i18n.get("uiR2EDNewProximityDialogAction"))
		local proxDialog = r2.newComponent("ProximityDialog")
		assert(proxDialog)
		proxDialog.Name = r2:genInstanceName(i18n.get("uiR2EDProximityDialog"))
		proxDialog.Base = r2.Translator.getDebugBase("palette.entities.botobjects.dialog")
		proxDialog.Position.x = x
		proxDialog.Position.y = y
		proxDialog.Position.z = r2:snapZToGround(x, y)
		proxDialog.AutoStart = 0
		onComponentCreated(proxDialog)
		r2:setCookie(proxDialog.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", proxDialog)	
	end

	local function posOk(x, y, z)
		if r2.mustDisplayInfo("ProximityDialog") == 1 then 
			r2.displayFeatureHelp("ProximityDialog")
		end
		createComponent(x, y)
	end
	
	local function posCancel() end

	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature") 
	r2:choosePos(creature, posOk, posCancel, "createFeatureProximityDialog")
end

component.getLogicAction = function(this, context, action)
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	local rtZoneTrigger = r2.Translator.getRtGroup(context, component._zoneTriggerId)
	assert(rtZoneTrigger)
	
	if action.Action.Type == "trigger" then
		local actionEvent = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1, 
		r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6))
		return actionEvent, actionEvent 
	end
	
	return r2.Translator.getDialogLogicAction(this, context, action)
end

component.getLogicEvent = function(this, context, event)
	local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
	assert(rtNpcGrp)
	
	local eventType = tostring(event.Event.Type)
	
	if eventType == "trigger" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	end

	return r2.Translator.getDialogLogicEvent(this, context, event)
end

component.getLogicCondition = function(this, context, condition)
	local rtNpcGrp = r2.Translator.getRtGroup(context, condition.Entity)
	assert(rtNpcGrp)

	if condition.Condition.Type == "is active" then
		local firstCondition = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1)
		return firstCondition, firstCondition
	end

	if condition.Condition.Type == "is inactive" then
		local firstCondition = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0)
		return firstCondition, firstCondition
	end

	return r2.Translator.getDialogLogicCondition(this, context, condition)
end

function component.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
	local entitiesTable = r2.Scenario:getAllInstancesByType(this.Name)
	for key, entity in pairs(entitiesTable) do
		subMenu:addLine(entity.Name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
	end

	if table.getn(entitiesTable)==0 then
		subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
	end
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["starts dialog"]			= { menu=i18n.get( "uiR2AA0ChatSeqStart"		), 
											text=i18n.get( "uiR2AA1ChatSeqStart"		)},
			["stops dialog"]			= { menu=i18n.get( "uiR2AA0ChatSeqStop"			), 
											text=i18n.get( "uiR2AA1ChatSeqStop"			)},
			["starts chat"]				= { menu=i18n.get( "uiR2AA0ChatStepStart"		), 
											text=i18n.get( "uiR2AA1ChatStepStart"		)},
			["trigger"]					= { menu=i18n.get( "uiR2AA0Trigger"		), 
											text=i18n.get( "uiR2AA1Trigger"		)},
		},
		["Events"] = {	
			["start of dialog"]			= { menu=i18n.get( "uiR2Event0ChatSeqStart"		), 
											text=i18n.get( "uiR2Event1ChatSeqStart"		)},
			["end of dialog"]			= { menu=i18n.get( "uiR2Event0ChatSeqEnd"		), 
											text=i18n.get( "uiR2Event1ChatSeqEnd"		)},
			["start of chat"]			= { menu=i18n.get( "uiR2Event0ChatStepStart"	), 
											text=i18n.get( "uiR2Event1ChatStepStart"	)},
			["end of chat"]				= { menu=i18n.get( "uiR2Event0ChatStepEnd"		), 
											text=i18n.get( "uiR2Event1ChatStepEnd"		)},
			["trigger"]					= { menu=i18n.get( "uiR2Event0Trigger"		), 
											text=i18n.get( "uiR2Event1Trigger"		)},
		},
		["Conditions"] = {	
			["is in dialog"]			= { menu=i18n.get( "uiR2Test0ChatSeq"			), 
											text=i18n.get( "uiR2Test1ChatSeq"			)},
			["is not in dialog"]		= { menu=i18n.get( "uiR2Test0ChatNotSeq"		), 
											text=i18n.get( "uiR2Test1ChatNotSeq"		)},
			["is in chat"]				= { menu=i18n.get( "uiR2Test0ChatStep"			), 
											text=i18n.get( "uiR2Test1ChatStep"			)},
			["is active"]				= { menu=i18n.get( "uiR2Test0Active"			), 
											text=i18n.get( "uiR2Test1Active"			)},
			["is inactive"]				= { menu=i18n.get( "uiR2Test0Inactive"			), 
											text=i18n.get( "uiR2Test1Inactive"			)},
		}
	}
	return logicTranslations
end

r2.Features["ProximityDialog"] =  feature


