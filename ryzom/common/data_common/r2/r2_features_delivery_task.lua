
r2.Features.DeliveryTask = {}

local feature = r2.Features.DeliveryTask

feature.Name="DeliveryTask"

feature.Description=""

feature.Components = {}

feature.Components.DeliveryTask =
	{
		BaseClass="LogicEntity",			
		Name="DeliveryTask",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "deliveryTaskDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = {  "activation", "deactivation", "mission asked", "succeeded", "delivered"},

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
			{Name="ItemNumber", Type="Number", Category="uiR2EDRollout_ItemsToDeliver", WidgetStyle="EnumDropDown", 
			Enum={"1", "2", "3"},	},
			{Name="Item1Qty", Type="Number", Category="uiR2EDRollout_ItemsToDeliver", Min="0", DefaultValue="1", Visible= function(this) return this:displayRefId(1) end},
			{Name="Item1Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToDeliver", Visible= function(this) return this:displayRefId(1) end},

			{Name="Item2Qty", Type="Number", Category="uiR2EDRollout_ItemsToDeliver", Min="0", DefaultValue="0", Visible= function(this) return this:displayRefId(2) end},
			{Name="Item2Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToDeliver", Visible= function(this) return this:displayRefId(2) end},

			{Name="Item3Qty", Type="Number", Category="uiR2EDRollout_ItemsToDeliver", Min="0", DefaultValue="0", Visible= function(this) return this:displayRefId(3) end},
			{Name="Item3Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToDeliver", Visible= function(this) return this:displayRefId(3) end},
			{Name="ValidationNeeded", Category="uiR2EDRollout_TextToSay", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100"},
			{Name="MissionText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="MissionSucceedText", Type="String", Category="uiR2EDRollout_TextToSay", Visible= function(this) 
						return this:IsValidationNeeded() end },
			{Name="BroadcastText", Type="String", Category="uiR2EDRollout_TextToSay", DefaultValue="", DefaultInBase = 1},
			{Name="WaitValidationText", Type="String", Category="uiR2EDRollout_TextToSay"},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue="1"},
			{Name="Repeatable", Type="Number", WidgetStyle="Boolean", DefaultValue="0"},
			
			
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


	}


local component = feature.Components.DeliveryTask

function component:IsValidationNeeded()
	local validationNeeded = self.ValidationNeeded
	if validationNeeded == 1 then
		return true
	end
	return false
end

function component:displayRefId(index)
	local nbItems = self.ItemNumber + 1
	if index <= nbItems then
		return true
	end
	return false
end

function component:getItems()
	local str =  ""
	
	local id = 1
	while id <= 3 do
		local item = self["Item"..tostring(id).."Id"]
		local item2 = self.Item1Id
		if (item) then
		
			local qt = tonumber(self["Item"..tostring(id).."Qty"])			

			local plotItem = r2:getInstanceFromId(item)
			if plotItem then
				local plotItemSheetId = plotItem.SheetId
				if str ~= "" then str =  str ..";" end
				local name = r2.getSheetIdName(plotItemSheetId)
				str = str .. tostring(name)..":"..qt
			end
		end
		id = id + 1
	end
	return str
end

function component:textAdapter(text)
	local str =  ""
	local items = {}
	local qts = {}

	local id = 1
	while id <= 3 do
		local item = self["Item"..tostring(id).."Id"]
		local item2 = self.Item1Id
		if (item) then
		
			local qt = tonumber(self["Item"..tostring(id).."Qty"])			
			qts[id] = qt

			local plotItem = r2:getInstanceFromId(item)
			if plotItem then
				items[id] = plotItem.Name
			else
				items[id] = ""
			end
		end
		id = id + 1
	end
	local str = text
	str=string.gsub (str, "<qt1>", tostring(qts[1]))
	str=string.gsub (str, "<qt2>", tostring(qts[2]))
	str=string.gsub (str, "<qt3>", tostring(qts[3]))
	str=string.gsub (str, "<item1>", tostring(items[1]))
	str=string.gsub (str, "<item2>", tostring(items[2]))
	str=string.gsub (str, "<item3>", tostring(items[3]))

	local mission_giver = ""
	if self.MissionGiver == nil then return end
	local npc = r2:getInstanceFromId(self.MissionGiver)
	if npc then
		mission_giver = npc.Name
	end
	str=string.gsub(str, "<mission_giver>", tostring(mission_giver))

	local mission_target = ""
	if self.MissionTarget == nil then return end
	local npc = r2:getInstanceFromId(self.MissionTarget)
	if npc then
		mission_target = npc.Name
	end
	str=string.gsub(str, "<mission_target>", tostring(mission_target))

	return str
