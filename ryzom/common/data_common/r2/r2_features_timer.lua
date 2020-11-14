-- In Transalation file
-- Category : uiR2EdTimer --
-- CreationFrom : uiR2EdTimerParameters
-- uiR2EDtooltipCreateFeatureTimer -> tooltip

r2.Features.TimerFeature = {}

local feature = r2.Features.TimerFeature 

feature.Name="TimerFeature"

feature.BanditCount = 0

feature.Description="A Timer"

feature.Components = {}


feature.Components.Timer =
	{
		PropertySheetHeader = r2.getDisplayButtonHeader("r2.events:openEditor()", "uiR2EdEditEventsButton"),
		BaseClass="LogicEntity",			
		Name="Timer",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},
		ApplicableActions = {
			"Activate", "Deactivate", "Pause", "Resume", "Trigger",
			"add seconds", "sub seconds",
--			"Add 10 Seconds", "Add 1 minute", "Sub 10 seconds", "Sub 1 minute"
		},
		Events = {
			"On Trigger", 
			"On Activation", "On Desactivation", 	
			"On Pause", "On Resume"
		},
		Conditions = {
			"is active", "is inactive", "is paused", 
			"is running", "is finished"
		},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		-- Category="uiR2EDRollout_Timer",
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="Components", Type="Table"},
			{Name="Minutes", Type="Number", Min="0", Max="120", DefaultValue="0"},
			{Name="Secondes",Type="Number", Min="0", Max="999", DefaultValue="30"},
			{Name="Cyclic", Type="Number",  WidgetStyle="Boolean", Min="0", Max="1", DefaultValue="0"},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},					
		},
		-----------------------------------------------------------------------------------------------		
		-- from base class
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,
		---------------------------------------------------------------------------------------------------------
		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
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
			
				
			local time = this.Secondes + this.Minutes * 60
			--local time = secs + min * 60
			local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)	
			
			if this.Active == 1 then
				local action1 =  r2.Translator.createAction("timer_set", rtNpcGrp.Id, 0, time)
				local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0)
				local action3 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "v2", 0)
				local action4 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 1)
				local action = r2.Translator.createAction("multi_actions", {action1, action2, action3, action4})

				r2.Translator.translateAiGroupInitialState(this, context, action)
			else
				local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "v2", 0)
				local action2 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 0)
				local action = r2.Translator.createAction("multi_actions", {action1, action2})

				r2.Translator.translateAiGroupInitialState(this, context, action)
			end	

			do
				local actionTrigger = r2.Translator.createAction("set_value", rtNpcGrp.Id, "v2", 1)
				r2.Translator.translateAiGroupEvent("timer_t0_triggered", this, context, actionTrigger)					
			end
			
			--TODO: gestion v2 pour trigger
			if this.Cyclic == 1 then
				local states = r2.Translator.getRtStatesNames(context, this.InstanceId)
				local eventHandler = r2.Translator.createEvent("timer_t0_triggered", states,  rtNpcGrp.Id)
				local action1 =  r2.Translator.createAction("timer_set", rtNpcGrp.Id, 0, time)
				local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0)
				local action = r2.Translator.createAction("multi_actions", {action1, action2})
				table.insert(context.RtAct.Events, eventHandler)		
				-- insert a npc_event_handler_action
				table.insert(eventHandler.ActionsId, action.Id)
				table.insert(context.RtAct.Actions, action)
			else
				local states = r2.Translator.getRtStatesNames(context, this.InstanceId)
				local eventHandler = r2.Translator.createEvent("timer_t0_triggered", states,  rtNpcGrp.Id)
				local actionEmit = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 1)
				table.insert(context.RtAct.Events, eventHandler)		
				-- insert a npc_event_handler_action
				table.insert(eventHandler.ActionsId, actionEmit.Id)
				table.insert(context.RtAct.Actions, actionEmit)
			end
		end,
	}

-- Specific to the component Timer
local component = feature.Components.Timer  


