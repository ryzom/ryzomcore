r2.logicEntityUIPath = "r2ed_logic_entities:content:menu:"

-- id of button which has just activated "which entity" menu
r2.whichEntitySelectedButtonId = nil

-- id of button which has just activated "right" menu
r2.rightSelectedButtonId = nil

-- table of buttons which can activate "which entity" menu
-- id of a such button give the method which must be called
r2.whichEntityButtons = {}

r2.logicEntityAttributes = {


	["ApplicableActions"] = {	
--								["Activate"]					= tostring(i18n.get( "uiR2EdActivate"				):toUtf8()), 
--								["Deactivate"]					= tostring(i18n.get( "uiR2EdDeactivate"				):toUtf8()), 
--								["Wander"]						= tostring(i18n.get( "uiR2EdWander"					):toUtf8()), 
--								["Sit Down"]					= tostring(i18n.get( "uiR2EdSitDown"				):toUtf8()),
--								["Stand Up"]					= tostring(i18n.get( "uiR2EdStandUp"				):toUtf8()),
--								["Respawn"]						= tostring(i18n.get( "uiR2EdRespawn"				):toUtf8()), 
--								["Kill"]						= tostring(i18n.get( "uiR2EdKill"					):toUtf8()),
--								["begin activity sequence"]		= tostring(i18n.get( "uiR2EdBeginActivitySequ"		):toUtf8()),
--								["begin chat sequence"]			= tostring(i18n.get( "uiR2EdBeginChatSequ"			))
							},

	["Events"] =			{	
--								["activation"]					= tostring(i18n.get( "uiR2EdActivation"				):toUtf8()), 
--								["desactivation"]				= tostring(i18n.get( "uiR2EdDesactivation"			):toUtf8()), 
--								["a member death"]				= tostring(i18n.get( "uiR2EdAMemberDeath"			):toUtf8()),
--								["member death"]				= tostring(i18n.get( "uiR2EdMemberDeath"			):toUtf8()),
--								["group death"]					= tostring(i18n.get( "uiR2EdGroupDeath"				):toUtf8()), 	
--								["head to wander zone"]			= tostring(i18n.get( "uiR2EdHeadWanderZone"			):toUtf8()), 
--								["arrive at wander zone"]		= tostring(i18n.get( "uiR2EdArriveWanderZone"		):toUtf8()), 
--								["head to camp"]				= tostring(i18n.get( "uiR2EdHeadCamp"				):toUtf8()), 
--								["arrive at camp"]				= tostring(i18n.get( "uiR2EdArriveCamp"				):toUtf8()),
--								["death"]						= tostring(i18n.get( "uiR2EdDeath"					):toUtf8()),
--								["end of activity step"]		= tostring(i18n.get( "uiR2EdEndActivityStep"		):toUtf8()), 
--								["begin of activity step"]		= tostring(i18n.get( "uiR2EdBeginActivityStep"		):toUtf8()),
--								["end of activity sequence"]	= tostring(i18n.get( "uiR2EdEndActivitySequ"		):toUtf8()),
--								["begin of activity sequence"]	= tostring(i18n.get( "uiR2EdBeginOfActivitySequ"	):toUtf8()),

--								["end of chat step"]			= tostring(i18n.get( "uiR2EdEndChatStep"			):toUtf8()), 
--								["end of chat sequence"]		= tostring(i18n.get( "uiR2EdEndChatSequ"			):toUtf8()),
--								["group death"]					= tostring(i18n.get( "uiR2EdGroupDeath"				):toUtf8()), 
							},


	["Conditions"] =		{	
--								["is active"]					= tostring(i18n.get( "uiR2EdIsActive"				):toUtf8()), 
--								["is inactive"]					= tostring(i18n.get( "uiR2EdIsInactive"				):toUtf8()), 
--								["is dead"]						= tostring(i18n.get( "uiR2EdIsDead"					):toUtf8()), 
--								["is alive"]					= tostring(i18n.get( "uiR2EdIsAlive"				):toUtf8()),
--								["is wandering"]				= tostring(i18n.get( "uiR2EdIsWandering"			):toUtf8()), 
--								["is sitting"]					= tostring(i18n.get( "uiR2EdIsSitting"				):toUtf8()), 
--								["is heading to wander zone"]	= tostring(i18n.get( "uiR2EdIsHeadingWanderZone"	):toUtf8()), 
--								["is heading to camp"]			= tostring(i18n.get( "uiR2EdIsHeadingCamp"			):toUtf8()),
--								["is in activity sequence"]		= tostring(i18n.get( "uiR2EdIsInActivitySequ"		):toUtf8()),
--								["is in activity step"]			= tostring(i18n.get( "uiR2EdIsInActivityStep"		):toUtf8()), 
--								["is in chat sequence"]			= tostring(i18n.get( "uiR2EdIsInChatSequ"			):toUtf8()), 
--								["is in chat step"]				= tostring(i18n.get( "uiR2EdIsInChatStep"			):toUtf8()),
							},
}
			
------------------------------------------------
-- private: implement mergeComponentsLogicEntityAttributes
function r2:registerComponentsLogicEntityAttributes(localLogicEntityAttributes)	
		local k, v = next(localLogicEntityAttributes, nil)
		while k do
			local k2, v2 = next(v, nil)
			while k2 do
				if not r2.logicEntityAttributes[k][k2] then
					r2.logicEntityAttributes[k][k2] = v2
				end
				k2, v2 = next(v, k2)
			end
			k, v = next(localLogicEntityAttributes, k)
		end
	end

------------------------------------------------
--
function r2:mergeComponentsLogicEntityAttributes()	
	local k, v = next(r2.Classes, nil)
	while k do
		if v.getLogicTranslations then
			local localLogicEntityAttributes = v:getLogicTranslations()
			if localLogicEntityAttributes then
				r2:registerComponentsLogicEntityAttributes(localLogicEntityAttributes)
			else
				debugInfo(colorTag(255,0,0).."No Translation for component "..k)
			end
		end
		k, v = next(r2.Classes, k)
	end
end




------------------------------------------- INITIALISATION ------------------------------
-- global initialization
function r2:initLogicEntityEditor()
	r2:initActionEditor()
	r2:initReactionEditor()
end

------------------ INIT EVENTS EDITOR ----------------------------------------------------------------
function r2:initEventsEditor()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	-- create action editor
	local actions = logicEntityUI:find("actions")
	assert(actions)

	local listActions = actions:find("elements_list")
	assert(listActions)

	local newEditorElt = createGroupInstance("template_edit_action", listActions.id, {id="edit_element", active="false"})
	assert(newEditorElt)
	listActions:addChild(newEditorElt)
	listActions.parent:updateCoords()

	newEditorElt.active = false
	newEditorElt.Env.isEditor = true

	-- add target scroll
	local actionsList = newEditorElt:find("actions_list")
	assert(actionsList)
	local conditionsList = newEditorElt:find("conditions_list")
	assert(conditionsList)

	local menusList = actionsList:find("menus_list")
	assert(menusList)
	local scroll = actionsList:find("scroll_bar")
	assert(scroll)
	scroll:setTarget(menusList.id)

	menusList = conditionsList:find("menus_list")
	assert(menusList)
	scroll = conditionsList:find("scroll_bar")
	assert(scroll)
	scroll:setTarget(menusList.id)

	-- create reaction editor
	local reactions = logicEntityUI:find("reactions")
	assert(reactions)

	local listReactions = reactions:find("elements_list")
	assert(listReactions)

	newEditorElt = createGroupInstance("template_edit_reaction", listReactions.id, {id="edit_element", active="false"})
	assert(newEditorElt)
	listReactions:addChild(newEditorElt)
	listReactions.parent:updateCoords()
	
	newEditorElt.active = false
	newEditorElt.Env.isEditor = true

	-- add target scroll
	actionsList = newEditorElt:find("actions_list")
	assert(actionsList)
	conditionsList = newEditorElt:find("conditions_list")
	assert(conditionsList)

	local menusList = actionsList:find("menus_list")
	assert(menusList)
	local scroll = actionsList:find("scroll_bar")
	assert(scroll)
	scroll:setTarget(menusList.id)

	menusList = conditionsList:find("menus_list")
	assert(menusList)
	scroll = conditionsList:find("scroll_bar")
	assert(scroll)
	scroll:setTarget(menusList.id)
end

function r2:getActionEditor()
	return r2:getElementEditor("actions")
end

function r2:getReactionEditor()
	return r2:getElementEditor("reactions")
end

function r2:getElementEditor(tabName)

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local actions = logicEntityUI:find(tabName)
	assert(actions)

	local listActions = actions:find("elements_list")
	assert(listActions)

	return listActions:find("edit_element")
end
 
function r2:initActionEditor()

	--local actionsEditor = getUI("ui:interface:r2ed_actions")
	local actionsEditor = r2:getActionEditor()
	assert(actionsEditor)

	-- Action editor / Actions / Which entity
	local whichEntityButton = actionsEditor:find("actions_list"):find("menus_list"):getChild(0):find("left_menu"):find("select")
	assert(whichEntityButton)
	r2.whichEntityButtons[whichEntityButton.id] = {paramsLineLeft=r2.actionWhichEntity, 
		paramsLeft="r2:openWhichEntityMenu()",
		paramsRight="r2:openRightMenu(true, 'ApplicableActions', 'r2:actionWhatAction')",
		removePair="r2:removeActionStep(true)", paramsOver="r2:activeTrashButton()"}

	-- Action editor / Extra conditions / Which entity
	whichEntityButton = actionsEditor:find("conditions_list"):find("menus_list"):getChild(0):find("left_menu"):find("select")
	assert(whichEntityButton)
	r2.whichEntityButtons[whichEntityButton.id] = {paramsLineLeft=r2.actionConditionWhichEntity, 
		paramsLeft="r2:openWhichEntityMenu()",
		paramsRight="r2:openRightMenu(true, 'Conditions', 'r2:actionWhatCondition')",
		removePair="r2:removeActionConditionStep()", paramsOver="r2:activeTrashButton()"}
end

function r2:initReactionEditor()

	--local reactionsEditor = getUI("ui:interface:r2ed_reactions")
	local reactionsEditor = r2:getReactionEditor()
	assert(reactionsEditor)

	-- Reaction editor / What triggers this reaction / Which entity
	local whichEntityButton = reactionsEditor:find("triggers"):find("combos"):find("left_menu"):find("select")
	assert(whichEntityButton)
	r2.whichEntityButtons[whichEntityButton.id] = {paramsLineLeft=r2.reactionWhatTriggersWhichEntity, 
		paramsLeft="r2:openWhichEntityMenu()",
		paramsRight="r2:openRightMenu(true, 'Events', 'r2:reactionWhichEvent')"}

	-- Rection editor / Actions / Which entity
	local whichEntityButton = reactionsEditor:find("actions_list"):find("menus_list"):getChild(0):find("left_menu"):find("select")
	assert(whichEntityButton)
	r2.whichEntityButtons[whichEntityButton.id] = {paramsLineLeft=r2.reactionActionWhichEntity, 
		paramsLeft="r2:openWhichEntityMenu()",
		paramsRight="r2:openRightMenu(true, 'ApplicableActions', 'r2:reactionWhatAction')",
		removePair="r2:removeActionStep(false)", paramsOver="r2:activeTrashButton()"}

	-- Reaction editor / Extra conditions / Which entity
	whichEntityButton = reactionsEditor:find("conditions_list"):find("menus_list"):getChild(0):find("left_menu"):find("select")
	assert(whichEntityButton)
	r2.whichEntityButtons[whichEntityButton.id] = {paramsLineLeft=r2.reactionConditionWhichEntity, 
		paramsLeft="r2:openWhichEntityMenu()",
		paramsRight="r2:openRightMenu(true, 'Conditions', 'r2:reactionWhatCondition')",
		removePair="r2:removeReactionConditionStep()", paramsOver="r2:activeTrashButton()"}
end

