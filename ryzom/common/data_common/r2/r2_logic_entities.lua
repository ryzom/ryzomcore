local logicEntity = 
{
	BaseClass = "WorldObject",
	Name="LogicEntity",
	InEventUI = false,

	Parameters = {},
	ApplicableActions = {},
	Events = {},
	Conditions = {},
	TextContexts = {},
	TextParameters = {},
	LiveParameters = {},

	DisplayerProperties = "R2::CDisplayerLua",
	DisplayerPropertiesParams = "logicEntityPropertySheetDisplayer",
	PermanentTreeIcon = "r2ed_icon_permanent_macro_components.tga",
	TreeIcon = "r2ed_icon_macro_components.tga",
	SelectBarType = i18n.get("uiR2EDMacroComponents"):toUtf8(),

	Prop = 
	{
		{Name="Behavior", Type="LogicEntityBehavior"},	
	},

	---------------------------------------------------------------------------------------------------------
	-- from base class
	getContextualTreeIcon = function(this)
		if this:getParentAct():isBaseAct() then
			return this:getPermanentTreeIcon()
		end
		return ""
	end,

	getSelectBarIcon = function(this)
		return r2.Classes.BaseClass.getContextualTreeIcon(this)
	end,

	---------------------------------------------------------------------------------------------------------
	-- from base class
	getPermanentStatutIcon = function(this)

		if not this:isBotObject() and this:getParentAct():isBaseAct() then
			return "r2ed_permanent_pins.tga"
		else
			return ""
		end
	end	,

   --------------------------------------------------------------------------------------------
   -- Get the currently selected sequence, or nil if there's no available sequence
   getSelectedSequenceIndex = function(this)
      local activities = this:getBehavior().Activities
      if activities.Size == 0 then return nil end
      local index = defaulting(this.User.SelectedSequence, 0)
      if index >=  activities.Size then
         index = activities.Size - 1
      end
      return index
    end,
	--------------------------------------------------------------------------------------------
	-- from WorldObject
	canChangeDisplayMode = function(this)
		return true
	end,
	---------------------------------------------------------------------------------------------------------
	-- get the "Category" for this logic entity (Category entry found in the palette)
	getCategory = function(this)
		return this.Category
	end,
	---------------------------------------------------------------------------------------------------------
	-- get the "Sub-Category" for this logic entity (Category entry found in the palette)
	getSubCategory = function(this)
		return this.SubCategory
	end,
	--------------------------------------------------------------------------------------------
	isNextSelectable = function(this)
		return true
	end,
	--------------------------------------------------------------------------------------------
	-- return the behavior object, depending on wether this npc is grouped or not
	getBehavior = function(this)					
		if this:isKindOf("NpcGrpFeature") then
			return this.Components[0].Behavior
		elseif this:isGrouped() and this.ParentInstance:isKindOf("NpcGrpFeature") then
			return this.ParentInstance.Components[0].Behavior
		else		
			return this.Behavior					
		end
	end,
	--------------------------------------------------------------------------------------------
	-- check if that npc is part of a group
	isGrouped = function(this)
		if this.User.Grouped == true then return true end
		if this.ParentInstance then
			return this.ParentInstance:isKindOf("NpcGrpFeature")
		end
		return false
	end,


	-- get list of command for display in the toolbar
	getAvailableCommands = function(this, dest)
		r2.Classes.WorldObject.getAvailableCommands(this, dest)
		if not this:isBotObject() and not this:isKindOf("Act") then
			table.insert(dest, this:buildCommand(this.editActions, "edit_actions", "uiR2EDEditEventsTriggers", "r2ed_edit_events.tga", false))				
		
			if not this:getParentAct():isBaseAct() then
				table.insert(dest, this:buildCommand(this.togglePermanentCurrentAct, "permanent_content", "uimR2EDMenuPermanentContent", "r2ed_permanent_content.tga", true))					
			else
				table.insert(dest, this:buildCommand(this.togglePermanentCurrentAct, "current_content", "uimR2EDMenuCurrentActContent", "r2ed_current_act_content.tga", true))					
			end
		end		
	end,

	--
	togglePermanentCurrentAct = function(this, noNewAction)

		local newAct
		local actionName=""
		if this:getParentAct():isBaseAct() then
			newAct = r2:getCurrentAct()
			actionName = "uiR2EDCurrentActEntityAction"
		else
			newAct = r2.Scenario:getBaseAct()
			actionName = "uiR2EDPermanentEntityAction"
		end

		local parent = newAct.Features[0]
		local attr = "Components"
		local instance = this
		if not this:isInDefaultFeature() then
			parent = newAct
			attr = "Features"
		end 

		if this:isGrouped() or this:isKindOf("Creature") then
			instance = this.ParentInstance
		end

		if noNewAction~=false then
			r2.requestNewAction(i18n.get(actionName))
		end
		r2.requestMoveNode(instance.InstanceId, "", -1, parent.InstanceId, attr, -1)
	end,

	--
	onPostCreate = function(this)
		if this.User.DisplayProp and this.User.DisplayProp == 1 then
			r2:setSelectedInstanceId(this.InstanceId)				
			r2:showProperties(this)		
			this.User.DisplayProp = nil
		end
	end,
	--
	editActions = function(this)

		r2.events:openEditor()
	end,
	--------------------------------------------------------------------------------------------
	-- Test if this entity is a bot object
	isBotObject = function(this)								
		return false
	end,
	--------------------------------------------------------------------------------------------
	-- Test if thisentity is a plant
	isPlant = function(this)							
		return false
	end,
	--------------------------------------------------------------------------------------------
	-- from base class
	onPostHrcMove = function (this)
		-- if no more in a group, then mark as 'ungrouped'
		local grouped = false
		if this.ParentInstance then
			grouped = this.ParentInstance:isKindOf("NpcGrpFeature")
		end
		this.User.Grouped = grouped
		-- force update of the available options
		--if this == r2:getSelectedInstance() then
		r2.ContextualCommands:update()
		--end
	end,

	getAiCost = function(this)
		if this.User.GhostDuplicate then return 0 end
		return r2.getAiCost(this)
	end,

	getStaticObjectCost = function(this)
		return r2.getStaticObjectCost(this)
	end,

	hasScenarioCost = function(this)
		return true;
	end,

	createProtected = function(this)
		
		if not r2:checkAiQuota() then return end

		if this.create then
			this.create()
		end
	end,

	-- 
	getApplicableActions = function(this)
		return r2.Classes[this.Class].ApplicableActions	
	end,

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	initEventTypeMenu = function(this, subMenu, eventCategory)

		local class = r2.Classes[this.Class]
		local eventsTable = {}
		if eventCategory=="ApplicableActions" then
			eventsTable = this:getApplicableActions()
		else
			eventsTable = class[eventCategory]
		end

		for k, eventType in pairs(eventsTable) do
			local endRequest = (r2.events.eventTypeWithValue[eventType]==nil)

			if not r2.getLogicAttribute(this.Class, eventCategory, eventType) then
				debugInfo("Error: '"..eventCategory.. "' '" ..eventType .. "' is not defined for class'"..this.Class.."'")
				assert(r2.getLogicAttribute(this.Class, eventCategory, eventType))
			end 
		
			local uc_eventType = ucstring()
			local menuTitle = r2.getLogicAttribute(this.Class, eventCategory, eventType)

			local addLine = true
			if r2.events.memberManagement and this:isKindOf("Npc") and this:isGrouped() and menuTitle.groupIndependant~=true then
				addLine = false
			end

			if addLine then
				uc_eventType:fromUtf8(menuTitle.menu)
				subMenu:addLine(uc_eventType, "lua", 
					"r2.events:setEventType('".. eventType .."','" .. tostring(endRequest) .. "','" .. eventCategory .. "')", eventType)
			end
		end

		if table.getn(class[eventCategory])==0 then
			subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
		end
	end,
}


