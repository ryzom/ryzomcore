-- In Translation file
-- Category : uiR2EdRequestItem --
-- CreationFrom : uiR2EdRequestItemParameters


r2.Features.RequestItemFeature = {}

local feature = r2.Features.RequestItemFeature

feature.Name="RequestItemFeature"

feature.Description="A feature that makes a NPC request some item(s) from the player"


feature.Components = {}

feature.Components.RequestItem =
	{
		BaseClass="LogicEntity",			
		Name="RequestItem",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "requestItemDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate"},

		Events = { "activation", "deactivation", "wait validation", "mission asked", "succeeded"},

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
			{Name="ItemNumber", Type="Number", Category="uiR2EDRollout_ItemsToRequest", WidgetStyle="EnumDropDown", DefaultValue="3", 
			Enum={"1", "2", "3"},	},
			{Name="Item1Qty", Type="Number", Category="uiR2EDRollout_ItemsToRequest", Min="0", DefaultValue="1", Visible= function(this) return this:displayRefId(1) end},
			{Name="Item1Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToRequest", Visible= function(this) return this:displayRefId(1) end},


			{Name="Item2Qty", Type="Number", Category="uiR2EDRollout_ItemsToRequest", Min="0", DefaultValue="0", Visible= function(this) return this:displayRefId(2) end},
			{Name="Item2Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToRequest", Visible= function(this) return this:displayRefId(2) end},

			{Name="Item3Qty", Type="Number", Category="uiR2EDRollout_ItemsToRequest", Min="0", DefaultValue="0", Visible= function(this) return this:displayRefId(3) end},
			{Name="Item3Id", Type="RefId", WidgetStyle="PlotItem", Category="uiR2EDRollout_ItemsToRequest", Visible= function(this) return this:displayRefId(3) end},

			
			{Name="ContextualText", Type="String", Category="uiR2EDRollout_TextToSay", MaxNumChar="100"},
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

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
		end,


	}


local component = feature.Components.RequestItem

function component:displayRefId(index)
	local nbItems = self.ItemNumber + 1
	if index <= nbItems then
		return true
	end
	return false
end


local requestItemDisplayerTable = clone(r2:propertySheetDisplayer())

function requestItemDisplayerTable:onAttrModified(instance, attributeName)
	if attributeName == "ItemNumber" then
		local propertySheet = r2:getPropertySheet(instance)
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
	r2:propertySheetDisplayer():onAttrModified(instance, attributeName)
end

function requestItemDisplayerTable:onSelect(instance, isSelected)
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

function r2:requestItemDisplayer()	
	return requestItemDisplayerTable  -- returned shared displayer to avoid wasting memory
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
				local name = ""
				if plotItemSheetId ~= nil then
					name = r2.getSheetIdName(plotItemSheetId)
				end
				str = str .. tostring(name)..":"..qt
			end
		end
		id = id + 1
	end
	return str
end

function component:textAdapter(text)
	assert(text)
	if type(text) ~= "string" then
		debugInfo("Wrong type "..type(text))
		assert(nil)
	end	
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
	return str
end

	

