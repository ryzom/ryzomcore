
r2.Features.HuntTask = {}

local feature = r2.Features.HuntTask

feature.Name="HuntTask"

feature.Description=""

feature.Components = {}

feature.Components.HuntTask =
	{
		BaseClass="LogicEntity",			
		Name="HuntTask",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "huntTaskDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate", "succeed", "succeedStep"},

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
			{Name="TargetNumber", Type="Number", Min="1", Default="1"},
			{Name="IdNumber", Type="Number", Category="uiR2EDRollout_Targets", WidgetStyle="EnumDropDown", Enum={"1", "2", "3", "4"},},
			{Name="Target1Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget",Visible= function(this) return this:displayRefId(1) end},
			{Name="Target2Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget",Visible= function(this) return this:displayRefId(2) end},
			{Name="Target3Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget",Visible= function(this) return this:displayRefId(3) end},
			{Name="Target4Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickNpcOrGroup", SetRefIdFunction="r2:setNpcOrGroupRefIdTarget",Visible= function(this) return this:displayRefId(4) end},
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


		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,

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


local component = feature.Components.HuntTask

function component:IsValidationNeeded()
	local validationNeeded = self.ValidationNeeded
	if validationNeeded == 1 then
		return true
	end
	return false
end

function component:displayRefId(index)
	local nbRefId = self.IdNumber + 1
	if index <= nbRefId then
		return true
	end
	return false
end
---------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------
function component:textAdapter(text)

	assert(self)
	assert(type(text) == "string")
	local str =  text
	local mission_giver = ""
	local nbTargets = ""

	if tostring(self.MissionGiver) ~= "" then
		local giver = r2:getInstanceFromId(self.MissionGiver)
		if giver then 	mission_giver = giver.Name end
	end

	if tostring(self.TargetNumber) ~= "" then
		nbTargets = tostring(self.TargetNumber)
	end
	
	str=string.gsub(str, "<mission_giver>", mission_giver)
	str=string.gsub(str, "<target_number>", nbTargets)
	return str
end

local huntTaskDisplayerTable = clone(r2:propertySheetDisplayer())

--
-- If the message is received by a client that didn't request the modification, we must make sure this client 
-- doesn't modify the data because it has already been done by the initial client. 
--
local function checkPickedEntity(this, instanceId, attributeName)
	if instanceId == "" then
		return false
	end
	local tmpInstance = r2:getInstanceFromId(instanceId)
	assert(tmpInstance)
	local i = 1
	while i < 5 do
		local attrName = "Target" ..i.. "Id"
		if attrName ~= attributeName and this[attrName] == tmpInstance.InstanceId then
			return false
		end
		if attributeName ~= "MissionGiver" and this["MissionGiver"] == tmpInstance.InstanceId then
			return false
		end
		i = i + 1
	end
	return true
end


local oldOnAttrModified = huntTaskDisplayerTable.onAttrModified
function huntTaskDisplayerTable:onAttrModified(instance, attributeName)
	
	oldOnAttrModified(self, instance, attributeName)
	if attributeName == "ValidationNeeded" then
		local propertySheet = r2:getPropertySheet(instance)
		propertySheet.Env.updatePropVisibility()
		return
	end
	if attributeName == "IdNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbRefId = instance.IdNumber + 1
		local i = 1
		while i <= 4 do
			if i > nbRefId then
				local name = "Target"..tostring(i).."Id"
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

	if (string.find(attributeName, "Id") == nil or attributeName == "InstanceId") and not attributeName == "MissionGiver" then return end 
	
	local propertySheet = r2:getPropertySheet(instance)
	local refId = propertySheet:find(attributeName)
	if refId == nil then return end
	local refIdName = refId:find("name")
	local instanceId = instance[attributeName]
	if not instanceId then
		return
	end
	
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

function huntTaskDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end


function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end


local function reattributeIdOnHrcMove(hunt, group, targetAttr)
	local propertySheet = r2:getPropertySheet(hunt)
	local refId = propertySheet:find(targetAttr)
	local refIdName = refId:find("name")
	
	r2.requestSetNode(hunt.InstanceId, targetAttr, group.InstanceId)
	refIdName.hardtext = group.Name
	hunt.User.onHrcMove = true

end


function component:onTargetInstancePostHrcMove(targetAttr, targetIndexInArray)
	if targetAttr == "MissionGiver" then 
		r2.requestSetNode(self.InstanceId, targetAttr, "")
		return
	end
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


