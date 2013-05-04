r2.dialogs = {

uiId = "ui:interface:r2ed_dialogs",
ownCreatedInstances = {},
openFirst = nil,
elementsIdCounter = 0,
elementOrder = true,
fromEmoteIdToName= {},
maxAndMin = true,
eltTemplateParams =	{
						selectElt="r2.dialogs:selectElement()", 
						openEltEditor="", 
						maxMinElt="r2.dialogs:maximizeMinimizeElement()", 
						removeElt="r2.dialogs:removeElementInst()",
						colOver="120 150 140 100",
						colPushed="120 150 140 255",
						multiMaxLine="3"
					},
elementEditorTemplate = "template_edit_chat",
elementInitialName=i18n.get("uiR2EdChat"):toUtf8(),
sequenceInitialName=i18n.get("uiR2EDDialog"):toUtf8(),

keepOpenedEditor = false,

currentEltUIID = nil, -- initialisé quand l'editeur est ouvert ou fermé

whoToWhoTranslation =	{
							["_DM"] = i18n.get("uiR2EdDonjonMaster"):toUtf8(),
							["_System"] = i18n.get("uiR2EdSystem"):toUtf8(),
							["_Nobody"] = i18n.get("uiR2EdNobody"):toUtf8(),
						},

maxVisibleLine = 10,
}



-- sequence --------------------------------------------------
function r2.dialogs:currentSequUI()
	return getUI(self.uiId):find("sequence_elts")
end

function r2.dialogs:currentSequInstId()
	return self:currentSequUI().Env.InstanceId
end

-- initialisé quand selection dialog dans menu
function r2.dialogs:setSequUIInstId(sequUI, id)
	sequUI.Env.InstanceId = id
end

function r2.dialogs:currentSequInst()
	return r2:getInstanceFromId(self:currentSequInstId())
end

-- element ---------------------------------------------------
function r2.dialogs:currentEltUIId()
	return self.currentEltUIID
end

function r2.dialogs:currentEltUI()
	if self.currentEltUIID then
		return getUI(self.currentEltUIID)
	end
	return nil
end

function r2.dialogs:setCurrentEltUIId(id)
	self.currentEltUIID = id
end

function r2.dialogs:currentEltInstId()
	if self.currentEltUIID then 
		return self:currentEltUI().Env.InstanceId
	end
	return nil
end

function r2.dialogs:currentEltInst()
	if self.currentEltUIID and self:currentEltInstId() then
		return r2:getInstanceFromId(self:currentEltInstId())
	end
	return nil
end

-- updated element and/or sequence (not necessary the same as current sequence or element)
function r2.dialogs:updatedSequUI()
	return self:currentSequUI()
end

function r2.dialogs:setUpdatedSequUIId(sequUIId)
end

function r2.dialogs:updatedEltUI()
	return self:currentEltUI()
end

function r2.dialogs:setUpdatedEltUIId(eltUIId)
end


------------------ INIT DIALOGS EDITOR -------------------------
function r2.dialogs:initEditor()

	local emoteTable = initEmotesMenu("ui:interface:r2ed_triggers_menu", "")
	for id, name in pairs(emoteTable) do
		self.fromEmoteIdToName[id] = name
	end
end

------ OPEN EDITOR ------------------------------------------

function r2.dialogs:checkDialog()
	
	local dialogUI = getUI(self.uiId)

	local dialog = r2:getSelectedInstance()
	if not dialog or not dialog:isKindOf("ChatSequence") then
		dialogUI.active = false
	end
end

