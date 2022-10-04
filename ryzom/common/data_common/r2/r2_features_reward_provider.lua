--------------------------------
-- Events for reward provider:
-- #4 : activation
-- #5 : deactivation
-- #6 : reward generated


r2.Features.RewardProviderFeature = {}

local feature = r2.Features.RewardProviderFeature

feature.Name="RewardProviderFeature"

feature.Description=""

feature.Components = {}


feature.Components.RewardProvider =
	{
		BaseClass="LogicEntity",			
		Name="RewardProvider",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "rewardProviderDisplayer",


		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {"activation", "deactivation", "reward given"},

		Conditions = { "is active", "is inactive"},

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="RewardGiver", Type="RefId", PickFunction="r2:canPickTalkingNpc", SetRefIdFunction="r2:setTalkingNpc"},

			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="OnTargetText", Type="String", Category="uiR2EDRollout_TextToSay" },
			{Name="RewardText", Type="String", Category="uiR2EDRollout_TextToSay" },
			{Name="RareRewardText", Type="String", Category="uiR2EDRollout_TextToSay" },
			{Name="InventoryFullText", Type="String", Category="uiR2EDRollout_TextToSay" },
			{Name="NotEnoughPointsText", Type="String", Category="uiR2EDRollout_TextToSay" },

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
				if component.appendInstancesByType then
					component:appendInstancesByType(destTable, kind)
				end
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



local component = feature.Components.RewardProvider

function component.pretranslate(this, context)
	local prop = component.Prop
	r2.Translator.CheckPickedEntity(this, prop)
	r2.Translator.createAiGroup(this, context)
end



local rewardProviderTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = rewardProviderTable.onAttrModified

function rewardProviderTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	local giverRefId = propertySheet:find("RewardGiver")
	local giverName = propertySheet:find("RewardGiver"):find("name")
	
	if attributeName == "RewardGiver" then
		local instanceId = instance[attributeName]
		if instanceId == "" then
			giverName.hardtext = "NONE"
			return
		end	
		local tmpInstance = r2:getInstanceFromId(instanceId)
		giverName.hardtext = tmpInstance.Name	
	end
	
	
	return 
end

function rewardProviderTable:onSelect(instance, isSelected)
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

function r2:rewardProviderDisplayer()	
	return rewardProviderTable  -- returned shared displayer to avoid wasting memory
end


function component:textAdapter(text)

	assert(self)
	assert(type(text) == "string")
	local str =  text
	local reward_giver = ""

	if tostring(self.RewardGiver) ~= "" then
		local giver = r2:getInstanceFromId(self.RewardGiver)
		if giver then 	reward_giver = giver.Name end
	end
	
	str=string.gsub(str, "<reward_giver>", reward_giver)
	return str
end


function component:getTextTable()
	local texts = {}
	texts["rewardText"]				= self:textAdapter(self.RewardText)
	texts["rareRewardText"]			= self:textAdapter(self.RareRewardText)
	texts["inventoryFullText"]		= self:textAdapter(self.InventoryFullText)
	texts["notEnoughPointsText"]	= self:textAdapter(self.NotEnoughPointsText)

	return texts
end


function component:translate(context)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	r2.Translator.translateAiGroup(self, context)

	local giver = r2:getInstanceFromId(self.RewardGiver)
	if not giver then return end
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
	
	-----------------
	--Contextual & onTarget text
	do	
		if giver then
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  
					r2.Translator.createAction("talk_to", rtGrp.Id,  self:textAdapter(self.ContextualText)))
			r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, rtAction)
		end
	end	

	do	
		if giver then
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, 
					r2.Translator.createAction("npc_say", self:textAdapter(self.OnTargetText),  rtGiverGrp.Id ..":"..giver.Name))
			r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, rtAction)
		end
	end	
	-------------------
	--give reward action
	do
		if giver then
			local texts = self:getTextTable()
			
			

			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,
						r2.Translator.createAction("multi_actions", {
							r2.Translator.createAction("give_reward", rtGiverGrp.Id, giver.Name, texts),
							r2.Translator.createAction("user_event_trigger", rtGrp.Id, 6) }))

			r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtAction)
		end
	end
	-------------------
	--Start of state	
	do
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0)
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2} )
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
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

	if eventType == "reward given" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	end
	
	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdRewardProvider_ContextualText"):toUtf8()
	local onTargetText = i18n.get("uiR2EdRewardProvider_OnTargetText"):toUtf8()
	local rewardText = i18n.get("uiR2EdRewardProvider_RewardText"):toUtf8()
	local rareRewardText = i18n.get("uiR2EdRewardProvider_RareRewardText"):toUtf8()
	local inventoryFullText = i18n.get("uiR2EdRewardProvider_InventoryFullText"):toUtf8()
	local notEnoughPointsText = i18n.get("uiR2EdRewardProvider_NotEnoughPointsText"):toUtf8()

	local comp = r2.newComponent("RewardProvider")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDRewardProvider")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.OnTargetText = onTargetText
	comp.RewardText = rewardText
	comp.RareRewardText = rareRewardText
	comp.InventoryFullText = inventoryFullText
	comp.NotEnoughPointsText = notEnoughPointsText

	comp.Repeatable = true
	

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
			r2.setDisplayInfo("RewardProvider", 0)
		else r2.setDisplayInfo("RewardProvider", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.RewardProvider.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'RewardProvider' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'RewardProvider' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("RewardProvider") == 1 then 
			r2.displayFeatureHelp("RewardProvider")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewRewardProviderFeatureAction"))
		local component = feature.Components.RewardProvider.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'RewardProvider' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureRewardProvider")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EDRollout_RewardProvider")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "RewardProvider")
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
			["reward given"]		= { menu=i18n.get( "uiR2Event0RewardGiven"			):toUtf8(), 
										text=i18n.get( "uiR2Event1RewardGiven"			):toUtf8()},
		},
		["Conditions"] = {	
			["is active"]			= { menu=i18n.get( "uiR2Test0Active"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Active"				):toUtf8()},
			["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"				):toUtf8(), 
										text=i18n.get( "uiR2Test1Inactive"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["RewardProvider"] =  feature





