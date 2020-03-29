r2.classLogicAttributes = {}

r2.events = {

uiId = "ui:interface:r2ed_events",
ownCreatedInstances = {},
openFirst = nil,
elementsIdCounter = 0,
elementOrder = false,
maxAndMin = false,
eltTemplateParams =	{
						selectElt="r2.events:selectElement()", 
						openEltEditor="", 
						maxMinElt="", 
						removeElt="r2.events:removeElementInst()",
						colOver="200 150 0 100",
						colPushed="200 150 0 255",
						multiMaxLine=""
					},

elementEditorTemplate = "template_edit_events",
elementInitialName=i18n.get("uiR2EdAction"):toUtf8(),

currentEltUIID = nil, -- initialisé quand l'editeur est ouvert ou fermé

logicTranslations = {
						["ApplicableActions"] = {},
						["Events"] = {},
						["Conditions"] = {}
					},

selectedLogicEntity,
selectedEventType,

eventTypeWithValue =	{
							["end of chat"]					= "ChatStep",
							["start of chat"]				= "ChatStep",
							["starts chat"]					= "ChatStep",
							["is in chat"]					= "ChatStep",

							["begin of activity step"]		= "ActivityStep", 
							["end of activity step"]		= "ActivityStep", 
							["is in activity step"]			= "ActivityStep",
							["is in activity sequence"]     = "ActivitySequence",
							["end of activity sequence"]    = "ActivitySequence",
							["begin activity sequence"]		= "ActivitySequence",
							["begin of activity sequence"]	= "ActivitySequence",
							["emits user event"]			= "Number",
							["user event emitted"]			= "Number",
							["add seconds"]					= "Number",
							["sub seconds"]					= "Number",
							["add scenario points"]			= "Number",



						},

filteredLogicEntityId = nil,
dismatchWithFilter = 0,

actionOrConditionUIId = nil,

keyWordsColor = "@{FFFF}",
communWordsColor = "@{FFFB}",
filterWordColor = "@{FF0F}",
entityWordColor = "@{F90B}",

maxVisibleLine = 10,

memberManagement = false,
}






-- sequence --------------------------------------------------
function r2.events:currentSequUI()
	return getUI(self.uiId):find("sequence_elts")
end

function r2.events:currentSequInstId()
	return self:currentSequUI().Env.InstanceId
end

-- initialisé quand selection dialog dans menu
function r2.events:setSequUIInstId(sequUI, id)
	sequUI.Env.InstanceId = id
end

function r2.events:currentSequInst()
	return r2:getInstanceFromId(self:currentSequInstId())
end

-- element ---------------------------------------------------
function r2.events:currentEltUIId()
	return self.currentEltUIID
end

function r2.events:currentEltUI()
	if self.currentEltUIID then
		return getUI(self.currentEltUIID)
	end
	return nil
end

function r2.events:setCurrentEltUIId(id)
	self.currentEltUIID = id
end

function r2.events:currentEltInstId()
	if self.currentEltUIID then 
		return self:currentEltUI().Env.InstanceId
	end
	return nil
end

function r2.events:currentEltInst()
	if self.currentEltUIID and self:currentEltInstId() then
		return r2:getInstanceFromId(self:currentEltInstId())
	end
	return nil
end

-- updated element and/or sequence (not necessary the same as current sequence or element)
function r2.events:updatedSequUI()
	return self:currentSequUI()
end

function r2.events:setUpdatedSequUIId(sequUIId)
end

function r2.events:updatedEltUI()
	return self:currentEltUI()
end

function r2.events:setUpdatedEltUIId(eltUIId)
end



------------------ INIT EVENTS EDITOR -------------------------
function r2.events:initEditor()
	self:mergeLogicTranslations()
end

------------------ TRANSLATION TEXT ---------------------------
function r2.events:getTranslationText(eventCategory, eventType)
	if self.logicTranslations[eventCategory][eventType] then
		return self.logicTranslations[eventCategory][eventType].text
	else
		debugInfo(eventType .. " is not translated")
		return eventType
	end			
end

------------------ TRANSLATION IN MENU ------------------------
function r2.events:getTranslationMenu(eventCategory, eventType)
	if self.logicTranslations[eventCategory][eventType] then
		return ucstring(self.logicTranslations[eventCategory][eventType].menu)
	else
		debugInfo(eventType .. " is not translated")
		return ucstring(eventType)
	end			
end

-------------------------------------------------
-- Another global map containing all logic attributes per class
function r2.events:registerClassLogicAttributes(class, logicTranslations)
	if r2.classLogicAttributes[class] then 
		r2.print("Logic attributes are already registered for class '" ..class.."'")
		return
	end

	r2.classLogicAttributes[class] = logicTranslations
end

-------------------------------------------------
--Looks for a logic attribute for a given class. If the attribute isn't in the corresponding map, looks 
-- in the class's baseclass map
function r2.getLogicAttribute(class, logicCategory, value)
	if not class or class == "" then return nil end
	
	local logicAttributes = r2.classLogicAttributes[class]
	
	if not logicAttributes then
		--look in parent map
		local baseclass = r2.Classes[class].BaseClass
		r2.print("Attributes not defined for class '"..class.."' looking in '"..baseclass.."' for inherited attributes")
		if baseclass then 
			return r2.getLogicAttribute(baseclass, logicCategory, value)
		end
		return nil
	end
	
	local attr = logicAttributes[logicCategory][value]
	
	if not attr then
		--look in parent map 
		local baseclass = r2.Classes[class].BaseClass
		if baseclass then
			return r2.getLogicAttribute(baseclass, logicCategory, value)
		end
		return nil
	end

	return attr
end
------------------------------------------------
function r2.events:mergeLogicTranslations()

	registerComponentsLogicTranslations = function(localLogicTranslations)	
		local k, v = next(localLogicTranslations, nil)
		while k do
			local k2, v2 = next(v, nil)
			while k2 do
				
				if not self.logicTranslations[k][k2] then
					self.logicTranslations[k][k2] = v2
				end
				k2, v2 = next(v, k2)
			end
			k, v = next(localLogicTranslations, k)
		end
	end
		
	local k, v = next(r2.Classes, nil)
	while k do
		if v.getLogicTranslations then
			local localLogicTranslations = v:getLogicTranslations()
			if localLogicTranslations then
				self:registerClassLogicAttributes(k, localLogicTranslations)
				--registerComponentsLogicTranslations(localLogicTranslations)
			else
				debugInfo(colorTag(255,0,0).."No Translation for component "..k)
			end
		end
		k, v = next(r2.Classes, k)
	end
