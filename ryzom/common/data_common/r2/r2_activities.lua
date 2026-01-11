r2.activities = {

uiId = "ui:interface:r2ed_activities",
sequenceEditorId = "ui:interface:r2ed_edit_activity_sequence",
ownCreatedInstances = {},
openFirst = nil,
elementsIdCounter = 0,
sequencesNb = 0,
sequencesIdCounter = 0,
elementOrder = true,
maxAndMin = false,
eltTemplateParams =	{
						selectElt="r2.activities.selectElement()", 
						openEltEditor="", 
						maxMinElt="", 
						removeElt="r2.activities.removeElementInst()",
						colOver="200 120 80 120",
						colPushed="200 120 80 255",
						multiMaxLine="3"
					},
elementEditorTemplate = "template_edit_activity",
elementInitialName=i18n.get("uiR2EdActivity"):toUtf8(),
sequenceInitialName=i18n.get("uiR2EdSeq"):toUtf8(),

isInitialized = false,

maxSequences = 7,
maxElements = 14,

firstSequence = {},

-- element ---------------------------------------------------
currentEltUIID = nil, -- initialisé quand l'editeur est ouvert ou fermé

updatedSequUIID = nil,	-- creation of sequence or of one of this elements
updatedEltUIID = nil,	-- creation or update of element


activityTypeTranslation =   {
						["Stand Still"]				={trans=i18n.get("uiR2EdStandStill"):toUtf8(),			zone=""},
						["Follow Route"]			={trans=i18n.get("uiR2EdFollowRoad"):toUtf8(),			zone="Road"},
						["Patrol"]					={trans=i18n.get("uiR2EdPatrol"):toUtf8(),				zone="Road"},
						["Repeat Road"]				={trans=i18n.get("uiR2EdRepeatRoad"):toUtf8(),			zone="Road"},
						["Wander"]					={trans=i18n.get("uiR2EdWander"):toUtf8(),				zone="Region"},
						["Stand On Start Point"]	={trans=i18n.get("uiR2EdStandOnStartPoint"):toUtf8(),	zone=""},
						["Go To Start Point"]		={trans=i18n.get("uiR2EdGoToStartPoint"):toUtf8(),		zone=""},
						["Go To Zone"]				={trans=i18n.get("uiR2EdGoToZone"):toUtf8(),			zone="Region"},
						["Sit Down"]				={trans=i18n.get("uiR2EdSitDown"):toUtf8(),				zone=""},
						["Stand Up"]				={trans=i18n.get("uiR2EdStandUp"):toUtf8(),				zone=""},
						["Rest In Zone"]			={trans=i18n.get("uiR2EDRest"):toUtf8(),				zone="Region"},
						["Feed In Zone"]			={trans=i18n.get("uiR2EDFeed"):toUtf8(),				zone="Region"},
						["Work In Zone"]			={trans=i18n.get("uiR2EDWork"):toUtf8(),				zone="Region"},
						["Hunt In Zone"]			={trans=i18n.get("uiR2EDHunt"):toUtf8(),				zone="Region"},
						["Guard Zone"]				={trans=i18n.get("uiR2EDGuard"):toUtf8(),				zone="Region"},	
					},

timeLimitsTranslation = {
							["No Limit"]	=i18n.get("uiR2EdNoTimeLimit"):toUtf8(),
							["Chat"]		=i18n.get("uiR2EdWhileChat"):toUtf8(),
							["Few Sec"]		=i18n.get("uiR2EdForCertainTime"):toUtf8(),
						},

timeLimitsProperties = {
							[i18n.get("uiR2EdNoTimeLimit"):toUtf8()]	="No Limit",
							[i18n.get("uiR2EdWhileChat"):toUtf8()]		="Chat",
							[i18n.get("uiR2EdForCertainTime"):toUtf8()]	="Few Sec"
						},
}


-- sequence --------------------------------------------------
function r2.activities:currentSequUI()
	local ui = getUI(self.uiId)
	local tab = ui:find("sequence_tabs")
	assert(tab)

	local sequenceUI = tab:getGroup(tab.selection)
	
	return sequenceUI
end

function r2.activities:currentSequInstId()
	if self:currentSequUI() then
		return self:currentSequUI().Env.InstanceId
	end
	return nil
end

--initialisé quand selection sequence dans tab ou dans barre menu
function r2.activities:setSequUIInstId(sequUI, id)
	sequUI.Env.InstanceId = id
end

function r2.activities:currentSequInst()
	if self:currentSequInstId() then
		return r2:getInstanceFromId(self:currentSequInstId())
	end
	return nil
end

-- element ---------------------------------------------------

function r2.activities:currentEltUIId()
	return self.currentEltUIID
end

function r2.activities:currentEltUI()
	if self.currentEltUIID then
		return getUI(self.currentEltUIID)
	end
	return nil
end

function r2.activities:setCurrentEltUIId(id)
	self.currentEltUIID = id
end

function r2.activities:currentEltInstId()
	if self.currentEltUIID then 
		return self:currentEltUI().Env.InstanceId
	end
	return nil
end

function r2.activities:currentEltInst()
	if self.currentEltUIID and self:currentEltInstId() then
		return r2:getInstanceFromId(self:currentEltInstId())
	end
	return nil
end

-- updated element and/or sequence (not necessary the same as current sequence or element)
function r2.activities:updatedSequUI()
	if self.updatedSequUIID then
		return getUI(self.updatedSequUIID)
	end
	return nil
end

function r2.activities:setUpdatedSequUIId(sequUIId)
	self.updatedSequUIID = sequUIId
end

function r2.activities:updatedEltUI()
	if self.updatedEltUIID then
		return getUI(self.updatedEltUIID)
	end
	return nil
end

function r2.activities:setUpdatedEltUIId(eltUIId)
	self.updatedEltUIID = eltUIId
end







------------------ INIT ACTIVITIES EDITOR --------------------------------------
function r2.activities:initEditor()

