-- In Translation file
-- Category : uiR2EdSceneryObjectRemover --
-- CreationFrom : uiR2EdSceneryObjectRemoverParameters


r2.Features.SceneryObjectRemoverFeature = {}

local feature = r2.Features.SceneryObjectRemoverFeature

feature.Name="SceneryObjectRemoverFeature"

feature.Description="Removes scenery objects at runtime"

feature.Components = {}

feature.Components.SceneryObjectRemover =
	{
		BaseClass="LogicEntity",			
		Name="SceneryObjectRemover",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "sceneryObjectRemoverDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = {},

		ApplicableActions = { "activate", "deactivate", "remove objects"},

		Events = {"removed objects"},

		Conditions = { "is active", "is inactive", "has removed objects", "has not removed objects"},

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},
		-----------------------------------------------------------------------------------------------	
		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible= false},
			{Name="Components", Type="Table"},
			{Name= "Ghosts", Type = "Table", Visible = false },
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name= "Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="SceneryObjectNumber", Type="Number", Category="uiR2EDRollout_Scenery_Objects", WidgetStyle="EnumDropDown", 
			Enum={"1", "2", "3", "4", "5"}, DefaultValue="0"},
			{Name="SceneryObject1Id", Type="RefId", Category="uiR2EDRollout_Scenery_Objects",PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget", Translation="uiR2EDProp_SceneryObject1Id", Visible= function(this) return this:displayRefId(1) end},
			{Name="SceneryObject2Id", Type="RefId", Category="uiR2EDRollout_Scenery_Objects",PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget", Translation="uiR2EDProp_SceneryObject2Id", Visible= function(this) return this:displayRefId(2) end},
			{Name="SceneryObject3Id", Type="RefId", Category="uiR2EDRollout_Scenery_Objects",PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget", Translation="uiR2EDProp_SceneryObject3Id", Visible= function(this) return this:displayRefId(3) end},
			{Name="SceneryObject4Id", Type="RefId", Category="uiR2EDRollout_Scenery_Objects",PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget", Translation="uiR2EDProp_SceneryObject4Id", Visible= function(this) return this:displayRefId(4) end},
			{Name="SceneryObject5Id", Type="RefId", Category="uiR2EDRollout_Scenery_Objects",PickFunction="r2:canPickSceneryObject", SetRefIdFunction="r2:setSceneryObjectTarget", Translation="uiR2EDProp_SceneryObject5Id", Visible= function(this) return this:displayRefId(5) end},
		},
		
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,
		
		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
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

		translate = function(this, context)
			r2.Translator.translateAiGroup(this, context)
			--r2.Translator.translateFeatureActivation(this, context)
			local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
			assert(rtNpcGrp)
			if this.Active and this.Active == 1 then
				local action1 = r2.Translator.createAction("set_value",  rtNpcGrp.Id, "Active", 1)
				local action2 = r2.Translator.createAction("set_value",  rtNpcGrp.Id, "v1", 0)
				local action3 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 4)
				local retAction = r2.Translator.createAction("multi_actions", {action1, action2, action3})
				r2.Translator.translateAiGroupEvent("start_of_state" , this, context, retAction)
			else
				local action1 = r2.Translator.createAction("set_value",  rtNpcGrp.Id, "Active", 0)
				local action2 = r2.Translator.createAction("set_value",  rtNpcGrp.Id, "v1", 0)
				local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
				r2.Translator.translateAiGroupEvent("start_of_state" , this, context, retAction)	
			end
		end
	}

-------------------------------------------------------------------------------------------------------------------------


local component = feature.Components.SceneryObjectRemover  

function component:displayRefId(index)
	local nbScObj = self.SceneryObjectNumber + 1
	if index <= nbScObj then
		return true
	end
	return false
end

component.getLogicAction = function(entity, context, action)

	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)


	if (action.Action.Type == "remove objects") then	
		local actionTrigger = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)
		local actionSetValue = r2.Translator.createAction("set_value", rtNpcGrp.Id, "v1", 1)
		local despawnActions = {}
		local i = 1
		while i <= component.SceneryObjectNumber + 1 do
			local rtScenObj = r2.Translator.getRtGroup(context, component["SceneryObject"..i.."Id"])
			if rtScenObj then
				local actionDespawn = r2.Translator.createAction("despawn", rtScenObj.Id)
				table.insert(despawnActions, actionDespawn)
			end
			i = i + 1
		end

		local removedEvent = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 6)

		table.insert(despawnActions, actionTrigger)
		table.insert(despawnActions, actionSetValue)
		table.insert(despawnActions, removedEvent)
		
		local retAction = r2.Translator.createAction("condition_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
			r2.Translator.createAction("condition_if", r2:getNamespace()..rtNpcGrp.Id..".v1 == 0",
			r2.Translator.createAction("multi_actions", despawnActions))
		)
		return retAction, retAction
	end	

	return r2.Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
end

component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	if condition.Condition.Type == "has removed objects" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v1", 1);
		return action1, action1
	elseif condition.Condition.Type == "has not removed objects" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v1", 0);
		return action1, action1
	end
	return r2.Translator.getFeatureActivationCondition(condition, rtNpcGrp)
end

component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component = this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local eventType = tostring(event.Event.Type)

	if eventType == "removed objects" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	end

	return nil, nil

end

--------------------------------------------------------------------------------------------------------------------
local sceneryObjectRemoverDisplayerTable = clone(r2:propertySheetDisplayer())