function r2.dialogs:openEditor()

	local dialogUI = getUI(self.uiId)

	if not dialogUI.active then

		self:cleanEditor()

		local sequenceUI = self:currentSequUI()
		assert(sequenceUI)

		-- select selected dialog in scene or first dialog of current act 
		local allDialogs = {}
		r2:getCurrentAct():appendInstancesByType(allDialogs, "ChatSequence")
		r2.Scenario:getBaseAct():appendInstancesByType(allDialogs, "ChatSequence")

		if table.getn(allDialogs) > 0 then
			sequenceUI.active = true

			local selectedInstance = r2:getSelectedInstance()
			if selectedInstance and selectedInstance:isKindOf("ChatSequence") then
				self:selectSequence(selectedInstance.InstanceId)
			else
				local firstDialog
				for k, dialog in pairs(allDialogs) do
					firstDialog = dialog
					break
				end
				self:selectSequence(firstDialog.InstanceId)
			end
		else
			sequenceUI.active = false	
			local dialogName = dialogUI:find("dialogMenu"):find("menu"):find("text")
			assert(dialogName)
			dialogName.uc_hardtext = i18n.get("uiR2EdNoSelelection")
		end

		-- active editor
		dialogUI.active = true
		dialogUI:updateCoords()
		
		if self.openFirst == nil then
			self.openFirst = true
			dialogUI:center()
		end
	else
		setTopWindow(dialogUI)
		dialogUI:blink(1)
	end
end

------ NEW SEQUENCE ------------------------------------------

function r2.dialogs:newSequenceInst(x, y, z)

	r2.requestNewAction(i18n.get("uiR2EDNewChatSequenceAction"))
	local dialog = r2.newComponent("ChatSequence")
	assert(dialog)
	
	dialog.Base = r2.Translator.getDebugBase("palette.entities.botobjects.dialog")
	dialog.Name = r2:genInstanceName(i18n.get("uiR2EDDialog")):toUtf8()	
	dialog.Position.x = x
	dialog.Position.y = y
	dialog.Position.z = r2:snapZToGround(x, y)
	
	if dialog.onComponentCreated then
		dialog:onComponentCreated()
	end
	
	r2:setCookie(dialog.InstanceId, "DisplayProp", 1)
	r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", dialog)
end


------ CLOSE EDITOR ------------------------------------------
function r2.dialogs:closeEditor()
	
	self:updateSaysWhat()

	r2.logicUI:closeEditor(r2.dialogs)
end

------ CLEAN EDITOR ------------------------------------------
function r2.dialogs:cleanEditor()

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

	-- hide maximize/minimize
	local minElts = sequenceUI:find("minimize_elements")
	assert(minElts)
	local maxElts = sequenceUI:find("maximize_elements")
	assert(maxElts)
	minElts.active = false
	maxElts.active = false

	-- hide down/up buttons
	local upDown = sequenceUI:find("order_group")
	assert(upDown)
	upDown.active = false
end

------ INIT DIALOGS MENU ------------------------------------------
function r2.dialogs:initDialogsMenu()

	local dialogMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(dialogMenu)

	dialogMenu = dialogMenu:getRootMenu()
	assert(dialogMenu)

	dialogMenu:reset()

	local allDialogs = {}
	r2:getCurrentAct():appendInstancesByType(allDialogs, "ChatSequence")

	local uc_dialog = ucstring()
	for k, dialog in pairs(allDialogs) do
		uc_dialog:fromUtf8(dialog.Name)
		dialogMenu:addLine(uc_dialog, "lua", "r2:setSelectedInstanceId('" .. dialog.InstanceId .."')", dialog.InstanceId)
	end

	dialogMenu:setMaxVisibleLine(self.maxVisibleLine)

	r2.logicUI:openLogicMenu(getUICaller())
end

------ SELECT ELEMENT --------------------------------------------
function r2.dialogs:selectElement(selectedButtonElt)
	
	r2.logicComponents:selectElement(self, selectedButtonElt)

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)

	local editElt = sequenceUI:find("edit_element")
	assert(editElt)

	if editElt.active then
		local editBox = editElt:find("says"):find("edit_box_group")
		assert(editBox)
		editBox:setFocusOnText()
	end
end