end

function r2.activities:initEditorAfterFirstCall()

	if not self.isInitialized then

		self:cleanEditor()

		-- create sequences UI for selected entity
		local entity = r2:getSelectedInstance()
		assert(entity)
		local activitySequences = entity:getBehavior().Activities

		local selectedSequenceIndex = entity:getSelectedSequenceIndex()
		
		for s = 0, activitySequences.Size - 1 do
			r2.activities:newSequenceUI(activitySequences[s])
		end
		
		-- 
		local ui = getUI(self.uiId)
		assert(ui)
		
		local tabActivities = ui:find("sequence_tabs")
		assert(tabActivities)
		tabActivities.selection = -1

		if activitySequences.Size>0 then
			self:triggerSelectSequence(activitySequences[selectedSequenceIndex].InstanceId)
		end
	
		local uc_title = ucstring()
		uc_title:fromUtf8(i18n.get("uiR2EDActivitySequenceEditor"):toUtf8() .. entity.Name)
		ui.uc_title = uc_title 

		self.isInitialized = true
	end
end

------ OPEN EDITOR ---------------------------------------------------------------
function r2.activities:openEditor()

	local ui = getUI(self.uiId)

	if not ui.active then

		self:initEditorAfterFirstCall()

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

------ OPEN SEQUENCE EDITOR ---------------------------------------------------------------
function r2.activities:openSequenceEditor()

	local sequenceEditor = getUI(self.sequenceEditorId)
	assert(sequenceEditor)
	sequenceEditor.active = false

	local ui = getUI(self.uiId)
	assert(ui)

	sequenceEditor.x = ui.x
	sequenceEditor.y = ui.y
	sequenceEditor.active = true

	-- update edit box text with current sequence name
	local editName = sequenceEditor:find("sequence_name"):find("edit_box_group")
	assert(editName)

	local tab = ui:find("sequence_tabs")
	assert(tab)

	local sequInstId = r2.logicUI:getSequUIInstId(ui:find(tab.associatedGroupSelection))
	assert(sequInstId)
	local activitySequence = r2:getInstanceFromId(sequInstId)
	assert(activitySequence)
	local tabName
	if activitySequence.Name~= "" then
		tabName = activitySequence.Name
	else
		local comps = activitySequence.Parent
		for i=0, comps.Size-1 do
			if comps[i].InstanceId == activitySequence.InstanceId then
				tabName = self.sequenceInitialName..(i+1)
				break
			end
		end
	end

	local uc_sequ = ucstring()
	uc_sequ:fromUtf8(tabName)
	editName.uc_input_string = uc_sequ
end


------ SET SEQUENCE NAME ---------------------------------------------------------------
function r2.activities:setSequenceName()
	
	r2.requestNewAction(i18n.get("uiR2EDSetSequenceNameAction"))

	local sequenceEditor = getUI(self.sequenceEditorId)
	assert(sequenceEditor)

	local editName = sequenceEditor:find("sequence_name"):find("edit_box_group")
	assert(editName)

	local sequenceInstId = self:currentSequInstId()
	r2.requestSetNode(sequenceInstId, "Name", editName.uc_input_string:toUtf8())

	sequenceEditor.active = false
end


------ NEW SEQUENCE INST -------------------------------------------------------------
function r2.activities:newSequenceInst()

	r2.requestNewAction(i18n.get("uiR2EDNewSequenceListAction"))

	local ui = getUI(self.uiId)
	assert(ui)

	local tab = ui:find("sequence_tabs")
	assert(tab)

	--if tab.tabButtonNb == self.maxSequences then return end

	local entity = r2:getSelectedInstance()
	assert(entity)

	local activitySequence = r2.newComponent("ActivitySequence")

	r2.requestInsertNode(entity:getBehavior().InstanceId, "Activities", -1, "", activitySequence)
	self.ownCreatedInstances[activitySequence.InstanceId] = true

	if entity:getBehavior().Activities.Size == 0 then
		self.firstSequence[entity.InstanceId] = activitySequence.InstanceId
	end

	return activitySequence.InstanceId
end