end


------ OPEN EDITOR ------------------------------------------
function r2.events:openEditor()

	local ui = getUI(self.uiId)

	if not ui.active then

		self:filterEvents()	

		-- active editor
		ui.active = true
		ui:updateCoords()
		
		if self.openFirst == nil then
			self.openFirst = true
			ui:center()
		end
	else
		setTopWindow(ui)
		ui:blink(1)
	end
end

------ CLOSE EDITOR ------------------------------------------
function r2.events:closeEditor()
	local ui = getUI(self.uiId)
	ui.active = false
	r2.logicUI:closeEditor(r2.events)
end

------ CLEAN EDITOR ------------------------------------------
function r2.events:cleanEditor()

	-- reset current dialog and current chat
	self:setCurrentEltUIId(nil)
	self:setUpdatedSequUIId(nil)
	self:setUpdatedEltUIId(nil)
	self.elementsIdCounter = 0

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	local eltsList = sequenceUI:find("elements_list")
	assert(eltsList)
	eltsList:clear()

	local dismatchFilterMenu = getUI(self.uiId):find("filterMenu"):find("dismatch_filter")
	assert(dismatchFilterMenu)
	dismatchFilterMenu.active = false

	self.dismatchWithFilter = 0
end

--- UPDATE SEQUENCE UI-------------------------------------------
function r2.events:updateSequenceUI()

	local ui = getUI(self.uiId)
	assert(ui)

	local filterMenuText = ui:find("filterMenu"):find("menu"):find("text")
	assert(filterMenuText)
	
	local logicEntity = r2:getInstanceFromId(self.filteredLogicEntityId)
	assert(logicEntity)

	filterMenuText.uc_hardtext = logicEntity:getDisplayName()

	local currentSequ = self:currentSequUI()
	assert(currentSequ)
	local eltsList = currentSequ:find("elements_list")
	assert(eltsList)
	for i=0, eltsList.childrenNb-1 do
		local elt = eltsList:getChild(i)
		assert(elt)
		--not element editor
		if r2.logicUI:getEltUIInstId(elt) then
			self:updateElementUI(elt)
		end
	end
end


function r2.events:getLogicEntityParent(event)

	local logicEntity = event.Parent.Parent.Parent

	if logicEntity:isGrouped() and logicEntity:isLeader() then
		logicEntity = logicEntity:getParentGroup()
	end
	return logicEntity
end

function r2.events:filterIsLogicEntityParent(event)
	return self.filteredLogicEntityId==self:getLogicEntityParent(event).InstanceId
end

function r2.events:getBehavior(logicEntity)
	return logicEntity.Behavior					
end


------ SELECT FILTER EVENTS --------------------------------------
function r2.events:filterEvents(logicEntityId)

	if logicEntityId==nil then
		logicEntityId = r2:getSelectedInstance().InstanceId
	end

	local logicEntity = r2:getInstanceFromId(logicEntityId)
	if logicEntity then  

		if not self.memberManagement and logicEntity:isGrouped() then   --TEMP
			logicEntity = logicEntity:getParentGroup()					--TEMP
			logicEntityId = logicEntity.InstanceId						--TEMP
		end																--TEMP

		-- clean editor
		self:cleanEditor()

		local ui = getUI(self.uiId)
		assert(ui)

		-- update filter text
		local filterMenuText = ui:find("filterMenu"):find("menu"):find("text")
		assert(filterMenuText)
		filterMenuText.uc_hardtext = logicEntity:getDisplayName()

		self.filteredLogicEntityId = logicEntityId

		-- recover all events of logic entities in current act
		local allEvents = {}
		local allLogicEntities = {}

		for i=0, r2.Scenario.Acts.Size-1 do
			local act = r2.Scenario.Acts[i]
			act:appendInstancesByType(allLogicEntities, "LogicEntity")
			table.insert(allLogicEntities, act)
		end

		table.insert(allLogicEntities, r2.Scenario)
		for k0, entity in pairs(allLogicEntities) do
			
			local behavior = self:getBehavior(entity)
			if not self.memberManagement then		-- TEMP
				behavior = entity:getBehavior()		-- TEMP
			end										-- TEMP

			for e=0, behavior.Actions.Size-1 do
				local event = behavior.Actions[e]
				assert(event)
				allEvents[event.InstanceId] = event
			end	
		end
		
		-- recover events which match with filter
		local eventsType = {}
		local eventsAction = {}
		local eventsCondition = {}

		for k, event in pairs(allEvents) do

			local isSelected = false

			local filterIsLogicEntityParentB = self:filterIsLogicEntityParent(event)
			if not self.memberManagement then	-- TEMP
				filterIsLogicEntityParentB = (event:getLogicEntityParent().InstanceId == logicEntityId) -- TEMP
			end									-- TEMP
			-- events type
			
			if filterIsLogicEntityParentB then
				table.insert(eventsType, event.InstanceId)
				isSelected = true
			end

			-- actions
			if not isSelected then
				
				for a=0, event.Actions.Size-1 do
					local action = event.Actions[a]
					assert(action)

					if tostring(action.Entity) == logicEntityId then
						table.insert(eventsAction, event.InstanceId)
						isSelected = true
						break
					end
				end
			end

			-- conditions
			if not isSelected then
				
				for a=0, event.Conditions.Size-1 do
					local condition = event.Conditions[a]
					assert(condition)

					if tostring(condition.Entity) == logicEntityId then
						table.insert(eventsCondition, event.InstanceId)
						break
					end
				end
			end
		end

		-- create event editor
		self:createElementEditor()

		-- display "events type"
		for k, eventId in pairs(eventsType) do
			local event = r2:getInstanceFromId(eventId)
			assert(event)
			self:newElementUI(event)
		end

		-- display "events action"
		for k, eventId in pairs(eventsAction) do
			local event = r2:getInstanceFromId(eventId)
			assert(event)
			self:newElementUI(event)
		end

		-- display "events condition"
		for k, eventId in pairs(eventsCondition) do
			local event = r2:getInstanceFromId(eventId)
			assert(event)
			self:newElementUI(event)
		end

	else

		debugInfo("UNKNOWN FILTER EVENTS")

		for k, eventId in pairs(allEvents) do
			local event = r2:getInstanceFromId(eventId)
			assert(event)
			self:newElementUI(event)
		end
	end

	r2.logicComponents:selectSequence(r2.events)
