
r2.Features.ZoneTrigger = {}

local feature = r2.Features.ZoneTrigger 

feature.Name="ZoneTrigger"


feature.Description="Triggers an event when a player enters or leaves a zone"

feature.Components = {}

local classZoneTriggerVersion = 1

feature.Components.ZoneTrigger =
	{
		PropertySheetHeader = r2.getDisplayButtonHeader("r2.events:openEditor()", "uiR2EdEditEventsButton"),
		BaseClass="LogicEntity",			
		Name="ZoneTrigger",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		Version=classZoneTriggerVersion ,
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},
		ApplicableActions = {
			"activate", "deactivate", "simulate on enter", "simulate on leave"
		},
		Events = {
			"On Player Arrived", 
			"On Player Left",
			"activation", "deactivation"
	--		"On First Player Arrived", 
	--		"On First Player Left",			
		},
		Conditions = {"is active", "is inactive", "is empty", "is full"},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="Cyclic", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Components", Type="Table"}		
		
		},

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,
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
			local rtNpcGrp, aiState = r2.Translator.createAiGroup(this, context)
			local zone = r2:getInstanceFromId(this._Zone)
			Logic.assignZone(aiState, zone)
			aiState.IsTriggerZone = 1
		end,

		translate = function(this, context)

			local instance = this		
			local instanceId =  this.InstanceId
			local rtNpcGrp = r2.Translator.getRtGroup(context, instanceId)
			local states = r2.Translator.getRtStatesNames(context, instanceId)

		

			
			
			do	
				local eventHandler = r2.Translator.createEvent("on_player_arrived", states,  rtNpcGrp.Id)
				eventHandler.IsTriggerZone = 1
				local action =  r2.Translator.createAction("on_player_arrived_impl", rtNpcGrp.Id)

				table.insert(context.RtAct.Events, eventHandler)		
				-- insert a npc_event_handler_action
				table.insert(eventHandler.ActionsId, action.Id)
				table.insert(context.RtAct.Actions, action)
			end

			do	
				local eventHandler = r2.Translator.createEvent("on_player_left", states,  rtNpcGrp.Id)
				eventHandler.IsTriggerZone = 1
				local action =  r2.Translator.createAction("on_player_left_impl", rtNpcGrp.Id)

				table.insert(context.RtAct.Events, eventHandler)		
				-- insert a npc_event_handler_action
				table.insert(eventHandler.ActionsId, action.Id)
				table.insert(context.RtAct.Actions, action)
			end

			do
				local eventHandler  =r2.Translator.createEvent("start_of_state", states,  rtNpcGrp.Id)
				eventHandler.IsTriggerZone = 1
				local action = r2.Translator.createAction("trigger_zone_init", rtNpcGrp.Id, instance.Active, instance.Cyclic )

				table.insert(context.RtAct.Events, eventHandler)		
				-- insert a npc_event_handler_action
				table.insert(eventHandler.ActionsId, action.Id)
				table.insert(context.RtAct.Actions, action)
			end
		
			-- AdD
			r2.Translator.translateAiGroup(this, context)
			--r2.Translator.translateEventHandlers( context, instance, instance.Behavior.Actions, rtNpcGrp)
		
		end,

	updateVersion = function(this, scenarioValue, currentValue )
			local patchValue = scenarioValue
			if patchValue < 1 then
				if not this.Active then r2.requestSetNode(this.InstanceId, "Active", 1)	end
				if not this.Cyclic then r2.requestSetNode(this.InstanceId, "Cyclic", 1)	end
				patchValue = 1
			end
			
			if patchValue == currentValue then return true end
			return false
		end,		


	}

-- Specific to the component Zone
local component = feature.Components.ZoneTrigger  


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)


	local theType = action.Action.Type
	local retAction = nil
	if theType == "activate" then 
		retAction = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0, 
			r2.Translator.createAction("trigger_zone_activates", rtNpcGrp.Id))
	elseif theType == "deactivate" then		
		retAction = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1,
			r2.Translator.createAction("trigger_zone_deactivates", rtNpcGrp.Id))
	elseif theType == "simulate on enter" then		
		retAction = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1,
			r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 1))
	elseif theType == "simulate on leave" then		
		retAction = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1,
			r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 2))

	end
	
	return retAction, retAction
