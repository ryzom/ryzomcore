
-- In Translation file
-- Category : uiR2EdUserTrigger --
-- CreationFrom : uiR2EdUserTriggerParameters


r2.Features.UserTriggerFeature = {}

local feature = r2.Features.UserTriggerFeature

feature.Name="UserTriggerFeature"

feature.Description="A feature that allows a DM to trigger an event"

feature.Components = {}


function feature.registerForms()
	r2.Forms.UserTriggerForm =
	{
		Caption = "uiR2EdUserTriggerParameters",
		PropertySheetHeader = 
		[[
			<view type="text" id="t" multi_line="true" sizeref="w" w="-36" x="4" y="-2" posref="TL TL" global_color="true" fontsize="14" shadow="true" hardtext="uiR2EdUserTriggerDescription"/>
		]],
		Prop =
		{
			{Name="Display", Type="Number", WidgetStyle="Boolean", DefaultValue="0", Translation="uiR2showMessageAgain", InvertWidget=true, CaptionWidth=5 },
		}
	}

end



feature.Components.UserTrigger =
	{
		PropertySheetHeader = r2.getDisplayButtonHeader("r2.events:openEditor()", "uiR2EdEditEventsButton"),
		BaseClass="LogicEntity",			
		Name="UserTrigger",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		-- 
		-- Supposed to be the specific attributes of the feature (ex: for a banditcamp, number of bandits,
		-- race, etc.) but apparently not used..
		--
		Parameters = {},
		--
 		-- The different actions that can be performed by the feature (activate, wander, spawn etc.)
		-- 
		 ApplicableActions = { "trigger"},
		--
		-- Events are what happen to the feature and may trigger actions on the feature. (ex: "on activation",
		-- "on arrive at camp" etc.)
		--
		Events = { "triggered"},
		
		-- Conditions are what can be tested on the feature, giving information about its state (ex: "is wandering", "is active" etc.)
		--
		Conditions = { },
		--
		-- TextContexts is what the feature might say upon events (2 different kinds: spontaneous & interrogation texts)
		--
		TextContexts =		{},
		--
		-- Not quite clear..
		--
		TextParameters =	{},
		--
		-- Feature's parameters which can be modified by the GM at runtime.
		--
		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		--
		-- Properties define the feature's parameters like this:
		-- {Name="Parameter_Name", Type="Param_Type", Category="??", WidgetStyle="UI_WidgetStyle", Min="min_value", Max="max_value", Default="defalut_value"
		-- Categories can be found in data_common/r2/r2.uxt??
		--
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table", Visible = false},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="Description", Type="String", }
		--	{Name="SheetId", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_Description"},
		},

		--	
		-- from base class
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,

		--
		-- from base class			
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
			r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
			for k, component in specPairs(this.Components) do
				component:appendInstancesByType(destTable, kind)
			end
		end,

		--
		-- from base class
		getSelectBarSons = function(this)
			return Components
		end,

		--
		-- from base class		
		canHaveSelectBarSons = function(this)
			return false;
		end,
		
		--
		-- Called when running EditMode to create locally the feature without sending anything into the act (speed purpose).
		--
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
	
			local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
			
			local rt = r2.newComponent("RtUserTrigger")
			assert(rt)
			rt.Name = this.Name
			rt.TriggerId = 1
			rt.Grp = rtNpcGrp.Id
			
			table.insert(context.RtAct.UserTriggers, rt)	
		end


	}


local component = feature.Components.UserTrigger

--
-- Create the logic actions relative to the feature via the translator.
--

component.getLogicAction = function(entity, context, action)
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if (action.Action.Type == "Trigger") then
		local retAction =r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 1)
		return retAction, retAction
	end

	local firstAction, lastAction = nil, nil
	return firstAction, lastAction
end

--
-- Checks the conditions defined for this feature
--
function component.getLogicCondition(this, context, condition)
	return nil,nil
end


function component.getLogicEvent(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component =  this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local eventType = tostring(event.Event.Type)
	
	local eventHandler, lastCondition = nil, nil

	if eventType == "triggered" then 
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 1)
	end
	
	return eventHandler, firstCondition, lastCondition
end




-- feature part


--
-- Creates an instance of the feature with attributes retrieved from ttriggeredd-
function component.createComponent(x, y)
	
	local comp = r2.newComponent("UserTrigger")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	comp.Name = r2:genInstanceName(i18n.get("uiR2ED" .. component.Name)):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	return comp
end




--
-- from ihm
-- Displays the creation form of the feature and calls CreateComponent with the user input values
--
component.create = function(this)	

	if not r2:checkAiQuota() then return end



	local function paramsOk(resultTable)

		r2.requestNewAction(i18n.get("uiR2EDNewUserTriggerFeatureAction"))

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end

		if showAgain == 1 then 
			r2.setDisplayInfo("UserTriggerForm", 0)
		else r2.setDisplayInfo("UserTriggerForm", 1) end

		local component = component.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end

	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'UserTrigger' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("UserTrigger") == 1 then 
			r2.displayFeatureHelp("UserTrigger")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewUserTriggerFeatureAction"))
		local component = component.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	
	local function posCancel()
		debugInfo("Cancel choice 'UserTriggerFeature' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureUserTrigger")
end



function component.getLogicAction(entity, context, action)
	assert( action.Class == "ActionStep") 
	
	local firstAction, lastAction = nil, nil

	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)


	if action.Action.Type == 'trigger' then
		local retAction =r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 1)
		return retAction, retAction
	end
		
	printWarning('Action not implemented yet :'.. action.Action.Type)
	assert(nil)
	return nil, nil
end

----------------------------------------------------------------------------
-- add a line to the event menu
function component:getLogicTranslations()

	local logicTranslations = {
		["ApplicableActions"] = {
			["trigger"]				= { menu=i18n.get( "uiR2AA0Trigger"			):toUtf8(), 
										text=i18n.get( "uiR2AA1Trigger"			):toUtf8()},
		},
		["Events"] = {	
			["triggered"]			= { menu=i18n.get( "uiR2Event0Trigger"		):toUtf8(), 
										text=i18n.get( "uiR2Event1Trigger"		):toUtf8()},
		}
	}
	return logicTranslations
end

r2.Features["UserTriggerFeature"] =  feature