-----------------------------------------------------------------------------------------------------------
------------------------------------ FIRST MENU -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-- when a "which entity" button is pushed, the "which entity" menu is open
function r2:openWhichEntityMenu()

	r2.whichEntitySelectedButtonId = getUICaller().id
	r2:initWhichEntityMenu()
	r2:openLogicEntityMenu(getUICaller())
end


----------------------------------------------------------------------------------------------------------
-- register features to menu LogicEntityMenu
function r2:registerFeaturesMenu(logicEntityMenu)
	do		
		local k,v = next(r2.Classes, nil)
		while k do
			if  v.initLogicEntitiesMenu then v:initLogicEntitiesMenu(logicEntityMenu) end
			k,v = next(r2.Classes, k)
		end
	end

end


-----------------------------------------------------------------------------------------------------------
-- returns a list of instance of all act of the scenario (not only the base act and the current act)
function r2:getAllActInstances()
	local ret = {}
	if not r2.Scenario then return ret end

	local base = true
	local k, v = next(r2.Scenario.Acts, nil)
	while k do
		if  base then
			base = false
		else
			table.insert(ret, v)
		end
		k, v = next(r2.Scenario.Acts, k)
	end
	return ret
end

-----------------------------------------------------------------------------------------------------------
-- "which entity" menu initialization, called when "which menu" is open
function r2:initWhichEntityMenu()

	-- which entity menu initialization
	local logicEntityMenu = getUI("ui:interface:r2ed_logic_entity_menu")
	assert(logicEntityMenu)

	local logicEntityMenu = logicEntityMenu:getRootMenu()
	assert(logicEntityMenu)

	logicEntityMenu:reset()

	local name = i18n.get("uiR2EdBanditCamps")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "BanditCampFeature")

	name = i18n.get("uiR2EdGroups")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "NpcGrpFeature")

	name = i18n.get("uiR2EdNPCs")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "Npc")

	r2:registerFeaturesMenu(logicEntityMenu)

	for i=0,logicEntityMenu:getNumLine()-1 do
		local lineId = tostring(logicEntityMenu:getLineId(i))
		local entitiesTable = nil
		if lineId == "Act" then
			entitiesTable = r2:getAllActInstances()
		else
			entitiesTable = r2.Scenario:getAllInstancesByType(lineId)
		end

		logicEntityMenu:addSubMenu(i)
		local subMenu = logicEntityMenu:getSubMenu(i)

		local count = 0
		for key, entity in pairs(entitiesTable) do
			local addLine = true
			if entity:isKindOf("Act") and (entity:isBaseAct()) then
				addLine = false
			end
			if entity:isKindOf("Npc") and (entity:isBotObject() or entity:isPlant()) then
				addLine = false
			end
			if addLine then
				subMenu:addLine(ucstring(entity.Name), "lua", "r2:selectWhichEntity(".. tostring(i) .. "," .. tostring(count) .. ")", entity.InstanceId)
				count=count+1
			end
		end

		if count == 0 then
			subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
		end
	end
end

-----------------------------------------------------------------------------------------------------------
-- the commun "logic entity" menu is open
function r2:openLogicEntityMenu(caller)

	local menuName = "ui:interface:r2ed_logic_entity_menu"
	launchContextMenuInGame(menuName)
	local menu = getUI(menuName)

	menu:updateCoords()	
	menu.y = caller.y_real - (menu.h - caller.h_real)
	menu.x = caller.x_real
	menu:updateCoords()	
end

-----------------------------------------------------------------------------------------------------------
-- a logical entity is selected in the "which entity" menu
-- the correct method is called in function of the "which entity" button which has been pushed previously
function r2:selectWhichEntity(subMenuIndex, lineIndex)

	local whichEntityMenu = getUI("ui:interface:r2ed_logic_entity_menu")
	assert(whichEntityMenu)

	whichEntityMenu = whichEntityMenu:getRootMenu()

	local whichEntitySubMenu = whichEntityMenu:getSubMenu(subMenuIndex)
	assert(whichEntitySubMenu)

	local entityId = tostring(whichEntitySubMenu:getLineId(lineIndex))

	local entityInstance = r2:getInstanceFromId(entityId)
	assert(entityInstance)

	local selectButton = getUI(r2.whichEntitySelectedButtonId)
	assert(selectButton)

	-- necessary data when right menu will be selected
	selectButton.parent.parent.Env.oldEntityId = selectButton.parent.parent.Env.entityId
	selectButton.parent.parent.Env.entityId = entityId

	-- update correct property
	local updateWhichEntity = r2.whichEntityButtons[r2.whichEntitySelectedButtonId].paramsLineLeft
	updateWhichEntity(entityInstance)
end


function r2:createNewMenuPairAndComponent(newComponentName)

	local leftButton = getUI(r2.whichEntitySelectedButtonId)
	local buttonParams = r2.whichEntityButtons[r2.whichEntitySelectedButtonId]

	local listMenu = leftButton.parent.parent.parent.parent
	assert(listMenu)

	-- the pushed "which entity" button is in a list
	-- if it's the last child of this list, a new pair of buttons must be added at the end
	if listMenu:getChild(listMenu.childrenNb-1).id == leftButton.parent.parent.parent.id then 

		-- create new menu pair template
		r2:createNewMenuPair(listMenu, buttonParams)

		-- create new lua component for menus whose left button has been pushed
		return r2.newComponent(newComponentName)
	end
	return nil
end

function r2:createNewMenuPair(listMenu, buttonParams)

	local newId = "menu_pair"..(listMenu.childrenNb+1)

	-- create new UI element
	local newButtonPair = createGroupInstance("two_menu_trash_template", listMenu.id, 
		{id=newId, params_left=buttonParams.paramsLeft, params_right=buttonParams.paramsRight,
		 remove_pair=buttonParams.removePair, col_pushed="255 255 255 255",
		 params_over=buttonParams.paramsOver})

	listMenu:addChild(newButtonPair)
	listMenu.parent:updateCoords()

	local newLeftButton = newButtonPair:find("left_menu"):find("select")
	assert(newLeftButton)

	r2.whichEntityButtons[newLeftButton.id] = {paramsLineLeft=buttonParams.paramsLineLeft, 
		paramsLeft=buttonParams.paramsLeft, paramsRight=buttonParams.paramsRight,
		removePair=buttonParams.removePair, paramsOver=buttonParams.paramsOver}

	return newButtonPair
end

-----------------------------------------------------------------------------------------------------------
-- update an action text
function r2:updateActionText(actionUI)

	if actionUI == nil then
		actionUI = r2:getSelectedEltUI(r2.logicEntityUIPath.."actions")
	end
	assert(actionUI)

	local actionInst = r2:getInstanceFromId(actionUI.Env.elementId)

	local actionText = actionUI:find("text_list")
	assert(actionText)

	actionText:clear()

	if actionInst ~= nil then

		local sep = actionUI:find("sep")
		assert(sep)

		if actionInst.Actions.Size > 0 then
			actionText:addColoredTextChild("\n"..tostring(i18n.get("uiR2EdActionStepList")), 250, 200, 0, 255)
			
			local totalText = ""
			
			for a = 0, actionInst.Actions.Size - 1 do
				local actionStep = actionInst.Actions[a]
				assert(actionStep)

				local entity = r2:getInstanceFromId(actionStep.Entity)
				local actionType = actionStep.Action
				
				if entity then
					totalText = totalText..entity.Name
					totalText = totalText .. " : " 
					
					if actionType.Type ~= "" then
						local textValue = r2.logicEntityAttributes["ApplicableActions"][actionType.Type]
						if textValue == nil then textValue = "?".. actionType.Type.."?" end
						totalText = totalText .. textValue
						if actionType.Value ~= "" then
							local inst = r2:getInstanceFromId(actionType.Value)
							if inst ~= nil then
								totalText = totalText .. " " .. inst.Name
							end
						end
					else
						totalText = totalText .. "..."
					end
				end

				totalText = totalText.."\n"
			end

			actionText:addTextChild(ucstring(totalText))
			sep.active = true
		else
			sep.active = false
		end
	end

	-- update titel
	r2:buildActionTitle(actionUI, false)

	local selectedAction = r2:getSelectedEltInstId(r2.logicEntityUIPath.."actions")
	if selectedAction and selectedAction == actionInst.InstanceId then
		r2:updateActionEditor()
	end
end

-----------------------------------------------------------------------------------------------------------
-- update a reaction text
function r2:updateReactionText(reactionUI)

	if reactionUI == nil then
		reactionUI = r2:getSelectedEltUI(r2.logicEntityUIPath.."reactions")
	end
	assert(reactionUI)

	local reactionInst = r2:getInstanceFromId(reactionUI.Env.elementId)
	assert(reactionInst)

	local actionInst = r2:getInstanceFromId(reactionInst.LogicEntityAction)

	local reactionText = reactionUI:find("text_list")
	assert(reactionText)

	local sep = reactionUI:find("sep")
	assert(sep)

	reactionText:clear()

	if actionInst ~= nil then

		if actionInst.Actions.Size > 1 then

			reactionText:addColoredTextChild("\n"..tostring(i18n.get("uiR2EdOtherActionStepList")), 170, 95, 235, 255)
			local totalText = ""

			for a = 0, actionInst.Actions.Size - 1 do
				local actionStep = actionInst.Actions[a]
				assert(actionStep)

				if actionStep.InstanceId ~= reactionInst.ActionStep then

					local entity = r2:getInstanceFromId(actionStep.Entity)
					local actionType = actionStep.Action
					
					if entity then
						totalText = totalText..entity.Name
						totalText = totalText .. " : " 
						
						if actionType.Type ~= "" then
							totalText = totalText .. r2.logicEntityAttributes["ApplicableActions"][actionType.Type]
							if actionType.Value ~= "" then
								local inst = r2:getInstanceFromId(actionType.Value)
								if inst ~= nil then
									totalText = totalText .. " " .. inst.Name
								end
							end
						else
							totalText = totalText .. "..."
						end
					end

					totalText = totalText .. "\n"
				end	
			end

			reactionText:addTextChild(ucstring(totalText))

			sep.active = true
		else
			sep.active = false
		end
	end

	-- update title reaction
	r2:buildReactionTitle(reactionUI, false)

	local selectedReaction = r2:getSelectedEltInstId(r2.logicEntityUIPath.."reactions")
	if selectedReaction and selectedReaction == reactionInst.InstanceId then
		r2:updateReactionEditor()
	end
end

-----------------------------------------------------------------------------------------------------------
-- selection of a logic entity in Action editor/Actions/Which entity menu
r2.actionWhichEntity = function(logicEntity)

	r2.requestNewAction(i18n.get("uiR2EDSetwhichEntityAction"))
	
	local actionInstId = r2:getSelectedEltInstId(r2.logicEntityUIPath.."actions")
	local action = r2:getInstanceFromId(actionInstId)
	assert(action)

	local actionStep = r2:createNewMenuPairAndComponent("ActionStep")
	if actionStep ~= nil then
		actionStep.Entity = r2.RefId(logicEntity.InstanceId)
		r2.requestInsertNode(actionInstId, "Actions", -1, "", actionStep)
		local reaction = r2.newComponent("LogicEntityReaction")
		reaction.LogicEntityAction = actionInstId
		reaction.ActionStep = actionStep.InstanceId

		r2.requestInsertNode(logicEntity:getBehavior().InstanceId, "Reactions", -1, "", reaction)
	else
		local leftButton = getUI(r2.whichEntitySelectedButtonId)
		assert(leftButton)

		local menuPair = leftButton.parent.parent.parent 
		assert(menuPair)

		local menuList = menuPair.parent
		assert(menuList)

		local actionStepIndex = menuList:getElementIndex(menuPair)
		local actionStep = action.Actions[actionStepIndex]
		assert(actionStep)

		local oldWhichEntity = r2:getInstanceFromId(tostring(actionStep.Entity))

		-- the associated reaction must be given to correct logic entity

		if oldWhichEntity and oldWhichEntity.InstanceId ~= logicEntity.InstanceId then
			local index
			for r = 0, oldWhichEntity:getBehavior().Reactions.Size - 1 do
				if oldWhichEntity:getBehavior().Reactions[r].ActionStep == actionStep.InstanceId then
					index = r
					break
				end
			end

			r2.requestMoveNode(oldWhichEntity:getBehavior().InstanceId, "Reactions", index,
							   logicEntity:getBehavior().InstanceId, "Reactions", -1)

			r2.requestSetNode(actionStep.InstanceId, "Entity", r2.RefId(logicEntity.InstanceId))

			-- empty property by waiting user choice
			r2.requestSetNode(actionStep.Action.InstanceId, "Type", "")
			r2.requestSetNode(actionStep.Action.InstanceId, "Value", r2.RefId(""))
		end
	end