end

------ REFRESH FILTER --------------------------------------------
function r2.events:refreshEvents()
	self:filterEvents(self.filteredLogicEntityId)
end


------ SELECT ELEMENT --------------------------------------------
function r2.events:selectElement(selectedButtonElt)
	r2.logicComponents:selectElement(r2.events, selectedButtonElt)
end

------ CREATE EDITOR -----------------------------------------------
function r2.events:createElementEditor()

	r2.logicComponents:createElementEditor(r2.events)
end

------ OPEN ELEMENT EDITOR -----------------------------------------------
function r2.events:updateElementEditor() 

	local instanceEvent = self:currentEltInst()
	if instanceEvent==nil then return end

	local ui = getUI(self.uiId)
	assert(ui)

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	
	local eventEditor = sequenceUI:find("edit_element")
	assert(eventEditor)

	-- editor title
	local editorTitleText = eventEditor:find("event_name"):find("name")
	assert(editorTitleText)

	local logicEntity = self:getLogicEntityParent(instanceEvent)
	if not self.memberManagement then						--TEMP
		logicEntity = instanceEvent:getLogicEntityParent()  --TEMP
	end														--TEMP
	assert(logicEntity)
	
	-- when menu text
	local when = eventEditor:find("when")
	assert(when)
	when.Env.InstanceId = logicEntity.InstanceId
	local whenMenuText = when:find("text")
	assert(whenMenuText)

	local coloredName = (logicEntity.InstanceId == self.filteredLogicEntityId)
	
	if coloredName then 
		eventType = self.filterWordColor 
	else
		eventType = self.communWordsColor 
	end

	local name = "No name"
	if logicEntity.getName then
		name = logicEntity:getName()
	elseif logicEntity.Name then
		name = logicEntity.Name
	end
	eventType = eventType .. name .. " " 
	if coloredName then eventType = eventType .. self.communWordsColor end

	
	local class = logicEntity.Class
	assert(class)
	eventType = eventType ..r2.getLogicAttribute(class, "Events", instanceEvent.Event.Type).text
	if instanceEvent.Event.Value~="" then
		local instance = r2:getInstanceFromId(instanceEvent.Event.Value)
		assert(instance)
		eventType = eventType .. " '" .. instance:getShortName() .. "'"
	end
	if instanceEvent.Event.ValueString and instanceEvent.Event.ValueString~="" then
		eventType = eventType .. " '" .. instanceEvent.Event.ValueString .. "'"
	end

	local uc_when_text = ucstring()	
	uc_when_text:fromUtf8(eventType)
	whenMenuText.uc_hardtext_single_line_format = uc_when_text 
	editorTitleText.uc_hardtext = whenMenuText.hardtext .. " ..."

	-- actions
	local actionsList = eventEditor:find("actions_list")
	assert(actionsList)
	actionsList:clear()

	if instanceEvent.Actions.Size > 0 then

		for i=0, instanceEvent.Actions.Size-1 do
			local actionInst = instanceEvent.Actions[i]
			
			local actionUI = self:newActionUI()
			actionUI.Env.InstanceId = actionInst.InstanceId
		
			local actionMenuText = actionUI:find("text")
			assert(actionMenuText)

			if actionInst.Entity~="" and actionInst.Action.Type~="" then

				local actionLogicEntity = r2:getInstanceFromId(tostring(actionInst.Entity))
				assert(actionLogicEntity)

				coloredName = (actionLogicEntity.InstanceId == self.filteredLogicEntityId)
				local actionType 
				if coloredName then 
					actionType = self.filterWordColor 
				else
					actionType = self.communWordsColor 
				end
				local name = "No name"
				if actionLogicEntity.getName then
					name = actionLogicEntity:getName()
				elseif actionLogicEntity.Name then
					name = actionLogicEntity.Name
				end
				actionType = actionType .. name .. " " 
				if coloredName then actionType = actionType .. self.communWordsColor end

				local class = actionLogicEntity.Class
				assert(class)
				actionType = actionType ..r2.getLogicAttribute(class, "ApplicableActions", actionInst.Action.Type).text
				if actionInst.Action.Value~="" then
					local instance = r2:getInstanceFromId(actionInst.Action.Value)
					assert(instance)
					actionType = actionType .. " '" .. instance:getShortName() .. "'"
				end
				if actionInst.Action.ValueString and actionInst.Action.ValueString ~= "" then
					if string.gfind(actionType, "%%1")() then
						actionType = string.gsub(actionType, "%%1", "'"..tostring(actionInst.Action.ValueString).."'")
					else
						actionType = actionType .. " '" .. tostring(actionInst.Action.ValueString).. "'"
					end
				end

				local uc_action_text = ucstring()
				uc_action_text:fromUtf8(actionType)
				actionMenuText.uc_hardtext_single_line_format = uc_action_text 
			end
		end
	else
		self:newActionUI()
	end

	-- conditions
	local conditionsList = eventEditor:find("conditions_list")
	assert(conditionsList)
	conditionsList:clear()

	for i=0, instanceEvent.Conditions.Size-1 do
		local conditionInst = instanceEvent.Conditions[i]
		
		local conditionUI = self:newConditionUI()
		conditionUI.Env.InstanceId = conditionInst.InstanceId
		
		local conditionMenuText = conditionUI:find("text")
		assert(conditionMenuText)

		if conditionInst.Entity~="" and conditionInst.Condition.Type~="" then

			local conditionLogicEntity = r2:getInstanceFromId(tostring(conditionInst.Entity))
			assert(conditionLogicEntity)

			coloredName = (conditionLogicEntity.InstanceId == self.filteredLogicEntityId)
			local conditionType 
			if coloredName then 
				conditionType = self.filterWordColor 
			else
				conditionType = self.communWordsColor 
			end
			conditionType = conditionType .. conditionLogicEntity.Name .. " " 
			if coloredName then conditionType = conditionType .. self.communWordsColor end

			local class = conditionLogicEntity.Class
			assert(class)
			conditionType = conditionType ..r2.getLogicAttribute(class, "Conditions", conditionInst.Condition.Type).text
			if conditionInst.Condition.Value~="" then
				local instance = r2:getInstanceFromId(conditionInst.Condition.Value)
				assert(instance)
				conditionType = conditionType .. " '" .. instance:getShortName() .."'"
			end

			local uc_condition_text = ucstring()
			uc_condition_text:fromUtf8(conditionType)
			conditionMenuText.uc_hardtext_single_line_format = uc_condition_text
		end
	end
	
	self:advancedEditor(((instanceEvent.Actions.Size > 1) or (instanceEvent.Conditions.Size > 0)))

	-- match filter?
	local matchFilter = self:logicEntityActionMatchFilter(instanceEvent)

	local dismatchFilter = eventEditor:find("dismatch_filter")
	assert(dismatchFilter)
	dismatchFilter.active = not matchFilter

	-- valid event?
	self:updateValiditySymbol(eventEditor, instanceEvent)
