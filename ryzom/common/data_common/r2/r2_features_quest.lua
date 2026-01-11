
r2.Features.Quest = {}

local feature = r2.Features.Quest

feature.Name="Quest"

feature.Description=""

feature.Components = {}

feature.Components.Quest =
	{
		BaseClass="LogicEntity",			
		Name="Quest",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "questDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},

		ApplicableActions = { "activate", "deactivate", "validate current task", "complete"},

		Events = {"activation", "deactivation", "success"},

		Conditions = { "is active", "is inactive", "is finished" },

		TextContexts =		{},

		TextParameters =	{},

		LiveParameters =	{},

		Prop =
		{
			{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
			{Name="Components", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			
			{Name="TaskNumber", Type="Number", Category="uiR2EDRollout_Targets", WidgetStyle="EnumDropDown", Enum={"1", "2", "3", "4", "5"},},
			{Name="TaskStep1Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickTaskComponent", SetRefIdFunction="r2:setTaskComponentTarget",Visible= function(this) return this:displayRefId(1) end},
			{Name="TaskStep2Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickTaskComponent", SetRefIdFunction="r2:setTaskComponentTarget",Visible= function(this) return this:displayRefId(2) end},
			{Name="TaskStep3Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickTaskComponent", SetRefIdFunction="r2:setTaskComponentTarget",Visible= function(this) return this:displayRefId(3) end},
			{Name="TaskStep4Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickTaskComponent", SetRefIdFunction="r2:setTaskComponentTarget",Visible= function(this) return this:displayRefId(4) end},
			{Name="TaskStep5Id", Type="RefId", Category="uiR2EDRollout_Targets", PickFunction="r2:canPickTaskComponent", SetRefIdFunction="r2:setTaskComponentTarget",Visible= function(this) return this:displayRefId(5) end},

			{Name="QuestCompletedText", Type="String", Category="uiR2EDRollout_TextToSay"},
			
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


local component = feature.Components.Quest


function component:displayRefId(index)
	local nbRefId = self.TaskNumber + 1
	if index <= nbRefId then
		return true
	end
	return false
end

function component:textAdapter(text)
	assert(self)
	assert(type(text) == "string")
	local str =  text
	
	str=string.gsub(str, "<quest_name>", self.Name)
	return str
end

-----------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
local questDisplayerTable = clone(r2:propertySheetDisplayer())

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
		local attrName = "Task" ..i.. "Id"
		if attrName ~= attributeName and this[attrName] == tmpInstance.InstanceId then
			return false
		end
		i = i + 1
	end
	return true
end


local oldOnAttrModified = questDisplayerTable.onAttrModified
function questDisplayerTable:onAttrModified(instance, attributeName)
	
	oldOnAttrModified(self, instance, attributeName)

	if attributeName == "TaskNumber" then
		local propertySheet = r2:getPropertySheet(instance)
		local nbRefId = instance.TaskNumber + 1
		local i = 1
		while i <= 5 do
			if i > nbRefId then
				local name = "TaskStep"..tostring(i).."Id"
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

	if (string.find(attributeName, "Id") == nil or attributeName == "InstanceId") then return end 
	
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

function questDisplayerTable:onSelect(instance, isSelected)
	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

function component:onTargetInstancePreHrcMove(targetAttr, targetIndexInArray)

	local targetId = self[targetAttr]
	local tmpInstance = r2:getInstanceFromId(targetId)
	tmpInstance.User.SelfModified = true
	
end



function r2:questDisplayer()	
	return questDisplayerTable  -- returned shared displayer to avoid wasting memory
end

---------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------
function component:createGhostComponents(act)
	local comp = self

	local nbTask = 0
	local firstId = true
		
	for id = 1, 5 do
		local propertyName = "TaskStep"..tonumber(id).."Id"
		if comp[propertyName] ~= nil and comp[propertyName] ~= "" then
			local taskInstance = r2:getInstanceFromId(comp[propertyName])
			if taskInstance then
				if firstId == true then
					if comp.Active == 1 then
						r2.requestSetGhostNode(taskInstance.InstanceId, "Active", 1)
					else
						local eventHandler = r2.newComponent("LogicEntityAction")
						eventHandler.Event.Type = "activation"
						eventHandler.Event.Value = ""
						eventHandler.Name = "activation"
						
						local action = r2.newComponent("ActionStep")
					
						action.Entity =  r2.RefId(taskInstance.InstanceId) 
						action.Action.Type = "activate"
						action.Action.Value = ""					
						
						table.insert(eventHandler.Actions, action)
						
						local behaviorId = comp.Behavior.InstanceId
						assert(behaviorId)
						r2.requestInsertGhostNode(behaviorId, "Actions", -1, "", eventHandler)
					end
					firstId = false
				else
					r2.requestSetGhostNode(taskInstance.InstanceId, "Active", 0)
				end
				r2.requestSetGhostNode(taskInstance.InstanceId, "Repeatable", 0)
				
				do
					local eventHandler = r2.newComponent("LogicEntityAction")
					eventHandler.Event.Type = "deactivation"
					eventHandler.Event.Value = ""
					eventHandler.Name = "deactivation"
					
					local action = r2.newComponent("ActionStep")
				
					action.Entity =  r2.RefId(taskInstance.InstanceId) 
					action.Action.Type = "deactivate"
					action.Action.Value = ""					
					
					table.insert(eventHandler.Actions, action)
					
					local behaviorId = comp.Behavior.InstanceId
					assert(behaviorId)
					r2.requestInsertGhostNode(behaviorId, "Actions", -1, "", eventHandler)
				
				end
				
					
			end
		end 
	end --!FOR
end

function component:getTaskRtIds(context)
	local rtGroups = {}
	for id = 1, 5 do
		local taskId = self["TaskStep"..id.."Id"]
		if taskId and taskId ~= "" then
			local rtGroup = r2.Translator.getRtGroup(context, taskId)
			local prefix = ""
			if rtGroup.Id and rtGroup.Id ~= "" then
				prefix = r2:getNamespace()..rtGroup.Id.."."
			end
			table.insert(rtGroups, prefix)
		end
	end
	return rtGroups 
end

function component:getTaskInstances(context)
	local instances = {}
	for id = 1, 5 do
		local taskId = self["TaskStep"..id.."Id"]
		if taskId and taskId ~= "" then
			local instance = r2:getInstanceFromId(taskId)
			if instance then
				table.insert(instances, instance)
			end
		end
	end
	return instances 
end

function component:translate(context)
	--EVENTS :
	-- 4: activate
	-- 5 : deactivate
	-- 4 (for steps) : init (activate reinit default values on mission steps)
	-- 8 : quest completed
	-- 9 : validate task step
	r2.Translator.translateAiGroup(self, context)
	
	local rtGrp = r2.Translator.getRtGroup(context, self.InstanceId)
	
	local taskInstances = self:getTaskInstances(context)
	if table.getn(taskInstances) == 0 then return end
	local taskRtGrps = self:getTaskRtIds(context)

	local baseAct = r2.Scenario:getBaseAct()
	local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		
	-- Start of state
	do
		-- v1 = repeatable
		-- v2 = current step index
		-- v3 = completed (at least once)
		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", self.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", self.Repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 1) 
		local rtAction4 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", 0) 
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3, } )
		r2.Translator.translateAiGroupEvent("start_of_state" , self, context, rtAction)
	end


	do
		local k, v = next(taskInstances, nil)
		while k do
			local taskIndex = k
			local taskInstance = v
			
			local rtActionIncrement = r2.Translator.createAction("increment_quest_step_index", rtGrp.Id, taskIndex)
			local event = r2.Translator.getEventFromType(taskInstance, context, "succeeded") --get the right event for "succeeded" 	
			r2.Translator.translateAiGroupEvent(event.Event, taskInstance, context, rtActionIncrement)

			k, v = next(taskInstances, k)
		end
	end

	do
		local actionValidateTask = r2.Translator.createAction("validate_quest_step", rtGrp.Id, taskRtGrps)
				
		r2.Translator.translateAiGroupEvent("user_event_9", self, context, actionValidateTask)
	end

	
	--Deactivation (event 5)	
	do
		local rtAction = r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				})
		r2.Translator.translateAiGroupEvent("user_event_5", self, context, rtAction)
	end
	
	--Mission completed (event 8)
	do
		local actionBroadcast = r2.Translator.createAction("broadcast_msg", baseActRtGrp.Id, self:textAdapter(self.QuestCompletedText))
		
		local actionRepeatable = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  --if Repeatable
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				})
			); 
		local actionNotRepeatable = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- if not Repeatable
				r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				})
				)

		local actionCompleted = r2.Translator.createAction("multi_actions", {
			actionRepeatable, actionNotRepeatable, actionBroadcast,
			})
		r2.Translator.translateAiGroupEvent("user_event_8", self, context, actionCompleted)
	end
	r2.Translator.translateFeatureActivation(self, context)