end

-----------------------------------------------------------------------------------------------------------
-- selection of a logic entity in Action editor/Extra conditions/Which entity menu
r2.actionConditionWhichEntity = function(logicEntity)
	
	r2.requestNewAction(i18n.get("uiR2EDSetWhichEntityConditionAction"))

	local actionInstId = r2:getSelectedEltInstId(r2.logicEntityUIPath.."actions")

	local conditionStep = r2:createNewMenuPairAndComponent("ConditionStep")
	if conditionStep ~= nil then
		conditionStep.Entity = r2.RefId(logicEntity.InstanceId)
		r2.requestInsertNode(actionInstId, "Conditions", -1, "", conditionStep)
	else

		local action = r2:getInstanceFromId(actionInstId)
		assert(action)

		local leftButton = getUI(r2.whichEntitySelectedButtonId)
		assert(leftButton)

		local menuPair = leftButton.parent.parent.parent 
		assert(menuPair)

		local menuList = menuPair.parent
		assert(menuList)

		local conditionStepIndex = menuList:getElementIndex(menuPair)
		local conditionStep = action.Conditions[conditionStepIndex]
		assert(conditionStep)

		r2.requestSetNode(conditionStep.InstanceId, "Entity", r2.RefId(logicEntity.InstanceId))

		-- empty property by waiting user choice
		r2.requestSetNode(conditionStep.Condition.InstanceId, "Type", "")
		r2.requestSetNode(conditionStep.Condition.InstanceId, "Value", r2.RefId(""))
	end
end

-----------------------------------------------------------------------------------------------------------
-- selection of a logic entity in Reaction editor/What triggers this reaction/Which entity menu
r2.reactionWhatTriggersWhichEntity = function(logicEntity)
	
	r2.requestNewAction(i18n.get("uiR2EDSetWhatTriggersWhichEntityAction"))
	local selectButton = getUI(r2.whichEntitySelectedButtonId)
	assert(selectButton)

	-- if left button is this of "Reaction editor/What triggers this reaction" 
	-- we create actually the "LogicEntityReaction" and its associated "LogicEntityAction" and "ActionStep"
	local uiName = r2.logicEntityUIPath.."reactions"
	local element = r2:getSelectedEltUI(uiName)
	if element.Env.WhatTriggers~=nil and not element.Env.WhatTriggers then

		element.Env.WhatTriggers = true
		
		local reactionEditor = r2:getReactionEditor()
		assert(reactionEditor)

		local actions = reactionEditor:find("actions_list")
		assert(actions)
		actions.active = true

		local conditions = reactionEditor:find("conditions_list")
		assert(conditions)
		conditions.active = true

		local reaction = r2.newComponent("LogicEntityReaction")
			
		r2:setSelectedEltInstId(uiName, reaction.InstanceId)

		-- we create associated LogicEntityAction and ActionStep
		local selectedLogicEntity = r2:getSelectedInstance()
		local action = r2.newComponent("LogicEntityAction")
		local actionStep = r2.newComponent("ActionStep")
		actionStep.Entity = r2.RefId(selectedLogicEntity.InstanceId)
		table.insert(action.Actions, actionStep)

		reaction.LogicEntityAction = action.InstanceId
		reaction.ActionStep = actionStep.InstanceId

		r2.requestInsertNode(logicEntity:getBehavior().InstanceId, "Actions", -1, "", action)
		r2.requestInsertNode(selectedLogicEntity:getBehavior().InstanceId, "Reactions", -1, "", reaction)

		element.Env.elementId = reaction.InstanceId

	-- the trigger logic entity is changed : the "LogicEntityAction" property of the reaction
	-- is moved to the new trigger 
	else
		local oldWhichEntity = r2:getInstanceFromId(selectButton.parent.parent.Env.oldEntityId)
		assert(oldWhichEntity)

		selectButton.parent.parent.Env.oldEntityId = nil

		local reactionInst = r2:getSelectedEltInst(uiName)
		local actionId = reactionInst.LogicEntityAction

		local index
		for a = 0, oldWhichEntity:getBehavior().Actions.Size - 1 do
			if oldWhichEntity:getBehavior().Actions[a].InstanceId == actionId then
				index = a
				break
			end
		end

		if oldWhichEntity.InstanceId ~= logicEntity.InstanceId then
			r2.requestMoveNode(oldWhichEntity:getBehavior().InstanceId, "Actions", index,
							   logicEntity:getBehavior().InstanceId, "Actions", -1)

			-- empty property by waiting user choice
			local actionInst = r2:getInstanceFromId(actionId)
			assert(actionInst)
			r2.requestSetNode(actionInst.Event.InstanceId, "Type", "")
			r2.requestSetNode(actionInst.Event.InstanceId, "Value", r2.RefId(""))
		end
	end
end

-----------------------------------------------------------------------------------------------------------
-- selection of a logic entity in Reaction editor/Actions/Which entity menu
r2.reactionActionWhichEntity = function(logicEntity)

	r2.requestNewAction(i18n.get("uiR2EDSetReactionActionWhichEntityAction"))

	local reactionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
	assert(reactionInst)

	local action = r2:getInstanceFromId(reactionInst.LogicEntityAction)
	
	local actionStep = r2:createNewMenuPairAndComponent("ActionStep")
	if actionStep ~= nil then
		actionStep.Entity = r2.RefId(logicEntity.InstanceId)
					
		if action then
			r2.requestInsertNode(action.InstanceId, "Actions", -1, "", actionStep)
		end

		-- new LogicEntityReaction
		local newReaction = r2.newComponent("LogicEntityReaction")
		newReaction.LogicEntityAction = reactionInst.LogicEntityAction
		newReaction.ActionStep = actionStep.InstanceId

		r2.requestInsertNode(logicEntity:getBehavior().InstanceId, "Reactions", -1, "", newReaction)
	else

		local leftButton = getUI(r2.whichEntitySelectedButtonId)
		assert(leftButton)

		local menuPair = leftButton.parent.parent.parent 
		assert(menuPair)

		local menuList = menuPair.parent
		assert(menuList)

		local actionIndex = menuList:getElementIndex(menuPair)
		assert(actionIndex)

		local maxSize = actionIndex
		if actionIndex >= action.Actions.Size then actionIndex = action.Actions.Size-1 end
		for i=0, maxSize do
			local actionStepInst = action.Actions[i]
			if actionStepInst.InstanceId == reactionInst.ActionStep then
				actionIndex = actionIndex+1
				break
			end
		end

		local actionInst = r2:getInstanceFromId(reactionInst.LogicEntityAction)
		assert(actionInst)

		local actionStepInst = actionInst.Actions[actionIndex]
		assert(actionStepInst)

		local oldWhichEntity = r2:getInstanceFromId(tostring(actionStepInst.Entity))

		
		-- the associated reaction must be given to correct logic entity
		if oldWhichEntity and oldWhichEntity.InstanceId ~= logicEntity.InstanceId then
			local index
			for r = 0, oldWhichEntity:getBehavior().Reactions.Size - 1 do
				if oldWhichEntity:getBehavior().Reactions[r].ActionStep == actionStepInst.InstanceId then
					index = r
					break
				end
			end

			r2.requestMoveNode(oldWhichEntity:getBehavior().InstanceId, "Reactions", index,
							   logicEntity:getBehavior().InstanceId, "Reactions", -1)

			r2.requestSetNode(actionStepInst.InstanceId, "Entity", r2.RefId(logicEntity.InstanceId))

			-- empty property by waiting user choice
			r2.requestSetNode(actionStepInst.Action.InstanceId, "Type", "")
			r2.requestSetNode(actionStepInst.Action.InstanceId, "Value", r2.RefId(""))
		end
	end
end

-----------------------------------------------------------------------------------------------------------
-- selection of a logic entity in Reaction editor/Extra conditions/Which entity menu
r2.reactionConditionWhichEntity = function(logicEntity)
	
	r2.requestNewAction(i18n.get("uiR2EDSetReactionConditionWhichEntityAction"))

	local reactionInstId = r2:getSelectedEltInstId(r2.logicEntityUIPath.."reactions")
	local reaction = r2:getInstanceFromId(reactionInstId)
	assert(reaction)
	
	local conditionStep = r2:createNewMenuPairAndComponent("ConditionStep")
	if conditionStep ~= nil then

		conditionStep.Entity = r2.RefId(logicEntity.InstanceId)
					
		local action = r2:getInstanceFromId(reaction.LogicEntityAction)
		if action then
			r2.requestInsertNode(action.InstanceId, "Conditions", -1, "", conditionStep)
		end
	else

		local leftButton = getUI(r2.whichEntitySelectedButtonId)
		assert(leftButton)

		local menuPair = leftButton.parent.parent.parent 
		assert(menuPair)

		local menuList = menuPair.parent
		assert(menuList)

		local conditionIndex = menuList:getElementIndex(menuPair)
		assert(conditionIndex)

		local actionInst = r2:getInstanceFromId(reaction.LogicEntityAction)
		assert(actionInst)

		local conditionInst = actionInst.Conditions[conditionIndex]
		assert(conditionInst)

		r2.requestSetNode(conditionInst.InstanceId, "Entity", r2.RefId(logicEntity.InstanceId))

		-- empty property by waiting user choice
		r2.requestSetNode(conditionInst.Condition.InstanceId, "Type", "")
		r2.requestSetNode(conditionInst.Condition.InstanceId, "Value", r2.RefId(""))
	end
end


function r2:activeTrashButton()

	local menuTrashPair = getUICaller().parent.parent.parent
	assert(menuTrashPair)

	local trashButtonCaller = menuTrashPair:find("remove_menu_pair")
	assert(trashButtonCaller)

	local pairList = menuTrashPair.parent
	assert(pairList)

	if menuTrashPair.id == pairList:getChild(pairList.childrenNb-1).id then return end

	for i=0, pairList.childrenNb-1 do
		local pair = pairList:getChild(i)
		assert(pair)

		local trashButton = pair:find("remove_menu_pair")
		assert(trashButton)

		if trashButton.active then
			if trashButton.id == trashButtonCaller.id then return end
			trashButton.active = false
			break
		end
	end

	trashButtonCaller.active = true
end

-----------------------------------------------------------------------------------------------------------
------------------------------------ SECOND MENU ----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
-- when the second button of a menu pair is pushed, the second menu is open
r2.openSequencesMenu = {			["begin activity sequence"]= {element="activity", singleSubMenu=true},
						["end of activity sequence"]={element="activity", singleSubMenu=true},
						["is in activity sequence"]= {element="activity", singleSubMenu=true},
						["begin chat sequence"]=     {element="chat", singleSubMenu=true},
						["end of chat sequence"]=    {element="chat", singleSubMenu=true},
						["is in chat sequence"]=     {element="chat", singleSubMenu=true},
						["is in activity step"]=     {element="activity", singleSubMenu=false},
						["end of activity step"]=    {element="activity", singleSubMenu=false},
						["begin of activity step"]=    {element="activity", singleSubMenu=false},
						["end of chat step"]=        {element="chat", singleSubMenu=false},
						["is in chat step"]=		 {element="chat", singleSubMenu=false},
						["starts chat"]=		 {element="chatstep", singleSubMenu=true}

						--["member death"]=			 {element="npc", singleSubMenu=true}

					   }

