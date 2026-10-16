

-- In Transalation file
-- Category : uiR2EdTimeTrigger --
-- CreationFrom : uiR2EdTimeTriggerParameters
-- uiR2EdtooltipCreateFeatureTimeTrigger -> tooltip

r2.Features.TimeTriggerFeature = {}

local feature = r2.Features.TimeTriggerFeature 

feature.Name="TimeTriggerFeature"

feature.Description="A Time Trigger"

feature.Components = {}

feature.Components.TimeTriggerFeature =
	{
		BaseClass="LogicEntity",			
		Name="TimeTriggerFeature",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},
		ApplicableActions = {
			"Activate", "Deactivate", "Trigger",
		},
		Events = {
			"On Trigger", 
			"On Activation",
			"On Desactivation"			
		},
		Conditions = {
			"is active", "is finished"
		},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText"},
			{Name="Name",  Category="uiR2EDRollout_TimeTrigger", Type="String", MaxNumChar="32"},
			{Name="Components", Type="Table"},
			{Name="AtWhichHour", Type="Number", Category="uiR2EDRollout_TimeTrigger", Min="0", Max="23", Default="0"},
			{Name="uiR2EDProp_AtWhichMinute", Type="Number", Category="uiR2EDRollout_TimeTrigger", Min="0", Max="59", Default="0"},
			{Name="Cyclic", Type="Number", Category="uiR2EDRollout_TimeTrigger",  WidgetStyle="Boolean", Min="0", Max="1", Default="0"},	

--			{Name="Secondes",Type="Number", Category="uiR2EDRollout_TimeTrigger", Min="0", Max="59", Default="0"}

		
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

		onPostCreate = function(this)	end,

		translate = function(this, context)end,

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,
	}

-- Specific to the component TimeTrigger
local component = feature.Components.TimeTriggerFeature  

function feature.getLogicActionActivate(entity, context, action, rtNpcGrp)
	local action1 = r2.Translator.createAction("timer_enable", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0)
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	return retAction, retAction
end

function feature.getLogicActionDeactivate(entity, context, action, rtNpcGrp)
	local action1 = r2.Translator.createAction("timer_disable", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 1)
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	return retAction, retAction
end


function feature.getLogicActionTrigger(entity, context, action, rtNpcGrp)
	local retAction = r2.Translator.createAction("timer_trigger", rtNpcGrp.Id, 0)
	assert(retAction)
	return retAction, retAction
end

function feature.getLogicActionAdd10Seconds(entity, context, action, rtNpcGrp)
	local retAction = r2.Translator.createAction("timer_add", rtNpcGrp.Id, 0, 10)
	return retAction, retAction
end

function feature.getLogicActionAdd1Minute(entity, context, action, rtNpcGrp)
	local retAction = r2.Translator.createAction("timer_add", rtNpcGrp.Id, 0, 60)
	return retAction, retAction
end

function feature.getLogicActionSub10Seconds(entity, context, action, rtNpcGrp)
	local retAction = r2.Translator.createAction("timer_sub", rtNpcGrp.Id, 0, 10)
	return retAction, retAction

end

function feature.getLogicActionSub1Minute(entity, context, action, rtNpcGrp)
	local retAction = r2.Translator.createAction("timer_sub", rtNpcGrp.Id, 0, 60)
	return retAction, retAction
end



component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)


	local funs = {
		["Activate"] = feature.getLogicActionActivate,
		["Deactivate"] =  feature.getLogicActionDeactivate,
		["Pause"] =  feature.getLogicActionPause,
		["Resume"] =  feature.getLogicActionResume,
		["Trigger"] =  feature.getLogicActionTrigger,
		["Add 10 Seconds"] =  feature.getLogicActionAdd10Seconds, 
		["Add 1 minute"] =  feature.getLogicActionAdd1Minute, 
		["Sub 10 seconds"] =  feature.getLogicActionSub0Seconds,
		["Sub 1 minute"] =  feature.getLogicActionSub1Minute,
	}


	local fun = funs[ action.Action.Type ]
	if fun then
		firstAction, lastAction =  fun(entity, context, action, rtNpcGrp)
	end
	
	return firstAction, lastAction
end



