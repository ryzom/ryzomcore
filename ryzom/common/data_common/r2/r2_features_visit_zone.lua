
-- In Translation file
-- Category : uiR2EdVisitZone --
-- CreationFrom : uiR2EdVisitZoneParameters


r2.Features.VisitZoneFeature = {}

local feature = r2.Features.VisitZoneFeature

feature.Name="VisitZoneFeature"

feature.Description=""

feature.Components = {}

feature.Components.VisitZone =
	{
		BaseClass="LogicEntity",			
		Name="VisitZone",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "visitZoneDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate", "succeed"},

		Events = {"activation", "deactivation", "wait validation", "mission asked", "succeeded"},

		Conditions = { "is active", "is inactive", "is succeeded" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="MissionGiver", Type="RefId", PickFunction="r2:canPickTalkingNpc", SetRefIdFunction="r2:setTalkingNpc"},
			{Name="ValidationNeeded", Category="uiR2EDRollout_TextToSay", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100" },
			{Name="MissionText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="WaitValidationText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="MissionSucceedText", Type="String", Category="uiR2EDRollout_TextToSay", Visible= function(this) 
						return this:IsValidationNeeded() end },
			{Name="BroadcastText", Type="String", Category="uiR2EDRollout_TextToSay", DefaultValue="", DefaultInBase = 1},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Repeatable", Type="Number", WidgetStyle="Boolean", DefaultValue="0"},
			
			
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
		end,


	}


local component = feature.Components.VisitZone

function component:IsValidationNeeded()
	local validationNeeded = self.ValidationNeeded
	if validationNeeded == 1 then
		return true
	end
	return false
end

----------------------------------------
----------------------------------------

local visitZoneDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = visitZoneDisplayerTable.onAttrModified
function visitZoneDisplayerTable:onAttrModified(instance, attributeName)
	
	oldOnAttrModified(self, instance, attributeName)
	r2:propertySheetDisplayer():onAttrModified(instance, attributeName)
	if attributeName == "ValidationNeeded" then
		local propertySheet = r2:getPropertySheet(instance)
		propertySheet.Env.updatePropVisibility()
		return
	end
end

function visitZoneDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)
	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
end

function component:onTargetInstancePostHrcMove(targetAttr, targetIndexInArray)
	local targetId = self[targetAttr]

	local tmpInstance = r2:getInstanceFromId(targetId)
	
	assert(tmpInstance)
	if tmpInstance.User.SelfModified and tmpInstance.User.SelfModified == true then
		if tmpInstance.ParentInstance and tmpInstance.ParentInstance:isKindOf("NpcGrpFeature") then
			r2.requestSetNode(self.InstanceId, targetAttr, r2.RefId(""))
		end
	end
	
end

function r2:visitZoneDisplayer()	
	return visitZoneDisplayerTable  -- returned shared displayer to avoid wasting memory
end
----------------------------------------
----------------------------------------
component.createGhostComponents = function(this, act)
	
	local comp = this

	local giver = r2:getInstanceFromId(comp.MissionGiver)
	if not giver then return end

	local zoneTrigger = r2:getInstanceFromId(comp._ZoneId)
	assert(zoneTrigger)
	
	local validationNeeded = comp.ValidationNeeded

	do
		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = "On Player Arrived"
		eventHandler.Event.Value = ""
		eventHandler.Name = "On Player Arrived"

		local actionSuccess = r2.newComponent("ActionStep")
		actionSuccess.Entity =  r2.RefId(comp.InstanceId) 
		if validationNeeded == 1 then
			actionSuccess.Action.Type = "validateTask"
		else
			actionSuccess.Action.Type = "succeed"
		end
		actionSuccess.Action.Value = ""

		table.insert(eventHandler.Actions, actionSuccess)
	
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


function component:textAdapter(text)

	assert(self)
	assert(type(text) == "string")
	local str =  text
	local mission_giver = ""
	local mission_target = ""

	if tostring(self.MissionGiver) ~= "" then
		local giver = r2:getInstanceFromId(self.MissionGiver)
		if giver then 	mission_giver = giver.Name end
	end
	
	str=string.gsub(str, "<mission_giver>", mission_giver)
	return str
end

--EVENT 
-- 1: mission given (plugged on talkto)
-- 2: wait validation (plugged on talkto)
-- 4: activation (generic)
-- 5: deactivation (generic)
-- 8: step validated
-- 9: mission successful (generic)