end

function r2.events:updateElementsUI()

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	
	local eltsList = sequenceUI:find("elements_list")
	assert(eltsList)

	for i=0,eltsList.childrenNb-1 do
		local elementUI = eltsList:getChild(i)
		if r2.logicUI:getEltUIInstId(elementUI) then
			self:updateElementUI(elementUI)
		end
	end
end

----- CLOSE ELEMENT EDITOR ---------------------------------------------
function r2.events:closeElementEditor()
	r2.logicComponents:closeElementEditor(r2.events)
end

------ NEW ELEMENT INST ------------------------------------------
function r2.events:newElementInst()
	
	r2.requestNewAction(i18n.get("uiR2EDNewEventAction"))

	local logicEntity
	if self.filteredLogicEntityId~=nil then
		logicEntity = r2:getInstanceFromId(self.filteredLogicEntityId)
	else
		logicEntity = r2:getSelectedInstance()
	end
	assert(logicEntity)
	
	local logicEvent = r2.newComponent("LogicEntityAction")

	-- default event
	local defaultEvent 
																
	if not self.memberManagement or not logicEntity:isGrouped() then
		local class = r2.Classes[logicEntity.Class]				
		assert(class)											
		if table.getn(class.Events) > 0 then					
			defaultEvent = class.Events[1]						
		else													
			debugInfo("Empty events table of lua class : "..logicEntity.Class)
			return												
		end	
	else
		messageBox(i18n.get("uiR2EDNoEventCreation"))
		return
	end
														
	local eventType = r2.newComponent("EventType")
	eventType.Type = defaultEvent
	logicEvent.Event = eventType
	
	local behavior = self:getBehavior(logicEntity)
	if not self.memberManagement then				--TEMP
			behavior = logicEntity:getBehavior()	--TEMP
	end												--TEMP
	r2.requestInsertNode(behavior.InstanceId, "Actions", -1, "", logicEvent)

	self.ownCreatedInstances[logicEvent.InstanceId] = true

	r2.logicComponents:newElementInst(r2.events)
end

-------- get first event in events list of a class -----------------
function r2.events:hasApplicableActions(logicEntity)

	local class = r2.Classes[logicEntity.Class]
	assert(class)

--	local eventsTable = {}
--	if categoryEvent=="ApplicableActions" then
--		eventsTable = logicEntity:getApplicableActions()
--	else
--		eventsTable = class[categoryEvent]
--	end
	local actionsTable = logicEntity:getApplicableActions()
	
	if table.getn(actionsTable) > 0 then

		if logicEntity:isGrouped() then
			local groupIndependantEvent 
			for k, eventType in pairs(actionsTable) do 
				local menuTitle = r2.getLogicAttribute(logicEntity.Class, "ApplicableActions", eventType)
				if menuTitle.groupIndependant==true then
					groupIndependantEvent = true
					break
				end
			end

			if groupIndependantEvent==nil then
				return nil
			end
		end
	else
		debugInfo("Empty 'ApplicableActions' table of lua class : "..logicEntity.Class)
		return nil
	end

	return true
end

------ NEW ELEMENT UI ------------------------------------------
function r2.events:newElementUI(newInst)
	r2.logicUI:newElementUI(r2.events, newInst, false)
end

------ REMOVE ELEMENT INST ----------------------------------------
function r2.events:removeElementInst()
	r2.requestNewAction(i18n.get("uiR2EDRemoveLogicElementAction"))
	r2.logicComponents:removeElementInst(r2.events)
	r2.requestEndAction()
end

------ REMOVE ELEMENT UI -------------------------------------------
function r2.events:removeElementUI(removedEltUI)
	r2.logicUI:removeElementUI(r2.events, removedEltUI)
end

------ UPDATE ELEMENT TITLE -------------------------------------------
function r2.events:updateElementTitle(eventUI)

	r2.logicComponents:updateElementTitle(r2.events, eventUI, false)
end

------ UPDATE ELEMENT UI -------------------------------------------
function r2.events:updateElementUI(elementUI)

	r2.logicUI:updateElementUI(r2.events, elementUI)

	local logicEntityAction = r2:getInstanceFromId(r2.logicUI:getEltUIInstId(elementUI))    
	assert(logicEntityAction)

	local matchFilter = self:logicEntityActionMatchFilter(logicEntityAction)

	local dismatchFilter = elementUI:find("dismatch_filter")
	assert(dismatchFilter)

	if dismatchFilter.active~=not matchFilter then
		if matchFilter then
			self.dismatchWithFilter = self.dismatchWithFilter-1
		else
			self.dismatchWithFilter = self.dismatchWithFilter+1
		end
	end

	local dismatchFilterMenu = getUI(self.uiId):find("filterMenu"):find("dismatch_filter")
	assert(dismatchFilterMenu)
	dismatchFilterMenu.active = (self.dismatchWithFilter>0)

	dismatchFilter.active = not matchFilter

	-- valid event?
	self:updateValiditySymbol(elementUI, logicEntityAction)
end

