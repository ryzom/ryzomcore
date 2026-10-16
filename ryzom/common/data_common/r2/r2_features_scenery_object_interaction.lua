
r2.Features.SceneryObjectInteractionFeature = {}

local feature = r2.Features.SceneryObjectInteractionFeature

feature.Name="SceneryObjectInteractionFeature"

feature.Description=""

feature.Components = {}

feature.Components.SceneryObjectInteraction =
	{
		BaseClass="LogicEntity",			
		Name="SceneryObjectInteraction",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "sceneryObjectInteractionDisplayer",


		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {"activation", "deactivation", "trigger"},

		Conditions = { "is active", "is inactive", "has triggered", "has not triggered" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="SceneryObject", Type="RefId", PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget"},

			--{Name="ContextualText", Type="String", ValidationFun="r2.refuseEmptyString", Category="uiR2EDRollout_TextToSay" },
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100" },
			
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



local component = feature.Components.SceneryObjectInteraction

function component.pretranslate(this, context)
	local prop = component.Prop
	r2.Translator.CheckPickedEntity(this, prop)
	r2.Translator.createAiGroup(this, context)
end



local sceneryObjectInteractionDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = sceneryObjectInteractionDisplayerTable.onAttrModified

function sceneryObjectInteractionDisplayerTable:onAttrModified(instance, attributeName)
	oldOnAttrModified(instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	local scObjRefId = propertySheet:find("SceneryObject")
	local scObjName = scObjRefId:find("name")
	
	

	if attributeName == "SceneryObject" then

		local instanceId = instance[attributeName]
		if instanceId == "" then
			scObjName.hardtext = "NONE"
			return
		end
		scObjName.hardtext = r2:getInstanceFromId(instance[attributeName]).Name
		return		
	end
	
	return 
end

function sceneryObjectInteractionDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)
	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
end

function component:onTargetInstancePostHrcMove(targetAttr, targetIndexInArray)
	debugInfo("postHrcMove!")
	local targetId = self[targetAttr]

	local tmpInstance = r2:getInstanceFromId(targetId)
	
	assert(tmpInstance)
	if tmpInstance.User.SelfModified and tmpInstance.User.SelfModified == true then
		if tmpInstance.ParentInstance and tmpInstance.ParentInstance:isKindOf("NpcGrpFeature") then
			r2.requestSetNode(self.InstanceId, targetAttr, r2.RefId(""))
		end
	end
	
end

function r2:sceneryObjectInteractionDisplayer()	
	return sceneryObjectInteractionDisplayerTable  -- returned shared displayer to avoid wasting memory
end


function component:textAdapter(text)

	assert(self)
	assert(type(text) == "string")
	local str =  text
	local object = ""


	if tostring(self.SceneryObject) ~= "" then
		local scObject = r2:getInstanceFromId(self.SceneryObject)
		if scObject then 	object = scObject.Name end
	end
	

	str=string.gsub(str, "<object_name>", object)
	return str
end

function component:pretranslate(context)
	if context.InteractingSceneryObjects then
		if self.SceneryObject and self.SceneryObject ~= "" then
			local scObj = r2:getInstanceFromId(self.SceneryObject)
			--assert(scObj)
			table.insert(context.InteractingSceneryObjects, scObj.InstanceId)
		end
	end
	--inspect(context.InteractingSceneryObjects)
	r2.Translator.createAiGroup(self, context)
end

function component:translate(context)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)

	r2.Translator.translateAiGroup(self, context)

	local sceneryObject = r2:getInstanceFromId(self.SceneryObject)
	
	-----------------
	--Contextual text
	do	
		if sceneryObject then
			local actionContextual = r2.Translator.createAction("talk_to", rtGrp.Id, self:textAdapter(tostring(self.ContextualText)))
			local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
				 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0, actionContextual))
			r2.Translator.translateAiGroupEvent("player_target_npc", sceneryObject, context, rtAction)
		end
	end
	
	r2.Translator.Tasks.setStatusLogic(self, context, rtGrp)
	-------------------
	--Trigger
	if sceneryObject then
		local rtSceneryObjectGrp = r2.Translator.getRtGroup(context, sceneryObject.InstanceId)

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, --if active
		  r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1, --and if the scenery object has been selected once by the player
			r2.Translator.createAction("multi_actions", { 
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 6) })
					))

		--r2.Translator.translateAiGroupEvent("player_target_npc", sceneryObject, context, rtAction)
		r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtAction)
	end
	
	-------------------
	--Start of state
	
	do

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- the scenery object hasn't been selected yet
		local rtAction4 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", 0)
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, rtAction4} )
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
	end


	do	
		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				})
			); 
		local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- Not Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			); 
				
		local rtAction = r2.Translator.createAction("multi_actions", {action2, action3})
		--Autodeactivate after having triggered
		r2.Translator.translateAiGroupEvent("user_event_6", self, context, rtAction)
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
	elseif condition.Condition.Type == "has triggered" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v3", 1 );
		return action1, action1
	elseif condition.Condition.Type == "has not triggered" then
		local action1 = r2.Translator.createAction("dynamic_if", prefix.."v3 == 0");
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
	
	if event.Event.Type == "trigger" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	end

	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local contextualText = i18n.get("uiR2EdSceneryObjectInteraction_ContextualText"):toUtf8()

	local comp = r2.newComponent("SceneryObjectInteraction")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdSceneryObjectInteraction")):toUtf8()			
	
	comp.ContextualText = contextualText

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
			r2.setDisplayInfo("SceneryObjectInteraction", 0)
		else r2.setDisplayInfo("SceneryObjectInteraction", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.SceneryObjectInteraction.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'SceneryObjectInteraction' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'SceneryObjectInteraction' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("SceneryObjectInteraction") == 1 then 
			r2.displayFeatureHelp("SceneryObjectInteraction")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewSceneryObjectInteractionFeatureAction"))
		local component = feature.Components.SceneryObjectInteraction.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'SceneryObjectInteraction' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureSceneryObjectInteraction")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdSceneryObjectInteraction")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "SceneryObjectInteraction")
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
			["has not triggered"]		= { menu=i18n.get( "uiR2Test0NotTrigger"				):toUtf8(), 
										text=i18n.get( "uiR2Test1NotTrigger"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["SceneryObjectInteractionFeature"] =  feature





