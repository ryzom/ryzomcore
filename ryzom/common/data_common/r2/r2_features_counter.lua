
-- In Translation file
-- Category : uiR2EdCounter --
-- CreationFrom : uiR2EdCounterParameters


r2.Features.CounterFeature = {}

local feature = r2.Features.CounterFeature

feature.Name="Counter"

feature.Description="A Counter"


function feature.registerForms()
	r2.Forms.CounterFeatureForm =
	{
		Caption = "uiR2EdCounterParameters",
		Prop =
		{
			-- following field are tmp for property sheet building testing
			{Name="TriggerValue", Type="Number", Category="uiR2EDRollout_CounterFeature", Min="0", Default="0"},
			{Name="Value", Type="Number", Category="uiR2EDRollout_CounterFeature", Min="1", Default="1"}		
		}
	}

end

feature.Components = {}

feature.Components.Counter =
	{
		BaseClass="LogicEntity",			
		Name="Counter",
		Menu="ui:interface:r2ed_feature_menu",
	
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},

		ApplicableActions = { "Activate", "Deactivate", "Increment", "Decrement", "Trigger"},

		Events = { "On Trigger", "On Activation", "On Deactivation"},

		Conditions = { "is active"},

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="Components", Type="Table"},
			{Name="Value", Type="Number"},
			{Name="TriggerValue", Type="Number", Min="0", Default="0"}
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
			--feature.createFeatureLocally(this)
		end,

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,

		translate = function(this, context)
			local initValue = this.Value
			local trigValue = this.TriggerValue	
		
			r2.Translator.translateAiGroup(this, context)

			local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
				
			local action1 =  r2.Translator.createAction("counter_init", rtNpcGrp.Id, initValue, trigValue)
			local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0)
			local action = r2.Translator.createAction("multi_actions", {action1, action2})

			r2.Translator.translateAiGroupInitialState(this, context, action)			
		end,

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
		end,
	}

local component = feature.Components.Counter 


function component.getLogicActionTrigger(entity, context, action, rtNpcGrp)
	local retAction = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 3)
	assert(retAction)
	return retAction, retAction
end

component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if (action.Action.Type == "Activate") then
		local action1 = r2.Translator.createAction("counter_enable", rtNpcGrp.Id, 0)
		local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 0)
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		return retAction, retAction
	elseif (action.Action.Type == "Deactivate") then
		local action1 = r2.Translator.createAction("counter_disable", rtNpcGrp.Id, 0)
		local action2 = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 1)
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		return retAction, retAction
	elseif (action.Action.Type == "Increment") then
		local action1 = r2.Translator.createAction("counter_inc", rtNpcGrp.Id, 0)
		return action1, action1
	elseif (action.Action.Type == "Decrement") then
		local action1 = r2.Translator.createAction("counter_dec", rtNpcGrp.Id, 0)
		return action1, action1
	elseif (action.Action.Type == "Trigger") then
		local retAction = r2.Translator.createAction("generic_event_trigger", rtNpcGrp.Id, 3)
		assert(retAction)
		return retAction, retAction
	end

	local firstAction, lastAction = nil, nil
	return firstAction, lastAction
end

component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("counter_is_enable", rtNpcGrp.Id, 0);
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

	if eventType == "On Trigger" then 
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 3)
	elseif eventType == "On Activation" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 0)
	elseif eventType == "On Deactivation" then
		return r2.Translator.getComponentGenericEvent(rtNpcGrp, 1)
	end
	
	return eventHandler, firstCondition, lastCondition
end


-- feature part

component.createComponent = function(x, y, value, tvalue)
	
	local comp = r2.newComponent("Counter")
	assert(comp)

	comp.Base = "palette.entities.botobjects.milestone"
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDRollout_CounterFeature")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	comp.TriggerValue = tvalue
	comp.Value = value

	comp._Seed = os.time() 

	return comp
end

component.create = function()	
	r2.requestNewAction(i18n.get("uiR2EDCreateCounterFeatureAction"))


	if not r2:checkAiQuota() then return end


	local function paramsOk(resultTable)

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local tvalue = tonumber( resultTable["TriggerValue"] )
		local value = tonumber( resultTable["Value"] )

		if not x or not y or not tvalue or not value
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.CounterFeature.createComponent( x, y, value, tvalue)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
		--component.TriggerValue = tvalue
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'CounterFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'CounterFeature' at pos (%d, %d, %d)", x, y, z))
		r2:doForm("CounterFeatureForm", {X=x, Y=y}, paramsOk, paramsCancel)
	end
	local function posCancel()
		debugInfo("Cancel choice 'CounterFeature' position")
	end	
	r2:choosePos("object_milestone.creature", posOk, posCancel, "createFeatureBanditCamp")
end

function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EDRollout_CounterFeature")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "CounterFeature")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
				["Activate"]		= { menu=i18n.get( "uiR2AA0Activate"			):toUtf8(),
										text=i18n.get( "uiR2AA1Activate"			):toUtf8()},
				["Desactivate"]		= { menu=i18n.get( "uiR2AA0Deactivate"			):toUtf8(),
										text=i18n.get( "uiR2AA1Deactivate"			):toUtf8()},
				["Increment"]		= { menu=i18n.get( "uiR2AA0CounterIncrement"	):toUtf8(),
										text=i18n.get( "uiR2AA1CounterIncrement"	):toUtf8()},
				["Decrement"]		= { menu=i18n.get( "uiR2AA0CounterDecrement"	):toUtf8(),
										text=i18n.get( "uiR2AA1CounterDecrement"	):toUtf8()},
		},
		["Events"] = {	
				["activation"]		= { menu=i18n.get( "uiR2Event0Activation"		):toUtf8(), 
										text=i18n.get( "uiR2Event1Activation"		):toUtf8()},
				["desactivation"]	= { menu=i18n.get( "uiR2Event0Deactivation"		):toUtf8(), 
										text=i18n.get( "uiR2Event1Deactivation"		):toUtf8()},
				["trigger"]			= { menu=i18n.get( "uiR2Event0Trigger"			):toUtf8(),
										text=i18n.get( "uiR2Event1Trigger"			):toUtf8()},
		},
		["Conditions"] = {	
				["is active"]		= { menu=i18n.get( "uiR2Test0Active"			):toUtf8(), 
										text=i18n.get( "uiR2Test1Active"			):toUtf8()}
		}
	}
	return logicTranslations
end

r2.Features["CounterFeature"] =  feature