-------------------------------------------------------------------------------
function r2.events:updateValiditySymbol(elementUI, eventInst)

	local invalidEvent = elementUI:find("invalid_event")
	assert(invalidEvent)

	local validEvent = elementUI:find("valid_event")
	assert(validEvent)

	local logicAct = eventInst:getLogicAct()
	invalidEvent.active = (logicAct==nil)
	validEvent.active = (logicAct~=nil and logicAct~=r2:getCurrentAct())
	if validEvent.active then
		local uc_other_act = ucstring()
		uc_other_act:fromUtf8(i18n.get("uiR2EDEventNotInCurrentAct"):toUtf8() .. "'" .. logicAct.Name .. "'")
		validEvent.tooltip = uc_other_act
	end
end

function r2.events:logicEntityActionMatchFilter(logicEntityAction)

	local logicEntity = self:getLogicEntityParent(logicEntityAction)
	if not self.memberManagement then							--TEMP
		logicEntity = logicEntityAction:getLogicEntityParent()	--TEMP
	end	
	
	local matchFilter = (logicEntity.InstanceId==self.filteredLogicEntityId)
	if matchFilter then 
		return true 
	end

	for i=0, logicEntityAction.Actions.Size-1 do
		local action = logicEntityAction.Actions[i]
		if action.Entity==self.filteredLogicEntityId then
			return true
		end
	end

	for i=0, logicEntityAction.Conditions.Size-1 do
		local condition = logicEntityAction.Conditions[i]
		if condition.Entity==self.filteredLogicEntityId then
			return true
		end
	end

	return false
end

--- INIT FILTER MENU ---------------------------------------------------
function r2.events:initFilterMenu()

	-- which entity menu initialization
	local logicEntityMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(logicEntityMenu)

	local logicEntityMenu = logicEntityMenu:getRootMenu()
	assert(logicEntityMenu)

	logicEntityMenu:reset()
	
	-- entity classes
	for k, class in pairs(r2.Classes) do
		if class.initLogicEntitiesMenu then class:initLogicEntitiesMenu(logicEntityMenu) end
	end
	logicEntityMenu:setMaxVisibleLine(self.maxVisibleLine)

	-- for each entity classes, list of instances in current act
	for c=0,logicEntityMenu:getNumLine()-1 do

		logicEntityMenu:addSubMenu(c)
		local subMenu = logicEntityMenu:getSubMenu(c)
		
		local entityClass = tostring(logicEntityMenu:getLineId(c))
		local class = r2.Classes[entityClass]

		if class.initLogicEntitiesInstancesMenu then
			class:initLogicEntitiesInstancesMenu(subMenu, "r2.events:filterEvents")
		end
		subMenu:setMaxVisibleLine(self.maxVisibleLine)
	end

	r2.logicUI:openLogicMenu(getUICaller())
end

------ INIT EVENT TYPE MENU -------------------------------------------
function r2.events:initEventMenu(categoryEvent)
	


	-- local startTime = nltime.getPreciseLocalTime()	

	self.actionOrConditionUIId = getUICaller().parent.parent.id

	-- which entity menu initialization
	local logicEntityMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(logicEntityMenu)

	local logicEntityMenu = logicEntityMenu:getRootMenu()
	assert(logicEntityMenu)

	logicEntityMenu:reset()
	
	-- if a logic entity has been already selected, we give a direct access to this one
	local caller = getUI(self.actionOrConditionUIId)
	assert(caller)
	local logicEntity 
	local firstLine = 0
	if caller.Env.InstanceId  ~= nil then
		if categoryEvent=="Events" then
			logicEntity = r2:getInstanceFromId(caller.Env.InstanceId)
		elseif categoryEvent=="ApplicableActions" or categoryEvent=="Conditions" then
			local instance = r2:getInstanceFromId(caller.Env.InstanceId)
			logicEntity = r2:getInstanceFromId(instance.Entity)
		end
	end

	
	--local endTime = nltime.getPreciseLocalTime()
	--debugInfo(string.format("time for 1 is %f", endTime - startTime))
	--startTime = nltime.getPreciseLocalTime()


	if logicEntity then

		firstLine = 2
		local uc_name = ucstring()
		uc_name:fromUtf8(logicEntity.Name)
		logicEntityMenu:addLine(uc_name, "lua", "r2.events:setLogicEntity('".. logicEntity.InstanceId .."')", logicEntity.InstanceId)
		logicEntityMenu:addSeparator()
	
		logicEntityMenu:addSubMenu(0)
		local subMenuEventType = logicEntityMenu:getSubMenu(0)

		logicEntity:initEventTypeMenu(subMenuEventType, categoryEvent)
				
		if logicEntity.initEventValuesMenu then
			logicEntity:initEventValuesMenu(subMenuEventType, categoryEvent)
		end
		subMenuEventType:setMaxVisibleLine(self.maxVisibleLine)
	end

	--local endTime = nltime.getPreciseLocalTime()
	--debugInfo(string.format("time for 2 is %f", endTime - startTime))
	--startTime = nltime.getPreciseLocalTime()

	-- entity classes
	for k, class in pairs(r2.Classes) do
		if class.initLogicEntitiesMenu and table.getn(class[categoryEvent])~=0 then 
			class:initLogicEntitiesMenu(logicEntityMenu, (categoryEvent=="ApplicableActions"))
		end
	end
	logicEntityMenu:setMaxVisibleLine(self.maxVisibleLine)

	--local endTime = nltime.getPreciseLocalTime()
	--debugInfo(string.format("time for 3 is %f", endTime - startTime))
	--startTime = nltime.getPreciseLocalTime()

	-- for each entity classes, list of instances in current act
	for c=firstLine,logicEntityMenu:getNumLine()-1 do

		logicEntityMenu:addSubMenu(c)
		local subMenu = logicEntityMenu:getSubMenu(c)

		local entityClass = tostring(logicEntityMenu:getLineId(c))
		local class = r2.Classes[entityClass]

		if class.initLogicEntitiesInstancesMenu then
			
			class:initLogicEntitiesInstancesMenu(subMenu, "r2.events:setLogicEntity")

			--local endTime = nltime.getPreciseLocalTime()
			--debugInfo(string.format("time for 6 is %f", endTime - startTime))
			--startTime = nltime.getPreciseLocalTime()

			-- for each entity, list of its events type
			for e=0,subMenu:getNumLine()-1 do
			
				local entityId = tostring(subMenu:getLineId(e))
				local entity = r2:getInstanceFromId(entityId)
				if entity then

					subMenu:addSubMenu(e)
					local subMenuEventType = subMenu:getSubMenu(e)
					
					entity:initEventTypeMenu(subMenuEventType, categoryEvent)

					--local endTime = nltime.getPreciseLocalTime()
					--debugInfo(string.format("time for 7 is %f", endTime - startTime))
					--startTime = nltime.getPreciseLocalTime()
				
					if entity.initEventValuesMenu then
						entity:initEventValuesMenu(subMenuEventType, categoryEvent)
					end
	
					--local endTime = nltime.getPreciseLocalTime()
					--debugInfo(string.format("time for 9 is %f", endTime - startTime))
					--startTime = nltime.getPreciseLocalTime()

					subMenuEventType:setMaxVisibleLine(self.maxVisibleLine)

					--local endTime = nltime.getPreciseLocalTime()
					--debugInfo(string.format("time for 8 is %f", endTime - startTime))
					--startTime = nltime.getPreciseLocalTime()
				end
			end
		end
		subMenu:setMaxVisibleLine(self.maxVisibleLine)
	end

	--local endTime = nltime.getPreciseLocalTime()
	--debugInfo(string.format("time for 4 is %f", endTime - startTime))
	--startTime = nltime.getPreciseLocalTime()

	r2.logicUI:openLogicMenu(getUICaller())

	--local endTime = nltime.getPreciseLocalTime()
	--debugInfo(string.format("time for 5 is %f", endTime - startTime))
	--startTime = nltime.getPreciseLocalTime()
	