------ NEW SEQUENCE UI --------------------------------------------------------------
function r2.activities:newSequenceUI(activitySequence)

	local templateParams = {
								newElt="r2.activities:newElementInst()",
								newEltText=i18n.get("uiR2EdNewActivity"):toUtf8(),
								eltOrderText=i18n.get("uiR2EdActivityOrder"):toUtf8(),
								upElt="r2.activities:upElementInst()",
								downElt="r2.activities:downElementInst()",
								maxMinElts="r2.activities:maximizeMinimizeElements()",
								downUpColor="220 140 100 255",
								overDownUpColor="255 180 170 255",
								colPushed = "220 140 100 255",
								paramsL= "r2.activities:selectSequence('"..activitySequence.InstanceId.."')"
						   }

	local ui = getUI(self.uiId)
	assert(ui)

	local menu = ui:find("sequence_menu")
	assert(menu)

	local tab = menu:find("sequence_tabs")
	assert(tab)

	-- NEW SEQUENCE GROUP --------------------------------------------
	local newTabNb = tab.tabButtonNb+1
	local posParent, posRef, id, group
	if newTabNb == 1 then
		posParent = "parent"
		posRef = "TL TL"
	else
		posParent = "tab"..(newTabNb-2)
		posRef = "TR TL"
	end

	local id = "tab"..(newTabNb-1)
	
	local groupId = "sequence"..self.sequencesIdCounter
	self.sequencesIdCounter = self.sequencesIdCounter+1

	local newTabGroup = createUIElement("sequence_elements_template", menu.id, {id=groupId,
		new_elt=templateParams.newElt, new_elt_text=templateParams.newEltText, 
		elt_order_text=templateParams.eltOrderText, up_elt=templateParams.upElt, 
		down_elt=templateParams.downElt, max_min_elts=templateParams.maxMinElts,
		down_up_color=templateParams.downUpColor, over_down_up_color=templateParams.overDownUpColor})
	assert(newTabGroup)

	menu:addGroup(newTabGroup)

	r2.logicUI:setSequUIInstId(newTabGroup, activitySequence.InstanceId)
	self:setUpdatedSequUIId(newTabGroup.id)

	-- NEW TAB ----------------------------------------------------
	local comps = activitySequence.Parent
	local sequIndex = -1
	for i=0, comps.Size-1 do
		if comps[i].InstanceId == activitySequence.InstanceId then
			sequIndex = i
			break
		end
	end

	local newTab = createUIElement("sequence_tab_template", tab.id, 
		{id=id, posparent=posParent, posref=posRef, group=groupId,
		 col_pushed =templateParams.colPushed, params_l=templateParams.paramsL})
	assert(newTab)

	tab:addTabWithOrder(newTab, sequIndex)

	local viewText = newTab:getViewText()
	assert(viewText)
	viewText.overExtendViewText = true
	viewText.overExtendViewTextUseParentRect = false
	viewText:setLineMaxW(40)

	newTab:updateCoords()

	-- if it's the first sequence, active all separators and buttons around it
	if tab.tabButtonNb == 1 then
		self:updateUI(true)								
	end

	-- create element editor of new sequence
	self:createElementEditor()

	-- create all elements ui of new sequence
	for a = 0, activitySequence.Components.Size - 1 do
		self:newElementUI(activitySequence.Components[a])	
	end

	-- if YOU just called creation of this sequence (or undo/redo request), it becomes current
	-- selected sequence
	if self.ownCreatedInstances[activitySequence.InstanceId] == true 
		or r2.logicComponents.undoRedoInstances[activitySequence.InstanceId]==true then

		-- right and left buttons to decal showed sequences
		self:triggerSelectSequence(activitySequence.InstanceId) 
		self.ownCreatedInstances[activitySequence.InstanceId] = nil

		r2.logicComponents.undoRedoInstances[activitySequence.InstanceId] = nil
	end

	-- update next sequences UI names
	for i=sequIndex, tab.tabButtonNb-1 do

		local tabButton = tab:getTabButton(i)
		assert(tabButton)
		local sequenceUI = tab:getGroup(i)
		assert(sequenceUI)
		local sequInst = r2:getInstanceFromId(r2.logicUI:getSequUIInstId(sequenceUI))
		assert(sequInst)

		sequName=""
		if sequInst.Name~= "" then
			sequName = sequInst.Name
		else
			sequName = self.sequenceInitialName .. (i+1)
		end

		local uc_name = ucstring()
		uc_name:fromUtf8(sequName)
		tabButton.uc_hardtext = uc_name
	end
end

---- REMOVE SEQUENCE INSTANCE ---------------------------------------------
function r2.activities:removeSequenceInst()
	r2.requestNewAction(i18n.get("uiR2EDRemoveSequenceListAction"))
	local sequenceInstId = self:currentSequInstId()
	assert(sequenceInstId)
	r2.requestEraseNode(sequenceInstId, "", -1)
end

---- REMOVE SEQUENCE UI --------------------------------------------------
function r2.activities:removeSequenceUI(instance)

	-- if sequenceUI to delete is the current sequenceUI, no current elementUI anymore
	local selectedElt = self:currentEltUI()
	if (selectedElt and selectedElt.parent.parent.parent == self:updatedSequUI()) then
		self:setCurrentEltUIId(nil)
	end
	local deleteCurrentSequUI = (self:updatedSequUI() == self:currentSequUI())
	
	-- rename sequences which have default name and are after the removed sequence in list
	local ui = getUI(self.uiId)
	assert(ui)
	local tab = ui:find("sequence_tabs")
	assert(tab)

	local selection = tab.selection
	
	local removedIndex = tab.tabButtonNb
	for i=0, tab.tabButtonNb-1 do
		local sequenceUI = tab:getGroup(i)
		assert(sequenceUI)
		if r2.logicUI:getSequUIInstId(sequenceUI) == instance.InstanceId then
			removedIndex = i
		end
		if i>removedIndex then
			local buttonTab = tab:find("tab"..i)
			assert(buttonTab)
			if buttonTab.hardtext == self.sequenceInitialName..(i+1) then
				buttonTab.uc_hardtext = self.sequenceInitialName..i
			end
		end
	end

	if removedIndex < selection then 
		selection = selection-1 
	end

	-- remove tab of the tab list
	tab:removeTab(removedIndex)

	local firstIndex 
	for i=0, tab.tabButtonNb-1 do
		local tabButton = tab:getTabButton(i)
		if tabButton.active then
			firstIndex = i
			break
		end
	end

	-- if no sequence anymore, hide separators, buttons...
	if tab.tabButtonNb == 0 then
		self:updateUI(false)
	end
end

----- REPEAT SEQUENCE -------------------------------------------
function r2.activities:repeatSequence()

	r2.requestNewAction(i18n.get("uiR2EDSetSequenceRepeatFlagAction"))


	local sequenceInst = self:currentSequInst()
	if sequenceInst==nil then return end

	local sequenceType = 1
	if getUICaller().pushed==true then sequenceType = 0 end

	r2.requestSetNode(sequenceInst.InstanceId, "Repeating", sequenceType)	
end

------ UPDATE SEQUENCE UI ------------------------------------------------------
function r2.activities:updateSequenceUI(instance, attributeName)

	local ui = getUI(self.uiId)
	assert(ui)

	if attributeName=="Name" then

		local name = instance:getName()

		local tab = ui:find("sequence_tabs")
		assert(tab)

		local tabId
		for i=0, tab.tabButtonNb-1 do
			local sequenceUI = tab:getGroup(i)
			if r2.logicUI:getSequUIInstId(sequenceUI) == instance.InstanceId then
				tabId = i
				break
			end
		end

		if tabId then
			local buttonTab = tab:find("tab"..tabId)
			assert(buttonTab)
			
			local uc_name = ucstring()
			uc_name:fromUtf8(name)
			buttonTab.uc_hardtext = uc_name

			local viewText = buttonTab:getViewText()
			assert(viewText)
		end