------ SELECT SEQUENCE --------------------------------------
function r2.dialogs:selectSequence(dialogId)

	local ui = getUI(self.uiId)
	assert(ui)

	-- clean editor
	self:cleanEditor()

	self:setSequUIInstId(self:currentSequUI(), dialogId)
	local sequenceInst = self:currentSequInst()
	if sequenceInst==nil then return end

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)

	sequenceUI.active = true

	-- create dialog editor
	self:createElementEditor()

	-- init dialog UI -------------------------
	-- add elts to sequence
	local elts = sequenceInst.Components
	for e = 0, elts.Size - 1 do
		self:newElementUI(elts[e])	
	end

	-- initialize repeat type
	local repeatButton = ui:find("repeat"):find("toggle_butt")
	assert(repeatButton)
	repeatButton.pushed = (sequenceInst.Repeating == 0)

	-- dialog editor title
	local uc_dialog = ucstring()
	uc_dialog:fromUtf8(i18n.get("uiR2EDChatSequenceEditor"):toUtf8() .. sequenceInst.Name)
	ui.uc_title = uc_dialog 

	-- dialog name in menu box
	local dialogName = ui:find("dialogMenu"):find("menu"):find("text")
	assert(dialogName)
	uc_dialog:fromUtf8(sequenceInst.Name)
	dialogName.uc_hardtext = uc_dialog

	-- initialize maximize/minimize
	local maxElts = sequenceUI:find("maximize_elements")
	assert(maxElts)
	maxElts.active = (elts.Size > 0)

	r2.logicComponents:selectSequence(r2.dialogs)
end

------ CREATE EDITOR -----------------------------------------------
function r2.dialogs:createElementEditor()

	r2.logicComponents:createElementEditor(r2.dialogs)

	-- not necessary current sequence UI
	local sequenceUI = self:updatedSequUI()

	local newEditorElt = sequenceUI:find("edit_element")
	assert(newEditorElt)

	local editBox = newEditorElt:find("edit_box_group")
	assert(editBox)

	local scroll = newEditorElt:find("edit_box_scroll_ed")
	assert(scroll)

	scroll:setTarget(editBox.id)
end

------ OPEN ELEMENT EDITOR -----------------------------------------------
function r2.dialogs:updateElementEditor()

	local instanceChat = self:currentEltInst()

	local dialogUI = getUI(self.uiId)
	assert(dialogUI)

	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)
	
	local chatEditor = sequenceUI:find("edit_element")
	assert(chatEditor)

	-- chat name
	local chatName = chatEditor:find("name")
	assert(chatName)

	-- time
	local minutesText = chatEditor:find("minutes"):find("text")
	assert(minutesText)

	local secondsText = chatEditor:find("seconds"):find("text")
	assert(secondsText)

	-- update NPC name lists
	local whoMenuText = chatEditor:find("whoMenu"):find("text")
	assert(whoMenuText)

	local toWhoMenuText = chatEditor:find("toWhoMenu"):find("text")
	assert(toWhoMenuText)

	local editBox = chatEditor:find("says"):find("edit_box_group")
	assert(editBox)

	local emoteButtonText = chatEditor:find("emote"):find("menu"):find("text")
	assert(emoteButtonText)

	if instanceChat then

		local uc_chat = ucstring()
		
		local index = r2.logicComponents:searchElementIndex(instanceChat)
		if index~= nil then
			uc_chat:fromUtf8(self.elementInitialName.." "..index.." : ")
		else
			uc_chat:fromUtf8(self.elementInitialName.." : ")
		end
		chatName.uc_hardtext = uc_chat

		-- after value
		local time = instanceChat.Time
		assert(time)
		local minNb, secNb = self:calculMinSec(time)

		minutesText.uc_hardtext = tostring(minNb)
		secondsText.uc_hardtext = tostring(secNb)
			
		-- who
		local whoInst = r2:getInstanceFromId(tostring(instanceChat.Actions[0].Who))
		local hideToWhoAndEmote = false
		if whoInst then
			uc_chat:fromUtf8(whoInst.Name)
		elseif instanceChat.Actions[0].WhoNoEntity=="_System" then
			uc_chat = i18n.get("uiR2EdSystem")
			hideToWhoAndEmote = true
		elseif instanceChat.Actions[0].WhoNoEntity=="_DM" then
			uc_chat = i18n.get("uiR2EdDonjonMaster")
			hideToWhoAndEmote = true
		else
			uc_chat = ucstring("")
		end
		whoMenuText.uc_hardtext = uc_chat
		
		-- says what
		editBox:cancelFocusOnText()
		local textID = instanceChat.Actions[0].Says
		if textID ~= "" and r2:getInstanceFromId(textID) then
			local uc_says_what = ucstring()
			uc_says_what:fromUtf8(r2:getInstanceFromId(textID).Text)
			editBox.uc_input_string = uc_says_what
		else
			editBox.uc_input_string = ""
		end

		local breakAtEnd = 0
		
		if instanceChat.BreakAtEnd then
			breakAtEnd = instanceChat.BreakAtEnd
		end
		local breakAtEndButton = chatEditor:find("break_group"):find("toggle_butt")
		assert(breakAtEndButton)
		breakAtEndButton.pushed = (breakAtEnd == 0)

		chatEditor:find("toWhoMenu").active = not hideToWhoAndEmote
		chatEditor:find("emote").active = not hideToWhoAndEmote
		if hideToWhoAndEmote then
			return
		end

		-- to who
		local toWhoInst = r2:getInstanceFromId(tostring(instanceChat.Actions[0].Facing))
		if toWhoInst then
			uc_chat:fromUtf8(toWhoInst.Name)
		else
			uc_chat = i18n.get("uiR2EdNobody")
		end
		toWhoMenuText.uc_hardtext = uc_chat

		-- emote
		local emoteName = self.fromEmoteIdToName[instanceChat.Actions[0].Emote]
		if emoteName then
			uc_chat:fromUtf8(emoteName)
		else
			uc_chat = i18n.get("uiR2EdNoElt")
		end
		emoteButtonText.uc_hardtext = uc_chat

	else
		local uc_chat = ucstring()
		uc_chat:fromUtf8(self.elementInitialName.." : ")
		chatName.uc_hardtext = uc_chat

		minutesText.uc_hardtext = tostring(0)
		secondsText.uc_hardtext = tostring(0)

		whoMenuText.uc_hardtext = ""

		editBox.uc_input_string = ""

		toWhoMenuText.uc_hardtext = ""
		emoteButtonText.uc_hardtext = i18n.get("uiR2EdNoElt")
	end