end

----- SET LOGIC ENTITY (selected in menu ) -------------------------------------------
function r2.events:setLogicEntity(entityId)
	self.selectedLogicEntity = entityId
end

----- SET EVENT TYPE ------------------------------------------------
function r2.events:setEventType(eventType, endRequest, categoryEvent)

	self.selectedEventType = eventType
	
	if endRequest==tostring(true) then
		if categoryEvent=="Events" then self:setEvent("")
		elseif categoryEvent=="ApplicableActions" then self:setAction("")
		elseif categoryEvent=="Conditions" then self:setCondition("")
		end
	end
end

----- SET EVENT VALUE ------------------------------------------------
function r2.events:setEventValue(instanceId, categoryEvent, optionalValueString)

	if categoryEvent=="Events" then self:setEvent(instanceId, optionalValueString)
	elseif categoryEvent=="ApplicableActions" then self:setAction(instanceId, optionalValueString)
	elseif categoryEvent=="Conditions" then self:setCondition(instanceId, optionalValueString)
	end
end

---- SET EVENT -----------------------------------------------------------------------
function r2.events:setEvent(valueInstId, optionalValueString)

	r2.requestNewAction(i18n.get("uiR2EDSetEventAction"))

	local eventInst = self:currentEltInst()
	assert(eventInst)

	local oldLogicEntity = self:getLogicEntityParent(eventInst) 
	if not self.memberManagement then
		oldLogicEntity = eventInst:getLogicEntityParent() 
	end
	assert(oldLogicEntity)

	local newLogicEntity = r2:getInstanceFromId(self.selectedLogicEntity)
	assert(newLogicEntity)

	if oldLogicEntity~=newLogicEntity then

		local index = r2.logicComponents:searchElementIndex(eventInst)
		index = index - 1
		if index >= 0 then
			local oldBehavior = self:getBehavior(oldLogicEntity)
			local newBehavior = self:getBehavior(newLogicEntity)
			if not self.memberManagement then				--TEMP
				oldBehavior = oldLogicEntity:getBehavior()	--TEMP
				newBehavior = newLogicEntity:getBehavior()	--TEMP
			end
			r2.requestMoveNode(oldBehavior.InstanceId, "Actions", index,
				newBehavior.InstanceId, "Actions", -1)
		end
	end

	r2.requestSetNode(eventInst.Event.InstanceId, "Type", self.selectedEventType)

	r2.requestSetNode(eventInst.Event.InstanceId, "Value", r2.RefId(valueInstId))
	if optionalValueString then 
		r2.requestSetNode(eventInst.Event.InstanceId, "ValueString", optionalValueString)
	elseif eventInst.ValueString and eventInst.ValueString ~="" then 
		r2.requestSetNode(eventInst.Event.InstanceId, "ValueString", optionalValueString) 
	end

	
	self.selectedLogicEntity = nil
	self.selectedEventType = nil
end

------ SET ACTION  ------------------------------------------------------
function r2.events:setAction(valueInstId, optionalValueString)

	r2.requestNewAction(i18n.get("uiR2EDSetEventActionAction"))

	local eventInst = self:currentEltInst()
	assert(eventInst)

	local actionLogicEntity = r2:getInstanceFromId(self.selectedLogicEntity)
	assert(actionLogicEntity)

	local caller = getUI(self.actionOrConditionUIId)
	assert(caller)

	local action 
	if caller.Env.InstanceId == nil then
		action = r2.newComponent("ActionStep")
		r2.requestInsertNode(eventInst.InstanceId, "Actions",-1,"", action)
		caller.Env.InstanceId = action.InstanceId
	else
		action = r2:getInstanceFromId(caller.Env.InstanceId)
	end
	assert(action)

	r2.requestSetNode(action.InstanceId, "Entity", r2.RefId(self.selectedLogicEntity))
	r2.requestSetNode(action.Action.InstanceId, "Type", self.selectedEventType)
        r2.requestSetNode(action.Action.InstanceId, "Value", r2.RefId(valueInstId))	
	if optionalValueString then 
		r2.requestSetNode(action.Action.InstanceId, "ValueString", optionalValueString)
	elseif action.Action.ValueString and action.Action.ValueString ~= "" then 
		r2.requestSetNode(action.Action.InstanceId, "ValueString", "") 
	end

	
	self.selectedLogicEntity = nil
	self.selectedEventType = nil
	self.actionOrConditionUIId = nil
end