--		r2.miniActivities:updateSequenceButtonBar()

	elseif attributeName=="Repeating" then
		local sequenceUI = self:updatedSequUI()
		assert(sequenceUI)

		local repeatButton = ui:find("repeat_group"):find("repeat"):find("toggle_butt")
		repeatButton.pushed = (instance.Repeating == 0)
	end
end

------  DECAL TO RIGHT SEQUENCES IN EDITOR ------------------------------
function r2.activities:rightSequenceUI()
	
	local ui = getUI(self.uiId)
	assert(ui)

	local tabGr = ui:find("sequence_tabs")
	assert(tabGr)

	local lastTab = tabGr.lastTabButton
	tabGr:showTabButton(lastTab+1);
end

------  DECAL TO LEFT SEQUENCES IN EDITOR -------------------------------
function r2.activities:leftSequenceUI()

	local ui = getUI(self.uiId)
	assert(ui)

	local tabGr = ui:find("sequence_tabs")
	assert(tabGr)

	local firstTab = tabGr.firstTabButton
	tabGr:showTabButton(firstTab-1);
end

function r2.activities:showSequencesUI()
	
	local ui = getUI(self.uiId)
	assert(ui)

	local tabGr = ui:find("sequence_tabs")
	assert(tabGr)

	local leftTabsButton = ui:find("left_sequences")
	assert(leftTabsButton)

	local rightTabsButton = ui:find("right_sequences")
	assert(rightTabsButton)

	local firstTab 
	local lastTab 
	for i=0, tabGr.tabButtonNb-1 do
		local tab = tabGr:getTabButton(i)
		assert(tab)
		if tab.active then 
			if not firstTab then
				firstTab = i 
			end
			lastTab = i
		elseif firstTab~=nil then
			break
		end	
	end

	if firstTab and lastTab then
		leftTabsButton.active = (firstTab~=0) 
		rightTabsButton.active = (lastTab~=tabGr.tabButtonNb-1)
	end
end

------ CLOSE EDITOR ------------------------------------------------------
function r2.activities:closeEditor()

	local ui = getUI(self.uiId)
	assert(ui)
	if ui.active then
		ui.active = false
	end

	local sequenceEditor = getUI(self.sequenceEditorId)
	assert(sequenceEditor)
	sequenceEditor.active = false

	self.isInitialized = false
end

------ CLEAN EDITOR -------------------------------------------------------
function r2.activities:cleanEditor()

	self:setCurrentEltUIId(nil)
	self:setUpdatedSequUIId(nil)
	self:setUpdatedEltUIId(nil)
	self.elementsIdCounter = 0
	self.sequencesNb = 0
	self.sequencesIdCounter = 0

	local ui = getUI(self.uiId)
	assert(ui)

	-- remove all tabs and associated groups
	local sequenceMenu = ui:find("sequence_menu")
	assert(sequenceMenu)

	local tabActivities = ui:find("sequence_tabs")
	assert(tabActivities)

	tabActivities:removeAll()

	-- hide separators and buttons around
	self:updateUI(false)
end

------ SELECT ELEMENT --------------------------------------------------
function r2.activities:selectElement(selectedButtonElt, sequInstId)
	if sequInstId then
		self:triggerSelectSequence(sequInstId)
	end
	r2.logicComponents:selectElement(r2.activities, selectedButtonElt)
end

------ SELECT SEQUENCE -----------------------------------------------
function r2.activities:triggerSelectSequence(sequInstId)

	local ui = getUI(self.uiId)
	assert(ui)

	local tabActivities = ui:find("sequence_tabs")
	assert(tabActivities)

	local sequInst = r2:getInstanceFromId(sequInstId)
	assert(sequInst)
	local tabNb = r2.logicComponents:searchElementIndex(sequInst)-1

	local sequenceUI = tabActivities:getGroup(tabNb)
	if sequenceUI and r2.logicUI:getSequUIInstId(sequenceUI)==sequInstId then

		-- triggers call of r2.activities:selectSequence
		tabActivities.selection = tonumber(tabNb)
	end
end

function r2.activities:selectSequence(sequInstId)

	r2.logicComponents:selectSequence(r2.activities)

	local ui = getUI(self.uiId)
	assert(ui)

	local sequInst = r2:getInstanceFromId(sequInstId)
	assert(sequInst)

	-- repeat sequence ?
	local repeatButton = ui:find("repeat_group"):find("repeat"):find("toggle_butt")
	repeatButton.pushed = (sequInst.Repeating == 0)

	-- update sequence button bar
	r2.miniActivities:updateSequenceButtonBar()

	-- update mini activities view
	r2.miniActivities:updateMiniActivityView()

	-- set the sequence in activity owner
	local owner = sequInst.ParentInstance.ParentInstance

	owner.User.SelectedSequence = sequInst.IndexInParent
end

------ CREATE EDITOR -------------------------------------------------------
function r2.activities:createElementEditor()

	local activityEditor = r2.logicComponents:createElementEditor(r2.activities)
	assert(activityEditor)

	-- time limit
	local timeLimitCB = activityEditor:find("time_limit"):find("combo_box")
	assert(timeLimitCB)
	timeLimitCB:resetTexts()
	timeLimitCB:addText(i18n.get("uiR2EdNoTimeLimit"))
	timeLimitCB:addText(i18n.get("uiR2EdForCertainTime"))

	do
		local cb = activityEditor:find("road_count_limit"):find("combo_box")
		assert(cb)
		cb:resetTexts()
		cb:addText(i18n.get("uiR2EdNoRoadCountLimit"))
		local i = 0
		while i < 20 do
			i = i + 1
			cb:addText(ucstring(tostring(i)))
		end

	end