end

local deliveryTaskDisplayerTable = clone(r2:propertySheetDisplayer())

local oldOnAttrModified = deliveryTaskDisplayerTable.onAttrModified
function deliveryTaskDisplayerTable:onAttrModified(instance, attributeName)
	
	oldOnAttrModified(self, instance, attributeName)
	
	local propertySheet = r2:getPropertySheet(instance)

	if attributeName == "ItemNumber" then
		local nbRefId = instance.ItemNumber + 1
		local i = 1
		while i <= 3 do
			if i > nbRefId then
				local name = "Item"..tostring(i).."Id"
				local qty = "Item"..tostring(i).."Qty"
				r2.requestSetNode(instance.InstanceId, name, r2.RefId(""))
				r2.requestSetNode(instance.InstanceId, qty, 0)
			end
			i = i + 1
		end
		propertySheet.Env.updatePropVisibility()
		return
	end
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

function deliveryTaskDisplayerTable:onSelect(instance, isSelected)
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

function r2:deliveryTaskDisplayer()	
	return deliveryTaskDisplayerTable  -- returned shared displayer to avoid wasting memory
end


-- EVENTS-----------------------------------------------------------------------
-- we don't use events 1 to 3 anymore because they were bypassed by the events 
-- emitted by "request_item" and "give_item" actions
-- 4 : activation
-- 5 : deactivation
-- 6 : mission given
-- 8 : items delivered
-- 9 : mission completed
---------------------------------------------------------------------------------
function component:translate(context)
	r2.Translator.translateAiGroup(self, context)

	local validationNeeded = self.ValidationNeeded
	debugInfo("VALIDATION VALUE : " ..self.ValidationNeeded)

	if self.MissionGiver == nil then return end
	local giver = r2:getInstanceFromId(self.MissionGiver)
	if not giver then return end
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
	assert(rtGiverGrp)

	if self.MissionTarget == nil then return end
	local target = r2:getInstanceFromId(self.MissionTarget)
	if not target then return end
	local rtTargetGrp = r2.Translator.getRtGroup(context, target.InstanceId)
	assert(rtTargetGrp)
	
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	local items = self:getItems()
		
	-- Start of state
	do
		local action = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 7)
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, action)
	end
	
	do
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0)
		local rtAction4 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", 0) 
		local rtAction5 = r2.Translator.createAction("set_value",  rtGrp.Id, "Validation", self.ValidationNeeded) 
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, rtAction4, rtAction5  } )
		r2.Translator.translateAiGroupEvent("user_event_7", self, context, rtAction)
	end
	
	-- (re)activation
	do	
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", 1)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0)
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,  } )
		r2.Translator.translateAiGroupEvent("user_event_4", self, context, rtAction)
	end


	-- giver propose la mission 
	do
		local actionSuccessText = r2.Translator.createAction("npc_say",  self:textAdapter(self.MissionSucceedText),  rtGiverGrp.Id ..":"..giver.Name)
		local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9)
			
		local multiActions = r2.Translator.createAction("multi_actions", {actionSuccessText, actionEvent})
		local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Validation", 1,		
			r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 3, multiActions))

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0,
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("npc_say", self:textAdapter(self.MissionText),   rtGiverGrp.Id ..":"..giver.Name), 
				r2.Translator.createAction("give_item", rtGrp.Id, items, self:textAdapter(self.ContextualText)),
				})
					))
		
		local fullAction = r2.Translator.createAction("multi_actions", {rtAction, action})
		
		r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, fullAction)
	end

	--when player takes item from giver
	do
		 local actionItemGiven = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0, --and if the scenery object has been selected once by the player
			r2.Translator.createAction("multi_actions", { 
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 6),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 )}))

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,actionItemGiven)
								
		r2.Translator.translateAiGroupEvent("user_event_1", self, context, rtAction)
	end


	-- When player talks to target
	do
		local actionSay = r2.Translator.createAction("npc_say", self:textAdapter(self.WaitValidationText),   rtTargetGrp.Id ..":"..target.Name)
		local actionRequest = r2.Translator.createAction("request_item", rtGrp.Id, items, self:textAdapter(self.ContextualText))		
	
		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,  -- if items have been taken from giver
				r2.Translator.createAction("multi_actions", {actionSay, actionRequest})
					))
		r2.Translator.translateAiGroupEvent("player_target_npc", target, context, rtAction)
	end

	--when player gives item to target
	do
		local actionItemTaken = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 8),
				})
		)

		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, actionItemTaken)
								
		r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtAction)
	end

	--when receiving ItemsDelivered event
	do
		local baseAct = r2.Scenario:getBaseAct()
		local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		
		local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, self:textAdapter(self.BroadcastText) )
		local actionSet = r2.Translator.createAction("set_value", rtGrp.Id, "v2", 3)
		
		local actionEventSuccess = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Validation", 0, 
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9))
	
		local multiaction = r2.Translator.createAction("multi_actions", {actionBroadcast, actionSet, actionEventSuccess})

		r2.Translator.translateAiGroupEvent("user_event_8", self, context, multiaction)
		 
	end

	-- when receiving user_event_9, which means success, reset mission values.
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
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 3 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5),
				})
			);
		local actions = r2.Translator.createAction("multi_actions", { action2, action3, })

		
		local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 3, actions)

		r2.Translator.translateAiGroupEvent("user_event_9", self, context, action)
	end

	--r2.Translator.translateFeatureActivation(self, context)
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
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 6)
	elseif eventType == "delivered" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 8)
	elseif eventType == "succeeded" then 
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 9)

	end
	
	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local comp = r2.newComponent("DeliveryTask")
	assert(comp)

	local contextualText = i18n.get("uiR2EdDeliveryTask_ContextualText"):toUtf8()
	local missionText = i18n.get("uiR2EdDeliveryTask_MissionText"):toUtf8()
	local missionSucceedText = i18n.get("uiR2EdDeliveryTask_MissionSucceededText"):toUtf8()
	local waitValidationText = i18n.get("uiR2EdDeliveryTask_WaitValidationText"):toUtf8()
	local broadcastText = i18n.get("uiR2EdDeliveryTask_BroadcastText"):toUtf8()
	
	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_request_item")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDDeliveryTask")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.MissionSucceedText = missionSucceedText
	comp.WaitValidationText = waitValidationText
	comp.BroadcastText = broadcastText

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	comp._Seed = os.time() 

	return comp