function r2:openRightMenu(isEntityOfLeftMenu, loadTable, selectLineFunction)

	-- logic entity menu initialization
	local logicEntityMenu = getUI("ui:interface:r2ed_logic_entity_menu")
	assert(logicEntityMenu)

	local logicEntityMenu = logicEntityMenu:getRootMenu()
	assert(logicEntityMenu)

	logicEntityMenu:reset()
	r2.rightSelectedButtonId = getUICaller().id

	-- either menu is relative to selected logic entity or to selected entity in left menu (which entity...)
	local entityInst 
	if isEntityOfLeftMenu then

		local entityId = getUICaller().parent.parent.parent.Env.entityId 
	
		if entityId == nil then
			entityId = getUICaller().parent.parent.Env.entityId
		end

		if entityId == nil then
			return
		else
			entityInst = r2:getInstanceFromId(entityId)
		end
	else
		entityInst = r2:getSelectedInstance()
	end
	assert(entityInst)

	-- initialization of "logic entity" menu
	local class = r2.Classes[entityInst.Class]
	assert(class)

	for i=1,table.getn(class[loadTable]) do
		local action = class[loadTable][i]
		local openSequencesMenu = r2.openSequencesMenu[action]
		local actionName = r2.logicEntityAttributes[loadTable][action]
		if actionName == nil then actionName = "?"..action.."?" end
		local actionText = ucstring(actionName)
		if openSequencesMenu ~= nil then

			-- first line
			logicEntityMenu:addLine(actionText, "", "", action)
			
			-- sequences sub menu
			logicEntityMenu:addSubMenu(i-1)
			local subMenu = logicEntityMenu:getSubMenu(i-1)

			local sequences			
			if openSequencesMenu.element == "activity" then
				sequences = entityInst:getBehavior().Activities
			elseif openSequencesMenu.element == "chat" then
				sequences = entityInst:getBehavior().ChatSequences
			elseif openSequencesMenu.element == "chatstep" then
				sequences = entityInst.Components
			elseif openSequencesMenu.element == "npc" then
				sequences = entityInst.Components
			end

			local count1 = 0
			for s = 0, sequences.Size - 1 do
				local sequence = sequences[s]

				if not openSequencesMenu.singleSubMenu then

					--subMenu:addLine(ucstring(sequence.Name), "", "", sequence.InstanceId)
					subMenu:addLine(ucstring(r2:getSequenceName(sequence)), "", "", sequence.InstanceId)
					
					-- steps sub menu
					subMenu:addSubMenu(s)
					local stepsMenu = subMenu:getSubMenu(s)

					local steps = sequence.Components
					if steps.Size == 0 then
						stepsMenu:addLine(i18n.get("uiR2EdNoSelelection"), "lua", selectLineFunction .. "()", "")
					end

					local count2 = 0
					for s = 0, steps.Size - 1 do
						local step = steps[s]
						local stepName = r2:getElementName(step)
						stepsMenu:addLine(ucstring(stepName), "lua", 
							selectLineFunction.. "('".. action .. "','" .. step.InstanceId .."')", step.InstanceId)
						count2=count2+1
					end
				else 
					--subMenu:addLine(ucstring(sequence.Name), "lua", 
					subMenu:addLine(ucstring(r2:getSequenceName(sequence)), "lua", 
						selectLineFunction.. "('".. action .. "','" .. sequence.InstanceId .. "')", sequence.InstanceId)
				end
				
				count1=count1+1
			end

			if sequences.Size == 0 then
				subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "lua", selectLineFunction .. "()", "")
			end

		else
			logicEntityMenu:addLine(actionText, "lua", selectLineFunction .. "('".. action .. "')", action)
		end
	end

	-- open menu
	r2:openLogicEntityMenu(getUICaller())
end

-----------------------------------------------------------------------------------------------------------
-- selection of an event in "Action editor/on which event" menu
function r2:actionWhichEvent(type, value)

	r2.requestNewAction(i18n.get("uiR2EDSetActionWhichEventAction"))
	
	local actionEditor = r2:getActionEditor()
	assert(actionEditor)

	local whichEventText = actionEditor:find("which_event"):find("menu"):find("text")
	assert(whichEventText)

	if value == nil then value = "" end
	
	-- update "Event" property 
	local actionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."actions")
	assert(actionInst)

	r2.requestSetNode(actionInst.Event.InstanceId, "Type", type)
	r2.requestSetNode(actionInst.Event.InstanceId, "Value", r2.RefId(value))
end

-----------------------------------------------------------------------------------------------------------
-- selection of an action in "Action editor/Actions/What action" menu
function r2:actionWhatAction(type, value)

	r2.requestNewAction(i18n.get("uiR2EDSetActionWhatActionAction"))
	
	local rightButton = getUI(r2.rightSelectedButtonId)
	assert(rightButton)

	if value == nil then value = "" end
	
	-- update ActionStep
	local action = r2:getSelectedEltInst(r2.logicEntityUIPath.."actions")
	assert(action)

	local menuPair = rightButton.parent.parent.parent 
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local actionStepIndex = menuList:getElementIndex(menuPair)

	local actionStepInst = action.Actions[actionStepIndex]
	assert(actionStepInst)

	r2.requestSetNode(actionStepInst.Action.InstanceId, "Type", type)
	r2.requestSetNode(actionStepInst.Action.InstanceId, "Value", r2.RefId(value))
end

-----------------------------------------------------------------------------------------------------------
-- selection of a condition in "Action editor/Extra conditions/What condition" menu
function r2:actionWhatCondition(type, value)
	
	r2.requestNewAction(i18n.get("uiR2EDSetActionWhatConditionAction"))

	local rightButton = getUI(r2.rightSelectedButtonId)
	assert(rightButton)

	if value == nil then value = "" end
	
	-- update ConditionStep
	local action = r2:getSelectedEltInst(r2.logicEntityUIPath.."actions")
	assert(action)

	local menuPair = rightButton.parent.parent.parent 
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local conditionStepIndex = menuList:getElementIndex(menuPair)

	local conditionStepInst = action.Conditions[conditionStepIndex]
	assert(conditionStepInst)

	r2.requestSetNode(conditionStepInst.Condition.InstanceId, "Type", type)
	r2.requestSetNode(conditionStepInst.Condition.InstanceId, "Value", r2.RefId(value))
end

-----------------------------------------------------------------------------------------------------------
-- selection of an event in "Reaction editor/Which event" menu
function r2:reactionWhichEvent(type, value)

	r2.requestNewAction(i18n.get("uiR2EDSetReactionWhichEventAction"))

	if value == nil then value = "" end
	
	-- update "Event" property of associated LogicEntityAction
	local reactionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
	
	local logicEntityAction = r2:getInstanceFromId(reactionInst.LogicEntityAction)
	assert(logicEntityAction)
	r2.requestSetNode(logicEntityAction.Event.InstanceId, "Type", type)
	r2.requestSetNode(logicEntityAction.Event.InstanceId, "Value", r2.RefId(value))
end

-----------------------------------------------------------------------------------------------------------
-- selection of an action in "Reaction editor/What action to apply to me" menu
function r2:reactionWhatAction(type, value)

	r2.requestNewAction(i18n.get("uiR2EDSetReactionWhatActionAction"))

	local rightButton = getUI(r2.rightSelectedButtonId)
	assert(rightButton)
	
	if value == nil then value = "" end
	
	-- update ActionStep
	local reactionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
	assert(reactionInst)

	local action = r2:getInstanceFromId(reactionInst.LogicEntityAction)
	assert(action)

	local menuPair = rightButton.parent.parent.parent
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local actionIndex = menuList:getElementIndex(menuPair)

	local maxSize = actionIndex
	if actionIndex >= action.Actions.Size then actionIndex = action.Actions.Size-1 end
	for i=0, maxSize do
		local actionStepInst = action.Actions[i]
		if actionStepInst.InstanceId == reactionInst.ActionStep then
			actionIndex = actionIndex+1
			break
		end
	end

	local actionStepInst = action.Actions[actionIndex]
	assert(actionStepInst)

	r2.requestSetNode(actionStepInst.Action.InstanceId, "Type", type)
	r2.requestSetNode(actionStepInst.Action.InstanceId, "Value", r2.RefId(value))
end

-----------------------------------------------------------------------------------------------------------
-- selection of an action in "Reaction editor/What action to apply to me" menu
function r2:reactionWhatMainAction(type, value)

	r2.requestNewAction(i18n.get("uiR2EDSetReactionWhatMainActionAction"))

	local rightButton = getUI(r2.rightSelectedButtonId)
	assert(rightButton)

	if value == nil then value = "" end
	
	-- update ActionStep
	local reactionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
	
	local actionStepInst = r2:getInstanceFromId(reactionInst.ActionStep)
	assert(actionStepInst)

	r2.requestSetNode(actionStepInst.Action.InstanceId, "Type", type)
	r2.requestSetNode(actionStepInst.Action.InstanceId, "Value", r2.RefId(value))
end

-----------------------------------------------------------------------------------------------------------
-- selection of a condition in "Reaction editor/Extra conditions/What condition" menu
function r2:reactionWhatCondition(type, value)

	r2.requestNewAction(i18n.get("uiR2EDSetReactionWhatConditionAction"))

	local rightButton = getUI(r2.rightSelectedButtonId)
	assert(rightButton)

	if value == nil then value = "" end
	
	-- update ConditionStep
	local reaction = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
	assert(reaction)

	local action = r2:getInstanceFromId(reaction.LogicEntityAction)
	assert(action)

	local menuPair = rightButton.parent.parent.parent
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local conditionIndex = menuList:getElementIndex(menuPair)

	local conditionInst = action.Conditions[conditionIndex]
	assert(conditionInst)

	r2.requestSetNode(conditionInst.Condition.InstanceId, "Type", type)
	r2.requestSetNode(conditionInst.Condition.InstanceId, "Value", r2.RefId(value))
end



-----------------------------------------------------------------------------------------------------------
------------------------------------ INTERFACE ------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------
------------------ CREATE NEW ACTION ----------------------------------------------------------------------
function r2:newAction(firstRequest, instanceElement)

	r2.requestNewAction(i18n.get("uiR2EDNewActionAction"))

	local uiName = r2.logicEntityUIPath.."actions"

	if firstRequest == true then
		local instanceElement = r2.newComponent("LogicEntityAction")
		
		local logicEntity = r2:getSelectedInstance()
		assert(logicEntity)

		r2.requestInsertNode(logicEntity:getBehavior().InstanceId, "Actions", -1, "", instanceElement)

		r2.ownCreatedInstances[instanceElement.InstanceId] = true
	else

		local templateParams = {
									selectElt="r2:selectAction()", 
									openEltEditor="r2:openActionEditor()", 
									maxMinElt="r2:maximizeMinimizeAction()", 
									removeElt="r2:removeAction()",
									colOver="200 150 0 100",
									colPushed="200 150 0 255"
							   }
	
		local element = r2:newLogicElement("actions", tostring(i18n.get("uiR2EdAction")), templateParams, instanceElement.InstanceId)

		r2:updateActionText(element)
	end
end