component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("condition_if", "is_enable == 1" );
		local multiactions= r2.Translator.createAction("multi_actions", {action1, action2});
		return multiactions, action2
	elseif condition.Condition.Type == "is paused" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("timer_is_suspended", rtNpcGrp.Id, 0);
		local action31 = r2.Translator.createAction("condition_if", "is_suspended == 1");
		local action3 = r2.Translator.createAction("condition_if", "is_enable == 1", action31);
		local multiactions = r2.Translator.createAction("multi_actions", {action1, action2, action3});
		table.insert(action3.Children, action31)
		return multiactions, action31
	elseif condition.Condition.Type == "is running" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("timer_is_suspended", rtNpcGrp.Id, 0);
		local action31 = r2.Translator.createAction("condition_if", "is_suspended == 0");
		local action3 = r2.Translator.createAction("condition_if", "is_enable == 1", action31);
		local multiactions = r2.Translator.createAction("multi_actions", {action1, action2, action3});
		table.insert(action3.Children, action31)
		return multiactions, action3
	elseif condition.Condition.Type == "is finished" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("condition_if", "is_enable == 0" );
		local multiactions= r2.Translator.createAction("multi_actions", {action1, action2});
		return multiactions, action2
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

	if eventType == "On Trigger" then 
		eventHandler, firstCondition, lastCondition = r2.Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)
	elseif eventType == "On Activation" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 0)
	elseif eventType == "On Desactivation" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 1)
	elseif eventType == "On Pause" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 2)
	end
	
	
	return eventHandler, firstCondition, lastCondition
end




-- feature part



feature.createComponent = function(x, y, hours, minutes, cyclic)
	
	local comp = r2.newComponent("TimeTriggerFeature")
	assert(comp)

	comp.Base = "palette.entities.botobjects.time_trigger"
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdNameTimeTriggerFeature")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	comp.Hours = hours
	comp.Minutes = minutes
	comp.Cyclic = cyclic
	
	return comp
end






-- TODO to up
-- Register an RtNpcGrp to a specific instnaceId
r2.Translator.registerManager = function(context, comp)
	local rtNpcGrp = r2.newComponent("RtNpcGrp")
	table.insert(context.RtAct.NpcGrps, rtNpcGrp)
	context.RtGroups[tostring(comp.InstanceId)] = rtNpcGrp
	rtNpcGrp.Name = rtNpcGrp.Id
end

feature.preTranslatFeature = function(context)
	--debugInfo("Pre")
	--feature.doLogic(context)
	local instance = r2:getInstanceFromId(context.Feature.InstanceId);
	r2.Translator.registerManager(context, context.Feature)
	
end

feature.Translator = function(context)
	local rtNpcGrp = r2.Translator.getRtGroup(context, context.Feature.InstanceId)

	local aiState = r2.newComponent("RtAiState")
	aiState.AiActivity = "normal"
	table.insert(context.RtAct.AiStates, aiState)
	table.insert(aiState.Children, rtNpcGrp.Id)

	context.Feature:translate(context)

end
-- from ihm
feature.create = function()	
	
	if not r2:checkAiQuota() then return end

	local function paramsOk(resultTable)

		r2.requestNewAction(i18n.get("uiR2EDNewTimeTriggerFeatureAction"))

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local minutes = tonumber( resultTable["Minutes"] )
		local hours = tonumber( resultTable["Hours"] )
		local cyclic  = tonumber( resultTable["Cyclic"] )

		if not x or not y or not minutes or not hours or hours <0 or hours >=24 or minutes <0 or minutes > 59
		then
			debugInfo("Can't create Component")
			return
		end
	
		local component = feature.createComponent( x, y, hours, cyclic)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
		component.Minutes = minutes
		component.Hours = hours
		
	end


	
	local function paramsCancel()
		debugInfo("Cancel form for 'TimeTrigger' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'TimeTrigger' at pos (%d, %d, %d)", x, y, z))
		r2:doForm("TimeTriggerForm", {X=x, Y=y}, paramsOk, paramsCancel)
	end
	local function posCancel()
		debugInfo("Cancel choice 'TimeTrigger' position")
	end	
	r2:choosePos("object_component_time_trigger.creature", posOk, posCancel, "createFeatureTimeTrigger")
end