end


----- CLOSE ELEMENT EDITOR ---------------------------------------------
function r2.dialogs:closeElementEditor()
	
	r2.logicComponents:closeElementEditor(r2.dialogs)
end

------ NEW ELEMENT INST ------------------------------------------
function r2.dialogs:newElementInst()

	r2.requestNewAction(i18n.get("uiR2EDNewChatStepAction"))

	local chatStep = r2.newComponent("ChatStep")

	local sequenceInst = self:currentSequInst()
	assert(sequenceInst)

	if sequenceInst.Components.Size == 0 then
		chatStep.Time = 0
	else
		chatStep.Time = 3
	end

	local chatAction = r2.newComponent("ChatAction")
	chatAction.WhoNoEntity = "_System"
	table.insert(chatStep.Actions, chatAction)

	r2.requestInsertNode(sequenceInst.InstanceId, "Components", -1, "", chatStep)

	self.ownCreatedInstances[chatStep.InstanceId] = true

	r2.logicComponents:newElementInst(r2.dialogs)
end

------ NEW ELEMENT UI ------------------------------------------
function r2.dialogs:newElementUI(newInst)
	r2.logicUI:newElementUI(r2.dialogs, newInst, true)
end

------ REMOVE ELEMENT INST ----------------------------------------
function r2.dialogs:removeElementInst()

	r2.requestNewAction(i18n.get("uiR2EDRemoveLogicElementAction"))

	local chatStep = self:currentEltInst()
	assert(chatStep)
	local says = chatStep.Actions[0].Says

	r2.logicComponents:removeElementInst(r2.dialogs)

	r2.unregisterTextFromId(chatStep.Actions[0].Says)

	r2.requestEndAction()
end

------ REMOVE ELEMENT UI -------------------------------------------
function r2.dialogs:removeElementUI(removedEltUI)
	r2.logicUI:removeElementUI(r2.dialogs, removedEltUI)
end

------ REMOVE SEQUENCE UI -------------------------------------------
function r2.dialogs:removeSequenceUI(instance)

	local allDialogs = {}
	if (not r2:getCurrentAct()) then return end
	r2:getCurrentAct():appendInstancesByType(allDialogs, "ChatSequence")
	-- be careful : instance is always in ChatSequences list 
	-- it's not removed yet when onErase is called
	if table.getn(allDialogs) > 1 then
		local firstDialog
		for k, dialog in pairs(allDialogs) do
			if dialog~=instance then
				firstDialog = dialog
				break
			end
		end
		self:selectSequence(firstDialog.InstanceId)
	else
		self.keepOpenedEditor = false
	end