end

------ OPEN ELEMENT EDITOR -----------------------------------------------
function r2.activities:updateElementEditor()

	local instanceActivity = self:currentEltInst()
	if instanceActivity==nil then return end
	
	local sequenceUI = self:currentSequUI()
	assert(sequenceUI)

	local activityEditor = sequenceUI:find("edit_element")
	assert(activityEditor)

	-- activity name
	local activityName = activityEditor:find("name")
	assert(activityName)

	-- activity type
	local activityButtonText = activityEditor:find("activity"):find("text")
	assert(activityButtonText)

	-- patrol / repeat road special casefalse 
	do
		local grp = activityEditor:find("road_count_group")
		if instanceActivity.Activity == "Patrol" or instanceActivity.Activity == "Repeat Road" then
			grp.active = true
		else
			grp.active = false
		end


		-- time limit
		local comboBox = activityEditor:find("road_count_limit").combo_box
		assert(comboBox)
		local value = select(instanceActivity.RoadCountLimit, instanceActivity.RoadCountLimit, "0")
		if value == "0" then
			value = i18n.get("uiR2EdNoRoadCountLimit"):toUtf8() 
		end
		comboBox.Env.locked = true
		comboBox.selection_text = value
		comboBox.Env.locked = false

	end

	-- time limit
	local comboBox = activityEditor:find("time_limit").combo_box
	assert(comboBox)

	local index = r2.logicComponents:searchElementIndex(instanceActivity)
	local uc_activityName = ucstring(self.elementInitialName.." : ")
	if index~= nil then
		uc_activityName:fromUtf8(self.elementInitialName.." "..index.." : ")
	end
	activityName.uc_hardtext = uc_activityName
	
	-- activity type
	local activityText = instanceActivity:getVerb()
	if instanceActivity.ActivityZoneId ~= "" then
		local place = r2:getInstanceFromId(instanceActivity.ActivityZoneId)
		assert(place)

		activityText = activityText .. " " .. place.Name
	end
	if activityText then
		local uc_activityText = ucstring()
		uc_activityText:fromUtf8(activityText)
		activityButtonText.uc_hardtext = uc_activityText
	end

	-- time limit
	local timeLimit = instanceActivity.TimeLimit
	
	local certainTime = activityEditor:find("certain_time")
	assert(certainTime)

	comboBox.Env.locked = true
	if timeLimit == "Few Sec" then

		local timeLimitValue = tonumber(instanceActivity.TimeLimitValue)

		if timeLimitValue ~= nil then
			
			local hoursNb, minNb, secNb = r2.logicComponents:calculHourMinSec(timeLimitValue)

			local timeLimitText = i18n.get("uiR2EdFor"):toUtf8() .. " "
			if hoursNb ~= 0 then timeLimitText = timeLimitText .. hoursNb .. i18n.get("uiR2EdShortHours"):toUtf8() .. " " end
			if minNb ~= 0 then timeLimitText = timeLimitText .. minNb .. i18n.get("uiR2EdShortMinutes"):toUtf8() .. " " end
			timeLimitText = timeLimitText .. secNb .. i18n.get("uiR2EdShortSeconds"):toUtf8() 

			certainTime.active = true
			local hoursMenu = certainTime:find("hours"):find("text")
			assert(hoursMenu)
			hoursMenu.uc_hardtext = tostring(hoursNb)

			local minutesMenu = certainTime:find("minutes"):find("text")
			assert(minutesMenu)
			minutesMenu.uc_hardtext = tostring(minNb)

			local secondsMenu = certainTime:find("seconds"):find("text")
			assert(secondsMenu)
			secondsMenu.uc_hardtext = tostring(secNb)

			local uc_time = ucstring()
			uc_time:fromUtf8(timeLimitText)
			comboBox.view_text = uc_time
		end
	else
		certainTime.active = false
		timeLimit = self.timeLimitsTranslation[timeLimit]
		if timeLimit~= nil then
			comboBox.selection_text = timeLimit
		end
	end
	comboBox.Env.locked = false
end

----- CLOSE ELEMENT EDITOR --------------------------------------------------------
function r2.activities:closeElementEditor()
	
	r2.logicComponents:closeElementEditor(r2.activities)
end