function logicEntity:getLogicTranslations()
	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]				= { menu=i18n.get( "uiR2AA0Activate"					):toUtf8(), 
											text=i18n.get( "uiR2AA1Activate"					):toUtf8()},
			["deactivate"]				= { menu=i18n.get( "uiR2AA0Deactivate"					):toUtf8(), 
											text=i18n.get( "uiR2AA1Deactivate"					):toUtf8()},
			["trigger"]					= { menu=i18n.get( "uiR2AA0Trigger"					):toUtf8(), 
											text=i18n.get( "uiR2AA1Trigger"					):toUtf8()},
		},
		["Events"] = {	
			["activation"]				= { menu=i18n.get( "uiR2Event0Activation"				):toUtf8(), 
											text=i18n.get( "uiR2Event1Activation"				):toUtf8()},
			["deactivation"]			= { menu=i18n.get( "uiR2Event0Deactivation"				):toUtf8(), 
											text=i18n.get( "uiR2Event1Deactivation"				):toUtf8()},
			["trigger"]					= { menu=i18n.get( "uiR2Event0Trigger"					):toUtf8(), 
											text=i18n.get( "uiR2Event1Trigger"					):toUtf8()},
		},	
		["Conditions"] = {	
			["is active"]				= { menu=i18n.get( "uiR2Test0Active"				):toUtf8(), 
											text=i18n.get( "uiR2Test1Active"				):toUtf8()},
			["is inactive"]				= { menu=i18n.get( "uiR2Test0Inactive"				):toUtf8(), 
											text=i18n.get( "uiR2Test1Inactive"				):toUtf8()},
		}
	}
	return logicTranslations