end

component.getLogicCondition = function(this, context, condition)
	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	local states = r2.Translator.getRtStatesNames(context, component.InstanceId)
	assert(states)

	local theType = condition.Condition.Type
	if theType == "is empty" then
		local retAction = r2.Translator.createAction("trigger_zone_min_player", rtNpcGrp.Id, states, 0)
		return retAction, retAction
	elseif theType == "is full" then
		local retAction = r2.Translator.createAction("trigger_zone_min_player", rtNpcGrp.Id, states, 1)
		return retAction, retAction
	end


	return r2.Translator.getFeatureActivationCondition(condition, rtNpcGrp)
end

component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component =  this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local eventType = tostring(event.Event.Type)
	
	local eventHandler, lastCondition = nil, nil
			
	if eventType == "On Player Arrived" then 
		local e, c1, c2 =r2.Translator.getComponentUserEvent(rtNpcGrp, 1);
		e.IsTriggerZone = 1
		return e, c1, c2
	elseif eventType == "On Player Left" then
		local e, c1, c2 = r2.Translator.getComponentUserEvent(rtNpcGrp, 2);
		e.IsTriggerZone = 1
		return e, c1, c2
	elseif eventType == "activation" then
		e, c1, c2 = r2.Translator.getComponentUserEvent(rtNpcGrp, 4)
		e.IsTriggerZone = 1
		return e, c1, c2
	elseif eventType == "deactivation" then
		e, c1, c2 = r2.Translator.getComponentUserEvent(rtNpcGrp, 5)
		e.IsTriggerZone = 1
		return e, c1, c2
 	end
	return eventHandler, firstCondition, lastCondition
end


local ZoneTriggerRadius = 5
local ZoneTriggerNumCorners = 4

component.createComponent = function(x, y)
	
	local comp = r2.newComponent("ZoneTrigger")
	assert(comp)
	assert(comp.Position)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.trigger_zone")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDRollout_ZoneTrigger")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	local zone = r2.newComponent("Region")
	r2.Utils.createTriggerRegion(zone, 0, 0, ZoneTriggerRadius) -- the region doesn't inherit the feature position, so must give its pos
	zone.Deletable = 0
	zone.Position.x = 0 -- comp.Position.x
	zone.Position.y = 0 --  comp.Position.y
	zone.Position.z = 0 -- comp.Position.z
	zone.InheritPos = 1 -- don't inherit position of parents
	zone.Name = r2:genInstanceName(i18n.get("uiR2EDPlaces")):toUtf8()			
	comp._Zone = zone.InstanceId
	table.insert(comp.Components, zone)

	
	return comp
end

component.create = function()	

	if not r2:checkAiQuota() then return end

	local function paramsOk(resultTable)

		r2.requestNewAction(i18n.get("uiR2EDNewZoneTriggersFeatureAction"))

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		if not x or not y 		then
			debugInfo("Can't create Component")
			return
		end
		
		if showAgain == 1 then 
			r2.setDisplayInfo("ZoneForm", 0)
		else r2.setDisplayInfo("ZoneForm", 1) end


		local component = feature.Components.ZoneTrigger.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end

	local function posOk(x, y, z)
		debugInfo("Validate creation of a Zone Trigger.")
		if r2.mustDisplayInfo("ZoneTrigger") == 1 then 
			r2.displayFeatureHelp("ZoneTrigger")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewZoneTriggersFeatureAction"))
		local component = feature.Components.ZoneTrigger.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end

	local function posCancel()	end

	local creature = r2.Translator.getDebugCreature("object_component_trigger_zone.creature") 
	--r2:choosePos(creature, posOk, posCancel, "createFeatureZoneTrigger")

	local poly = {}
	local step = 2 * math.pi / ZoneTriggerNumCorners
	for k = 0, ZoneTriggerNumCorners - 1 do
		table.insert(poly, CVector2f(ZoneTriggerRadius * math.cos(k * step), ZoneTriggerRadius * math.sin(k * step)))
	end	
	r2:choosePos(creature, posOk, posCancel, "createFeatureZoneTrigger", 
				 "curs_create.tga",
				 "curs_stop.tga",
	             { poly }, r2.PrimRender.ComponentRegionLook, r2.PrimRender.ComponentRegionInvalidLook)