--
-- v1: repeatable 
-- v2: mission state 	0= the player didn't talk to the mission giver, 
--						1= player talked to the mission giver, but didn't visit the zone yet
--						2= player did visit the zone but didn't come back to talk to the mission giver yet
-- v3: mission completed (0=never completed, 1=completed at least once)
--
function component:translate(context)
	r2.Translator.translateAiGroup(self, context)
	
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	local validationNeeded = self.ValidationNeeded

	local giver = r2:getInstanceFromId(self.MissionGiver)
	if not giver then return end
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
	
	-- Start of state	
	r2.Translator.Tasks.startOfStateLogic(self, context, rtGrp)

	-- Activation
	r2.Translator.Tasks.activationLogic(self, context, rtGrp)

	--Deactivation
	r2.Translator.Tasks.deactivationLogic(self, context, rtGrp)

	--Set mission status to 1 when mission is taken
	r2.Translator.Tasks.setStatusLogic(self, context, rtGrp)

	-- Mission giver must either give the mission (1st time) or say WaitValidationText
	r2.Translator.Tasks.giverLogic(self, giver, context, rtGrp)
	
	--depending on validationNeeded prop, either set 		
	if validationNeeded == 1 then
		r2.Translator.Tasks.validationByMissionGiver(self, giver, context, rtGrp)

		r2.Translator.Tasks.broadcastOnEvent8(self, context)
	end

	--success with broadcast
	r2.Translator.Tasks.successBroadcastLogic(self, context, rtGrp)
	
	r2.Translator.translateFeatureActivation(self, context)
	
end


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtGrp)
	
	if action.Action.Type == "validateTask" then
		local action = r2.Translator.createAction("validate_task", rtGrp.Id)
		return action, action
	elseif action.Action.Type == "succeed" then
		local actionComplete = r2.Translator.createAction("complete_mission", rtGrp.Id)
		return actionComplete, actionComplete
	end

	return r2.Translator.getFeatureActivationLogicAction(rtGrp, action)
end

component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	elseif condition.Condition.Type == "is succeeded" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v3", 1);
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

	local eventType = tostring(event.Event.Type)
	
	local eventHandler, lastCondition = nil, nil

	if eventType == "mission asked" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 3)
	elseif eventType == "wait validation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 2)
	elseif eventType == "succeeded" then 
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 9)

	end
	
	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdVisitZone_ContextualText"):toUtf8()
	local missionText = i18n.get("uiR2EdVisitZone_MissionText"):toUtf8()
	local waitValidationText = i18n.get("uiR2EdVisitZone_WaitValidationText"):toUtf8()
	local missionSucceededText = i18n.get("uiR2EdVisitZone_MissionSucceededText"):toUtf8()
	local broadcastText = i18n.get("uiR2EdVisitZone_BroadcastText"):toUtf8()

	local comp = r2.newComponent("VisitZone")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdVisitZone")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.WaitValidationText = waitValidationText
	comp.MissionSucceedText = missionSucceededText
	comp.BroadcastText = broadcastText

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	local zoneTrigger = r2.Features["ZoneTrigger"].Components.ZoneTrigger.createComponent(x + 3, y + 3)
	zoneTrigger.Name = comp.Name.." "..i18n.get("uiR2EdZoneTrigger"):toUtf8() --r2:genInstanceName(i18n.get("uiR2EdZoneTrigger")):toUtf8()
	zoneTrigger.InheritPos = 0
	zoneTrigger.Deletable = false
	table.insert(comp.Components, zoneTrigger)
	comp._ZoneId = zoneTrigger.InstanceId

	comp._Seed = os.time() 

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
			r2.setDisplayInfo("VisitZoneForm", 0)
		else r2.setDisplayInfo("VisitZoneForm", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.VisitZone.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'VisitZone' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'VisitZone' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("VisitZone") == 1 then 
			r2.displayFeatureHelp("VisitZone")
		end
		r2.requestNewAction(i18n.get("ui2REDNewVisitZoneFeatureAction"))
		local component = feature.Components.VisitZone.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'VisitZone' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureVisitZone")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdVisitZone")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "VisitZone")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]			= { menu=i18n.get( "uiR2AA0Activate"				):toUtf8(), 
										text=i18n.get( "uiR2AA1Activate"				):toUtf8()}, 
			["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"				):toUtf8(), 
										text=i18n.get( "uiR2AA1Deactivate"				):toUtf8()}, 
			["succeed"]				= { menu=i18n.get( "uiR2AA0SucceedTask"				):toUtf8(),
										text=i18n.get( "uiR2AA1SucceedTask"				):toUtf8()},
		},
		["Events"] = {	
			["activation"]			= { menu=i18n.get( "uiR2Event0Activation"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Activation"			):toUtf8()},
			["deactivation"]		= { menu=i18n.get( "uiR2Event0Deactivation"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Deactivation"			):toUtf8()},
			["mission asked"]		= { menu=i18n.get( "uiR2Event0MissionGiven"			):toUtf8(), 
										text=i18n.get( "uiR2Event1MissionGiven"			):toUtf8()},
			["wait validation"]		= { menu=i18n.get( "uiR2Event0TaskWaitValidation"	):toUtf8(), 
										text=i18n.get( "uiR2Event1TaskWaitValidation"	):toUtf8()},
			["succeeded"]			= { menu=i18n.get( "uiR2Event0TaskSuccess"			):toUtf8(),
										text=i18n.get( "uiR2Event1TaskSuccess"			):toUtf8()},
		},
		["Conditions"] = {	
			["is active"]			= { menu=i18n.get( "uiR2Test0Active"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Active"				):toUtf8()},
			["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Inactive"				):toUtf8()},
			["is succeeded"]		= { menu=i18n.get( "uiR2Test0TaskSuccess"				):toUtf8(), 
										text=i18n.get( "uiR2Test1TaskSuccess"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["VisitZone"] =  feature