------ NEW ELEMENT INST ----------------------------------------------------------
function r2.activities:newElementInst(tableInit)

	local sequenceUI = self:currentSequUI()
	if sequenceUI then
		local eltsList = sequenceUI :find("elements_list")
		assert(eltsList)

		if eltsList.childrenNb >= self.maxElements then return end
	end

	local sequenceInstId = self:currentSequInstId()  
	-- when no sequences, we create the first
	local selectedInst = r2:getSelectedInstance()
	assert(selectedInst)
	local selectedInstId = selectedInst.InstanceId
	if sequenceInstId == nil then
		if self.firstSequence[selectedInstId] then
			sequenceInstId = self.firstSequence[selectedInstId]
		else
			sequenceInstId = self:newSequenceInst()
		end
	end

	local instanceElement = r2.newComponent("ActivityStep")

	local existZone = false
	local actionToInitDefaultActivity = ""

	if tableInit ~= nil then
		instanceElement.Activity = tableInit.Activity
		instanceElement.ActivityZoneId = r2.RefId(tableInit.ActivityZoneId)
		instanceElement.TimeLimit = tableInit.TimeLimit
		instanceElement.TimeLimitValue = tableInit.TimeLimitValue

		if tableInit.RoadCountLimit then instanceElement.RoadCountLimit = tableInit.RoadCountLimit end
	else
		instanceElement.TimeLimit = "No Limit"
		
		local activitiesTable = {}
		selectedInst:getAvailableActivities(activitiesTable)
		local standStillAct = false
		local roadAct = false
		local regionAct = false
		local activityZone 
		for k, activity in pairs(activitiesTable) do
			if activity=="Stand Still" then 
				standStillAct=true
				break
			elseif self.activityTypeTranslation[activity].zone=="Road" then
				roadAct = true
				activityZone = activity
			elseif self.activityTypeTranslation[activity].zone=="Region" then
				regionAct = true
				activityZone = activity
			end
		end

		if standStillAct then
			instanceElement.Activity = "Stand Still"
		elseif roadAct then
			local roadsTable = r2.Scenario:getAllInstancesByType("Road")
			if table.getn(roadsTable) > 0 then
				instanceElement.Activity = activityZone
				local zoneId
				for k, v in pairs(roadsTable) do
					zoneId = v.InstanceId
					break
				end
				instanceElement.ActivityZoneId = zoneId
				existZone = true
			end
		elseif regionAct then
			local regionsTable = r2.Scenario:getAllInstancesByType("Region")
			if table.getn(regionsTable) > 0 then
				instanceElement.Activity = activityZone
				local zoneId
				for k, v in pairs(regionsTable) do
					zoneId = v.InstanceId
					break
				end
				instanceElement.ActivityZoneId = zoneId
				existZone = true
			end
		else
			debugInfo("No activity type to initialize default activity step")
		end

		if roadAct then
			actionToInitDefaultActivity = i18n.get("uiR2EdRoad"):toUtf8() .. " "
		end

		if regionAct then
			if actionToInitDefaultActivity~="" then
				actionToInitDefaultActivity = actionToInitDefaultActivity .. i18n.get("uiR2EdOr"):toUtf8().. " "
			end
			actionToInitDefaultActivity = actionToInitDefaultActivity .. i18n.get("uiR2EdRegion"):toUtf8() .. " "
		end
	end

	if instanceElement.Activity~="" then
		r2.requestNewAction(i18n.get("uiR2EDNewSequenceElementAction"))
		r2.requestInsertNode(sequenceInstId, "Components", -1, "", instanceElement)
		self.ownCreatedInstances[instanceElement.InstanceId] = true

		r2.logicComponents:newElementInst(r2.activities)

	elseif not existZone then
		local message = i18n.get("uiR2EdCreateZone"):toUtf8() .. actionToInitDefaultActivity .. i18n.get("uiR2EdFor"):toUtf8() .. " " .. selectedInst.Name
		local uc_message = ucstring()
		uc_message:fromUtf8(message)
		messageBox(uc_message)	
	end
	r2.requestForceEndMultiAction()
end

------ NEW ELEMENT UI -------------------------------------------------------------
function r2.activities:newElementUI(newInst)
	r2.logicUI:newElementUI(r2.activities, newInst, true)
end

------ REMOVE ELEMENT INST --------------------------------------------------------
function r2.activities:removeElementInst()
	r2.requestNewAction(i18n.get("uiR2EDRemoveLogicElementAction"))
	r2.logicComponents:removeElementInst(r2.activities)
	r2.requestEndAction()
end

------ REMOVE ELEMENT UI ---------------------------------------------------------
function r2.activities:removeElementUI(removedEltUI)
	r2.logicUI:removeElementUI(r2.activities, removedEltUI)
end

------ UP ELEMENT INST ------------------------------------------------------------
function r2.activities:upElementInst()
	r2.logicComponents:upElementInst(r2.activities)
end

------ DOWN ELEMENT INST ---------------------------------------------------------
function r2.activities:downElementInst()
	r2.logicComponents:downElementInst(r2.activities)
end

------ MAX/MIN ELEMENTS UI --------------------------------------------------------
function r2.activities:maximizeMinimizeElements()
	r2.logicUI:maximizeMinimizeElements(r2.activities)
end

------ MAX/MIN ELEMENT UI ------------------------------------------------------------
function r2.activities:maximizeMinimizeElement(element, allMinimize)

	r2.logicUI:maximizeMinimizeElement(element, allMinimize)
end

------ DOWN/UP ELEMENT UI -----------------------------------------------------------
function r2.activities:downUpElementUI(elementUI, instance)
	r2.logicUI:downUpElementUI(r2.activities, elementUI, instance)
end

------ UPDATE ELEMENT UI -------------------------------------------------------------
function r2.activities:updateElementUI(elementUI)

	r2.logicUI:updateElementUI(r2.activities, elementUI)
end

------ UPDATE ELEMENT TITLE -----------------------------------------------------------
function r2.activities:updateElementTitle(activityUI)

	r2.logicComponents:updateElementTitle(r2.activities, activityUI, true)
end

------ INIT ACTIVITY MENU -------------------------------------------------------------------
function r2.activities:initActivityMenu()

	local menuName = "ui:interface:r2ed_triggers_menu"
	local activityMenu = getUI(menuName)
	local activityMenu = activityMenu:getRootMenu()
	assert(activityMenu)
	activityMenu:reset()
	local uc_activity = ucstring()

	local activitiesTable = {}
	r2:getSelectedInstance():getAvailableActivities(activitiesTable)

	local activitiesNb = 0
	for k, actType in pairs(activitiesTable) do
		local uc_activity = ucstring()
		local translation = self.activityTypeTranslation[actType].trans
		uc_activity:fromUtf8(translation)
		
		local zone = self.activityTypeTranslation[actType].zone
		if zone == "" then
			activityMenu:addLine(uc_activity, "lua", "r2.activities:setActivityType('"..actType.."')", actType)
		else
			activityMenu:addLine(uc_activity, "", "", actType)

			local textureName = ""
			if zone == "Road" then
				textureName = "r2ed_icon_road.tga"
			elseif zone == "Region" then
				textureName = "r2ed_icon_region.tga"
			end

			local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = textureName, size="14" })
			activityMenu:setUserGroupLeft(activitiesNb, menuButton)
			activityMenu:addSubMenu(activitiesNb)
			local zonesMenu = activityMenu:getSubMenu(activitiesNb)
			local zonesTable = r2.Scenario:getAllInstancesByType(zone)
			for key, z in pairs(zonesTable) do
				uc_activity:fromUtf8(z.Name)
				zonesMenu:addLine(uc_activity, "lua", "r2.activities:setActivityType('"..actType.."', '".. z.InstanceId .."')", z.InstanceId)
			end
			if table.getn(zonesTable) == 0 then
				zonesMenu:addLine(i18n.get("uiR2EdNoSelelection"), "lua", "r2.activities:setActivityType()", "")
			end
		end

		activitiesNb = activitiesNb+1
	end

	r2.logicUI:openLogicMenu(getUICaller())