end


component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtGrp)

	local prefix = ""
	if rtGrp.Id and rtGrp.Id ~= "" then
		prefix = r2:getNamespace() .. rtGrp.Id.."."
	end
	
	local taskRtGrps = component:getTaskRtIds(context)

	if action.Action.Type == "validate current task" then
		local actionSet = r2.Translator.createAction("set_value", rtGrp.Id, "v2", prefix.."v2 + 1")
		local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9)
		--local actionValidateStep = r2.Translator.createAction("validate_task_step", rtGrp.Id, taskRtGrps)	
		local rtAction = r2.Translator.createAction("multi_actions", {actionSet, actionEvent})
		local ifActive = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, rtAction)
		return ifActive, ifActive

	elseif action.Action.Type == "complete" then
		
		local actionEventComplete = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 8)
		local ifActive = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, actionEventComplete)

		return ifActive, ifActive
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
	elseif condition.Condition.Type == "is finished" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "v3", 1);
		return action1, action1
	end
	return nil,nil
end


component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
	assert(rtNpcGrp)

	local eventType = tostring(event.Event.Type)
	
	local eventHandler, lastCondition = nil, nil

	if eventType == "success" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 8)
	end
	
	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.createComponent = function(x, y)
	
	local questCompletedText = i18n.get("uiR2EdQuest_QuestCompletedText"):toUtf8()
	
	local comp = r2.newComponent("Quest")
	assert(comp)

	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.bot_chat")
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdQuest")):toUtf8()			
	
	comp.QuestCompletedText = questCompletedText
	comp.TaskNumber = 0

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
			r2.setDisplayInfo("Quest", 0)
		else r2.setDisplayInfo("Quest", 1) end
		
		if not x or not y
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.Quest.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'Quest' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'Quest' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("Quest") == 1 then 
			r2.displayFeatureHelp("Quest")
		end
		r2.requestNewAction(i18n.get("uiR2EDNewQuestAction"))
		local component = feature.Components.Quest.createComponent( x, y)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)	
	end
	local function posCancel()
		debugInfo("Cancel choice 'Quest' position")
	end
	local creature = r2.Translator.getDebugCreature("object_component_bot_chat.creature")	
	r2:choosePos(creature, posOk, posCancel, "createFeatureQuest")
