

r2.Features.NpcInteraction = {}

local feature = r2.Features.NpcInteraction

feature.Name="NpcInteraction"

feature.Description=""

feature.Components = {}

feature.Components.NpcInteraction =
	{
		BaseClass="LogicEntity",			
		Name="NpcInteraction",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "npcInteractionDisplayer",


		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {"activation", "deactivation", "trigger"},

		Conditions = { "is active", "is inactive", "has triggered" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="InteractionTarget", Type="RefId", PickFunction="r2:canPickTalkingNpc", SetRefIdFunction="r2:setTalkingNpc"},

			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100" },
			{Name="PreInteractionText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="PostInteractionText", Type="String", Category="uiR2EDRollout_TextToSay"},

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



local component = feature.Components.NpcInteraction

function component.pretranslate(this, context)
	local prop = component.Prop
	r2.Translator.CheckPickedEntity(this, prop)
	r2.Translator.createAiGroup(this, context)
end



local npcInteractionDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = npcInteractionDisplayerTable.onAttrModified

function npcInteractionDisplayerTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	local targetRefId = propertySheet:find("InteractionTarget")
	local targetName = targetRefId:find("name")
		
	if attributeName == "InteractionTarget" then
		local instanceId = instance[attributeName]
		if instanceId == "" then
			targetName.hardtext = "NONE"
			return
		end

		targetName.hardtext = r2:getInstanceFromId(instance[attributeName]).Name
	end

	return 
end

function npcInteractionDisplayerTable:onSelect(instance, isSelected)
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

function r2:npcInteractionDisplayer()	
	return npcInteractionDisplayerTable  -- returned shared displayer to avoid wasting memory
end


function component:textAdapter(text)

	assert(self)
	assert(type(text) == "string")
	local str =  text
	local mission_giver = ""
	local mission_target = ""

	if tostring(self.InteractionTarget) ~= "" then
		local target = r2:getInstanceFromId(self.InteractionTarget)
		if target then mission_target = target.Name end
	end

	str=string.gsub(str, "<mission_target>", mission_target)
	return str
end


function component:translate(context)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	r2.Translator.translateAiGroup(self, context)

	local target = r2:getInstanceFromId(self.InteractionTarget)

	local rtTargetGrp = nil
	if target then
		rtTargetGrp = r2.Translator.getRtGroup(context, target.InstanceId)
	end
	
	-----------------
	--Contextual text
	do	
		if target then
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
					r2.Translator.createAction("talk_to", rtGrp.Id,  self:textAdapter(self.ContextualText)))
			r2.Translator.translateAiGroupEvent("player_target_npc", target, context, rtAction)
		end
	end
	
	-------------------
	--Start of state
	
	do

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value", rtGrp.Id, "v3", 0)
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,})
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
	end


	do	
		local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  -- Repeatable
			r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 )) 
		
		local action4 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			); 
		local action5 = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 6)
		local action1 = r2.Translator.createAction("npc_say",  self:textAdapter(self.PostInteractionText),  rtTargetGrp.Id ..":"..target.Name); 
		local action2 = r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1)
		local rtAction = r2.Translator.createAction("multi_actions", {action1, action2, action3, action4, action5})

		r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtAction)
	end


	do
		if rtTargetGrp then
			local rtAction = r2.Translator.createAction("npc_say",  self:textAdapter(self.PreInteractionText),  rtTargetGrp.Id ..":"..target.Name); 
			r2.Translator.translateAiGroupEvent("user_event_1", self, context, rtAction)
		end
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

	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	elseif condition.Condition.Type == "has triggered" then
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

	if eventType == "trigger" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	else	
		return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
	end
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdNpcInteraction_ContextualText"):toUtf8()
	local preInteractionText = i18n.get("uiR2EdNpcInteraction_PreInteractionText"):toUtf8()
	local postInteractionText = i18n.get("uiR2EdNpcInteraction_PostInteractionText"):toUtf8()

	local comp = r2.newComponent("NpcInteraction")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdNpcInteraction")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.PreInteractionText = preInteractionText
	comp.PostInteractionText = postInteractionText
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

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
			r2.setDisplayInfo("NpcInteractionForm", 0)
		else r2.setDisplayInfo("NpcInteractionForm", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.NpcInteraction.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'NpcInteraction' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'NpcInteraction' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("NpcInteraction") == 1 then 
			r2.displayFeatureHelp("NpcInteraction")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewNpcInteractionFeatureAction"))
		local component = feature.Components.NpcInteraction.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'NpcInteraction' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureNpcInteraction")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdNpcInteraction")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "NpcInteraction")
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
			["trigger"]				= { menu=i18n.get( "uiR2Event0Trigger"			):toUtf8(), 
										text=i18n.get( "uiR2Event1Trigger"			):toUtf8()},

		},
		["Conditions"] = {	
			["is active"]			= { menu=i18n.get( "uiR2Test0Active"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Active"				):toUtf8()},
			["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Inactive"				):toUtf8()},
			["has triggered"]		= { menu=i18n.get( "uiR2Test0Trigger"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Trigger"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["NpcInteraction"] =  feature