function component.getLogicActionActivate(entity, context, action, rtNpcGrp)

	local time = entity.Secondes + entity.Minutes * 60
	local action1 =  r2.Translator.createAction("timer_set", rtNpcGrp.Id, 0, time)
--	local action1 = r2.Translator.createAction("timer_enable", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0)
	local actionIfInactive = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0, 
			r2.Translator.createAction("multi_actions", {action1, action2}))

	return actionIfInactive, actionIfInactive
end

function component.getLogicActionDeactivate(entity, context, action, rtNpcGrp)
	local action2 = r2.Translator.createAction("timer_disable", rtNpcGrp.Id, 0)
	local action1 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 1)
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	local actionIfActive = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1, retAction)
	return actionIfActive, actionIfActive
end

function component.getLogicActionPause(entity, context, action, rtNpcGrp)
	local action1 = r2.Translator.createAction("timer_suspend", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 2)

	local multiAction = r2.Translator.createAction("multi_actions", {action1, action2})

	local actionIsEnable = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
	local actionIf = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "is_enable", 1, multiAction)
	local retAction = r2.Translator.createAction("multi_actions", {actionIsEnable, actionIf})
	assert(retAction)
	return retAction, retAction
end

function component.getLogicActionResume(entity, context, action, rtNpcGrp)

	local action1 = r2.Translator.createAction("timer_resume", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 3)

	local multiAction = r2.Translator.createAction("multi_actions", {action1, action2})

	local actionIsEnable = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
	local actionIf = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "is_enable", 1, multiAction)
	local retAction = r2.Translator.createAction("multi_actions", {actionIsEnable, actionIf})
	assert(retAction)
	return retAction, retAction
end

function component.getLogicActionTrigger(entity, context, action, rtNpcGrp)
	local multiAction = r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtNpcGrp.Id, "v2", 1),
				r2.Translator.createAction("timer_trigger", rtNpcGrp.Id, 0)})
	local actionIsEnable = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
	local actionIf = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "is_enable", 1, multiAction)
	local retAction = r2.Translator.createAction("multi_actions", {actionIsEnable, actionIf})
	assert(retAction)
	return retAction, retAction
end

function component.getLogicActionAddSeconds(entity, context, action, rtNpcGrp)
	local value = 0
	if action.Action.ValueString then value = tonumber(action.Action.ValueString) end
	local timerAdd = r2.Translator.createAction("timer_add", rtNpcGrp.Id, 0, value)

	local actionIsEnable = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
	local actionIf = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "is_enable", 1, timerAdd)
	local retAction = r2.Translator.createAction("multi_actions", {actionIsEnable, actionIf})
	assert(retAction)
	return retAction, retAction
end

function component.getLogicActionSubSeconds(entity, context, action, rtNpcGrp)
	local value = 0
	if action.Action.ValueString then value = tonumber(action.Action.ValueString) end
	local actionSub = r2.Translator.createAction("timer_sub", rtNpcGrp.Id, 0, value)

	local actionIsEnable = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
	local actionIf = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "is_enable", 1, actionSub)
	local retAction = r2.Translator.createAction("multi_actions", {actionIsEnable, actionIf})
	assert(retAction)
	return retAction, retAction