------ SET CONDITION  ------------------------------------------------------
function r2.events:setCondition(valueInstId)

	r2.requestNewAction(i18n.get("uiR2EDSetEventConditionAction"))

	local eventInst = self:currentEltInst()
	assert(eventInst)

	local actionLogicEntity = r2:getInstanceFromId(self.selectedLogicEntity)
	assert(actionLogicEntity)

	local caller = getUI(self.actionOrConditionUIId)
	assert(caller)

	local condition 
	if caller.Env.InstanceId == nil then
		condition = r2.newComponent("ConditionStep")
		r2.requestInsertNode(eventInst.InstanceId, "Conditions",-1,"", condition)
		caller.Env.InstanceId = condition.InstanceId
	else
		condition = r2:getInstanceFromId(caller.Env.InstanceId)
	end
	assert(condition)

	r2.requestSetNode(condition.InstanceId, "Entity", r2.RefId(self.selectedLogicEntity))
	r2.requestSetNode(condition.Condition.InstanceId, "Type", self.selectedEventType)
	
	r2.requestSetNode(condition.Condition.InstanceId, "Value", r2.RefId(valueInstId))
	
	self.selectedLogicEntity = nil
	self.selectedEventType = nil
	self.actionOrConditionUIId = nil
end

---- EDITOR ELEMENT IN ADVANCED OR EASY MODE
function r2.events:advancedEditor(advancedMode) 

	local instanceEvent = self:currentEltInst()
	if instanceEvent==nil then return end

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	
	local eventEditor = sequenceUI:find("edit_element")
	assert(eventEditor)

	local addActionButton = eventEditor:find("what_happens"):find("add")
	assert(addActionButton)
	addActionButton.active = advancedMode

	local conditionsGr = eventEditor:find("conditions")
	assert(conditionsGr)
	conditionsGr.active = advancedMode

	local addConditionButton = conditionsGr:find("add")
	assert(addConditionButton)
	addConditionButton.active = advancedMode

	local advancedButton = eventEditor:find("advanced")
	advancedButton.active = not advancedMode

	local conditionsList = eventEditor:find("conditions_gr")
	assert(conditionsList)
	conditionsList.active = (instanceEvent.Conditions.Size > 0)
end

-- NEW ACTION -------------------------------------------------
function r2.events:newActionUI()

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	
	local eventEditor = sequenceUI:find("edit_element")
	assert(eventEditor)

	local actionsList = eventEditor:find("actions_list")
	assert(actionsList)

	local idActionUI = "action" .. actionsList.childrenNb
	
	local params = {
						id=idActionUI, x="0", y="0", sizeref="w", 
						posparent="", posref="TL TL", w="0", h="20", 
						params_l="r2.events:initEventMenu('ApplicableActions')",
						remove_menu="r2.events:removeActionInst()", params_over="r2.events:activeTrashButton('true')",
					}
	

	local newActionUI = createGroupInstance("menu_trash_template", actionsList.id, params)
	assert(newActionUI)
	actionsList:addChild(newActionUI)
	actionsList.parent:updateCoords()

	newActionUI.Env.InstanceId = nil
	
	return 	newActionUI	
end

-- NEW CONDITION -------------------------------------------------
function r2.events:newConditionUI()

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	
	local eventEditor = sequenceUI:find("edit_element")
	assert(eventEditor)

	local conditionsGr = eventEditor:find("conditions_gr")
	assert(conditionsGr)
	conditionsGr.active = true

	local conditionsList = eventEditor:find("conditions_list")
	assert(conditionsList)

	local idConditionUI = "condition" .. conditionsList.childrenNb
	
	local params = {
						id=idConditionUI, x="0", y="0", sizeref="w", 
						posparent="", posref="TL TL", w="0", h="20", 
						params_l="r2.events:initEventMenu('Conditions')",
						remove_menu="r2.events:removeConditionInst()", params_over="r2.events:activeTrashButton('false')",
					}
	

	local newConditionUI = createGroupInstance("menu_trash_template", conditionsList.id, params)
	assert(newConditionUI)
	conditionsList:addChild(newConditionUI)
	conditionsList.parent:updateCoords()

	newConditionUI.Env.InstanceId = nil

	return newConditionUI
end


--- ACTIVE TRASH BUTTON ----------------------------------------------------------
function r2.events:activeTrashButton(actionsList)

	local overMenuTrash = getUICaller().parent.parent
	assert(overMenuTrash)

	local trashButtonCaller = overMenuTrash:find("remove_menu")
	assert(trashButtonCaller)

	local menuTrashList = overMenuTrash.parent
	assert(menuTrashList)

	if actionsList==tostring(true) and menuTrashList.childrenNb==1 then return end

	for i=0, menuTrashList.childrenNb-1 do
		local menuTrash = menuTrashList:getChild(i)
		assert(menuTrash)

		local trashButton = menuTrash:find("remove_menu")
		assert(trashButton)

		if trashButton.active then
			if trashButton.id == trashButtonCaller.id then return end
			trashButton.active = false
			break
		end
	end

	trashButtonCaller.active = true
end


--- REMOVE CONDITION INSTANCE -----------------------------------------------------
function r2.events:removeConditionInst()

	r2.requestNewAction(i18n.get("uiR2EDRemoveEventConditionAction"))

	local menuTrash = getUICaller().parent.parent.parent.parent
	assert(menuTrash)

	local menuTrashList = menuTrash.parent
	assert(menuTrashList)

	if menuTrash.Env.InstanceId~=nil then
		r2.requestEraseNode(menuTrash.Env.InstanceId, "", -1)	
	else
		menuTrashList:delChild(menuTrash)	
	end
end

--- REMOVE ACTION INSTANCE -----------------------------------------------------
function r2.events:removeActionInst()

	r2.requestNewAction(i18n.get("uiR2EDRemoveEventActionAction"))

	local menuTrash = getUICaller().parent.parent.parent.parent
	assert(menuTrash)

	local menuTrashList = menuTrash.parent
	assert(menuTrashList)

	if menuTrash.Env.InstanceId~=nil then
		r2.requestEraseNode(menuTrash.Env.InstanceId, "", -1)	
	else
		menuTrashList:delChild(menuTrash)	
	end
end








--------------------------------------------------------------------------------------------------
-------------------------------- LOGIC ENTITY DisplayerProperties -------------------------
--------------------------------------------------------------------------------------------------
local logicEntityPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function logicEntityPropertySheetDisplayerTable:onPostCreate(instance)
end
------------------------------------------------
function logicEntityPropertySheetDisplayerTable:onErase(instance)

	local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, instance)
	
	if elementUI then
		r2.events:removeElementUI(elementUI)
	end
end
------------------------------------------------
function logicEntityPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function logicEntityPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function logicEntityPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function logicEntityPropertySheetDisplayerTable:onSelect(instance, isSelected)
	
	if not isSelected then
		r2.events:closeEditor()	
	end