end

--- SET ACTIVITY TYPE --------------------------------------------------------------------
function r2.activities:setActivityType(activityType, placeId)

	local activityInstId = self:currentEltInstId()
	assert(activityInstId)

	if activityType == nil then
		return
	elseif placeId == nil then
		r2.requestNewAction(i18n.get("uiR2EDSetActivityTypeAction"))
		r2.requestSetNode(activityInstId, "Activity", activityType)
		r2.requestSetNode(activityInstId, "ActivityZoneId", r2.RefId(""))
	else
		r2.requestNewAction(i18n.get("uiR2EDSetActivityTypeAction"))
		r2.requestSetNode(activityInstId, "Activity", activityType)
		r2.requestSetNode(activityInstId, "ActivityZoneId", r2.RefId(placeId))
	end
end

--- SET TIME LIMIT --------------------------------------------------------------------
function r2.activities:setTimeLimit(timeLimit)
	
	r2.requestNewAction(i18n.get("uiR2EDSetActivityTimeLimitAction"))

	if timeLimit == nil then
		timeLimit = getUICaller().selection_text 
		if getUICaller().Env.locked then return end
	end

	local activityInstId = self:currentEltInstId()
	assert(activityInstId)

	-- TimeLimitValue
	if timeLimit == i18n.get("uiR2EdForCertainTime"):toUtf8() then
		r2.requestSetNode(activityInstId, "TimeLimitValue", tostring(20))
	else	
		r2.requestSetNode(activityInstId, "TimeLimitValue", "")
	end

	-- TimeLimit
	timeLimit = self.timeLimitsProperties[timeLimit]
	assert(timeLimit)
	r2.requestSetNode(activityInstId, "TimeLimit", timeLimit)
end

--- SET TIME LIMIT --------------------------------------------------------------------
function r2.activities:setRoadCountLimit(limit)
	
	r2.requestNewAction(i18n.get("uiR2EDSetActivityRoadCountAction"))

	if limit == nil then
		timeLimit = getUICaller().selection_text 
		if getUICaller().Env.locked then return end
	end

	local activityInstId = self:currentEltInstId()
	assert(activityInstId)

	-- RoadCountLimit
	if limit == i18n.get("uiR2EdNoRoadCountLimit"):toUtf8() then
		r2.requestSetNode(activityInstId, "RoadCountLimit", "0")
	else	
		r2.requestSetNode(activityInstId, "RoadCountLimit", timeLimit)
	end
end

------ INIT TIME MENU -----------------------------------------------------------------
function r2.activities:initTimeMenu(timeFunction, isHours)

	local timeMenu = getUI("ui:interface:r2ed_triggers_menu")
	assert(timeMenu)

	local timeMenu = timeMenu:getRootMenu()
	assert(timeMenu)

	timeMenu:reset()

	for i=0,9 do
		timeMenu:addLine(ucstring(tostring(i)), "lua", timeFunction .. "(" .. tostring(i) .. ")", tostring(i))
	end

	if isHours == true then
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
		end
	end

	r2.logicUI:openLogicMenu(getUICaller())
end

----- SET HOURS/MINUTES/SECONDS -----------------------------------------------------------------
function r2.activities:activityForHours(hourNb)

	local activityInst = self:currentEltInst()
	assert(activityInst)

	local lastHourNb, minNb, secNb = r2.logicComponents:calculHourMinSec(tonumber(activityInst.TimeLimitValue))
	
	self:setLimitTimeValue(hourNb, minNb, secNb)
end

function r2.activities:activityForMinutes(minNb)

	local activityInst = self:currentEltInst()
	assert(activityInst)

	local hoursNb, lastMinNb, secNb = r2.logicComponents:calculHourMinSec(tonumber(activityInst.TimeLimitValue))
	
	self:setLimitTimeValue(hoursNb, minNb, secNb)
end

function r2.activities:activityForSeconds(secNb)

	local activityInst = self:currentEltInst()
	assert(activityInst)

	local hoursNb, minNb, lastSecNb = r2.logicComponents:calculHourMinSec(tonumber(activityInst.TimeLimitValue))
	
	self:setLimitTimeValue(hoursNb, minNb, secNb)
end

-------- SET LIMIT TIME VALUE -------------------------------------------------------------------
function r2.activities:setLimitTimeValue(hourNb, minNb, secNb)
	r2.requestNewAction(i18n.get("uiR2EDSetActivityTimeLimitValueAction"))	

	local activityInstId = self:currentEltInstId()
	assert(activityInstId)

	local totalSec = tostring(hourNb*3600 + minNb*60 + secNb)
	r2.requestSetNode(activityInstId, "TimeLimitValue", totalSec)
end


----- UTILS ---------------------------------------------------------

function r2.activities:findSequenceUIFromInstance(instance)

	local ui = getUI(r2.activities.uiId)
	assert(ui)

	local sequenceTabs = ui:find("sequence_tabs")
	assert(sequenceTabs)

	for i=0,sequenceTabs.tabButtonNb-1 do
		local sequenceUI = sequenceTabs:getGroup(i)
		if r2.logicUI:getSequUIInstId(sequenceUI) == instance.InstanceId then
			r2.activities:setUpdatedSequUIId(sequenceUI.id)
			return sequenceUI
		end
	end

	return nil
end

