
r2.Features.SceneryObjectInteractionTaskStepTaskStep = {}

local feature = r2.Features.SceneryObjectInteractionTaskStepTaskStep

feature.Name="SceneryObjectInteractionTaskStepTaskStep"

feature.Description=""

feature.Components = {}

feature.Components.SceneryObjectInteractionTaskStep =
	{
		BaseClass="LogicEntity",			
		Name="SceneryObjectInteractionTaskStep",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "sceneryObjectInteractionTaskStepDisplayer",


		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {"activation", "deactivation", "succeeded"},

		Conditions = { "is active", "is inactive", "in progress", "is succeeded" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Repeatable", Type="Number", WidgetStyle="Boolean", DefaultValue="0"},
			{Name="ValidationNeeded", Category="uiR2EDRollout_TextToSay", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="SceneryObject", Type="RefId", PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget"},
			{Name="MissionGiver", Type="RefId", PickFunction="r2:canPickTalkingNpc", SetRefIdFunction="r2:setTalkingNpc"},

			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="MissionText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="NotValidatedText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="MissionSucceedText", Type="String", Category="uiR2EDRollout_TextToSay", Visible= function(this) 
						return this:IsValidationNeeded() end, DefaultInBase = 1},
			{Name="BroadcastText", Type="String", Category="uiR2EDRollout_TextToSay", DefaultValue="", DefaultInBase = 1},			
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
	}



local component = feature.Components.SceneryObjectInteractionTaskStep

function component:IsValidationNeeded()
	local validationNeeded = self.ValidationNeeded
	if validationNeeded == 1 then
		return true
	end
	return false
end

function component.pretranslate(this, context)
	local prop = component.Prop
	r2.Translator.CheckPickedEntity(this, prop)
	r2.Translator.createAiGroup(this, context)
end



local sceneryObjectInteractionTaskStepDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = sceneryObjectInteractionTaskStepDisplayerTable.onAttrModified

function sceneryObjectInteractionTaskStepDisplayerTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	local scObjRefId = propertySheet:find("SceneryObject")
	local scObjName = scObjRefId:find("name")
	
	if attributeName == "ValidationNeeded" then
		local propertySheet = r2:getPropertySheet(instance)
		propertySheet.Env.updatePropVisibility()
		return
	end

	if attributeName == "SceneryObject" then

		local instanceId = instance[attributeName]
		if instanceId == "" then
			scObjName.hardtext = "NONE"
			return
		end
		scObjName.hardtext = r2:getInstanceFromId(instance[attributeName]).Name
		return		
	end
	
	local giverRefId = propertySheet:find("MissionGiver")
	local giverName = propertySheet:find("MissionGiver"):find("name")

	if attributeName == "MissionGiver" then

		local instanceId = instance[attributeName]
		if instanceId == "" then
			giverName.hardtext = "NONE"
			return
		end

		if instance["MissionTarget"] == instance[attributeName] then
			giverName.hardtext = "NONE"
			r2.requestSetNode(instance.InstanceId, "MissionGiver", "") 
		else
			giverName.hardtext = r2:getInstanceFromId(instance[attributeName]).Name
		end
		return		
	end

	return 
end

function sceneryObjectInteractionTaskStepDisplayerTable:onSelect(instance, isSelected)
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

function r2:sceneryObjectInteractionTaskStepDisplayer()	
	return sceneryObjectInteractionTaskStepDisplayerTable  -- returned shared displayer to avoid wasting memory
end


function component:textAdapter(text)

	assert(self)
	assert(type(text) == "string")
	local str =  text
	local object = ""
	local mission_giver = ""

	if tostring(self.SceneryObject) ~= "" then
		local scObject = r2:getInstanceFromId(self.SceneryObject)
		if scObject then 	object = scObject.Name end
	end

	if tostring(self.MissionGiver) ~= "" then
		local giver = r2:getInstanceFromId(self.MissionGiver)
		if giver then 	mission_giver = giver.Name end
	end

	str=string.gsub(str, "<object_name>", object)
	str=string.gsub(str, "<mission_giver>", mission_giver)

	return str
end

function component:pretranslate(context)
	if context.InteractingSceneryObjects then
		if self.SceneryObject and self.SceneryObject ~= "" then
			local scObj = r2:getInstanceFromId(self.SceneryObject)
			if scObj then
				table.insert(context.InteractingSceneryObjects, scObj.InstanceId)
			end
		end
	end
	--inspect(context.InteractingSceneryObjects)
	r2.Translator.createAiGroup(self, context)
end

function component:translate(context)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	r2.Translator.translateAiGroup(self, context)
	
	if not self.SceneryObject or self.SceneryObject == "" then return end
	if not self.MissionGiver or self.MissionGiver == "" then return end

	local validationNeeded = self.ValidationNeeded

	local sceneryObject = r2:getInstanceFromId(self.SceneryObject)
	if not sceneryObject then return end
	
	local rtSceneryObjectGrp = r2.Translator.getRtGroup(context, sceneryObject.InstanceId)
	
	local giver = r2:getInstanceFromId(self.MissionGiver)
	if not giver then return end
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)

	local baseAct = r2.Scenario:getBaseAct()
	local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)

	-------------------
	--Start of state
	do
		local action = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 7)
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, action)
	end

	do

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- the scenery object hasn't been selected yet
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, } )
		r2.Translator.translateAiGroupEvent("user_event_7", self, context, rtAction)
	end


	--Activation	
	do	
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", 1)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- the scenery object hasn't been selected yet
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, } )
		r2.Translator.translateAiGroupEvent("user_event_4", self, context, rtAction)
	end


	-------------------
	--Mission text
	if giver then	
		local actionValidation = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1, --giver already said mission text
				r2.Translator.createAction("npc_say", self:textAdapter(self.NotValidatedText),   rtGiverGrp.Id ..":"..giver.Name))
							
		local actionMission = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0, --giver didn't say mission text yet
				r2.Translator.createAction("multi_actions", {
					r2.Translator.createAction("npc_say", self:textAdapter(self.MissionText),   rtGiverGrp.Id ..":"..giver.Name), 
					r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ), }))
		
		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,
			r2.Translator.createAction("multi_actions", {actionValidation, actionMission}))


		r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, rtAction)
	end




	--Contextual text
	do	
		if sceneryObject then
			local actionContextual = r2.Translator.createAction("talk_to", rtGrp.Id, self:textAdapter(tostring(self.ContextualText)))
			
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
				 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,  actionContextual))
			r2.Translator.translateAiGroupEvent("player_target_npc", sceneryObject, context, rtAction)
		end
	end

	--when the player did interact with the scenery obj : trigger
	if sceneryObject then
		local eventId = 9	
		if validationNeeded == 1 then
			eventId = 8
		end
		local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, eventId)

		local actionSet = r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1)

		local actionSetStatus = r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2)

		local actions = r2.Translator.createAction("multi_actions", {actionSetStatus, actionEvent, actionSet})

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, --if active
		  r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1, actions))--and if the scenery object has been selected once by the player

		r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtAction)
	end
	

	--depending on validationNeeded prop 		
	if validationNeeded == 1 then
		do
			local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9)
			local actionSaySuccessText = r2.Translator.createAction("npc_say",  self:textAdapter(self.MissionSucceedText),  rtGiverGrp.Id ..":"..giver.Name); 
				
			local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2, 
				r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, 
				r2.Translator.createAction("multi_actions", {actionEvent, actionSaySuccessText})))
			
			r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, action)
		end

		do
			local actionBroadcast = r2.Translator.createAction("broadcast_msg", baseActRtGrp.Id, self:textAdapter(self.BroadcastText))
			r2.Translator.translateAiGroupEvent("user_event_8", self, context, actionBroadcast)
		end
	end

	--user event 9 : success
	do	
		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				})
			); 
		local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- Not Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			); 
				
		local actions = {}
		if validationNeeded == 1 then
			actions = {action2, action3}
		else
			local actionBroadcast = r2.Translator.createAction("broadcast_msg", baseActRtGrp.Id, self:textAdapter(self.BroadcastText))
					
			actions = {action2, action3, actionBroadcast}
		end

		local rtAction = r2.Translator.createAction("multi_actions", actions)


		--Autodeactivate after having triggered?
		r2.Translator.translateAiGroupEvent("user_event_9", self, context, rtAction)
	end

	r2.Translator.translateFeatureActivation(self, context)
	