end

-----------------------------------------
--- register the curent Feature to menu

--function logicEntity.initLogicEntitiesMenu(this, logicEntityMenu)
--	
--	if this.InEventUI == true then
--		local name = i18n.get("uiR2ED" .. this.Name)
--		local tableInstances = r2.Scenario:getAllInstancesByType(this.Name)
--		if table.getn(tableInstances) > 0 then
--			logicEntityMenu:addLine(name, "lua", "", this.Name)
--		end
--	end
--end

----------------------------------------------------------------------------
-- add a line to the event sub menu
--function logicEntity.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
--
--	local entitiesTable = r2.Scenario:getAllInstancesByType(this.Name)
--	for key, entity in pairs(entitiesTable) do
--		local uc_name = ucstring()
--		uc_name:fromUtf8(entity.Name)
--		subMenu:addLine(uc_name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
--	end
--
--	if table.getn(entitiesTable)==0 then
--		subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
--	end
--end

-----------------------------------------
--- register the curent Feature to menu

function logicEntity.initLogicEntitiesMenu(this, logicEntityMenu)
	--debugInfo("####5")
	if this.InEventUI == true then
		local name = i18n.get("uiR2ED" .. this.Name)
		--local startTime = nltime.getPreciseLocalTime()				
		local enumerator = r2:enumInstances(this.Name)
		--local endTime = nltime.getPreciseLocalTime()	
		--debugInfo(string.format("time for enumInstances is %f", endTime - startTime))	
		startTime = endTime
		if enumerator:next() then			
			logicEntityMenu:addLine(name, "lua", "", this.Name)
		end
		--endTime = nltime.getPreciseLocalTime()	
		--debugInfo(string.format("time for next is %f", endTime - startTime))	
	end
end

------------------------------------------------------------------------------
---- add a line to the event sub menu
function logicEntity.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
	local enumerator = r2:enumInstances(this.Name)
	local found = false
	while 1 do		
		local entity = enumerator:next()
		if not entity then break end
		found= true
		local uc_name = ucstring()
		uc_name:fromUtf8(entity.Name)
		subMenu:addLine(uc_name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
	end
	if not found then
		subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
	end
end





r2.registerComponent(logicEntity)

---------------------------------------------------
--useful to represent a mission
--an empty group, an active state, a inactive state
--and a finished state. Use the createPseudoGroup
--function to directly add these in the context
--local pseudoGroup=
--{
--	BaseClass="BaseClass",
--	Name="PseudoGroup",
--	Prop=
--	{
--		{Name="RtGroup",Type="RtNpcGrp"},
--		{Name="InactiveState",Type="RtAiState"},
--		{Name="ActiveState",Type="RtAiState"},
--		{Name="FinishedState",Type="RtAiState"},
--	}
--}
--r2.registerComponent(pseudoGroup)

--insert a new text in the textManager, after it has been translated
local insertTextRt = function(text,context)
	local rtEntry = r2.newComponent("RtEntryText")
	rtEntry.Text = text
	debugInfo(colorTag(128,128,0).."insertion : "..text.." "..rtEntry.Id)
	table.insert(context.RtScenario.Texts.Texts,rtEntry)
	debugInfo(colorTag(128,128,0).."new size : "..table.getn(context.RtScenario.Texts.Texts))
	return rtEntry.Id
end

----------------------------------------------
----create a psudo group to translate missions
----insert the elements in the context
--r2.createPseudoGroup = function(context)
--	local PseudoGroup = r2.newComponent("PseudoGroup")
--	local group = PseudoGroup.RtGroup
--	group.Name = group.Id
--
--	table.insert(context.RtAct.NpcGrps, group)
--
--	do
--		local state
--
--		state= PseudoGroup.InactiveState
--		state.Name = state.Id
--		table.insert(context.RtAct.AiStates,state)
--		table.insert(state.Children,group.Id)
--
--		state= PseudoGroup.ActiveState
--		state.Name = state.Id
--		table.insert(context.RtAct.AiStates,state)
--		
--
--		state= PseudoGroup.FinishedState
--		state.Name = state.Id
--		table.insert(context.RtAct.AiStates,state)
--
--	end
--	return PseudoGroup
--end
--
--
--
local feature = {}

--
--local logicTexts ={
--	BaseClass="BaseClass",
--	Name="LogicTexts",
--	Prop=
--	{
--		{Name="Spontaneous",Type="Table"},
--		{Name="Interrogated",Type="Table"},
--	},
--}
--
--local LogicTextsTranslator = function(this,context,logicEntity)
--	local addTriggeredAction = r2.Features["ActivitySequence"].addTriggeredAction
--	local action
--	local index
--	local counter =context.Counter 
--
--	if logicEntity == nil
--	then
--		debugInfo("logic entity nil!!")
--	end
--	
--	if context.RtGroups == nil
--	then
--		debugInfo("context.RtGroups nil!!")
--	end
--	
--	if context.RtGroups[logicEntity.NpcGrpId] == nil
--	then
--		debugInfo("context.RtGroups[logicEntity.NpcGrpId] nil!!")
--	end
--
--	for k,v in pairs(this.Spontaneous)
--	do
--		if k ~= "Keys"
--		then
--			--get the "npc_say" action
--			action = Translator.LogicEntityTranslator(v,context,logicEntity)
--			--add this action to the triggerable actions of the npc group which give the mission
--			index = addTriggeredAction(context,context.RtGroups[logicEntity.NpcGrpId].Name,action)
--
--			local name = context.RtGroups[logicEntity.NpcGrpId].Name
--
--			--create an action to trigg the action to the npc group
--			action = Actions.createAction("modify_variable",name..":v3 = "..index)
--
--			--then, add the action to the correct state/event
--
--			if k=="Activated"
--			then
--				r2.Utils.addReaction(counter.ActiveState,"start_of_state",action)
--			end
--
--			if k=="Deactivated"
--			then
--				r2.Utils.addReaction(counter.InactiveState,"start_of_state",action)
--			end
--
--			if k=="Progresses"
--			then
--				r2.Utils.addReaction(counter.ActiveState,"user_event_0",action)
--			end
--		end
--	end
--
--	for k,v in pairs(this.Interrogated)
--	do
--		if k ~= "Keys"
--		then
--			--get the "npc_say" action
--			action = r2.Translator.LogicEntityTranslator(v,context)
--			--add this action to the triggerable actions of the npc group which give the mission
--			index = addTriggeredAction(context,context.RtGroups[logicEntity.NpcGrpId].Name,action)
--			local name = context.RtGroups[logicEntity.NpcGrpId].Name
--			--create an action to trigg the action to the npc group
--			action = Actions.createAction("modify_variable",name..":v3 = "..index)
--
--			--TODO insert the action in the correct state/event ...
--		end
--	end
--
--end
--
--r2.registerComponent(logicTexts)
--
--
--r2.registerComponent(varText)
---------------------------------------------------------
--This component is linked to a BaseCounter.It's used in
--the ParametrableText component to represent the value
--of the variable
--local var =
--{
--	BaseClass="BaseClass",
--	Name="TextParam",
--	Prop=
--	{
--		{Name="BaseCounterId",Type="String"}
--	}
--}
--r2.registerComponent(var)
--

----------------------------------------------------
--this component represents a text with parameters.
--It's a table of strings and/or TextParam elements.
--It allows to build sentences with variables.
--The Translator build a sentence with the strings
-- and parameters, and returns a "npc_say" action.
--see the UnitTest.testCounter() function.
--local paramText =
--{
--	BaseClass="BaseClass",
--	Name="ParametrableText",
--	Prop=
--	{
--		--components are : "TextParam" objects or strings
--		{Name="Components",Type="Table"},
--		{Name="Npc",Type="String"}
--	}	
--
--}
--r2.registerComponent(paramText)
--
--
--local ParamTextTranslator = function(this,context)
--	local text=""
--	local param=""
--	for k,v in pairs(this.Components)
--	do
--		if k~="Keys"
--		then
--			if type(v) == "string"
--			then
--				text = text..v
--			else
--				if v.Class == "TextParam"
--				then
--					local id = v.BaseCounterId
--					local baseCounter = context.Components[id]
--					local counterName = context.CounterNames[id]
--					param = r2.Utils.concat(param,counterName)
--					text = text.."$d "..baseCounter.Object.." "
--				end
--			end
--			
--		end
--	end
--	local action = r2.newComponent("RtNpcEventHandlerAction")
--	action.Action = "npc_say"
--	local who = r2.Utils.getNpcParam(this.Npc,context)
--	action.Parameters = who.."\n"..insertTextRt(text,context).."\n"..param
--	return action
--end
--
-- Obsolette?
--Translator.LogicEntityTranslator = function(logicEntity,context,ParentLogicEntity)
--	local class = logicEntity.Class
--	if class == "VarText"
--	then
--		return VarTextTranslator(logicEntity,context,ParentLogicEntity)
--	end
--	if class == "Counter"
--	then
--		return CounterTranslator(logicEntity,context,ParentLogicEntity)
--	end
--	if class == "LogicTexts"
--	then
--		return LogicTextsTranslator(logicEntity,context,ParentLogicEntity)
--	end
--	if class == "BaseCounter"
--	then
--		return BaseCounterTranslator(logicEntity,context,ParentLogicEntity)
--	end
--	if class == "ParametrableText"
--	then
--		return ParamTextTranslator(logicEntity,context)
--	end
--end