function component:translate(context)
	r2.Translator.translateAiGroup(self, context)

	if self.MissionGiver == nil or self.MissionGiver == "" then return end
	local npc = r2:getInstanceFromId(self.MissionGiver)
	if not npc then return end

	local npcGrp = r2.Translator.getRtGroup(context, npc.InstanceId)
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	local items = self:getItems()


	--START OF STATE
	do
		local action = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 7)
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, action)
	end

	do
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- Success
		
		local rtAction = r2.Translator.createAction("multi_actions", {rtAction1, rtAction2, rtAction3})

		r2.Translator.translateAiGroupEvent("user_event_7", self, context, rtAction)
	end

	do	
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", 1)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0)
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, } )
		r2.Translator.translateAiGroupEvent("user_event_4", self, context, rtAction)
	end

	--CONTEXTUAL TEXT
	do
		local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			r2.Translator.createAction("request_item", rtGrp.Id, items, self:textAdapter(self.ContextualText))
		)
		r2.Translator.translateAiGroupEvent("player_target_npc", npc, context, rtAction)
	end



	--MISSION SUCCEEDED TEXT
	do	
	
		local actionSaySucceedText = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2),
				r2.Translator.createAction("npc_say", self:textAdapter(self.MissionSucceedText),  npcGrp.Id ..":"..npc.Name),
				}));

		local actionRepeatable = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  -- Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 )
				})
			); 
		local actionNotRepeatable = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- Not Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
				})
			);

		local baseAct = r2.Scenario:getBaseAct()
		local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, self:textAdapter(self.BroadcastText) )


		--reset all features var after success (depending on "repeatable")
		local actionReset = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2,
				r2.Translator.createAction("multi_actions", {actionRepeatable, actionNotRepeatable}))
		
		local rtActionSucceed = r2.Translator.createAction("multi_actions", {actionSaySucceedText, actionReset, actionBroadcast})
		
		r2.Translator.translateAiGroupEvent("user_event_3", self, context, rtActionSucceed)
	end


	--MISSION TEXT
	do
		local actionSayMissionText = r2.Translator.createAction("npc_say", self:textAdapter(self.MissionText),   npcGrp.Id ..":"..npc.Name); 
		local actionSetStatus = r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1)
		
		--local rtAction = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0,actionSayMissionText)
		local rtAction = r2.Translator.createAction("multi_actions", {actionSayMissionText, actionSetStatus})

		r2.Translator.translateAiGroupEvent("user_event_2", self, context, rtAction)
	end
	
	--WAIT VALIDATION TEXT
	do	
		 

		-- if the player carries the requested items when talking to the giver for the first time
		local actionMission = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0,
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("npc_say", self:textAdapter(self.MissionText), npcGrp.Id ..":"..npc.Name),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1)
				}))

 
		-- if the player has already talked to the giver and brings the requested item


		local actionValidation = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,
			r2.Translator.createAction("npc_say",  self:textAdapter(self.WaitValidationText),  npcGrp.Id ..":"..npc.Name)
			)
		local rtAction = r2.Translator.createAction("multi_actions", {actionValidation, actionMission})


		r2.Translator.translateAiGroupEvent("user_event_1", self, context, rtAction)
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
	elseif condition.Condition.Type == "is succeeded" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v2", 2);
		return action1, action1

	else
		assert(nil)
	end
	return nil,nil
end


component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component =  this--r2:getInstanceFromId(event.Entity)
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
	
	local comp = r2.newComponent("RequestItem")
	assert(comp)

	local contextualText = i18n.get("uiR2EdRequestItem_ContextualText"):toUtf8()
	local missionText = i18n.get("uiR2EdRequestItem_MissionText"):toUtf8()
	local waitValidationText = i18n.get("uiR2EdRequestItem_WaitValidationText"):toUtf8()
	local missionSucceededText = i18n.get("uiR2EdRequestItem_MissionSucceededText"):toUtf8()
	local broadcastText = i18n.get("uiR2EdRequestItem_BroadcastText"):toUtf8()

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_request_item")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDRequestItem")):toUtf8()			
	
	comp.ContextualText = contextualText
	comp.MissionText = missionText
	comp.WaitValidationText = waitValidationText
	comp.MissionSucceedText = missionSucceededText
	comp.BroadcastText = broadcastText

	comp.ItemNumber = 0

	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
--	comp.ItemQty = 1

	comp._Seed = os.time() 

	return comp
end

component.create = function()	

	if not r2:checkAiQuota() then return end


	local function paramsOk(resultTable)

		


		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		local showAgain = tonumber(resultTable["Display"])

		if showAgain == 1 then 
			r2.setDisplayInfo("RequestItemForm", 0)
		else r2.setDisplayInfo("RequestItemForm", 1) end

		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.RequestItem.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'RequestItem' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'RequestItem' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("RequestItem") == 1 then 
			r2.displayFeatureHelp("RequestItem")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewRequestItemFeatureAction"))
		local component = feature.Components.RequestItem.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'RequestItem' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_request_item.creature")
	r2:choosePos(creature, posOk, posCancel, "createFeatureRequestItem")
end


--function component:registerMenu(logicEntityMenu)
--	local name = i18n.get("uiR2EDRequestItem")
--	logicEntityMenu:addLine(ucstring(name), "lua", "", "RequestItem")
--end

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
			["is succeeded"]		= { menu=i18n.get( "uiR2Test0TaskSuccess"				):toUtf8(), 
										text=i18n.get( "uiR2Test1TaskSuccess"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["RequestItem"] =  feature






