
r2.Features.KillNpc = {}

local feature = r2.Features.KillNpc

feature.Name="KillNpc"

feature.Description=""

feature.Components = {}

feature.Components.KillNpc =
	{
		BaseClass="LogicEntity",			
		Name="KillNpc",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "killNpcDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {"activation", "deactivation", "wait validation", "mission asked", "succeeded"},

		Conditions = { "is active", "is inactive", "is completed" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="MissionGiver", Type="RefId", PickFunction="r2:canPickTalkingNpc", SetRefIdFunction="r2:setTalkingNpc"},
			{Name="MissionTarget", Type="RefId", PickFunction="r2:canPickNotGroupedNpc", SetRefIdFunction="r2:setNotGroupedNpcRefIdTarget"},
			{Name="ValidationNeeded", Category="uiR2EDRollout_TextToSay", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100" },
			{Name="MissionText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="WaitValidationText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="MissionSucceedText", Type="String", Category="uiR2EDRollout_TextToSay", Visible= function(this) 
						return this:IsValidationNeeded() end },
			{Name="BroadcastText", Type="String", Category="uiR2EDRollout_TextToSay", DefaultValue="", DefaultInBase = 1},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			
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


local component = feature.Components.KillNpc

function component:IsValidationNeeded()
	local validationNeeded = self.ValidationNeeded
	if validationNeeded == 1 then
		return true
	end
	return false
end

local killNpcDisplayerTable = clone(r2:propertySheetDisplayer())
local oldOnAttrModified = killNpcDisplayerTable.onAttrModified

function killNpcDisplayerTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(self, instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	if attributeName == "ValidationNeeded" then
		propertySheet.Env.updatePropVisibility()
		return
	end

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



	r2:propertySheetDisplayer():onAttrModified(instance, attributeName)
	return
end


function killNpcDisplayerTable:onSelect(instance, isSelected)
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

function r2:killNpcDisplayer()	
	return killNpcDisplayerTable  -- returned shared displayer to avoid wasting memory
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

-- EVENTS-----------------------------------------------------------------------
-- 4 : activation
-- 5 : deactivation
-- 3 : mission given
-- 2 : wait validation
-- 9 : mission completed
---------------------------------------------------------------------------------
function component:translate(context)
	
	r2.Translator.translateAiGroup(self, context)

	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	local validationNeeded = tonumber(self.ValidationNeeded)

	local giver = r2:getInstanceFromId(self.MissionGiver)
	if not giver then return end
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)

	local target = r2:getInstanceFromId(self.MissionTarget)
	if not target then return end
	local rtTargetGrp = r2.Translator.getRtGroup(context, target.InstanceId)

	if tostring(self.MissionTarget) == ""  or self.MissionTarget == nil then return end

	-- Start of state	
	r2.Translator.Tasks.startOfStateLogic(self, context, rtGrp)

	-- Activation
	r2.Translator.Tasks.activationLogic(self, context, rtGrp)

	--Deactivation
	r2.Translator.Tasks.deactivationLogic(self, context, rtGrp)

	
	-- when player targets mission giver 
	r2.Translator.Tasks.giverLogic(self, giver, context, rtGrp)
	
	--Set mission status to 1 when mission is taken
	r2.Translator.Tasks.setStatusLogic(self, context, rtGrp)

	-- Qd player target le targetNpc :	si validation sur giver = event 2 (waitValidation)
	--									si validation sur target = event 3 (success)
	do
		local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9)
	
		local actionSet = r2.Translator.createAction("set_value", rtGrp.Id,  "v2", 2)
		
		local baseAct = r2.Scenario:getBaseAct()
		local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, self:textAdapter(self.BroadcastText) )
		
		local actions = {actionSet, actionBroadcast}
		if validationNeeded == 0 then
			table.insert(actions, actionEvent)
		end

		local multiactions = r2.Translator.createAction("multi_actions", actions)

		
		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, 
			 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1, multiactions))
		r2.Translator.translateAiGroupEvent("group_eliminated", target, context, rtAction)
	end
	
	r2.Translator.Tasks.validationByMissionGiver(self, giver, context, rtGrp)

	r2.Translator.Tasks.successNoBroadcastLogic(self, context, rtGrp)


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

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	elseif condition.Condition.Type == "is completed" then
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
	
	local contextualText = i18n.get("uiR2EdKillNpc_ContextualText"):toUtf8()
	local missionText = i18n.get("uiR2EdKillNpc_MissionText"):toUtf8()
	local waitValidationText = i18n.get("uiR2EdKillNpc_WaitValidationText"):toUtf8()
	local missionSucceededText = i18n.get("uiR2EdKillNpc_MissionSucceededText"):toUtf8()
	local broadcastText = i18n.get("uiR2EdKillNpc_BroadcastText"):toUtf8()

	local comp = r2.newComponent("KillNpc")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdKillNpc")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.WaitValidationText = waitValidationText
	comp.MissionSucceedText = missionSucceededText
	comp.BroadcastText= broadcastText

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
--	comp.ItemQty = 1

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
			r2.setDisplayInfo("KillNpc", 0)
		else r2.setDisplayInfo("KillNpc", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.KillNpc.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'KillNpc' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'KillNpc' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("KillNpc") == 1 then 
			r2.displayFeatureHelp("KillNpc")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewKillNpcFeatureAction"))
		local component = feature.Components.KillNpc.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'KillNpc' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureKillNpc")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdKillNpc")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "KillNpc")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
				["activate"]			= { menu=i18n.get( "uiR2AA0Activate"				):toUtf8(),
											text=i18n.get( "uiR2AA1Activate"				):toUtf8()},
				["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"				):toUtf8(),
											text=i18n.get( "uiR2AA1Deactivate"				):toUtf8()}
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
				["is completed"]		= { menu=i18n.get( "uiR2Test0TaskSuccess"				):toUtf8(), 
											text=i18n.get( "uiR2Test1TaskSuccess"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["KillNpc"] =  feature