end


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)


	local funs = {
		["Activate"] = component.getLogicActionActivate,
		["Deactivate"] =  component.getLogicActionDeactivate,
		["Pause"] =  component.getLogicActionPause,
		["Resume"] =  component.getLogicActionResume,
		["Trigger"] =  component.getLogicActionTrigger,
		["Add 10 Seconds"] =  component.getLogicActionAdd10Seconds, 
		["Add 1 minute"] =  component.getLogicActionAdd1Minute, 
		["Sub 10 seconds"] =  component.getLogicActionSub0Seconds,
		["Sub 1 minute"] =  component.getLogicActionSub1Minute,
		["sub seconds"] =  component.getLogicActionSubSeconds,
		["add seconds"] =  component.getLogicActionAddSeconds,

	}


	local fun = funs[ action.Action.Type ]
	if fun then
		firstAction, lastAction =  fun(entity, context, action, rtNpcGrp)
	
	end

	assert(firstAction)
	return firstAction, lastAction
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
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("condition_if", prefix.."is_enable == 1" );
		local multiactions= r2.Translator.createAction("multi_actions", {action1, action2});
		return multiactions, action2
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0)
		local action2 = r2.Translator.createAction("condition_if", prefix.."is_enable == 0")
		local multiActions = r2.Translator.createAction("multi_actions", {action1, action2})
		return multiActions, action2
	elseif condition.Condition.Type == "is paused" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("timer_is_suspended", rtNpcGrp.Id, 0);
		local actionSuspended = r2.Translator.createAction("condition_if", prefix.."is_suspended == 1");
		local action3 = r2.Translator.createAction("condition_if", prefix.."is_enable == 1", actionSuspended);
		local multiactions = r2.Translator.createAction("multi_actions", {action1, action2, action3});	
		return multiactions, actionSuspended
	elseif condition.Condition.Type == "is running" then
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("timer_is_suspended", rtNpcGrp.Id, 0);
		local actionSuspended = r2.Translator.createAction("condition_if", prefix.."is_suspended == 0");
		local action3 = r2.Translator.createAction("condition_if", prefix.."is_enable == 1", actionSuspended);
		local multiactions = r2.Translator.createAction("multi_actions", {action1, action2, action3});
		return multiactions, actionSuspended
	elseif condition.Condition.Type == "is finished" then	
		local action1 = r2.Translator.createAction("timer_is_enable", rtNpcGrp.Id, 0);
		local action2 = r2.Translator.createAction("condition_if", prefix.."is_enable == 0");
		local action3 = r2.Translator.createAction("condition_if", prefix.."v2 == 1", action2);
		
		local multiactions= r2.Translator.createAction("multi_actions", {action1, action3});
		
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
		--return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	elseif eventType == "On Activation" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 0)
	elseif eventType == "On Desactivation" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 1)
	elseif eventType == "On Pause" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 2)
	elseif eventType == "On Resume" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 3)
	end
	
	
	return eventHandler, firstCondition, lastCondition
end

component.createComponent = function(x, y, secondes, minutes, cyclic)
	
	local comp = r2.newComponent("Timer")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.timer")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdNameTimerFeature")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	if minutes then comp.Minutes = minutes end
	if secondes then comp.Secondes = secondes end
	if cyclic then comp.Cyclic = cyclic end

	return comp
end