--
-- If the message is received by a client that didn't request the modification, we must make sure this client 
-- doesn't modify the data because it has already been performed by the initial client. 
--
local function checkPickedEntity(this, instanceId, attributeName)
	
	if instanceId == "" then
		return false
	end
	local tmpInstance = r2:getInstanceFromId(instanceId)
	assert(tmpInstance)
	
	local i = 1
	while i < 6 do
		local attrName = "Npc" ..i.. "Id"
		if attrName ~= attributeName and this[attrName] == tmpInstance.InstanceId then
			--if not tmpInstance.User.SelfModified or tmpInstance.User.SelfModified == false then 
			--	messageBox("'"..tmpInstance.Name.."' has already been picked.") 
			--else
			--	tmpInstance.User.SelfModified = false
			--end
			return false
		end
		i = i + 1
	end

	return true
end



local oldOnAttrModified = sceneryObjectRemoverDisplayerTable.onAttrModified

function sceneryObjectRemoverDisplayerTable:onAttrModified(instance, attributeName)
	--if not instance.User.SelfModified then return end
	-- call base version
	oldOnAttrModified(self, instance, attributeName)
	
	if attributeName == "NpcNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbScObjs = instance.SceneryObjectNumber + 1
		local i = 1
		while i <= 5 do
			if i > nbScObjs then
				local name = "SceneryObject"..tostring(i).."Id"
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

	if string.find(attributeName, "Id") == nil or attributeName == "InstanceId" then return end 

	local propertySheet = r2:getPropertySheet(instance)
	local refId = propertySheet:find(attributeName)
	local refIdName = refId:find("name")
	
	local instanceId = instance[attributeName]
	if instanceId == "" then
		refIdName.hardtext = "NONE"
		return
	end
	
	if checkPickedEntity(instance, instanceId, attributeName) then
		local tmpInstance = r2:getInstanceFromId(instanceId)
		refIdName.hardtext = tmpInstance.Name
	else
		r2.requestSetNode(instance.InstanceId, attributeName, "")
	end
	
end	

function sceneryObjectRemoverDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end


local function reattributeIdOnHrcMove(scObjRemover, group, targetAttr)
	local propertySheet = r2:getPropertySheet(scObjRemover)
	local refId = propertySheet:find(targetAttr)
	local refIdName = refId:find("name")
	
	r2.requestSetNode(scObjRemover.InstanceId, targetAttr, group.InstanceId)
	refIdName.hardtext = group.Name

end


function component:onTargetInstancePostHrcMove(targetAttr, targetIndexInArray)

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




function r2:sceneryObjectRemoverDisplayer()	
	return sceneryObjectRemoverDisplayerTable  -- returned shared displayer to avoid wasting memory
end
--------------------------------------------------------------------------------------------------------------------


component.createGhostComponents= function(this, act)

end

component.createComponent = function(x, y, tvalue)
	
	local comp = r2.newComponent("SceneryObjectRemover")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDRollout_SceneryObjectRemover")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	comp._Seed = os.time() 

	comp.SceneryObjectId = r2.RefId("")
	comp.SceneryObject2Id = r2.RefId("")
	comp.SceneryObject3Id = r2.RefId("")
	comp.SceneryObject4Id = r2.RefId("")
	comp.SceneryObject5Id = r2.RefId("")
	comp.SceneryObjectNumber = 0
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
			r2.setDisplayInfo("SceneryObjectRemover", 0)
		else r2.setDisplayInfo("SceneryObjectRemover", 1) end
	
		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.SceneryObjectRemover.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'SceneryObjectRemoverFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'SceneryObjectRemoverFeature' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("SceneryObjectRemover") == 1 then 
			r2.displayFeatureHelp("SceneryObjectRemover")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewSceneryObjectRemoverFeatureAction"))
		local component = feature.Components.SceneryObjectRemover.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'SceneryObjectRemoverFeature' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureSceneryObjectRemover")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EDRollout_SceneryObjectRemover")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "SceneryObject")
end


function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]		= { menu=i18n.get( "uiR2AA0Activate"			):toUtf8(),
									text=i18n.get( "uiR2AA1Activate"			):toUtf8()},
			["deactivate"]		= { menu=i18n.get( "uiR2AA0Deactivate"			):toUtf8(),
									text=i18n.get( "uiR2AA1Deactivate"			):toUtf8()},
			["remove objects"]		= { menu=i18n.get( "uiR2AA0RemoveObject"			):toUtf8(),
									text=i18n.get( "uiR2AA1RemoveObject"			):toUtf8()},
		},
		["Events"] = {
			["removed objects"]				= { menu=i18n.get( "uiR2Event0RemovedObjects"			):toUtf8(), 
											text=i18n.get( "uiR2Event1RemovedObjects"			):toUtf8()},									
		},
		["Conditions"] = {
			["is active"]		= { menu=i18n.get( "uiR2Test0Active"		):toUtf8(),
									text=i18n.get( "uiR2Test1Active"		):toUtf8()},
			["is inactive"]		= { menu=i18n.get( "uiR2Test0Inactive"		):toUtf8(),
									text=i18n.get( "uiR2Test1Inactive"		):toUtf8()},
			["has removed objects"]		= { menu=i18n.get( "uiR2Test0HasRemoved"		):toUtf8(),
									text=i18n.get( "uiR2Test1HasRemoved"		):toUtf8()},
			["has not removed objects"]		= { menu=i18n.get( "uiR2Test0HasNotRemoved"		):toUtf8(),
									text=i18n.get( "uiR2Test1HasNotRemoved"		):toUtf8()}
		}
	}
	return logicTranslations
end


r2.Features["SceneryObjectRemoverFeature"] =  feature