end


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	return r2.Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
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
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	elseif condition.Condition.Type == "in progress" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v2", 1 );
		return action1, action1
	elseif condition.Condition.Type == "is succeeded" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v3", 1 );
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
	
	if event.Event.Type == "succeeded" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 9)
	end

	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdSceneryObjectInteractionTaskStep_ContextualText"):toUtf8()
	local missionText = i18n.get("uiR2EdSceneryObjectInteractionTaskStep_MissionText"):toUtf8()
	local notValidatedText = i18n.get("uiR2EdSceneryObjectInteractionTaskStep_NotValidatedText"):toUtf8()
	local broadcastText = i18n.get("uiR2EdSceneryObjectInteractionTaskStep_BroadcastText"):toUtf8()
	local missionSucceedText = i18n.get("uiR2EdSceneryObjectInteractionTaskStep_MissionSucceedTextText"):toUtf8()


	local comp = r2.newComponent("SceneryObjectInteractionTaskStep")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdSceneryObjectInteractionTaskStep")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.NotValidatedText = notValidatedText
	comp.BroadcastText = broadcastText
	comp.MissionSucceedText = missionSucceedText

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
--	comp.ItemQty = 1

	comp._Seed = os.time() 

	return comp
end

component.create = function()	

	r2:checkAiQuota()

	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'SceneryObjectInteractionTaskStep' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("SceneryObjectInteractionTaskStep") == 1 then 
			r2.displayFeatureHelp("SceneryObjectInteractionTaskStep")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewSceneryObjectInteractionTaskStepAction"))
		local component = feature.Components.SceneryObjectInteractionTaskStep.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'SceneryObjectInteractionTaskStep' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureSceneryObjectInteractionTaskStep")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdSceneryObjectInteractionTaskStep")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "SceneryObjectInteractionTaskStep")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]			= { menu=i18n.get( "uiR2AA0Activate"				):toUtf8(), 
										text=i18n.get( "uiR2AA1Activate"				):toUtf8()}, 
			["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"				):toUtf8(), 
										text=i18n.get( "uiR2AA1Deactivate"				):toUtf8()}, 
		},
		["Events"] = {	
			["activation"]			= { menu=i18n.get( "uiR2Event0Activation"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Activation"			):toUtf8()},
			["deactivation"]		= { menu=i18n.get( "uiR2Event0Deactivation"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Deactivation"			):toUtf8()},
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
			["in progress"]			= { menu=i18n.get( "uiR2Test0InProgress"			):toUtf8(), 
										text=i18n.get( "uiR2Test1InProgress"			):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["SceneryObjectInteractionTaskStep"] =  feature