end

------ UPDATE SEQUENCE UI -------------------------------------------
function r2.dialogs:updateSequenceUI(sequenceInst)

	local ui = getUI(self.uiId)
	assert(ui)

	-- initialize repeat type
	local repeatButton = ui:find("repeat"):find("toggle_butt")
	assert(repeatButton)
	repeatButton.pushed = (sequenceInst.Repeating == 0)

	-- dialog editor title
	local uc_title = ucstring()
	uc_title:fromUtf8(i18n.get("uiR2EDChatSequenceEditor"):toUtf8() .. sequenceInst.Name)
	ui.uc_title = uc_title

	-- dialog name in menu box
	local dialogName = ui:find("dialogMenu"):find("menu"):find("text")
	assert(dialogName)
	dialogName.uc_hardtext = sequenceInst.Name
end

------ UP ELEMENT INST -------------------------------------------
function r2.dialogs:upElementInst()
	r2.logicComponents:upElementInst(r2.dialogs)
end

------ DOWN ELEMENT INST -----------------------------------------
function r2.dialogs:downElementInst()
	r2.logicComponents:downElementInst(r2.dialogs)
end

------ MAX/MIN ELEMENTS UI -------------------------------------------
function r2.dialogs:maximizeMinimizeElements()
	r2.logicUI:maximizeMinimizeElements(r2.dialogs)
end

------ MAX/MIN ELEMENT UI -------------------------------------------
function r2.dialogs:maximizeMinimizeElement(element, allMinimize)

	r2.logicUI:maximizeMinimizeElement(element, allMinimize)
end

------ DOWN/UP ELEMENT UI -------------------------------------------
function r2.dialogs:downUpElementUI(elementUI, instance)
	r2.logicUI:downUpElementUI(r2.dialogs, elementUI, instance)
end

------ UPDATE ELEMENT TITLE -------------------------------------------
function r2.dialogs:updateElementTitle(chatUI)
	r2.logicComponents:updateElementTitle(r2.dialogs, chatUI, true)
end

------ UPDATE CHAT TEXT -------------------------------------------
function r2.dialogs:updateElementUI(elementUI)
	
	local chatStep = r2:getInstanceFromId(r2.logicUI:getEltUIInstId(elementUI))   
	assert(chatStep)

	local chatText = elementUI:find("text_list")
	assert(chatText)
	chatText:clear()

	local who = tostring(chatStep.Actions[0].Who)
	if who=="" then who=chatStep.Actions[0].WhoNoEntity end
	if who and who ~= "" then
		
		local text = ""
		local textEmpty = true

		local says = chatStep.Actions[0].Says
		
		if r2:getInstanceFromId(says)~=nil and r2:getInstanceFromId(says).Text ~= "" then
			text = "\n" .. r2:getInstanceFromId(says).Text 
			textEmpty = false
		end
		--text = text.."\n"

		-- Add a white line that indicate that the dialog pause at end (ChatStep) in reduced view
		chatText:addColoredTextChild(text, 220, 140, 100, 255)
		if chatStep.BreakAtEnd and chatStep.BreakAtEnd == 1 then
			chatText:addColoredTextChild("\n"..i18n.get("uiR2EDThenPause"):toUtf8(), 255, 255, 255, 255)
		end

		local sep = elementUI:find("sep")
		assert(sep)
		if textEmpty == false then
			sep.active = true
		else
			chatText:clear()
			sep.active = false
		end
	end

	r2.logicUI:updateElementUI(r2.dialogs, elementUI)
end