end

component.create = function()	

	if not r2:checkAiQuota() then return end

	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'DeliveryTask' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("DeliveryTask") == 1 then 
			r2.displayFeatureHelp("DeliveryTask")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewDeliveryTaskFeature"))
		local component = feature.Components.DeliveryTask.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'DeliveryTask' position")
	end	
	local creature = r2.Translator.getDebugCreature("object_component_bot_request_item.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureDeliveryTask")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdDeliveryTask")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "DeliveryTask")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
				["activate"]			= { menu=i18n.get( "uiR2AA0Activate"					):toUtf8(),
											text=i18n.get( "uiR2AA1Activate"					):toUtf8()},
				["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"					):toUtf8(),
											text=i18n.get( "uiR2AA1Deactivate"					):toUtf8()}
		},
		["Events"] = {	
				["activation"]			= { menu=i18n.get( "uiR2Event0Activation"				):toUtf8(), 
											text=i18n.get( "uiR2Event1Activation"				):toUtf8()},
				["deactivation"]		= { menu=i18n.get( "uiR2Event0Deactivation"				):toUtf8(), 
											text=i18n.get( "uiR2Event1Deactivation"				):toUtf8()},
				["mission asked"]		= { menu=i18n.get( "uiR2Event0MissionGiven"				):toUtf8(), 
											text=i18n.get( "uiR2Event1MissionGiven"				):toUtf8()},
--				["wait validation"]		= { menu=i18n.get( "uiR2Event0TaskWaitValidation"		):toUtf8(), 
--											text=i18n.get( "uiR2Event1TaskWaitValidation"		):toUtf8()},
				["delivered"]			= { menu=i18n.get( "uiR2Event0Delivered"	):toUtf8(),
											text=i18n.get( "uiR2Event1Delivered"	):toUtf8()},
				["succeeded"]			= { menu=i18n.get( "uiR2Event0TaskSuccess"				):toUtf8(),
											text=i18n.get( "uiR2Event1TaskSuccess"				):toUtf8()},
		},
		["Conditions"] = {	
				["is active"]			= { menu=i18n.get( "uiR2Test0Active"					):toUtf8(), 
											text=i18n.get( "uiR2Test1Active"					):toUtf8()},
				["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"					):toUtf8(), 
											text=i18n.get( "uiR2Test1Inactive"					):toUtf8()},

				["is succeeded"]		= { menu=i18n.get( "uiR2Test0TaskSuccess"				):toUtf8(), 
											text=i18n.get( "uiR2Test1TaskSuccess"				):toUtf8()},

		}
	}
	return logicTranslations
end


r2.Features["DeliveryTask"] =  feature