function r2.activities:findElementUIInSequenceUI(sequenceUI, instance)

	local eltsList = sequenceUI:find("elements_list")
	assert(eltsList)

	for i=0,eltsList.childrenNb-1 do
		local element = eltsList:getChild(i)
		if r2.logicUI:getEltUIInstId(element) == instance.InstanceId then
			return element
		end
	end

	return nil
end

function r2.activities:updateUI(active)

	local ui = getUI(self.uiId)

	local sepTop = ui:find("sep_top")
	assert(sepTop)
	local sepBottom = ui:find("sep_bottom")
	assert(sepBottom)
	local sepLeft = ui:find("sep_left")
	assert(sepLeft)
	local sepRight = ui:find("sep_right")
	assert(sepRight)
	local removeSequence = ui:find("remove_sequence_button")
	assert(removeSequence)
	local editSequence = ui:find("edit_sequence")
	assert(editSequence)
	local repeatButtonGr = ui:find("repeat_group")
	assert(repeatButtonGr)

	sepTop.active = active
	sepBottom.active = active
	sepLeft.active= active
	sepRight.active= active
	removeSequence.active = active
	editSequence.active = active
	repeatButtonGr.active = active
end







--------------------------------------------------------------------------------------------------
-------------------------------- ACTIVITY SEQUENCE DisplayerProperties ---------------------------
--------------------------------------------------------------------------------------------------
local activitySequencePropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onPostCreate(instance)	

	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activeLogicEntityParent = instance:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end

	if r2.activities.isInitialized then
		r2.activities.firstSequence[activeLogicEntity.InstanceId] = nil
		r2.activities:newSequenceUI(instance)
	end

	r2.miniActivities:updateSequenceButtonBar()
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onErase(instance)

	instance.User.Deleted = true

	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activeLogicEntityParent = instance:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end

	if r2.activities.isInitialized and r2.activities:findSequenceUIFromInstance(instance) then
		r2.activities:removeSequenceUI(instance)
	end

	-- last sequence
	if instance.Parent.Size==1 then
		r2.miniActivities:updateMiniActivityView()
		r2.miniActivities:updateSequenceButtonBar()
	end
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onPostHrcMove(instance)		
end
------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function activitySequencePropertySheetDisplayerTable:onAttrModified(instance, attributeName)	

	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activeLogicEntityParent = instance:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end
	
	if r2.activities.isInitialized and r2.activities:findSequenceUIFromInstance(instance) then
		r2.activities:updateSequenceUI(instance, attributeName)
	end		

	if attributeName=="Name" then
		r2.miniActivities:updateSequenceButtonBar()
	end
end	

------------------------------------------------
function r2:activitySequencePropertySheetDisplayer()	
	return activitySequencePropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end


--------------------------------------------------------------------------------------------------
-------------------------------- ACTIVITY STEP DisplayerProperties--------------------------------
--------------------------------------------------------------------------------------------------
local activityStepPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onPostCreate(instance)	

	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activitySequInst = instance.Parent.Parent
	local activeLogicEntityParent = activitySequInst:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end

	if r2.activities.isInitialized and r2.activities:findSequenceUIFromInstance(activitySequInst) then
		r2.activities:newElementUI(instance)
	end

	-- update mini activities view
	r2.miniActivities:updateMiniActivityView()
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onErase(instance)

	instance.User.Deleted = true
	
	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activitySequInst = instance.Parent.Parent
	local activeLogicEntityParent = activitySequInst:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end

	local sequenceUI = r2.activities:findSequenceUIFromInstance(activitySequInst)
	if r2.activities.isInitialized and sequenceUI then
		local eltUI = r2.activities:findElementUIInSequenceUI(sequenceUI, instance)
		if eltUI then 
			r2.activities:removeElementUI(eltUI) 
		end
	end

	r2.miniActivities:updateMiniActivityView()
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onPreHrcMove(instance)			
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onPostHrcMove(instance)

	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activitySequInst = instance.Parent.Parent
	local activeLogicEntityParent = activitySequInst:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end

	local sequenceUI = r2.activities:findSequenceUIFromInstance(activitySequInst)
	if r2.activities.isInitialized and sequenceUI then
		local eltUI = r2.activities:findElementUIInSequenceUI(sequenceUI, instance)
		if eltUI then r2.activities:downUpElementUI(eltUI, instance) end
	end

	r2.miniActivities:updateMiniActivityView()
end
------------------------------------------------
function activityStepPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onSelect(instance, isSelected)	
end

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
	
	local activeLogicEntity = r2:getSelectedInstance()
	if activeLogicEntity and activeLogicEntity.isGrouped and (activeLogicEntity:isGrouped() or activeLogicEntity:isKindOf("NpcGrpFeature")) then
		activeLogicEntity = r2:getLeader(activeLogicEntity)
	end

	if activeLogicEntity ~= instance:getParentOfKind("LogicEntity") then return end

	local activitySequInst = instance.Parent.Parent
	local activeLogicEntityParent = activitySequInst:getActiveLogicEntityParent() 

	if activeLogicEntity==nil or activeLogicEntity~=activeLogicEntityParent then
		return
	end

	local sequenceUI = r2.activities:findSequenceUIFromInstance(activitySequInst)
	if r2.activities.isInitialized and sequenceUI then
		local eltUI = r2.activities:findElementUIInSequenceUI(sequenceUI, instance)
		if eltUI then r2.activities:updateElementUI(eltUI) end
	end

	-- update mini activities view
	r2.miniActivities:updateMiniActivityView()
end	

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onTargetInstanceEraseRequested(instance, refIdName, refIdIndexInArray)
	if refIdName == "ActivityZoneId" then
		r2.requestEraseNode(instance.InstanceId, "", -1)
	end
end

------------------------------------------------
function activityStepPropertySheetDisplayerTable:onTargetInstanceAttrModified(instance, refIdName, refIdIndexInArray, targetAttrName, targetAttrIndexInArray)	
end

-------------------------------------------------
function r2:activityStepPropertySheetDisplayer()	
	return activityStepPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end