end


function feature.registerForms()
	r2.Forms.ZoneForm =
	{
		Caption = "uiR2EdTriggerZoneParameters",
		PropertySheetHeader = 
		[[
			<view type="text" id="t" multi_line="true" sizeref="w" w="-36" x="4" y="-2" posref="TL TL" global_color="true" fontsize="14" shadow="true" hardtext="uiR2EDTriggerZoneDescription"/>
		]],
		Prop =
		{
			{Name="Display", Type="Number", WidgetStyle="Boolean", DefaultValue="0", Translation="uiR2showMessageAgain", InvertWidget=true, CaptionWidth=5 },
		}
	}

end

-----------------------------------------
--- register the curent Feature to menu

function component:getLogicTranslations()
	-- register trad
	local logicTranslations = {
		["ApplicableActions"] = {	
			["activate"]				= { menu=i18n.get( "uiR2AA0Activate"			):toUtf8(), 
											text=i18n.get( "uiR2AA1Activate"			):toUtf8()}, 
			["deactivate"]				= { menu=i18n.get( "uiR2AA0Deactivate"			):toUtf8(), 
											text=i18n.get( "uiR2AA1Deactivate"			):toUtf8()}, 
			["simulate on enter"]		= { menu=i18n.get( "uiR2AA0ZoneTriggerSimEnter"	):toUtf8(), 
											text=i18n.get( "uiR2AA1ZoneTriggerSimEnter"	):toUtf8()},
			["simulate on leave"]		= { menu=i18n.get( "uiR2AA0ZoneTriggerSimExit"	):toUtf8(), 
											text=i18n.get( "uiR2AA1ZoneTriggerSimExit"	):toUtf8()},
		},
		["Events"] = {
			["activation"]				= { menu=i18n.get( "uiR2Event0Activation"		):toUtf8(), 
											text=i18n.get( "uiR2Event1Activation"		):toUtf8()},
			["deactivation"]			= { menu=i18n.get( "uiR2Event0Deactivation"		):toUtf8(), 
											text=i18n.get( "uiR2Event1Deactivation"		):toUtf8()},
			["On Player Arrived"]		= { menu=i18n.get( "uiR2Event0ZoneTriggerEntry"	):toUtf8(), 
											text=i18n.get( "uiR2Event1ZoneTriggerEntry"	):toUtf8()},
			["On Player Left"]			= { menu=i18n.get( "uiR2Event0ZoneTriggerExit"	):toUtf8(), 
											text=i18n.get( "uiR2Event1ZoneTriggerExit"	):toUtf8()},
		},
		["Conditions"] = {
			["is active"]				= { menu=i18n.get( "uiR2Test0Active"			):toUtf8(), 
											text=i18n.get( "uiR2Test1Active"			):toUtf8()},
			["is inactive"]				= { menu=i18n.get( "uiR2Test0Inactive"			):toUtf8(), 
											text=i18n.get( "uiR2Test1Inactive"			):toUtf8()},
			["is empty"]				= { menu=i18n.get( "uiR2Test0ZoneTriggerEmpty"			):toUtf8(), 
											text=i18n.get( "uiR2Test1ZoneTriggerEmpty"			):toUtf8()},
			["is full"]				= { menu=i18n.get( "uiR2Test0ZoneTriggerFull"			):toUtf8(), 
											text=i18n.get( "uiR2Test1ZoneTriggerFull"			):toUtf8()},


		}
	}

	return logicTranslations
end

r2.Features["ZoneTrigger"] =  feature