end


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdQuest")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "Quest")
end

function component:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
				["activate"]			= { menu=i18n.get( "uiR2AA0Activate"				):toUtf8(),
											text=i18n.get( "uiR2AA1Activate"				):toUtf8()},
				["deactivate"]			= { menu=i18n.get( "uiR2AA0Deactivate"				):toUtf8(),
											text=i18n.get( "uiR2AA1Deactivate"				):toUtf8()},
				["validate current task"]			= { menu=i18n.get( "uiR2AA0ValidateCurrentTask"				):toUtf8(),
														text=i18n.get( "uiR2AA1ValidateCurrentTask"				):toUtf8()},
				["complete"]						= { menu=i18n.get( "uiR2AA0CompleteQuest"				):toUtf8(),
														text=i18n.get( "uiR2AA1CompleteQuest"				):toUtf8()},
		},
		["Events"] = {	
				["activation"]			= { menu=i18n.get( "uiR2Event0Activation"			):toUtf8(), 
											text=i18n.get( "uiR2Event1Activation"			):toUtf8()},
				["deactivation"]		= { menu=i18n.get( "uiR2Event0Deactivation"			):toUtf8(), 
											text=i18n.get( "uiR2Event1Deactivation"			):toUtf8()},
				["success"]				= { menu=i18n.get( "uiR2Event0TaskSuccess"			):toUtf8(), 
											text=i18n.get( "uiR2Event1TaskSuccess"			):toUtf8()},
		},
		["Conditions"] = {	
				["is active"]			= { menu=i18n.get( "uiR2Test0Active"				):toUtf8(), 
											text=i18n.get( "uiR2Test1Active"				):toUtf8()},
				["is inactive"]			= { menu=i18n.get( "uiR2Test0Inactive"				):toUtf8(), 
											text=i18n.get( "uiR2Test1Inactive"				):toUtf8()},
				["is finished"]			= { menu=i18n.get( "uiR2Test0TaskSuccess"				):toUtf8(), 
											text=i18n.get( "uiR2Test1TaskSuccess"				):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["Quest"] =  feature