function feature.registerForms()
	r2.Forms.TimeTriggerForm =
	{
		Caption = "uiR2EdTimeTriggerParameters",
		Prop =
		{
			-- following field are tmp for property sheet building testing
 			{Name="Hours", Type="Number", Category="uiR2EDRollout_TimeTrigger",  Min="0", Max="23", Default="0"},			
			{Name="Minutes", Type="Number", Category="uiR2EDRollout_TimeTrigger",  Min="0", Max="59", Default="0"},			
--			{Name="Secondes", Type="Number", Category="uiR2EDRollout_TimeTrigger",  Min="0", Max="59", Default="0"},			
			{Name="Cyclic", Type="Number", Category="uiR2EDRollout_TimeTrigger",  WidgetStyle="Boolean", Min="0", Max="1", Default="0"},			

		}
	}

end


feature.preTranslatFeature = function(context)
	local instance = r2:getInstanceFromId(context.Feature.InstanceId);
	r2.Translator.registerManager(context, context.Feature)
	
end

feature.Translator = function(context)
	local rtNpcGrp = r2.Translator.getRtGroup(context, context.Feature.InstanceId)

	local aiState = r2.newComponent("RtAiState")
	aiState.AiActivity = "normal"
	table.insert(context.RtAct.AiStates, aiState)
	table.insert(aiState.Children, rtNpcGrp.Id)
	
	local instance = context.Feature
	r2.Translator.translateEventHandlers( context, instance, instance.Behavior.Actions, rtNpcGrp)

	do	
		local eventHandler = r2.Translator.createEvent("start_of_state", aiState.Id,  rtNpcGrp.Id)

		local action1 =  r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0, time)
		local action2 = r2.Translator.createAction("timer_set_daytime", rtNpcGrp.Id, 0, instance.Hours, instance.Minutes)
		local action = r2.Translator.createAction("multi_actions", {action1, action2})

		table.insert(context.RtAct.Events, eventHandler)		
		-- insert a npc_event_handler_action
		table.insert(eventHandler.ActionsId, action.Id)
		table.insert(context.RtAct.Actions, action)
	end

	if instance.Cyclic == 1 then
		local eventHandler  =r2.Translator.createEvent("timer_t0_triggered", aiState.Id,  rtNpcGrp.Id)
		local action1 =  r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0, time)
		local action2 = r2.Translator.createAction("timer_set_daytime", rtNpcGrp.Id, 0, instance.Hours, instance.Minutes)
		local action = r2.Translator.createAction("multi_actions", {action1, action2})

		table.insert(context.RtAct.Events, eventHandler)		
		-- insert a npc_event_handler_action
		table.insert(eventHandler.ActionsId, action.Id)
		table.insert(context.RtAct.Actions, action)
	end


end

function component:getLogicTranslations()
	-- register trad
	local logicTranslations = {
		["ApplicableActions"] = {	
			["activate"]			= { menu=i18n.get( "uiR2AA0Activate"		):toUtf8(), 
										text=i18n.get( "uiR2AA1Activate"		):toUtf8()}, 
			["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"		):toUtf8(), 
										text=i18n.get( "uiR2AA1Deactivate"		):toUtf8()}, 
			["trigger"]				= { menu=i18n.get( "uiR2AA0Trigger"			):toUtf8(), 
										text=i18n.get( "uiR2AA1Trigger"			):toUtf8()},
		},
		["Events"] = {	
			["On Activation"]		= { menu=i18n.get( "uiR2Event0Activation"	):toUtf8(), 
										text=i18n.get( "uiR2Event1Activation"	):toUtf8()}, 
			["On Desactivation"]	= { menu=i18n.get( "uiR2Event0Deactivation"	):toUtf8(), 
										text=i18n.get( "uiR2Event1Deactivation"	):toUtf8()},
			["On Trigger"]			= { menu=i18n.get( "uiR2Event0Trigger"		):toUtf8(), 
										text=i18n.get( "uiR2Event1Trigger"		):toUtf8()},
		},
		["Conditions"] = {	
			["is active"]			= { menu=i18n.get( "uiR2Test0Active"		):toUtf8(),
										text=i18n.get( "uiR2Test1Active"		):toUtf8()},
			["is finished"]			= { menu=i18n.get( "uiR2Test0TimerFinished"	):toUtf8(), 
										text=i18n.get( "uiR2Test1TimerFinished"	):toUtf8()},
		}
	}
	return logicTranslations
end

r2.Features["TimeTriggerFeature"] =  feature