--------------------- BUILD ACTION TITLE ----------------------------------------------------------
function r2:buildActionTitle(actionUI, erase)

	local actionInst = r2:getInstanceFromId(actionUI.Env.elementId)
	assert(actionInst)

	-- part1
	local index = r2:searchElementIndex(actionInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdAction")).." "..index.." : "

	-- part2
	local eventType = "..."
	if tostring(actionInst.Event.Type) ~= "" then
		eventType = actionInst.Event.Type
	end
	if tostring(actionInst.Event.Value) ~= "" then
		local inst = r2:getInstanceFromId(tostring(actionInst.Event.Value)) 
		assert(inst)
		eventType = eventType .. " " .. inst.Name
	end

	if actionInst.Event.ValueString and tostring(actionInst.Event.ValueString) ~= "" then
		eventType = eventType .. " " .. actionInst.Event.ValueString
	end


	local part2 = tostring(i18n.get("uiR2EdEventTxtPreEvent")).." '" .. eventType .. "' "..string.lower(tostring(i18n.get("uiR2EdAction")))

	-- title
	local title = actionUI:find("title")
	assert(title)
	title.uc_hardtext= part1..part2
end

function r2:getActionName(actionInst)

	-- part1
	local index = r2:searchElementIndex(actionInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdAction")).." "..index.." : "

	-- part2
	local eventType = "..."
	if tostring(actionInst.Event.Type) ~= "" then
		eventType = actionInst.Event.Type
	end
	if tostring(actionInst.Event.Value) ~= "" then
		local inst = r2:getInstanceFromId(tostring(actionInst.Event.Value)) 
		assert(inst)
		eventType = eventType .. " " .. inst.Name
	end
	if actionInst.Event.ValueString and tostring(actionInst.Event.ValueString) ~= "" then
		eventType = eventType .. " " .. actionInst.Event.ValueString
	end


	local part2 = tostring(i18n.get("uiR2EdEventTxtPreEvent")).." '" .. eventType .. "' "..string.lower(tostring(i18n.get("uiR2EdAction")))

	return part1..part2
end

------------------ SELECT ACTION ------------------------------------------------------------------
function r2:selectAction()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("actions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:selectElement(sequence, r2.logicEntityUIPath.."actions", false)

	if getUICaller().pushed == true then
		r2:openActionEditor() --TEMP
	end
end

------------------ REMOVE ACTION ------------------------------------------------------------------
function r2:removeAction(removedElement)

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("actions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	local uiName = r2.logicEntityUIPath.."actions"

	r2:removeElement(sequence, uiName, tostring(i18n.get("uiR2EdAction")), removedElement)
end

------------------ REMOVE ACTION ------------------------------------------------------------------
function r2:removeActionUI(removedElement)

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("actions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	local uiName = r2.logicEntityUIPath.."actions"

	if (removedElement==nil) or (removedElement.id == r2:getSelectedEltUI(uiName)) then
		r2:closeActionEditor()
	end

	r2:removeElementUI(sequence, uiName, removedElement)
end

------------------- UP ACTION ----------------------------------------------------------------------
function r2:upAction()
	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("actions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:upElement(sequence, r2.logicEntityUIPath.."actions")
end

--------------------- DOWN ACTION ------------------------------------------------------------------
function r2:downAction()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("actions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:downElement(sequence, r2.logicEntityUIPath.."actions")	
end

-------------- MAX/MIN ACTIONS ----------------------------------------------------------------------
function r2:maximizeMinimizeActions()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("actions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:maximizeMinimizeElements(sequence, r2.logicEntityUIPath.."actions")
end

----------------MAX/MIN ACTION -----------------------------------------------------------------------
function r2:maximizeMinimizeAction()
	r2:maximizeMinimizeElement(r2.logicEntityUIPath.."actions")
end

function r2:removeActionStep(fromActionEditor)

	r2.requestNewAction(i18n.get("uiR2EDRemoveActionStepAction"))

	local menuPair = getUICaller().parent.parent.parent.parent
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local actionStepIndex = menuList:getElementIndex(menuPair)

	local action
	if fromActionEditor == true then
		action = r2:getSelectedEltInst(r2.logicEntityUIPath.."actions")
	else
		local reaction = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
		assert(reaction)
		action = r2:getInstanceFromId(reaction.LogicEntityAction)
	end
	assert(action)


	if not fromActionEditor then
		local reactionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
		assert(reactionInst)

		local maxSize = actionStepIndex
		if actionStepIndex >= action.Actions.Size then actionStepIndex = action.Actions.Size-1 end
		for i=0, maxSize do
			local actionStepInst = action.Actions[i]
			if actionStepInst.InstanceId == reactionInst.ActionStep then
				actionStepIndex = actionStepIndex+1
				break
			end
		end
	end


	if actionStepIndex >= action.Actions.Size then return end

	local actionStepInst = action.Actions[actionStepIndex]
	assert(actionStepInst)

	r2.requestEraseNode(actionStepInst.InstanceId, "", -1)
end

function r2:removeActionConditionStep()
	
	r2.requestNewAction(i18n.get("uiR2EDRemoveActionConditionStepAction"))

	local menuPair = getUICaller().parent.parent.parent.parent
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local conditionStepIndex = menuList:getElementIndex(menuPair)

	local action = r2:getSelectedEltInst(r2.logicEntityUIPath.."actions")
	assert(action)

	if conditionStepIndex >= action.Conditions.Size then return end

	local conditionStepInst = action.Conditions[conditionStepIndex]
	assert(conditionStepInst)

	r2.requestEraseNode(conditionStepInst.InstanceId, "", -1)

	r2:updateActionEditor()
end

function r2:removeReactionConditionStep()

	r2.requestNewAction(i18n.get("uiR2EDRemoveReactionConditionStepAction"))
	
	local menuPair = getUICaller().parent.parent.parent.parent
	assert(menuPair)

	local menuList = menuPair.parent
	assert(menuList)

	local conditionStepIndex = menuList:getElementIndex(menuPair)

	local reaction = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
	assert(reaction)

	local action = r2:getInstanceFromId(reaction.LogicEntityAction)
	assert(action)

	if conditionStepIndex >= action.Conditions.Size then return end

	local conditionStepInst = action.Conditions[conditionStepIndex]
	assert(conditionStepInst)

	r2.requestEraseNode(conditionStepInst.InstanceId, "", -1)

	r2:updateReactionEditor()
end

------------------ OPEN ACTION EDITOR ----------------------------------------------------------------
function r2:openActionEditor()

	r2:updateActionEditor()
	r2:openElementEditor(r2.logicEntityUIPath.."actions", "uiR2EDActionEditor", "r2ed_logic_entities")
end

function r2:updateActionEditor()

	-- delete menu list in editor and add first double menu in every list
	local actionEditor = r2:getActionEditor()
	assert(actionEditor)

	--
	local actionsList = actionEditor:find("actions_list")
	assert(actionsList)
	actionsList = actionsList:find("menus_list")
	assert(actionsList)
	actionsList:clear()

	local newButtonPair = createGroupInstance("two_menu_trash_template", actionsList.id, 
		{id="menu_pair1", params_left="r2:openWhichEntityMenu()", 
		 params_right="r2:openRightMenu(true, 'ApplicableActions', 'r2:actionWhatAction')",
		 remove_pair="r2:removeActionStep(true)", col_pushed="255 255 255 255",
		 params_over="r2:activeTrashButton()"})

	actionsList:addChild(newButtonPair)
	actionsList.parent:updateCoords()

	--
	local conditionsList = actionEditor:find("conditions_list"):find("menus_list")
	assert(conditionsList)
	conditionsList:clear()

	newButtonPair = createGroupInstance("two_menu_trash_template", conditionsList.id, 
		{id="menu_pair1", params_left="r2:openWhichEntityMenu()",
		 params_right="r2:openRightMenu(true, 'Conditions', 'r2:actionWhatCondition')",
		 remove_pair="r2:removeActionConditionStep()", col_pushed="255 255 255 255",
		 params_over="r2:activeTrashButton()"})

	conditionsList:addChild(newButtonPair)
	conditionsList.parent:updateCoords()

	r2:initActionEditor()

	-- action name
	local actionName = actionEditor:find("name")
	assert(actionName)

	-- update buttons text
	local actionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."actions")

	-- which event text
	local whichEvent = actionEditor:find("which_event"):find("menu")
	assert(whichEvent)
	local whichEventText = whichEvent:find("text")
	assert(whichEventText)
	
	-- which event label
	local whichEventLabel = actionEditor:find("which_event"):find("label")
	assert(whichEventLabel)
	local whichEventNameText = whichEventLabel:find("name_text")
	assert(whichEventNameText)
	text = r2:getSelectedInstance().Name
	whichEventNameText.uc_hardtext = text
	
	if actionInst ~= nil then

		local index = r2:searchElementIndex(actionInst)
		if index~= nil then
			actionName.uc_hardtext = tostring(i18n.get("uiR2EdAction")).." "..index.." : "
		else
			actionName.uc_hardtext = tostring(i18n.get("uiR2EdAction")).." : "
		end

		-- which event text
		local text = r2.logicEntityAttributes["Events"][actionInst.Event.Type]
		if text == nil then text = "" end
		if tostring(actionInst.Event.Value) ~= "" then
			--text = text .. " : " .. r2:getInstanceFromId(tostring(actionInst.Event.Value)).Name
			local inst = r2:getInstanceFromId(tostring(actionInst.Event.Value))
			assert(inst)
			text = text .. " : " .. r2:getElementName(inst)
		end
		whichEventText.uc_hardtext = text

		local previousButtonPair = actionsList:getChild(0)
		-- ActionStep list
		for i=0, actionInst.Actions.Size-1 do

			local actionStepInst = actionInst.Actions[i]
			assert(actionStepInst)

			local buttonParams = r2.whichEntityButtons[actionsList:getChild(0):find("left_menu"):find("select").id]

			newButtonPair = r2:createNewMenuPair(actionsList, buttonParams)

			local whichEntityMenu = previousButtonPair:find("left_menu")
			assert(whichEntityMenu)
			local whichEntityText = whichEntityMenu:find("text")
			assert(whichEntityText)
			text = r2:getInstanceFromId(tostring(actionStepInst.Entity)).Name
			whichEntityText.uc_hardtext = text
			

			local whatActionMenu = previousButtonPair:find("right_menu")
			assert(whatActionMenu)
			local whatActionText = whatActionMenu:find("text")
			assert(whatActionText)
			text = r2.logicEntityAttributes["ApplicableActions"][actionStepInst.Action.Type]
			if text == nil then text = "" end
			if tostring(actionStepInst.Action.Value) ~= "" then
				--text = text .. " : " .. r2:getInstanceFromId(tostring(actionStepInst.Action.Value)).Name
				local inst = r2:getInstanceFromId(tostring(actionStepInst.Action.Value))
				assert(inst)
				text = text .. " : " .. r2:getElementName(inst)
			end
			whatActionText.uc_hardtext = text

			previousButtonPair.Env.entityId = tostring(actionStepInst.Entity)
			previousButtonPair = newButtonPair
		end

		-- Conditions list
		previousButtonPair = conditionsList:getChild(0)
		for i=0, actionInst.Conditions.Size-1 do

			local conditionInst = actionInst.Conditions[i]
			assert(conditionInst)

			local buttonParams = r2.whichEntityButtons[conditionsList:getChild(0):find("left_menu"):find("select").id]

			newButtonPair = r2:createNewMenuPair(conditionsList, buttonParams)

			local whichEntityMenu = previousButtonPair:find("left_menu")
			assert(whichEntityMenu)
			local whichEntityText = whichEntityMenu:find("text")
			assert(whichEntityText)
			text = r2:getInstanceFromId(tostring(conditionInst.Entity)).Name
			whichEntityText.uc_hardtext = text

			local whatConditionMenu = previousButtonPair:find("right_menu")
			assert(whatConditionMenu)
			local whatConditionText = whatConditionMenu:find("text")
			assert(whatConditionText)
			text = r2.logicEntityAttributes["Conditions"][conditionInst.Condition.Type]
			if text == nil then text = "" end
			if tostring(conditionInst.Condition.Value) ~= "" then
				--text = text .. " : " .. r2:getInstanceFromId(tostring(conditionInst.Condition.Value)).Name
				local inst = r2:getInstanceFromId(tostring(conditionInst.Condition.Value))
				assert(inst)
				text = text .. " : " .. r2:getElementName(inst)
			end
			whatConditionText.uc_hardtext = text

			previousButtonPair.Env.entityId = tostring(conditionInst.Entity)
			previousButtonPair = newButtonPair
		end
	else

		local name = tostring(i18n.get("uiR2EdAction")).." : "
		actionName.uc_hardtext = name

		whichEventText.uc_hardtext = "" 
	end
end


------------------- CREATE NEW REACTION ---------------------------------------------------------------
function r2:newReaction(firstRequest, instanceElement)

	local templateParams = {
								selectElt="r2:selectReaction()", 
								openEltEditor="r2:openReactionEditor()", 
								maxMinElt="r2:maximizeMinimizeReaction()", 
								removeElt="r2:removeReaction()",
								colOver="120 45 185 90",
								colPushed="120 45 185 255"
						   }
	
	local uiName = r2.logicEntityUIPath.."reactions"

	local element

	if firstRequest == true then
		r2.ownCreatedInstances["emptyReaction"]=true
		element = r2:newLogicElement("reactions", tostring(i18n.get("uiR2EdReaction")), templateParams, "emptyReaction")
		element.Env.WhatTriggers = false

		r2:updateReactionEditor() 
	else
		local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
		assert(logicEntityUI)

		local reactionsUI = logicEntityUI:find("reactions")
		assert(reactionsUI)

		local reactions = reactionsUI:find("elements_list")
		assert(reactions)

		for i=0, reactions.childrenNb-1 do
			local reactionUI = reactions:getChild(i)
			if reactionUI.Env.elementId == instanceElement.InstanceId then
				element = reactionUI
				break
			end
		end

		if element == nil then
			element = r2:newLogicElement("reactions", tostring(i18n.get("uiR2EdReaction")), templateParams, instanceElement.InstanceId)
		end

		element.Env.WhatTriggers = true
		
		-- update reaction text and title
		r2:updateReactionText(element)
	end
end

------------ BUILD REACTION TITLE ----------------------------------------------------------
function r2:buildReactionTitle(reactionUI, erase)

	local title = reactionUI:find("title")
	assert(title)

	if reactionUI.Env.WhatTriggers==true then
		local reactionInst = r2:getInstanceFromId(reactionUI.Env.elementId)
		assert(reactionInst)

		-- part1
		local index = r2:searchElementIndex(reactionInst)
		if erase==true then index = index-1 end
		local part1 = tostring(i18n.get("uiR2EdReaction")).." "..index.." : "

		-- part2
		local actionStep = r2:getInstanceFromId(reactionInst.ActionStep)
		assert(actionStep)
		local value = tostring(actionStep.Action.Value)
		local type = actionStep.Action.Type
		local actionType = r2.logicEntityAttributes["ApplicableActions"][type]
		if actionType == nil then actionType="" end
		if value ~= "" then
			local inst = r2:getInstanceFromId(value)
			assert(inst)
			actionType = actionType .. " " .. inst.Name
		end
		local part2 = actionType .. " "

		-- part3
		local actionInst = r2:getInstanceFromId(reactionInst.LogicEntityAction)
		assert(actionInst)
		value = tostring(actionInst.Event.Value)
		type = r2.logicEntityAttributes["Events"][actionInst.Event.Type]
		if type == nil then type="..." end
		local eventType = type 
		if value ~= "" then
			local inst = r2:getInstanceFromId(value)
			assert(inst)
			eventType = eventType .. " " .. inst.Name
		end


		if actionStep.Action.ValueString and actionStep.Action.ValueString ~= "" then
			if string.gfind(eventType, "%%1")() then
				eventType = string.gsub(eventType, "%%1", "'"..tostring(actionStep.Action.ValueString).."'")
			else
				eventType = eventType .. " '" .. tostring(actionStep.Action.ValueString).. "'"
			end
		end


		local whichEntity = actionInst.Parent.Parent.Parent
		assert(whichEntity)

		local part3 = tostring(i18n.get("uiR2EdEventTxtPreEvent")).." '".. eventType .."' "..tostring(i18n.get("uiR2EdEventOf")).." ".. whichEntity.Name

		-- title
		title.uc_hardtext= part1..part2..part3

	elseif erase==true then
		local eltsList = reactionUI.parent
		local indexReactionUI = eltsList:getElementIndex(reactionUI) - 1
		title.uc_hardtext = tostring(i18n.get("uiR2EdReaction")).." "..indexReactionUI.." : "
	end
end

function r2:getReactionName(reactionInst)

	-- part1
	local index = r2:searchElementIndex(reactionInst)
	if erase==true then index = index-1 end
	local part1 = tostring(i18n.get("uiR2EdReaction")).." "..index.." : "

	-- part2
	local actionStep = r2:getInstanceFromId(reactionInst.ActionStep)
	assert(actionStep)
	local value = tostring(actionStep.Action.Value)
	local type = actionStep.Action.Type
	local actionType = r2.logicEntityAttributes["ApplicableActions"][type]
	if actionType == nil then actionType="" end
	if value ~= "" then
		local inst = r2:getInstanceFromId(value)
		assert(inst)
		actionType = actionType .. " " .. inst.Name
	end
	local part2 = actionType .. " "

	-- part3
	local actionInst = r2:getInstanceFromId(reactionInst.LogicEntityAction)
	assert(actionInst)
	value = tostring(actionInst.Event.Value)
	type = r2.logicEntityAttributes["Events"][actionInst.Event.Type]
	if type == nil then type="..." end
	local eventType = type 
	if value ~= "" then
		local inst = r2:getInstanceFromId(value)
		assert(inst)
		eventType = eventType .. " " .. inst.Name
	end

	if actionStep.Action.ValueString and tostring(actionStep.Action.ValueString) ~= "" then
		eventType = eventType .. " " .. actionStep.Action.ValueString
	end

	if actionStep.Action.ValueString and actionStep.Action.ValueString ~= "" then
		if string.gfind(eventType, "%%1")() then
			eventType = string.gsub(eventType, "%%1", "'"..tostring(actionStep.Action.ValueString).."'")
		else
			eventType = eventType .. " '" .. tostring(actionStep.Action.ValueString).. "'"
		end
	end



	local whichEntity = actionInst.Parent.Parent.Parent
	assert(whichEntity)

	local part3 = tostring(i18n.get("uiR2EdEventTxtPreEvent")).." '".. eventType .."' "..tostring(i18n.get("uiR2EdEventOf")).." ".. whichEntity.Name

	return part1..part2..part3
end

------------------ SELECT REACTION ------------------------------------------------------------------
function r2:selectReaction()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("reactions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:selectElement(sequence, r2.logicEntityUIPath.."reactions", false)

	if getUICaller().pushed == true then
		r2:openReactionEditor() --TEMP
	end
end

------------------ REMOVE REACTION ------------------------------------------------------------------
function r2:removeReaction(reactionUI)

	r2.requestNewAction(i18n.get("uiR2EDRemoveReactionAction"))

	local uiName = r2.logicEntityUIPath.."reactions"

	local reaction
	if reactionUI == nil then
		reactionUI = r2:getSelectedEltUI(uiName)
		reaction = r2:getSelectedEltInst(uiName)
	else
		reaction = r2:getInstanceFromId(reactionUI.Env.elementId)
	end

	if reactionUI.Env.WhatTriggers==true then
		local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
		assert(logicEntityUI)

		local tabUI = logicEntityUI:find("reactions")
		assert(tabUI)

		local sequence = tabUI:find("sequence_elts")
		assert(sequence)

		-- delete associated action step
		r2:removeElement(sequence, uiName, tostring(i18n.get("uiR2EdReaction")), reactionUI)
		if reaction~= nil then
			local actionStepId = reaction.ActionStep
			if r2:getInstanceFromId(actionStepId) then
				r2.requestEraseNode(actionStepId, "", -1)
			end
		end
	else
		r2:removeReactionUI(reactionUI)
	end
end

------------------ REMOVE REACTION ------------------------------------------------------------------
function r2:removeReactionUI(reactionUI)

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("reactions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	local uiName = r2.logicEntityUIPath.."reactions"

	if reactionUI.id == r2:getSelectedEltUI(uiName) then
		r2:closeReactionEditor()
	end
	
	r2:removeElementUI(sequence, uiName, reactionUI)
end

------------------ MAX/MIN REACTION ----------------------------------------------------------------
function r2:maximizeMinimizeReaction()
	r2:maximizeMinimizeElement(r2.logicEntityUIPath.."reactions")
end

------------------- UP REACTION --------------------------------------------------------------------
function r2:upReaction()
	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("reactions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:upElement(sequence, r2.logicEntityUIPath.."reactions")
end

------------------ DOWN REACTION --------------------------------------------------------------------
function r2:downReaction()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("reactions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:downElement(sequence, r2.logicEntityUIPath.."reactions")
end

------------------ MAX/MIN REACTIONS -----------------------------------------------------------------
function r2:maximizeMinimizeReactions()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("reactions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	r2:maximizeMinimizeElements(sequence, r2.logicEntityUIPath.."reactions")
end

------------------ OPEN ACTION EDITOR ----------------------------------------------------------------
function r2:openReactionEditor()

	r2:updateReactionEditor()
	r2:openElementEditor(r2.logicEntityUIPath.."reactions", "uiR2EDReactionEditor", "r2ed_logic_entities")
end

function r2:updateReactionEditor()

	local reactionEltUI = r2:getSelectedEltUI(r2.logicEntityUIPath.."reactions")
	assert(reactionEltUI)

	-- delete menu lists in editor and add first double menu in list
	local reactionEditor = r2:getReactionEditor()
	assert(reactionEditor)

	local conditionsList = reactionEditor:find("conditions_list"):find("menus_list")
	assert(conditionsList)
	conditionsList:clear()

	local newButtonPair = createGroupInstance("two_menu_trash_template", conditionsList.id, 
		{id="menu_pair1", params_left="r2:openWhichEntityMenu()",
		 params_right="r2:openRightMenu(true, 'Conditions', 'r2:reactionWhatCondition')",
		 remove_pair="r2:removeReactionConditionStep()", col_pushed="255 255 255 255",
		 params_over="r2:activeTrashButton()"})

	conditionsList:addChild(newButtonPair)
	conditionsList.parent:updateCoords()

	local actionsList = reactionEditor:find("actions_list"):find("menus_list")
	assert(actionsList)
	actionsList:clear()

	newButtonPair = createGroupInstance("two_menu_trash_template", actionsList.id, 
		{id="menu_pair1", params_left="r2:openWhichEntityMenu()",
		 params_right="r2:openRightMenu(true, 'ApplicableActions', 'r2:reactionWhatAction')",
		 remove_pair="r2:removeActionStep(false)", col_pushed="255 255 255 255",
		 params_over="r2:activeTrashButton()"})

	actionsList:addChild(newButtonPair)
	actionsList.parent:updateCoords()

	r2:initReactionEditor()

	local actions = reactionEditor:find("actions_list")
	assert(actions)

	local whatAction = actions:find("what_action")
	assert(whatAction)

	-- reaction name
	local reactionName = reactionEditor:find("name")
	assert(reactionName)
	local eltsList = reactionEltUI.parent
	assert(eltsList)

	local indexReactionUI = eltsList:getElementIndex(reactionEltUI) + 1
	local name = tostring(i18n.get("uiR2EdReaction")).." "..indexReactionUI.." : "

	-- what action label
	local whatActionLabel = whatAction:find("label")
	assert(whatActionLabel)
	local whatActionText = whatActionLabel:find("name_text")
	assert(whatActionText)
	local text = " " .. r2:getSelectedInstance().Name
	whatActionText.uc_hardtext = text
	
	local conditions = reactionEditor:find("conditions_list")
	assert(conditions)

	local whatActionMenu = whatAction:find("menu")
	assert(whatActionMenu)
	local whatActionText = whatActionMenu:find("text")
	assert(whatActionText)

	local menuPair = reactionEditor:find("combos")
	assert(menuPair)

	local whatTriggersMenu = menuPair:find("left_menu")
	assert(whatTriggersMenu)
	local whatTriggersText = whatTriggersMenu:find("text")
	assert(whatTriggersText)

	local whichEventMenu = menuPair:find("right_menu")
	assert(whichEventMenu)
	local whichEventText = whichEventMenu:find("text")
	assert(whichEventText)

	if not reactionEltUI.Env.WhatTriggers then

		actions.active = false
		conditions.active = false

		whatActionText.uc_hardtext = ""
		whatTriggersText.uc_hardtext = ""
		whichEventText.uc_hardtext = ""

		if reactionEditor.Env.minimize ~= true then 
			reactionEditor.h = reactionEditor.h - 320
			reactionEditor.Env.minimize = true
		end
	else

		actions.active = true
		conditions.active = true

		if reactionEditor.Env.minimize == true then 
			reactionEditor.h = reactionEditor.h + 320
			reactionEditor.Env.minimize = false
		end

		local reactionInst = r2:getSelectedEltInst(r2.logicEntityUIPath.."reactions")
		assert(reactionInst)

		local index = r2:searchElementIndex(reactionInst)
		if index~= nil then
			name = tostring(i18n.get("uiR2EdReaction")).." "..index.." : "
		else
			name = tostring(i18n.get("uiR2EdReaction")).." : "
		end
		
		local actionInst = r2:getInstanceFromId(reactionInst.LogicEntityAction)
		assert(actionInst)

		local actionStepInst = r2:getInstanceFromId(reactionInst.ActionStep)
		assert(actionStepInst)

		-- "what action" button text
		text = r2.logicEntityAttributes["ApplicableActions"][actionStepInst.Action.Type]
		if text == nil then text = "" end
		if tostring(actionStepInst.Action.Value) ~= "" then
			--text = text .. " : " .. r2:getInstanceFromId(tostring(actionStepInst.Action.Value)).Name
			local inst = r2:getInstanceFromId(tostring(actionStepInst.Action.Value))
			assert(inst)
			text = text .. " : " .. r2:getElementName(inst)
		end
		whatActionText.uc_hardtext = text

		-- "what triggers..." and "which event" buttons text
		local entityName = actionInst.ParentInstance.Parent.Name
		local text = r2.logicEntityAttributes["Events"][actionInst.Event.Type]
		if text == nil then text = "" end
		if tostring(actionInst.Event.Value) ~= "" then
			--text = text .. " : " .. r2:getInstanceFromId(tostring(actionInst.Event.Value)).Name
			local inst = r2:getInstanceFromId(tostring(actionInst.Event.Value))
			assert(inst)
			text = text .. " : " .. r2:getElementName(inst)
		end

		whatTriggersText.uc_hardtext = entityName

		whichEventText.uc_hardtext = text

		menuPair.Env.entityId = actionInst.ParentInstance.Parent.InstanceId

		-- Actions list
		local previousButtonPair = actionsList:getChild(0)
		
		for i=0, actionInst.Actions.Size-1 do

			local actionStepInst = actionInst.Actions[i]
			assert(actionStepInst)

			if actionStepInst.InstanceId ~= reactionInst.ActionStep then

				local buttonParams = r2.whichEntityButtons[actionsList:getChild(0):find("left_menu"):find("select").id]

				newButtonPair = r2:createNewMenuPair(actionsList, buttonParams)

				local whichEntityMenu = previousButtonPair:find("left_menu")
				assert(whichEntityMenu)
				local whichEntityText = whichEntityMenu:find("text")
				assert(whichEntityText)
				text = r2:getInstanceFromId(tostring(actionStepInst.Entity)).Name
				whichEntityText.uc_hardtext = text

				local whatActionMenu = previousButtonPair:find("right_menu")
				assert(whatActionMenu)
				local whatActionText = whatActionMenu:find("text")
				assert(whatActionText)
				text = r2.logicEntityAttributes["ApplicableActions"][actionStepInst.Action.Type]
				if text == nil then text = "" end
		
				if tostring(actionStepInst.Action.Value) ~= "" then
					--text = text .. " : " .. r2:getInstanceFromId(tostring(actionStepInst.Action.Value)).Name
					local inst = r2:getInstanceFromId(tostring(actionStepInst.Action.Value))
					assert(inst)
					text = text .. " : " .. r2:getElementName(inst)
				end
				whatActionText.uc_hardtext = text

				previousButtonPair.Env.entityId = tostring(actionStepInst.Entity)
				previousButtonPair = newButtonPair
			end
		end
		
		-- Conditions list
		local previousButtonPair = conditionsList:getChild(0)
		
		for i=0, actionInst.Conditions.Size-1 do

			local conditionInst = actionInst.Conditions[i]
			assert(conditionInst)

			local buttonParams = r2.whichEntityButtons[conditionsList:getChild(0):find("left_menu"):find("select").id]

			newButtonPair = r2:createNewMenuPair(conditionsList, buttonParams)

			local whichEntityMenu = previousButtonPair:find("left_menu")
			assert(whichEntityMenu)
			local whichEntityText = whichEntityMenu:find("text")
			assert(whichEntityText)
			text = r2:getInstanceFromId(tostring(conditionInst.Entity)).Name
			whichEntityText.uc_hardtext = text

			local whatConditionMenu = previousButtonPair:find("right_menu")
			assert(whatConditionMenu)
			local whatConditionText = whatConditionMenu:find("text")
			assert(whatConditionText)
			text = r2.logicEntityAttributes["Conditions"][conditionInst.Condition.Type]
			if text == nil then text = "" end
			if tostring(conditionInst.Condition.Value) ~= "" then
				local inst = r2:getInstanceFromId(tostring(conditionInst.Condition.Value))
				assert(inst)
				text = text .. " : " .. r2:getElementName(inst)
			end
			whatConditionText.uc_hardtext = text

			previousButtonPair.Env.entityId = tostring(conditionInst.Entity)
			previousButtonPair = newButtonPair
		end
	end

	reactionName.uc_hardtext = name
end

---------------------------- new element --------------------------------------------------------------
function r2:newLogicElement(tabName, elementName, templateParams, instanceId)

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find(tabName)
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)	

	local newElement = r2:newElement(sequence, elementName, templateParams, false)

	if instanceId~=nil and instanceId~="emptyReaction" then
		newElement.Env.elementId = instanceId
	end

	if r2.ownCreatedInstances[instanceId]==true then
		newElement:find("select").pushed = true 
 		r2:selectElement(sequence, r2.logicEntityUIPath..tabName, false, newElement:find("select"))
		r2.ownCreatedInstances[instanceId] = nil
		if tabName == "actions" then
			r2:updateActionEditor()
		elseif tabName == "reactions" then
			r2:updateReactionEditor()
		end
	end

	r2:maximizeMinimizeElement(uiName, newElement)

	return newElement
end

------------------------------------------------
function r2:cleanLogicEntityUI()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local actionSequence = logicEntityUI:find("actions")
	assert(actionSequence)
	local logicEntityActions = actionSequence:find("elements_list")
	assert(logicEntityActions)
	logicEntityActions.Env.minimize = nil

	local reactionSequence = logicEntityUI:find("reactions")
	assert(reactionSequence)
	local logicEntityReactions = reactionSequence:find("elements_list")
	assert(logicEntityReactions)
	logicEntityReactions.Env.minimize = nil

	logicEntityActions:clear()
	logicEntityReactions:clear()

	-- clean actions sequence
	r2:setSelectedEltUIId(r2.logicEntityUIPath.."actions", nil)
	local minElts = actionSequence:find("minimize_elements")
	assert(minElts)

	local maxElts = actionSequence:find("maximize_elements")
	assert(maxElts)
		
	minElts.active = false
	maxElts.active = false

	logicEntityActions.Env.elementsCount = nil

	-- clean reactions sequence
	r2:setSelectedEltUIId(r2.logicEntityUIPath.."reactions", nil)
	minElts = reactionSequence:find("minimize_elements")
	assert(minElts)

	maxElts = reactionSequence:find("maximize_elements")
	assert(maxElts)
		
	minElts.active = false
	maxElts.active = false

	logicEntityReactions.Env.elementsCount = nil

	r2:initEventsEditor()
end


function r2:updateLogicEntityUI(instance)

	if r2.logicEntityUIUpdated == true then return end
	r2.logicEntityUIUpdated = true

	-- remove all actions and reactions
	r2:cleanLogicEntityUI()

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local actionSequence = logicEntityUI:find("actions")
	assert(actionSequence)
	local logicEntityActions = actionSequence:find("elements_list")
	assert(logicEntityActions)
	

	local reactionSequence = logicEntityUI:find("reactions")
	assert(reactionSequence)
	local logicEntityReactions = reactionSequence:find("elements_list")
	assert(logicEntityReactions)
	
	-- build actions and reactions of selected logic entity
	local behavior = instance:getBehavior()
	local actions = instance:getBehavior().Actions
	local reactions = instance:getBehavior().Reactions

	local uiName = r2.logicEntityUIPath.."actions"
	for s = 0, actions.Size - 1 do
		local action = actions[s]
		r2:newAction(false, action)	
	end
	if actions.Size > 0 then
		local firstElt = logicEntityActions:getChild(1)
		local selectedButton = firstElt:find("select")
		selectedButton.pushed = true
		local sequenceUI = actionSequence:find("sequence_elts")
		assert(sequenceUI)
		
		r2:selectElement(sequenceUI, uiName, false, selectedButton)

		r2:updateActionEditor()
	end

	uiName = r2.logicEntityUIPath.."reactions"
	for s = 0, reactions.Size - 1 do
		local reaction = reactions[s]
		r2:newReaction(false, reaction)
	end
	if reactions.Size > 0 then
		local firstElt = logicEntityReactions:getChild(1)
		local selectedButton = firstElt:find("select")
		selectedButton.pushed = true
		local sequenceUI = reactionSequence:find("sequence_elts")
		assert(sequenceUI)
		
		r2:selectElement(sequenceUI, uiName, false, selectedButton)

		r2:updateReactionEditor()
	end

	logicEntityUI.uc_title = tostring(i18n.get("uiR2EDEventsTriggersEditor")) .. r2:getSelectedInstance().Name
end

----------------------------- CLOSE ACTION / REACTION EDITOR ------------------------------------
function r2:closeActionEditor()
	local actionsEditor = r2:getActionEditor() 
	assert(actionsEditor)

	actionsEditor.active = false
end

function r2:closeReactionEditor()
	local reactionsEditor = r2:getReactionEditor() 
	assert(reactionsEditor)

	reactionsEditor.active = false
end



--------------------------------------------------------------------------------------------------
-------------------------------- LOGIC ENTITY DisplayerProperties -------------------------
--------------------------------------------------------------------------------------------------
--local logicEntityPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function logicEntityPropertySheetDisplayerTable:onPostCreate(instance)
--end
------------------------------------------------
--function logicEntityPropertySheetDisplayerTable:onErase(instance)
--end
------------------------------------------------
--function logicEntityPropertySheetDisplayerTable:onPreHrcMove(instance)		
--end
------------------------------------------------
--function logicEntityPropertySheetDisplayerTable:onPostHrcMove(instance)		
--end
------------------------------------------------
--function logicEntityPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
--end

------------------------------------------------
r2.logicEntityUIUpdated = false
--function logicEntityPropertySheetDisplayerTable:onSelect(instance, isSelected)
	
--	if not isSelected then
--		r2.events:closeEditor()	
--	end
--end

------------------------------------------------
--function logicEntityPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	

--	if not r2.logicEntityUIUpdated or instance ~= r2:getSelectedInstance() then
--		return
--	end


--	if attributeName == "Name" then

--		local actionEditor = r2:getActionEditor() 
--		assert(actionEditor)

--		local reactionEditor = r2:getReactionEditor() 
--		assert(reactionEditor)

--		if actionEditor.active == true then
--			r2:updateActionEditor()
--		end

--		if reactionEditor.active == true then
--			r2:updateReactionEditor()
--		end

--		local logicEntityWnd = getUI("ui:interface:r2ed_logic_entities")
--		assert(logicEntityWnd)

--		logicEntityWnd.uc_title = tostring(i18n.get("uiR2EDEventsTriggersEditor")) .. instance[attributeName]	
--	end
--end	

------------------------------------------------
--function r2:logicEntityPropertySheetDisplayer()	
--	return logicEntityPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end




--------------------------------------------------------------------------------------------------
-------------------------------- LOGIC ENTITY ACTION DisplayerProperties -------------------------
--------------------------------------------------------------------------------------------------
--local logicEntityActionPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onPostCreate(instance)

--	local logicEntity = r2:getSelectedInstance()
--	local logicEntityParent = instance.Parent.Parent.Parent
	
--	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
--		return
--	end

--	r2:newAction(false, instance)
--end
------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onErase(instance)

--	local logicEntity = r2:getSelectedInstance()
--	local logicEntityParent = instance.Parent.Parent.Parent
	
--	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
--		return
--	end

--	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
--	assert(logicEntityUI)

--	local tabUI = logicEntityUI:find("actions")
--	assert(tabUI)

--	local sequence = tabUI:find("sequence_elts")
--	assert(sequence)

--	local uiName = r2.logicEntityUIPath.."actions"

--	local actionList = sequence:find("elements_list")
--	assert(actionList)

--	local actionUI = nil
--	for i=0, actionList.childrenNb-1 do
--		local action = actionList:getChild(i)
--		assert(action)

--		if (action.Env.elementId~=nil) and (action.Env.elementId == instance.InstanceId) then
--			actionUI = action
--			break
--		end
--	end

--	if actionUI~= nil then
--		r2:removeActionUI(actionUI)
--	end
--end
------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onPreHrcMove(instance)

--	local logicEntity = r2:getSelectedInstance()
--	local logicEntityParent = instance.Parent.Parent.Parent
	
--	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
--		return
--	end

	-- if this LogicEntityAction was on the selected logic entity, 
	-- we must update interface by removing the associated action UI
--	local actionsUI = getUI("ui:interface:r2ed_logic_entities"):find("actions")
--	assert(actionsUI)

--	local actionsList = actionsUI:find("elements_list")
--	assert(actionsList)

--	for r=0, actionsList.childrenNb-1 do
--		local action = actionsList:getChild(r)
--		assert(action)

--		if action.Env.elementId~=nil then
--			local actionInst = r2:getInstanceFromId(action.Env.elementId)

--			if actionInst and actionInst.InstanceId == instance.InstanceId then
--				r2:removeActionUI(action)
--				break
--			end
--		end
--	end
--end

------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onPostHrcMove(instance)	

--	local logicEntity = r2:getSelectedInstance()
--	local logicEntityParent = instance.Parent.Parent.Parent
	
--	if not r2.logicEntityUIUpdated then
--		return
--	end

--	if logicEntity~=nil and logicEntity == logicEntityParent then
--		r2:newAction(false, instance)
--	end

	-- update the reaction UI which are associated to this action
--	local reactionsUI = getUI("ui:interface:r2ed_logic_entities"):find("reactions")
--	assert(reactionsUI)

--	local reactionsList = reactionsUI:find("elements_list")
--	assert(reactionsList)

--	for r=0, reactionsList.childrenNb-1 do
--		local reaction = reactionsList:getChild(r)
--		assert(reaction)

--		if reaction.Env.elementId~=nil then
--			local reactionInst = r2:getInstanceFromId(reaction.Env.elementId)

--			if reactionInst and reactionInst.LogicEntityAction == instance.InstanceId then
--				r2:updateReactionText(reaction)
--			end
--		end
--	end
--end
------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onFocus(instance, hasFocus)	
--end

------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onSelect(instance, isSelected)	
--end

------------------------------------------------
--function logicEntityActionPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	

--	if not r2.logicEntityUIUpdated then return end

--	local logicEntity = r2:getSelectedInstance()
--	local logicEntityParent = instance.Parent.Parent.Parent
	
--	if logicEntity~=nil and logicEntity == logicEntityParent then
	
		-- update action UI text
--		local actionsUI = getUI("ui:interface:r2ed_logic_entities"):find("actions")
--		assert(actionsUI)

--		local actionsList = actionsUI:find("elements_list")
--		assert(actionsList)

--		for a=0, actionsList.childrenNb-1 do
--			local action = actionsList:getChild(a)
--			assert(action)

--			if (action.Env.elementId~=nil) and (action.Env.elementId == instance.InstanceId) then
--				r2:updateActionText(action)
--				break
--			end
--		end
--	end

	-- update reaction UI text
--	local reactionsUI = getUI("ui:interface:r2ed_logic_entities"):find("reactions")
--	assert(reactionsUI)

--	local reactionsList = reactionsUI:find("elements_list")
--	assert(reactionsList)

	-- several LogicEntityReaction !!!
--	for r=0, reactionsList.childrenNb-1 do
--		local reaction = reactionsList:getChild(r)
--		assert(reaction)

--		if reaction.Env.elementId~=nil then
--			local reactionInst = r2:getInstanceFromId(reaction.Env.elementId)

--			if reactionInst and reactionInst.LogicEntityAction == instance.InstanceId 
--				and r2:getInstanceFromId(reactionInst.ActionStep)~=nil then
				
--				r2:updateReactionText(reaction)
--			end
--		end
--	end
--end	

------------------------------------------------
--function r2:logicEntityActionPropertySheetDisplayer()	
--	return logicEntityActionPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end


--------------------------------------------------------------------------------------------------
-------------------------------- LOGIC ENTITY REACTION DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local logicEntityReactionPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onPostCreate(instance)	

	local logicEntity = r2:getSelectedInstance()
	local logicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
		return
	end

	r2:newReaction(false, instance)
end
------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onErase(instance)

	local logicEntity = r2:getSelectedInstance()
	local logicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
		return
	end

	local logicEntityUI = getUI("ui:interface:r2ed_logic_entities")
	assert(logicEntityUI)

	local tabUI = logicEntityUI:find("reactions")
	assert(tabUI)

	local sequence = tabUI:find("sequence_elts")
	assert(sequence)

	local uiName = r2.logicEntityUIPath.."reactions"

	local reactionList = sequence:find("elements_list")
	assert(reactionList)

	local reactionUI = nil
	for i=0, reactionList.childrenNb-1 do
		local reaction = reactionList:getChild(i)
		assert(reaction)

		if (reaction.Env.elementId~=nil) and (reaction.Env.elementId == instance.InstanceId) then
			reactionUI = reaction
			break
		end
	end

	if reactionUI~=nil then
		r2:removeReactionUI(reactionUI)
	end
end
------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onPreHrcMove(instance)	

	local logicEntity = r2:getSelectedInstance()
	local logicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
		return
	end

	-- if this LogicEntityReaction was on the selected logic entity, 
	-- we must update interface by removing the associated reaction UI
	local reactionsUI = getUI("ui:interface:r2ed_logic_entities"):find("reactions")
	assert(reactionsUI)

	local reactionsList = reactionsUI:find("elements_list")
	assert(reactionsList)

	for r=0, reactionsList.childrenNb-1 do
		local reaction = reactionsList:getChild(r)
		assert(reaction)

		if reaction.Env.elementId~=nil then
			local reactionInst = r2:getInstanceFromId(reaction.Env.elementId)

			if reactionInst and reactionInst.InstanceId == instance.InstanceId then
				r2:removeReactionUI(reaction)
				break
			end
		end
	end
end
------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onPostHrcMove(instance)	

	local logicEntity = r2:getSelectedInstance()
	local logicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
		return
	end

	r2:newReaction(false, instance)
end
------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
	
	local logicEntity = r2:getSelectedInstance()
	local logicEntityParent = instance.Parent.Parent.Parent
	
	if not r2.logicEntityUIUpdated or logicEntity==nil or logicEntity ~= logicEntityParent then
		return
	end
end	

function r2:findElementUIFromInstance(instance, uiName)

	local sequenceInst = instance.Parent.Parent

	local sequenceUI = getUI("ui:interface:"..uiName)
	assert(sequenceUI)

	local eltsList = sequenceUI:find("elements_list")
	assert(eltsList)

	for i=0,eltsList.childrenNb-1 do
		local element = eltsList:getChild(i)
		if element.Env.elementId == instance.InstanceId then
			return element
		end
	end
end

------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onTargetInstanceCreated(instance, refIdName, refIdIndexInArray)
end
------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onTargetInstanceErased(instance, refIdName, refIdIndexInArray)	
end
------------------------------------------------
function logicEntityReactionPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
end

------------------------------------------------
function r2:logicEntityReactionPropertySheetDisplayer()	
	return logicEntityReactionPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- EVENT TYPE DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
--local eventTypePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onPostCreate(instance)	
--end
------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onErase(instance)
--end
------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onPreHrcMove(instance)		
--end
------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onPostHrcMove(instance)		
--end
------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
--end

------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onSelect(instance, isSelected)	
--end

------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
--end	

------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
--	r2.requestSetNode(instance.InstanceId, "Type", "")
--	r2.requestSetNode(instance.InstanceId, "Value", r2.RefId(""))
--end
------------------------------------------------
--function eventTypePropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
--	if targetAttrName == "Name" then
--		r2:logicEntityActionPropertySheetDisplayer():onAttrModified(instance.Parent)
--	end
--end

------------------------------------------------
--function r2:eventTypePropertySheetDisplayer()	
--	return eventTypePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end



--------------------------------------------------------------------------------------------------
-------------------------------- ACTION STEP DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
--local actionStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onPostCreate(instance)
--end
------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onErase(instance)
	
--	local logicEntity = r2:getInstanceFromId(tostring(instance.Entity))
--	if logicEntity then

--		for i=0, logicEntity:getBehavior().Reactions.Size-1 do
			
--			local reaction = logicEntity:getBehavior().Reactions[i]
--			assert(reaction)

--			if (reaction.ActionStep == instance.InstanceId) then
--				r2.requestEraseNode(reaction.InstanceId, "", -1)
--				break
--			end
--		end
--	end
--end
------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
--end
------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onPostHrcMove(instance)		
--end
------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
--end

------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
--end

------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)		
--end	

------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onTargetInstanceErased(instance, refIdName, refIdIndexInArray)
--	r2.requestEraseNode(instance.InstanceId, "", -1)
--end
------------------------------------------------
--function actionStepPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
--	if targetAttrName == "Name" then
--		r2:logicEntityActionPropertySheetDisplayer():onAttrModified(instance.Parent.Parent)
--	end
--end

------------------------------------------------
--function r2:actionStepPropertySheetDisplayer()	
--	return actionStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end



--------------------------------------------------------------------------------------------------
-------------------------------- ACTION TYPE DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
--local actionTypePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onPostCreate(instance)	
--end
------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onErase(instance)
--end
------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onPreHrcMove(instance)		
--end
------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onPostHrcMove(instance)		
--end
------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
--end

------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onSelect(instance, isSelected)	
--end

------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
--end	

------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
--	r2.requestSetNode(instance.InstanceId, "Type", "")
--	r2.requestSetNode(instance.InstanceId, "Value", r2.RefId(""))
--end
------------------------------------------------
--function actionTypePropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
--	if targetAttrName == "Name" then
--		r2:logicEntityActionPropertySheetDisplayer():onAttrModified(instance.Parent.Parent.Parent)
--	end
--end

------------------------------------------------
--function r2:actionTypePropertySheetDisplayer()	
--	return actionTypePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end



--------------------------------------------------------------------------------------------------
-------------------------------- CONDITION STEP DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
--local conditionStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onPostCreate(instance)
--end
------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onErase(instance)
--end
------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
--end
------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onPostHrcMove(instance)		
--end
------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
--end

------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
--end

------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
--end	

------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
--	r2.requestEraseNode(instance.InstanceId, "", -1)
--end
------------------------------------------------
--function conditionStepPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
--	if targetAttrName == "Name" then
--		r2:logicEntityActionPropertySheetDisplayer():onAttrModified(instance.Parent.Parent)
--	end
--end

------------------------------------------------
--function r2:conditionStepPropertySheetDisplayer()	
--	return conditionStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end




--------------------------------------------------------------------------------------------------
-------------------------------- CONDITION TYPE DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
--local conditionTypePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onPostCreate(instance)	
--end
------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onErase(instance)
--end
------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onPreHrcMove(instance)		
--end
------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onPostHrcMove(instance)		
--end
------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
--end

------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onSelect(instance, isSelected)	
--end

------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	
--end	

------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
--	r2.requestSetNode(instance.InstanceId, "Type", "")
--	r2.requestSetNode(instance.InstanceId, "Value", r2.RefId(""))
--end
------------------------------------------------
--function conditionTypePropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
--	if targetAttrName == "Name" then
--		r2:logicEntityActionPropertySheetDisplayer():onAttrModified(instance.Parent.Parent.Parent)
--	end
--end

------------------------------------------------
--function r2:conditionTypePropertySheetDisplayer()	
--	return conditionTypePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
--end


