function r2:huntTaskDisplayer()	
	return huntTaskDisplayerTable  -- returned shared displayer to avoid wasting memory
end

---------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------

function component:createGhostComponents(act)
	local comp = self
	
	local counter = r2.newComponent("Counter")
	assert(counter)

	counter.Base = "palette.entities.botobjects.milestone"
	counter.Name = "TargetCounter"
	counter.Position.x = comp.Position.x
	counter.Position.y = comp.Position.y
	counter.Position.z = 0

	local counterId = counter.InstanceId
	assert(counterId)
	r2.requestSetGhostNode(comp.InstanceId, "_CounterId", counterId)
	local nbTarget = 0
	local eventType = "" --depends on the instance type (groupe or npc)
	local eventName = ""
	
	-- Add to each guard a 'OnDeath EventHandler' which decrements the counter 
	for id = 1, 4 do
		local propertyName = "Target"..tonumber(id).."Id"
		if comp[propertyName] ~= nil and comp[propertyName] ~= "" then
			local targetInstance = r2:getInstanceFromId(comp[propertyName])
			if targetInstance then
				
				if targetInstance:isKindOf("Npc") then
					eventType = "death"
					eventName = "On Death"
				elseif targetInstance:isKindOf("NpcGrpFeature") then
					eventType = "member death"
					eventName = "On Member Death"
				end
					 
				local eventHandler = r2.newComponent("LogicEntityAction")
				--eventHandler.Event.Type = "death"
				eventHandler.Event.Type = eventType
				eventHandler.Event.Value = ""
				--eventHandler.Name = "On Death"
				eventHandler.Event.Name = eventName
				
				local action = r2.newComponent("ActionStep")
				table.insert(eventHandler.Actions, action)
				action.Entity = r2.RefId(comp.InstanceId)
				action.Action.Type = "succeedStep"
				action.Action.Value = ""
				
				if targetInstance:isKindOf("Npc") then
					r2.requestInsertGhostNode(targetInstance.Behavior.InstanceId, "Actions", -1, "", eventHandler)
				elseif targetInstance:isKindOf("NpcGrpFeature") then
					r2.requestInsertGhostNode(targetInstance.Components[0].Behavior.InstanceId, "Actions", -1, "", eventHandler)
				end
				--r2.requestInsertGhostNode(guardInstance.Behavior.InstanceId, "Actions", -1, "", eventHandler)
				
				nbTarget = nbTarget + 1
			end
		end
	end
	
	if nbTarget == 0 then
		debugInfo("hunt task: No target has been picked.")
		return
	end

	counter.Value = tonumber(comp.TargetNumber)
	counter.TriggerValue = 0

	do
		local validationNeeded = tonumber(comp.ValidationNeeded)

		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Event.Type = "On Trigger"
		eventHandler.Event.Value = ""
		eventHandler.Name = "On Trigger"
		
		local action = r2.newComponent("ActionStep")	
		action.Entity =  r2.RefId(comp.InstanceId) 

		if validationNeeded == 1 then
			action.Action.Type = "validateTask"
		else
			action.Action.Type = "succeed"
		end

		action.Action.Value = ""
							
		
		table.insert(eventHandler.Actions, action)

		table.insert(counter.Behavior.Actions, eventHandler)
	
	end
	
	r2.requestInsertGhostNode(comp.InstanceId, "Components", -1, "", counter)

end