-------------------- INIT TIME MENU --------------------------------
function r2.dialogs:initTimeMenu(timeFunction, isHours)

	local timeMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(timeMenu)

	local timeMenu = timeMenu:getRootMenu()
	assert(timeMenu)

	timeMenu:reset()

	for i=0,9 do
		timeMenu:addLine(ucstring(tostring(i)), "lua", timeFunction .. "(" .. tostring(i) .. ")", tostring(i))
	end

	if isHours then
		timeMenu:addLine(ucstring(tostring(10)), "lua", timeFunction .. "(" .. tostring(10) .. ")", tostring(10))
	else

		local lineNb = 9
		for i=10, 50, 10 do
			local lineStr = tostring(i).."/"..tostring(i+9)
			timeMenu:addLine(ucstring(lineStr), "", "", tostring(i))
			lineNb = lineNb+1

			timeMenu:addSubMenu(lineNb)
			local subMenu = timeMenu:getSubMenu(lineNb)

			for s=0,9 do
				lineStr = tostring(i+s) 
				subMenu:addLine(ucstring(lineStr), "lua", timeFunction .. "(" .. tostring(i+s) .. ")", lineStr)
			end

			timeMenu:setMaxVisibleLine(self.maxVisibleLine)
		end
	end

	timeMenu:setMaxVisibleLine(self.maxVisibleLine)

	r2.logicUI:openLogicMenu(getUICaller())
end

----- CHAT MINUTES -----------------------------------------------------------------
function r2.dialogs:chatAfterMinutes(minNb)
	
	-- recover "says what" (equiv change focus)
	self:updateSaysWhat()

	local chatStep = self:currentEltInst()
	assert(chatStep)

	local hours, mins, secs = r2.logicComponents:calculHourMinSec(chatStep.Time)
	self:setTime(minNb*60 + secs)
end

-------------------- CHAT SECONDS -----------------------------------------------------------------
function r2.dialogs:chatAfterSeconds(secNb)
	
	-- recover "says what" (equiv change focus)
	self:updateSaysWhat()

	local chatStep = self:currentEltInst()
	assert(chatStep)

	local hours, mins, secs = r2.logicComponents:calculHourMinSec(chatStep.Time)
	self:setTime(mins*60 + secNb)
end

---- SET TIME --------------------------------------------------------------------
function r2.dialogs:setTime(secNb)

	r2.requestNewAction(i18n.get("uiR2EDSetDialogTime"))

	local chatStepId = self:currentEltInstId()
	assert(chatStepId)
	
	r2.requestSetNode(chatStepId, "Time", secNb)
end

---- SET BREAK AT END OF CHATSTEP --------------------------------------------------------------------
function r2.dialogs:setChatBreak()
	self:updateSaysWhat()
	local mustBreak = 0
	if getUICaller().pushed==false then mustBreak = 1 end

	r2.requestNewAction(i18n.get("uiR2EDSetDialogChatBreak"))

	local chatStepId = self:currentEltInstId()
	assert(chatStepId)
	r2.requestSetNode(chatStepId, "BreakAtEnd", mustBreak)
end

------- INIT ENTITIES MENU -----------------------------------------
function r2.dialogs:initWhoMenu(whoFunction, towho)

	local menuName = "ui:interface:r2ed_triggers_menu"
	
	local whoMenu = getUI(menuName)
	local whoMenu = whoMenu:getRootMenu()
	assert(whoMenu)

	whoMenu:reset()

	local npcTable = r2.Scenario:getAllInstancesByType("Npc")

	if towho == true then
		whoMenu:addLine(i18n.get("uiR2EdNobody"), "lua", whoFunction.."('" ..i18n.get("uiR2EdNobody"):toUtf8().. "')", "Nobody")
	else
		whoMenu:addLine(i18n.get("uiR2EdSystem"), "lua", whoFunction.."('_System')", "System")
		whoMenu:addLine(i18n.get("uiR2EdDonjonMaster"), "lua", whoFunction.."('_DM')", "DonjonMaster")	
	end
	
	local uc_who = ucstring()
	for key, npc in pairs(npcTable) do
		local addLine = true

		if not npc:isBotObject() and not npc:isPlant() and not r2.isCreature(npc.InstanceId) then
			if towho == true then
				local chatStepInst= self:currentEltInst()
				assert(chatStepInst)

				local whoId = chatStepInst.Actions[0].Who
				if whoId~="" and whoId == npc.InstanceId then
					addLine = false	
				end
			end
			if addLine then
				uc_who:fromUtf8(npc.Name)
				whoMenu:addLine(uc_who, "lua", whoFunction.."('" ..npc.InstanceId.. "')", npc.InstanceId)	
			end
		end
	end

	whoMenu:setMaxVisibleLine(self.maxVisibleLine)
	
	r2.logicUI:openLogicMenu(getUICaller())
