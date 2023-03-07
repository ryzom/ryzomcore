
-- In Translation file
-- Category : uiR2EdTalkTo --
-- CreationFrom : uiR2EdTalkToParameters


r2.Features.TalkToFeature = {}

local feature = r2.Features.TalkToFeature

feature.Name="TalkToFeature"

feature.Description="A feature that makes a NPC request the player to talk to an other player"

feature.Components = {}

feature.Components.TalkTo =
	{
		BaseClass="LogicEntity",			
		Name="TalkTo",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "talkToDisplayer",


		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

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
			{Name="MissionTarget", Type="RefId", PickFunction="r2:canPickTalkingNpc", SetRefIdFunction="r2:setTalkingNpc"},

			--{Name="ContextualText", Type="String", ValidationFun="r2.refuseEmptyString", Category="uiR2EDRollout_TextToSay" },
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100" },
			{Name="MissionText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="WaitValidationText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="MissionSucceedText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="BroadcastText", Type="String", Category="uiR2EDRollout_TextToSay", DefaultValue="", DefaultInBase = 1},

			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Repeatable", Type="Number", WidgetStyle="Boolean", DefaultValue="0"}
			
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



local component = feature.Components.TalkTo

function component.pretranslate(this, context)
	local prop = component.Prop
	r2.Translator.CheckPickedEntity(this, prop)
	r2.Translator.createAiGroup(this, context)
end



local talkToDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = talkToDisplayerTable.onAttrModified

function talkToDisplayerTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	local targetRefId = propertySheet:find("MissionTarget")
	local targetName = targetRefId:find("name")
	
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
	
	if attributeName == "MissionTarget" then
		local instanceId = instance[attributeName]
		if instanceId == "" then
			targetName.hardtext = "NONE"
			return
		end

		if instance["MissionGiver"] == instance[attributeName] then
			targetName.hardtext = "NONE"
			r2.requestSetNode(instance.InstanceId, "MissionTarget", "") 
		else
			targetName.hardtext = r2:getInstanceFromId(instance[attributeName]).Name
		end
		return
	end

	return 
end

function talkToDisplayerTable:onSelect(instance, isSelected)
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

function r2:talkToDisplayer()	
	return talkToDisplayerTable  -- returned shared displayer to avoid wasting memory
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
	
	if tostring(self.MissionTarget) ~= "" then
		local target = r2:getInstanceFromId(self.MissionTarget)
		if target then mission_target = target.Name end
	end

	str=string.gsub(str, "<mission_giver>", mission_giver)
	str=string.gsub(str, "<mission_target>", mission_target)
	return str
end


function component:translate(context)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	r2.Translator.translateAiGroup(self, context)

	local giver = r2:getInstanceFromId(self.MissionGiver)



	local target = r2:getInstanceFromId(self.MissionTarget)

	local rtTargetGrp = nil
	if target then
		rtTargetGrp = r2.Translator.getRtGroup(context, target.InstanceId)
	end
	
	-----------------
	--Contextual text
	do	
		if target then
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
				 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v3", 1,  -- giver has been spoken to
					r2.Translator.createAction("talk_to", rtGrp.Id,  self:textAdapter(self.ContextualText)) )
			)
			r2.Translator.translateAiGroupEvent("player_target_npc", target, context, rtAction)
		end
	end
	
	-------------------
	--Mission text
	if giver then
		local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
		--local rtAction2 = r2.Translator.createAction("condition_if", r2:getNamespace()..rtGrp.Id..".Active == 1")
		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("npc_say", self:textAdapter(self.MissionText),   rtGiverGrp.Id ..":"..giver.Name), 
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 2) })
					)

		r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, rtAction)
	end
	
	--if there's no giver, target should speak anyway
	local v3
	if giver == nil then v3 = 1 else v3 = 0 end
	-------------------
	--Start of state
	
	
	do
		local action = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 7)
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, action)
	end
	
	do

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- Success
		local rtAction4 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", v3 ) -- Has
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, rtAction4,  } )
		--r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
		r2.Translator.translateAiGroupEvent("user_event_7", self, context, rtAction)
	end
	
	do	
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", 1)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- Success
		local rtAction4 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", v3 ) -- Has
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, rtAction4,  } )
		r2.Translator.translateAiGroupEvent("user_event_4", self, context, rtAction)
	end

	do	
		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3",  v3 )
				})
			); 
		local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3",  v3 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			); 
		
		local rtAction
		local baseAct = r2.Scenario:getBaseAct()
		local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, self:textAdapter(self.BroadcastText) )

		if rtTargetGrp then
			local action1 = r2.Translator.createAction("npc_say",  self:textAdapter(self.MissionSucceedText),  rtTargetGrp.Id ..":"..target.Name); 
			rtAction = r2.Translator.createAction("multi_actions", {action1, action2, action3, actionBroadcast})
		else
			rtAction = r2.Translator.createAction("multi_actions", {action2, action3, actionBroadcast})
		end

		r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtAction)
	end

	
	
	

	do
		if rtTargetGrp then
			local rtAction = r2.Translator.createAction("npc_say",  self:textAdapter(self.WaitValidationText),  rtTargetGrp.Id ..":"..target.Name); 
			r2.Translator.translateAiGroupEvent("user_event_1", self, context, rtAction)
		end
	end
	r2.Translator.translateFeatureActivation(self, context)
	