end

------------------------------------------------
local oldOnAttrModified = logicEntityPropertySheetDisplayerTable.onAttrModified
function logicEntityPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	

	if attributeName == "Name" then
		local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, instance)
	
		if elementUI then
			r2.events:updateElementUI(elementUI)
		end
	end
   oldOnAttrModified(self, instance, attributeName)
end	

------------------------------------------------
function r2:logicEntityPropertySheetDisplayer()	
	return logicEntityPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- LOGIC ENTITY ACTION DisplayerProperties -------------------------
--------------------------------------------------------------------------------------------------
local logicEntityActionPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onPostCreate(instance)

	local filterIsLogicEntityParentB = r2.events:filterIsLogicEntityParent(instance)
	if not r2.events.memberManagement then
		filterIsLogicEntityParentB = (instance:getLogicEntityParent().InstanceId==r2.events.filteredLogicEntityId)
	end
	if filterIsLogicEntityParentB then
		r2.events:newElementUI(instance)
	end
end
------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onErase(instance)

	instance.User.Deleted = true

	local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, instance)
	
	if elementUI then
		r2.events:removeElementUI(elementUI)
	end
end
------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onPreHrcMove(instance)
end

------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onPostHrcMove(instance)	
	
	local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, instance)
	
	local filterIsLogicEntityParentB = r2.events:filterIsLogicEntityParent(instance)
	if not r2.events.memberManagement then
		filterIsLogicEntityParentB = (instance:getLogicEntityParent().InstanceId==r2.events.filteredLogicEntityId)
	end

	if elementUI then
		r2.events:updateElementUI(elementUI)
	elseif filterIsLogicEntityParentB then
		r2.events:newElementUI(instance)
	end
end
------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onFocus(instance, hasFocus)	
end

------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function logicEntityActionPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	

	local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, instance)
	
	if elementUI then
		r2.events:updateElementUI(elementUI)
	end
end	

------------------------------------------------
function r2:logicEntityActionPropertySheetDisplayer()	
	return logicEntityActionPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end


--------------------------------------------------------------------------------------------------
-------------------------------- EVENT TYPE DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local eventTypePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function eventTypePropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function eventTypePropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function eventTypePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function eventTypePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function eventTypePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function eventTypePropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function eventTypePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
end	

------------------------------------------------
function eventTypePropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	r2.requestEraseNode(instance.Parent.InstanceId, "", -1)
end
------------------------------------------------
function eventTypePropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
	if targetAttrName == "Name" then
		
		local eventInst = instance.Parent
		assert(eventInst.Class=="LogicEntityAction")
		local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, eventInst)
	
		if elementUI then
			r2.events:updateElementUI(elementUI)
		end
	end
end

------------------------------------------------
function r2:eventTypePropertySheetDisplayer()	
	return eventTypePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- ACTION STEP DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local actionStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function actionStepPropertySheetDisplayerTable:onPostCreate(instance)
end
------------------------------------------------
function actionStepPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function actionStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function actionStepPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function actionStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function actionStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function actionStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)		
end	

------------------------------------------------
function actionStepPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	
	if refIdName == "Entity" then
		r2.requestEraseNode(instance.InstanceId, "", -1)
	end
end
------------------------------------------------
function actionStepPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
	if targetAttrName == "Name" then
		
		local eventInst = instance.Parent.Parent
		assert(eventInst.Class=="LogicEntityAction")
		local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, eventInst)

		if elementUI then
			r2.events:updateElementUI(elementUI)
		end
	end
end

------------------------------------------------
function actionStepPropertySheetDisplayerTable:onTargetInstancePostHrcMove(instance, refIdName, refIdIndexInArray)

	if refIdName=="Entity" then
		r2.requestSetNode(instance.InstanceId, "Entity", r2.RefId(""))
		r2.requestSetNode(instance.Action.InstanceId, "Type", "")
		r2.requestSetNode(instance.Action.InstanceId, "Value", r2.RefId(""))
	end
end


------------------------------------------------
function r2:actionStepPropertySheetDisplayer()	
	return actionStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- ACTION TYPE DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local actionTypePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function actionTypePropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function actionTypePropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function actionTypePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function actionTypePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function actionTypePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function actionTypePropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function actionTypePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
end	

------------------------------------------------
function actionTypePropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	r2.requestSetNode(instance.InstanceId, "Type", "")
	r2.requestSetNode(instance.InstanceId, "Value", r2.RefId(""))
end
------------------------------------------------
function actionTypePropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
	if targetAttrName == "Name" then
		
		local eventInst = instance.Parent.Parent.Parent
		assert(eventInst.Class=="LogicEntityAction")
		local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, eventInst)

		if elementUI then
			r2.events:updateElementUI(elementUI)
		end
	end
end

------------------------------------------------
function r2:actionTypePropertySheetDisplayer()	
	return actionTypePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- CONDITION STEP DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local conditionStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onPostCreate(instance)
end
------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
end	

------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	
	if refIdName == "Entity" then
		r2.requestEraseNode(instance.InstanceId, "", -1)
	end
end
------------------------------------------------
function conditionStepPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
	if targetAttrName == "Name" then
		
		local eventInst = instance.Parent.Parent
		assert(eventInst.Class=="LogicEntityAction")
		local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, eventInst)

		if elementUI then
			r2.events:updateElementUI(elementUI)
		end
	end
end

------------------------------------------------
function r2:conditionStepPropertySheetDisplayer()	
	return conditionStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end




--------------------------------------------------------------------------------------------------
-------------------------------- CONDITION TYPE DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local conditionTypePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
end	

------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	r2.requestSetNode(instance.InstanceId, "Type", "")
	r2.requestSetNode(instance.InstanceId, "Value", r2.RefId(""))
end
------------------------------------------------
function conditionTypePropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
	if targetAttrName == "Name" then
		
		local eventInst = instance.Parent.Parent.Parent
		assert(eventInst.Class=="LogicEntityAction")
		local elementUI = r2.logicUI:findElementUIFromInstance(r2.events, eventInst)

		if elementUI then
			r2.events:updateElementUI(elementUI)
		end
	end
end

------------------------------------------------
function r2:conditionTypePropertySheetDisplayer()	
	return conditionTypePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end