end

---- UPDATE SAYS WHAT -----------------------------------------------------------------
function r2.dialogs:updateSaysWhat()

	-- recover "says what" (equiv change focus)
	local sequenceUI = self:currentSequUI()
	if sequenceUI then
		local chatEditor = sequenceUI:find("edit_element")
		if chatEditor then
			local saysWhat = chatEditor:find("says"):find("edit_box_group").uc_input_string:toUtf8()
			self:setSaysWhat(saysWhat)
		end
	end
end

---- SET WHO --------------------------------------------------------------------------
function r2.dialogs:setWho(who)

	-- recover "says what" (equiv change focus)
	self:updateSaysWhat()

	r2.requestNewAction(i18n.get("uiR2EDChatSetWhoAction"))

	local chatStep = self:currentEltInst()
	assert(chatStep)
	
	if r2:getInstanceFromId(who) then
		r2.requestSetNode(chatStep.Actions[0].InstanceId, "Who", r2.RefId(who))
		r2.requestSetNode(chatStep.Actions[0].InstanceId, "WhoNoEntity", "")

		if who == chatStep.Actions[0].Facing then
			self:setToWho(i18n.get("uiR2EdNobody"):toUtf8())
		end
	else
		r2.requestSetNode(chatStep.Actions[0].InstanceId, "Who", r2.RefId(""))
		r2.requestSetNode(chatStep.Actions[0].InstanceId, "WhoNoEntity", who)

		r2.requestSetNode(chatStep.Actions[0].InstanceId, "Facing", r2.RefId(""))
	end
end

----- SET TO WHO -----------------------------------------------------------------------
function r2.dialogs:setToWho(toWho)

	-- recover "says what" (equiv change focus)
	self:updateSaysWhat()

	r2.requestNewAction(i18n.get("uiR2EDChatSetToWhoAction"))
	
	local chatStep = self:currentEltInst()
	assert(chatStep)

	if toWho == i18n.get("uiR2EdNobody"):toUtf8() then toWho="" end
	r2.requestSetNode(chatStep.Actions[0].InstanceId, "Facing", r2.RefId(toWho))
end

-------------------- SAYS WHAT -------------------------------------------
function r2.dialogs:setSaysWhat(whatText)

	if whatText == nil then
		whatText = getUICaller().uc_input_string:toUtf8()
	end

	r2.requestNewAction(i18n.get("uiR2EDChatSetSayWhatAction"))
	
	local chatStep = self:currentEltInst()
	if chatStep ~= nil then
		local oldSays = r2:getInstanceFromId(chatStep.Actions[0].Says)
		if (oldSays==nil and whatText~="") or (oldSays~=nil and oldSays.Text~=whatText) then
		
			r2.unregisterTextFromId(chatStep.Actions[0].Says)
			local whatId=r2.registerText(whatText).InstanceId
			r2.requestSetNode(chatStep.Actions[0].InstanceId, "Says", whatId)
		end
	end

	r2.requestEndAction()
end

----- INIT EMOTE MENU ---------------------------------------------------
function r2.dialogs:initEmoteMenu()

	local menuName = "ui:interface:r2ed_triggers_menu"
	
	local emoteMenu = getUI(menuName)
	local emoteMenu = emoteMenu:getRootMenu()
	assert(emoteMenu)

	emoteMenu:reset()
	initEmotesMenu(menuName, "r2.dialogs:setEmote")
	r2.logicUI:openLogicMenu(getUICaller())
end

----- SET EMOTE  -------------------------------------------------------
function r2.dialogs:setEmote(emoteId)
	
	-- recover "says what" (equiv change focus)
	self:updateSaysWhat()

	r2.requestNewAction(i18n.get("uiR2EDChatSetEmoteAction"))

	local chatStep = self:currentEltInst()
	assert(chatStep)

	r2.requestSetNode(chatStep.Actions[0].InstanceId, "Emote", emoteId)
end

------ REPEAT SEQUENCE -------------------------------------------
function r2.dialogs:repeatSequence()

	r2.requestNewAction(i18n.get("uiR2EDChatRepeatSequenceAction"))

	local dialogInst = self:currentSequInst()
	if dialogInst==nil then return end

	local sequenceType = 1
	if getUICaller().pushed==true then sequenceType = 0 end

	r2.requestSetNode(dialogInst.InstanceId, "Repeating", sequenceType)	