function component:translate(context)
	r2.Translator.translateAiGroup(self, context)
	
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	local validationNeeded = self.ValidationNeeded

	local giver = r2:getInstanceFromId(self.MissionGiver)
	if not giver then return end
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
	
	-- Start of state
	do
		local action = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 7)
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, action)
	end
	
	do
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- Success
		local rtAction4 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", 0) 
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,rtAction4  } )
		r2.Translator.translateAiGroupEvent("user_event_7", self, context, rtAction)
	end

	do	
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", 1)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- Success
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,  } )
		r2.Translator.translateAiGroupEvent("user_event_4", self, context, rtAction)
	end



	-- when player targets mission giver 
	do
		local action1 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,  -- giver has been spoken to
				r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 2), 
				r2.Translator.createAction("npc_say", self:textAdapter(self.WaitValidationText),   rtGiverGrp.Id ..":"..giver.Name)}) )
		)
		
		local multiActions = r2.Translator.createAction("multi_actions", {
			r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ),
				r2.Translator.createAction("npc_say", self:textAdapter(self.MissionText),   rtGiverGrp.Id ..":"..giver.Name), 
				r2.Translator.createAction("talk_to", rtGrp.Id,  self:textAdapter(self.ContextualText)),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 1)})

		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0, multiActions))

		local rtAction = r2.Translator.createAction("multi_actions", {action1, action2})

		r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, rtAction)
	end
			
	if validationNeeded == 1 then
		do
			local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9)
			local actionSuccessTxt = r2.Translator.createAction("npc_say",  self:textAdapter(self.MissionSucceedText),  rtGiverGrp.Id ..":"..giver.Name)
			
			local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2, 
				r2.Translator.createAction("condition_if", r2:getNamespace()..rtGrp.Id..".Active == 1", 
				r2.Translator.createAction("multi_actions", {actionEvent, actionSuccessTxt})))
			
			r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, action)
		end
		
		do 
			local baseAct = r2.Scenario:getBaseAct()
			local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
			local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, self:textAdapter(self.BroadcastText) )
			
			r2.Translator.translateAiGroupEvent("user_event_8", self, context, actionBroadcast)
		end
	end
	
	do
		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  --if Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				})
			); 
		local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- if not Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			); 
		
		local actions = {}
		if validationNeeded == 1 then
			actions = {action2, action3}
		else
			local baseAct = r2.Scenario:getBaseAct()
			local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
			local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, self:textAdapter(self.BroadcastText) )
		
			actions = {action2, action3, actionBroadcast}
		end
		local multiActions = r2.Translator.createAction("multi_actions", actions)
				
		local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2, 
			r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, multiActions))
		r2.Translator.translateAiGroupEvent("user_event_9", self, context, action)
	end
	
	--deactivate	
	do
		local rtAction = r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				})
		r2.Translator.translateAiGroupEvent("user_event_5", self, context, rtAction)
	end
	r2.Translator.translateFeatureActivation(self, context)
end


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtGrp)
	local rtGrpCounter = r2.Translator.getRtGroup(context, component._CounterId)
	assert(rtGrpCounter)

	if action.Action.Type == "validateTask" then	
		local action = r2.Translator.createAction("validate_task", rtGrp.Id)
		return action, action
	elseif action.Action.Type == "succeed" then
		local action = r2.Translator.createAction("complete_mission", rtGrp.Id)
		return action, action
	elseif action.Action.Type == "succeedStep" then
		local actionDec = r2.Translator.createAction("counter_dec", rtGrpCounter.Id)
		local actionIf = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1, 
			r2.Translator.createAction("condition_if", r2:getNamespace()..rtGrp.Id..".Active == 1",actionDec))
		return actionIf, actionIf
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

	local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
	assert(rtNpcGrp)

	local eventType = tostring(event.Event.Type)
	
	local eventHandler, lastCondition = nil, nil

	if eventType == "mission asked" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 1)
	elseif eventType == "wait validation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 2)
	elseif eventType == "succeeded" then 
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 9)

	end
	
	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdHuntTask_ContextualText"):toUtf8()
	local missionText = i18n.get("uiR2EdHuntTask_MissionText"):toUtf8()
	local waitValidationText = i18n.get("uiR2EdHuntTask_WaitValidationText"):toUtf8()
	local missionSucceededText = i18n.get("uiR2EdHuntTask_MissionSucceededText"):toUtf8()
	local broadcastText = i18n.get("uiR2EdHuntTask_BroadcastText"):toUtf8()


	local comp = r2.newComponent("HuntTask")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdHuntTask")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.WaitValidationText = waitValidationText
	comp.MissionSucceedText = missionSucceededText
	comp.BroadcastText= broadcastText
	comp.TargetNumber = 1

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

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
			r2.setDisplayInfo("HuntTask", 0)
		else r2.setDisplayInfo("HuntTask", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.HuntTask.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'HuntTask' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'HuntTask' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("HuntTask") == 1 then 
			r2.displayFeatureHelp("HuntTask")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewHuntTaskFeatureAction"))
		local component = feature.Components.HuntTask.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'HuntTask' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureHuntTask")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdHuntTask")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "HuntTask")
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
				["succeedStep"]			= { menu=i18n.get( "uiR2AA0SucceedStep"				):toUtf8(),
											text=i18n.get( "uiR2AA1SucceedStep"				):toUtf8()}
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
				["is succeeded"]		= { menu=i18n.get( "uiR2Test0TaskSuccess"			):toUtf8(), 
											text=i18n.get( "uiR2Test1TaskSuccess"			):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["HuntTask"] =  feature