component.create = function()	

	if not r2:checkAiQuota() then return end


	local function paramsOk(resultTable)

		

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local minutes = tonumber( resultTable["Minutes"] )
		local secondes = tonumber( resultTable["Secondes"] )
		local cyclic  = tonumber( resultTable["Cyclic"] )
		local showAgain = tonumber(resultTable["Display"])

		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		
		local component = feature.Components.Timer.createComponent( x, y, secondes, minutes, cyclic)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end

	local function paramsCancel()
		debugInfo("Cancel form for 'Timer' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'Timer' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("Timer") == 1 then 
			r2.displayFeatureHelp("Timer")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewTimerFeatureAction"))
		local component = feature.Components.Timer.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	
	end
	local function posCancel()
		debugInfo("Cancel choice 'Timer' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_timer.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureTimer")
end



function feature.registerForms()
	r2.Forms.TimerForm =
	{
		Caption = "uiR2EdTimerParameters",
		PropertySheetHeader = 
		[[
			<view type="text" id="t" multi_line="true" sizeref="w" w="-36" x="4" y="-2" posref="TL TL" global_color="true" fontsize="14" shadow="true" hardtext="uiR2EDTimerDescription"/>
		]],

		Prop =
		{
			{Name="Display", Type="Number", WidgetStyle="Boolean", DefaultValue="0", Translation="uiR2showMessageAgain", InvertWidget=true, CaptionWidth=5 },
			-- following field are tmp for property sheet building testing
		--	{Name="Minutes", Type="Number", Category="uiR2EDRollout_Timer",  Min="0", Max="59", Default="0"},			
		--	{Name="Secondes", Type="Number", Category="uiR2EDRollout_Timer",  Min="0", Max="59", Default="10"},			
		--	{Name="Cyclic", Type="Number", Category="uiR2EDRollout_Timer",  WidgetStyle="Boolean", Min="0", Max="1", Default="0"},			
		--	{Name="Active", Type="Number", WidgetStyle="Boolean", Category="uiR2EDRollout_Default", DefaultValue="1"},
		}
		
	}

end

function component:getLogicTranslations()
	-- register trad
	local logicTranslations = {
		["ApplicableActions"] = {	
			["Activate"]		= { menu=i18n.get( "uiR2AA0Activate"			):toUtf8(),
									text=i18n.get( "uiR2AA1Activate"			):toUtf8()},
			["Deactivate"]		= { menu=i18n.get( "uiR2AA0Deactivate"			):toUtf8(),
									text=i18n.get( "uiR2AA1Deactivate"			):toUtf8()},
			["Trigger"]			= { menu=i18n.get( "uiR2AA0Trigger"				):toUtf8(), 
									text=i18n.get( "uiR2AA1Trigger"				):toUtf8()},
			["Pause"]			= { menu=i18n.get( "uiR2AA0TimerPause"			):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerPause"			):toUtf8()},
			["Resume"]			= { menu=i18n.get( "uiR2AA0TimerResume"			):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerResume"			):toUtf8()},
			["Add 10 Seconds"]	= { menu=i18n.get( "uiR2AA0TimerAdd10s"			):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerAdds10s"		):toUtf8()},
			["Add 1 minute"]	= { menu=i18n.get( "uiR2AA0TimerAdd1m"			):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerAdds1m"			):toUtf8()},
			["Sub 10 seconds"]	= { menu=i18n.get( "uiR2AA0TimerSub10s"			):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerSubs10s"		):toUtf8()},
			["Sub 1 minute"]	= { menu=i18n.get( "uiR2AA0TimerSub1m"			):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerSubs1m"			):toUtf8()},
			["add seconds"]		= { menu=i18n.get( "uiR2AA0TimerAddNSeconds"	):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerAddNSeconds"	):toUtf8()},
			["sub seconds"]		= { menu=i18n.get( "uiR2AA0TimerSubNSeconds"	):toUtf8(), 
									text=i18n.get( "uiR2AA1TimerSubNSeconds"	):toUtf8()},
		},
		["Events"] = {	
			["On Activation"]	= { menu=i18n.get( "uiR2Event0Activation"		):toUtf8(), 
									text=i18n.get( "uiR2Event1Activation"		):toUtf8()},
			["On Desactivation"]= { menu=i18n.get( "uiR2Event0Deactivation"		):toUtf8(), 
									text=i18n.get( "uiR2Event1Deactivation"		):toUtf8()},
			["On Trigger"]		= { menu=i18n.get( "uiR2Event0Trigger"			):toUtf8(), 
									text=i18n.get( "uiR2Event1Trigger"			):toUtf8()},
			["On Pause"]		= { menu=i18n.get( "uiR2Event0TimerPause"		):toUtf8(), 
									text=i18n.get( "uiR2Event1TimerPause"		):toUtf8()},
			["On Resume"]		= { menu=i18n.get( "uiR2Event0TimerResume"		):toUtf8(), 
									text=i18n.get( "uiR2Event1TimerResume"		):toUtf8()},
		},
		["Conditions"] = {	
			["is active"]		= { menu=i18n.get( "uiR2Test0Active"			):toUtf8(), 
									text=i18n.get( "uiR2Test1Active"			):toUtf8()},
			["is inactive"]		= { menu=i18n.get( "uiR2Test0Inactive"			):toUtf8(), 
									text=i18n.get( "uiR2Test1Inactive"			):toUtf8()},
			["is paused"]		= { menu=i18n.get( "uiR2Test0TimerPaused"		):toUtf8(), 
									text=i18n.get( "uiR2Test1TimerPaused"		):toUtf8()},
			["is running"]		= { menu=i18n.get( "uiR2Test0TimerRunning"		):toUtf8(), 
									text=i18n.get( "uiR2Test1TimerRunning"		):toUtf8()},
			["is finished"]		= { menu=i18n.get( "uiR2Test0TimerFinished"		):toUtf8(), 
									text=i18n.get( "uiR2Test1TimerFinished"			):toUtf8()}, 
		}
	}
	return logicTranslations
end

function component.initEventValuesMenu(this, menu, categoryEvent)
	--local startTime = nltime.getPreciseLocalTime()	
	for ev=0,menu:getNumLine()-1 do

		local eventType = tostring(menu:getLineId(ev))
		
		--local endTime = nltime.getPreciseLocalTime()
		--debugInfo(string.format("time for 10 is %f", endTime - startTime))
		--startTime = nltime.getPreciseLocalTime()

		if r2.events.eventTypeWithValue[eventType] == "Number" then
			menu:addSubMenu(ev)
			local subMenu = menu:getSubMenu(ev)
			local func = ""
	--		for i=0, 9 do
	--			local uc_name = ucstring()
	--			uc_name:fromUtf8( tostring(i) )
	--			func = "r2.events:setEventValue('','" .. categoryEvent .."','".. tostring(i).."')"
	--			subMenu:addLine(uc_name, "lua", func, tostring(i))
	--		end

			--endTime = nltime.getPreciseLocalTime()
			--debugInfo(string.format("time for 11 is %f", endTime - startTime))
			--startTime = nltime.getPreciseLocalTime()

			local lineNb = 0
			for i=0, 50, 10 do
				local lineStr = tostring(i).."/"..tostring(i+9)
				subMenu:addLine(ucstring(lineStr), "", "", tostring(i))

				--endTime = nltime.getPreciseLocalTime()
				--debugInfo(string.format("time for 12 is %f", endTime - startTime))
				--startTime = nltime.getPreciseLocalTime()

				subMenu:addSubMenu(lineNb)
				local subMenu2= subMenu:getSubMenu(lineNb)
				for s=0,9 do
					lineStr = tostring(i+s) 
					local func = "r2.events:setEventValue('','" .. categoryEvent .."','".. lineStr.."')"
					subMenu2:addLine(ucstring(lineStr), "lua", func, lineStr)
				end
				lineNb = lineNb+1

				--endTime = nltime.getPreciseLocalTime()
				--debugInfo(string.format("time for 13 is %f", endTime - startTime))
				--startTime = nltime.getPreciseLocalTime()
			end

			for i=0, 50, 10 do
				local lineStr = tostring(i).." m /"..tostring(i+9).." m"
				subMenu:addLine(ucstring(lineStr), "", "", tostring(i))

				subMenu:addSubMenu(lineNb)
				local subMenu2= subMenu:getSubMenu(lineNb)
				local index = 0

				--endTime = nltime.getPreciseLocalTime()
				--debugInfo(string.format("time for 14 is %f", endTime - startTime))
				--startTime = nltime.getPreciseLocalTime()

				for m=0,9 do
					lineStr = tostring( (i+m)*60) 
--					local func = "r2.events:setEventValue('','" .. categoryEvent .."','".. lineStr.."')"
					subMenu2:addLine(ucstring(tostring(i+m) .. "m"), "", "", lineStr)
					subMenu2:addSubMenu(index)
					local subMenu3= subMenu2:getSubMenu(index)
					index = index + 1
					for s=0, 55, 5 do
						lineStr = tostring( (i+m)*60 + s) 
						local func = "r2.events:setEventValue('','" .. categoryEvent .."','".. lineStr.."')"
						subMenu3:addLine(ucstring(tostring(i+m) .. "m ".. s .. "s"), "lua", func, lineStr)				
					end										
				end
				lineNb = lineNb+1

				--endTime = nltime.getPreciseLocalTime()
				--debugInfo(string.format("time for 15 is %f", endTime - startTime))
				--startTime = nltime.getPreciseLocalTime()
			end
		end
	end
end

r2.Features["TimerFeature"] =  feature