end

------- UTIL ---------------------------------------------------
function r2.dialogs:calculMinSec(totalSecNb)

	assert(totalSecNb)
	local minNb, secNb = 0, totalSecNb
	while secNb > 59 do
		minNb = minNb+1
		secNb = secNb - 60
	end

	return minNb, secNb
end







--------------------------------------------------------------------------------------------------
-------------------------------- CHAT STEP DisplayerProperties------------------------------------
--------------------------------------------------------------------------------------------------
local chatStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function chatStepPropertySheetDisplayerTable:onPostCreate(instance)

	if instance.Parent.Parent.InstanceId == r2.dialogs:currentSequInstId() then
		r2.dialogs:newElementUI(instance)
	end	
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onErase(instance)

	instance.User.Deleted = true

	local elementUI = r2.logicUI:findElementUIFromInstance(r2.dialogs, instance)
	
	if elementUI then
		r2.dialogs:removeElementUI(elementUI)
	end
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onPostHrcMove(instance)

	local elementUI = r2.logicUI:findElementUIFromInstance(r2.dialogs, instance)
	if elementUI then
		r2.dialogs:downUpElementUI(elementUI, instance)
	end
end
------------------------------------------------
function chatStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function chatStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function chatStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
	
	local elementUI = r2.logicUI:findElementUIFromInstance(r2.dialogs, instance)
	if elementUI then
		r2.dialogs:updateElementUI(elementUI)
	end
end	

--------------------------------------------------
function r2:chatStepPropertySheetDisplayer()	
	return chatStepPropertySheetDisplayerTable 
end



--------------------------------------------------------------------------------------------------
-------------------------------- CHAT SEQUENCE DisplayerProperties ---------------------------
--------------------------------------------------------------------------------------------------
local chatSequencePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onErase(instance)

	instance.User.Deleted = true

	local currentDialogId = r2.dialogs:currentSequInstId()
	if currentDialogId ~= instance.InstanceId then return end

	r2.dialogs:removeSequenceUI(instance)
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
r2.dialogEditorRemainsOpened = false
function chatSequencePropertySheetDisplayerTable:onSelect(instance, isSelected)	

	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)

	local dialogUI = getUI(r2.dialogs.uiId)
	assert(dialogUI)
	
	if not isSelected then
		r2.dialogs.keepOpenedEditor = dialogUI.active
		dialogUI.active = false

	elseif isSelected and r2.dialogs.keepOpenedEditor==true then
		r2.dialogs.keepOpenedEditor = false
		dialogUI.active = true
		r2.dialogs:selectSequence(instance.InstanceId)	
	end
end

------------------------------------------------
function chatSequencePropertySheetDisplayerTable:onAttrModified(instance, attributeName)

	if attributeName == "Name" or attributeName == "Repeating" then

		local currentDialogId = r2.dialogs:currentSequInstId()
		if currentDialogId == instance.InstanceId then 
			r2.dialogs:updateSequenceUI(instance)
		end
		
		if r2.events.filteredLogicEntityId==instance.InstanceId then
			r2.events:updateSequenceUI()
		end
	end			
end	

------------------------------------------------
function r2:chatSequencePropertySheetDisplayer()	
	return chatSequencePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end



--------------------------------------------------------------------------------------------------
-------------------------------- CHAT ACTION DisplayerProperties -----------------------
--------------------------------------------------------------------------------------------------
local chatActionPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onPostCreate(instance)
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
end	

------------------------------------------------
function chatActionPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	if refIdName == "Who" then
		r2.requestSetNode(instance.InstanceId, "Who", r2.RefId(""))
		r2.requestSetNode(instance.InstanceId, "WhoNoEntity", "_System")
	elseif refIdName == "Facing" then
		r2.requestSetNode(instance.InstanceId, "Facing", r2.RefId(""))
	end
end
------------------------------------------------
function chatActionPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
end

------------------------------------------------
function r2:chatActionPropertySheetDisplayer()	
	return chatActionPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end