-- ()receiveMissionItems("system_mp.sitem:2;system_mp_choice.sitem:1;system_mp_supreme.sitem:3", "Give some stuff h�!", @groupToNotify);

	


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

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	elseif condition.Condition.Type == "is succeeded" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v2", 1);
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
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 2)
	elseif eventType == "wait validation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 1)
	elseif eventType == "succeeded" then 
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 3)

	end
	
	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdTalkTo_ContextualText")
	local missionText = i18n.get("uiR2EdTalkTo_MissionText")
	local waitValidationText = i18n.get("uiR2EdTalkTo_WaitValidationText")
	local missionSucceededText = i18n.get("uiR2EdTalkTo_MissionSucceededText")
	local broadcastText = i18n.get("uiR2EdTalkTo_BroadcastText")

	local comp = r2.newComponent("TalkTo")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdTalkTo"))			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.WaitValidationText = waitValidationText
	comp.MissionSucceedText = missionSucceededText
	comp.BroadcastText = broadcastText

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
--	comp.ItemQty = 1

	comp._Seed = os.time() 

	return comp
end

component.create = function()	

	r2:checkAiQuota()


	local function paramsOk(resultTable)

			
		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		
		if showAgain == 1 then 
			r2.setDisplayInfo("TalkToForm", 0)
		else r2.setDisplayInfo("TalkToForm", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.TalkTo.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'TalkTo' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'TalkTo' at pos (%f, %f, %f)", x, y, z))
		if r2.mustDisplayInfo("TalkTo") == 1 then 
			r2.displayFeatureHelp("TalkTo")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewTalkToFeatureAction"))
		local component = feature.Components.TalkTo.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'TalkTo' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureTalkTo")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdTalkTo")
	logicEntityMenu:addLine(name, "lua", "", "TalkTo")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]			= { menu=i18n.get( "uiR2AA0Activate"				), 
										text=i18n.get( "uiR2AA1Activate"				)}, 
			["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"				), 
										text=i18n.get( "uiR2AA1Deactivate"				)}, 
		},
		["Events"] = {	
			["activation"]			= { menu=i18n.get( "uiR2Event0Activation"			), 
										text=i18n.get( "uiR2Event1Activation"			)},
			["deactivation"]		= { menu=i18n.get( "uiR2Event0Deactivation"			), 
										text=i18n.get( "uiR2Event1Deactivation"			)},
			["mission asked"]		= { menu=i18n.get( "uiR2Event0MissionGiven"			), 
										text=i18n.get( "uiR2Event1MissionGiven"			)},
			["wait validation"]		= { menu=i18n.get( "uiR2Event0TaskWaitValidation"	), 
										text=i18n.get( "uiR2Event1TaskWaitValidation"	)},
			["succeeded"]			= { menu=i18n.get( "uiR2Event0TaskSuccess"			),
										text=i18n.get( "uiR2Event1TaskSuccess"			)},
		},
		["Conditions"] = {	
			["is active"]			= { menu=i18n.get( "uiR2Test0Active"				), 
										text=i18n.get( "uiR2Test1Active"				)},
			["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"				), 
										text=i18n.get( "uiR2Test1Inactive"				)},
			["is succeeded"]		= { menu=i18n.get( "uiR2Test0TaskSuccess"				), 
										text=i18n.get( "uiR2Test1TaskSuccess"				)},
		}
	}
	return logicTranslations
end


r2.Features["TalkTo"] =  feature





