-----------------
-- SCENARIO UI --
-----------------


r2.ScenarioWindow = 
{
	IgnoreActChange = false, -- when the content of the combo box is changed manually, allow to know we should not process the
                            -- the 'onchangeAct' msg
	LeftQuota = nil,

	LeftStaticQuota = nil,

	Languages = {},

	Types = {},
	Locked = {},

	lockComboBoxes = false,
	-- chosen act in combo box
	showedActId = nil,

	TitleEditBoxLocked = false
}

-----------------------------------------------------------------------------------------------------------
-- init scenario window (language and scenario types from cfg files) --------------------------------------
function r2.ScenarioWindow:initScenarioWindow()	

	local languages = getClientCfgVar("ScenarioLanguages")
	local types = getClientCfgVar("ScenarioTypes")

	local locked = {"Full", "RunOnly"}

	-- stores values and init combo boxes
	local scenarioWnd = self:getContainer()
	local languageCB = scenarioWnd:find("language_combo_box")
	local typeCB = scenarioWnd:find("type_combo_box")
	local lockCB = scenarioWnd:find("locked_combo_box")
	local levelCB = scenarioWnd:find("level_combo_box")
	local rulesCB = scenarioWnd:find("rules_combo_box")

	local languages2 = {}
	for k, lang in pairs(languages) do
		self.Languages[i18n.get("uiR2ED"..lang):toUtf8()] = lang
		table.insert(languages2, i18n.get("uiR2ED"..lang))
	end
	table.sort(languages2)
	languageCB:resetTexts()
	for k, lang in pairs(languages2) do
		languageCB:addText(lang)
	end

	local types2 = {}
	for k, type in pairs(types) do
		self.Types[i18n.get("uiR2ED"..type):toUtf8()] = type
		table.insert(types2, i18n.get("uiR2ED"..type))
	end
	table.sort(types2)
	typeCB:resetTexts()

	for k, type in pairs(types2) do
		typeCB:addText(type)
	end

	do

		local tmp = {}
		if config.R2EDExtendedDebug == 1 then
			table.insert(locked, "RoSOnly")
		end
		for k, type in pairs(locked) do
			self.Locked[i18n.get("uiR2ED"..type):toUtf8()] = type
			table.insert(tmp, i18n.get("uiR2ED"..type))
		end
--		table.sort(tmp)
		lockCB:resetTexts()
		for k, type in pairs(tmp) do
			lockCB:addText(type)
		end

	end


	levelCB:resetTexts()
	for i = 0, 5 do		
		levelCB:addText(i18n.get("uiRAP_Level".. tostring(i)))
	end

	rulesCB:resetTexts()
	rulesCB:addText(i18n.get("uiR2EDstrict"))
	rulesCB:addText(i18n.get("uiR2EDliberal"))	
end

-----------------------------------------------------------------------------------------------------------
-- retrieve the table of acts (table that map each line of the combo box to an act, see r2.ActUIDisplayer)
function r2.ScenarioWindow:getActTable()
	return r2.ActUIDisplayer:getActTable()
end

-----------------------------------------------------------------------------------------------------------
-- retrieve the 'scenario' window container (TODO nico : code duplicated with r2.ActUIDisplayer)
function r2.ScenarioWindow:getContainer()
	return getUI("ui:interface:r2ed_scenario")	
end

-----------------------------------------------------------------------------------------------------------
-- retrieve the 'acts' combo box (TODO nico : code duplicated with r2.ActUIDisplayer)
function r2.ScenarioWindow:getActComboBox()
	return self:getContainer():find("act_combo_box")
end

-----------------------------------------------------------------------------------------------------------
-- handler selection of a new act by the user 
function r2.ScenarioWindow:onChangeActFromUI()
	if self.IgnoreActChange then return end	
	self:setAct(self:getActTable()[self:getActComboBox().selection + 1].Act)		
end

-----------------------------------------------------------------------------------------------------------
-- update ui from the current act
function r2.ScenarioWindow:updateUIFromCurrentAct()

	local act = r2:getCurrentAct()
	-- look in the act table to change the selection
	self.IgnoreActChange = true -- don't want to trigger the 'onChangeAct' msg here
	if self.getActComboBox == nil then
		debugInfo(debug.traceback())
		inspect(self)
	end
	if not act:isBaseAct() then
		self:getActComboBox().selection = self:findComboBoxLineFromAct(act)
	end
	self.IgnoreActChange = false	
	self:updateActTreeVisibility()
	self:updateLeftQuota()
end


-----------------------------------------------------------------------------------------------------------
-- get left quota, possibly updating it if it was modified
-- called by client
function r2.ScenarioWindow:getLeftQuota()
	if r2.UIMainLoop.LeftQuotaModified then
		self:updateLeftQuota()
	end
	local quota = 0
	if self.LeftQuota < self.LeftStaticQuota then
		quota = self.LeftQuota
	else
		quota = self.LeftStaticQuota
	end
	return quota, self.LeftQuota, self.LeftStaticQuota
end			

-----------------------------------------------------------------------------------------------------------
-- update display of left quota for the current act
function r2.ScenarioWindow:updateLeftQuota()
	function protected()		
		if not r2.Scenario then return end
		local quotaViewText = self:getContainer():find("left_quota")
		local quotaTooltip = self:getContainer():find("tab4")
		local quotaMacroViewText = self:getContainer():find("left_quota_macro")
		local quotaMacroTooltip = self:getContainer():find("tab1")
		local quotaViewTextMisc = self:getContainer():find("left_quota_misc")
		local quotaViewTextMisc2 = self:getContainer():find("left_quota_misc_2")
		local quotaMiscTooltip = self:getContainer():find("tab2")

		local act = r2:getCurrentAct()
		
		if act then
			r2.updateActCost(act)
			if not act:isBaseAct() then
				r2.updateActCost(r2.Scenario:getBaseAct())
			end

			self.LeftQuota, self.LeftStaticQuota = act:getLeftQuota() 

			local uc_tooltip = ucstring()
			quotaViewText.hardtext = string.format("(%d)", self.LeftQuota)
			uc_tooltip:fromUtf8(i18n.get("uiR2EDScene"):toUtf8() .. " (" .. self.LeftQuota .. ")")
			quotaTooltip.tooltip = uc_tooltip
			quotaMacroViewText.hardtext = string.format("(%d)", self.LeftQuota)	
			uc_tooltip:fromUtf8(i18n.get("uiR2EDMacroComponents"):toUtf8() .. " (" .. self.LeftQuota .. ")")
			quotaMacroTooltip.tooltip = uc_tooltip		
			quotaViewTextMisc.hardtext = string.format("(%d)", self.LeftStaticQuota)
			quotaViewTextMisc2.hardtext = string.format("(%d)", self.LeftStaticQuota)
			uc_tooltip:fromUtf8(i18n.get("uiR2EDbotObjects"):toUtf8() .. " (" .. self.LeftStaticQuota .. ")")
			quotaMiscTooltip.tooltip = uc_tooltip			
			
			quotaViewText.color_rgba = select(self.LeftQuota >= 0, CRGBA(255, 255, 255), CRGBA(255, 0, 0))
			quotaViewTextMisc.color_rgba = select(self.LeftStaticQuota >= 0, CRGBA(255, 255, 255), CRGBA(255, 0, 0))
			quotaViewTextMisc2.color_rgba = select(self.LeftStaticQuota >= 0, CRGBA(255, 255, 255), CRGBA(255, 0, 0))
					
		else
			quotaViewText.hardtext = ""
			quotaMacroViewText.hardtext = ""
		end	
		r2.UIMainLoop.LeftQuotaModified = false
	end
	pcall(protected)
end

-----------------------------------------------------------------------------------------------------------
-- change act into editor, and in the ui
r2.ScenarioWindow.setActId = nil
function r2.ScenarioWindow:setAct(act)

	if act ~= r2:getCurrentAct()  then   
		self:updateComboBoxCurrentAct(act) 
		r2:setCurrentActFromId(act.InstanceId) -- will also trigger ui update (called by C++)
		if act.InstanceId~=self.setActId then
			if not r2:isClearingContent() then
				displaySystemInfo(concatUCString(i18n.get("uiR2EDCurrentActNotify"), act:getDisplayName()), "BC")
			end
			self.setActId = act.InstanceId
		end
	end
end

-----------------------------------------------------------------------------------------------------------
-- 
function r2.ScenarioWindow:updateComboBoxCurrentAct(newAct)

	local comboB = self:getActComboBox()
	local text = ucstring()	
	local currentAct = r2:getCurrentAct()
	if not (currentAct==nil or currentAct.isNil) then
		local currentActIndex = r2.ScenarioWindow:findComboBoxLineFromAct(currentAct)
		if currentActIndex then
			text:fromUtf8(currentAct:getName())	
			comboB:setText(currentActIndex, text)
		end
	end

	local newActIndex = r2.ScenarioWindow:findComboBoxLineFromAct(newAct)
	if newActIndex then
		text:fromUtf8(newAct:getName() .. "  [" .. i18n.get("uiR2EDCurrentActComboBox"):toUtf8() .."]")	
		comboB:setText(newActIndex, text)
	end
end

-----------------------------------------------------------------------------------------------------------
-- find line in combo box for the given act (index is 0 based)
function r2.ScenarioWindow:findComboBoxLineFromAct(act)
	local actTable = self:getActTable()
	for index, entry in pairs(actTable) do				
		if entry.Act == act then
			return index - 1
		end
	end	
	return nil
end

-----------------------------------------------------------------------------------------------------------
-- selection of a new act (called by combo box)
function r2.ScenarioWindow:updateActTreeVisibility()
	local currentAct = r2:getCurrentAct()
	local container = self:getContainer()
	-- show the good tree control
	for i = 0, r2:getMaxNumberOfAdditionnalActs() - 1 do
		local tree = container:find("act_tree_" .. tostring(i))
		assert(tree)		
		tree.active = (tree == currentAct:getContentTree())	
		
		local macroTree = container:find("macro_act_tree_" .. tostring(i))
		assert(macroTree)		
		macroTree.active = (macroTree == currentAct:getMacroContentTree())	
	end
	-- allow to delete an act only if it is not the base act	
	container:find("delete_act").active = not currentAct:isBaseAct()
end

-----------------------------------------------------------------------------------------------------------
-- pop window for new act creation

function r2.ScenarioWindow:newAct()

	if r2.Scenario.Acts.Size == r2:getMaxNumberOfAdditionnalActs() + 1 then -- '+1' because first act is the base add, which can't be deleted/created
		messageBox(i18n.get("uiR2EDMaxActCountReached"))
		return
	end	

	r2.acts:openScenarioActEditor(false)
end

-----------------------------------------------------------------------------------------------------------
-- Ask the user to confirm deletion of act
function r2.ScenarioWindow:deleteAct()
	validMessageBox(i18n.get("uiR2EDconfirmDeleteAct"), "lua", "r2.ScenarioWindow:confirmDeleteAct()", "lua", "r2.ScenarioWindow:cancelDeleteAct()", "ui:interface")
end

-----------------------------------------------------------------------------------------------------------
-- Act deletion has been confirmed
function r2.ScenarioWindow:confirmDeleteAct()	
	
	r2.requestNewAction(i18n.get("uiR2EDRemoveActAction"))

	assert(not r2:getCurrentAct():isBaseAct())	
	local actId = r2:getCurrentAct().InstanceId
	local firstAct = (r2.Scenario.Acts.Size>1 and (r2:getCurrentAct() == r2.Scenario.Acts[1]))
	
	-- update other acts name
	local afterDeletedAct = false
	for i=0, r2.Scenario.Acts.Size-1 do
		local act = r2.Scenario.Acts[i]

		if (not act:isBaseAct()) and afterDeletedAct==true then

			local name = act.Name
			
			local firstPart = i18n.get("uiR2EDDefaultActTitle"):toUtf8() .. i 
			local firstPartSpace = i18n.get("uiR2EDDefaultActTitle"):toUtf8().. " " .. i 

			if name == firstPart or name == firstPartSpace then
				name = i18n.get("uiR2EDDefaultActTitle"):toUtf8() .. " " .. (i-1) 
				r2.requestSetNode(act.InstanceId, "Name", name)
			end

		elseif act.InstanceId == actId then
			afterDeletedAct = true
		end
	end	

	-- delete permanent content of the act's island if this one isn't used in another location
	local pointToIsland = 0
	local locationId = r2:getCurrentAct().LocationId
	local islandName = r2:getInstanceFromId(locationId).IslandName
	for i=0, r2.Scenario.Locations.Size-1 do
		local location = r2.Scenario.Locations[i]
		assert(location)

		if location.IslandName == islandName then
			pointToIsland = pointToIsland + 1
		end
	end

	if pointToIsland <= 1 then
		-- search island attributes
		local islandTable = nil
		for ecoSystem, ecoSysTable in pairs(r2.acts.islands) do
			local islandNb = r2.acts:getIslandNb(islandName)
			if ecoSysTable[islandNb] and ecoSysTable[islandNb].name == islandName then
				ecoSysName = ecoSystem
				islandTable = ecoSysTable[islandNb].table
				break
			end
		end

		-- delete permanent content positionned in deleted location
		if false and islandTable then
			local permanentContent = {}
			local baseAct = r2.Scenario:getBaseAct()
			baseAct:appendInstancesByType(permanentContent, "WorldObject")
			for k, permObj in pairs(permanentContent) do
				local isInX = (islandTable.xmin<=permObj.Position.x and permObj.Position.x<=islandTable.xmax)
				local isInY = (islandTable.ymin<=permObj.Position.y and permObj.Position.y<=islandTable.ymax)
				if isInX and isInY then
					r2.requestEraseNode(permObj.InstanceId)	
				end
			end
		end
	end

	-- delete the act location if any other act doesn't point to this location
	local pointToLocation = 0
	for i=0, r2.Scenario.Acts.Size-1 do
		local act = r2.Scenario.Acts[i]
		assert(act)

		if act.LocationId == locationId then
			pointToLocation = pointToLocation + 1
		end
	end

	if pointToLocation <= 1 then
		r2.requestEraseNode(locationId)
	end

	-- if Act[1] is deleted, we must create another one
	if actId==r2.Scenario.Acts[1].InstanceId and table.getn(r2.Scenario.Acts)<=2 then
		r2.acts:openScenarioActEditor(false, true, true)
	end

	r2.requestEraseNode(actId)

	r2.requestEndAction()
end

-----------------------------------------------------------------------------------------------------------
-- Act deletion has been canceled
function r2.ScenarioWindow:cancelDeleteAct()
	debugInfo("Cancel deletion of current act")
end

-----------------------------------------------------------------------------------------------------------
-- Reset content of the scenario window
function r2.ScenarioWindow:resetWindow()			
	local function cleanContentTree(contentTree)
		if contentTree and not contentTree.isNil then
			r2:cleanTreeNode(contentTree, "people")
			r2:cleanTreeNode(contentTree, "creatures")
			contentTree:forceRebuild()
		end
	end

	local function cleanContentMacroTree(contentTree)
		if contentTree and not contentTree.isNil then
			--r2:cleanTreeNode(contentTree, "macro_components")
			r2:cleanTreeRootNode(contentTree)
			contentTree:forceRebuild()
		end
	end

	--
	local container = self:getContainer()
	if not container then
		debugInfo("Scenario window not found")
		return
	end

	-- content & geo now merged
	 local contentTree = container:find("content_tree_list")
	 if contentTree and not contentTree.isNil then
		 --r2:cleanTreeNode(contentTree, "scenery_objects")
		 r2:cleanTreeRootNode(contentTree)
		 contentTree:forceRebuild()
	 end

	--
	for i = 0, r2:getMaxNumberOfAdditionnalActs() - 1 do		
		local actTree = container:find("act_tree_" .. tostring(i))
		actTree.active = false
		cleanContentTree(actTree)

		local actMacroTree = container:find("macro_act_tree_" .. tostring(i))
		actMacroTree.active = false
		cleanContentMacroTree(actMacroTree)
	end
	container:find("delete_act").active = false
	-- clean list of acts
	local comboBox = self:getActComboBox()
	if comboBox then
		while comboBox:getNumTexts() ~= 0 do			
			comboBox:removeTextByIndex(0)
		end
	end
	self.IgnoreActChange = true  -- don't whant to trigger a 'onChangeAct' request here (supposed to be called at init)
	comboBox.selection = 0
	self.IgnoreActChange = false
end


----- WEATHER SLIDER VALUE  ---------------------------------------------------------
function r2.ScenarioWindow:weatherValue(requestType)

	if not self.showedActId or not r2:getInstanceFromId(self.showedActId) then return end
	local act = r2:getInstanceFromId(self.showedActId)
	assert(act)

	self:setActNotes()

	local scenarioUI = self:getContainer()
	assert(scenarioUI)

	local actGr = scenarioUI:find("act_properties")
	assert(actGr)

	local weatherValue = actGr:find("weather"):find("slider").value
	r2.requestNewAction(i18n.get("uiR2EDChangeActWeather"))

	if requestType == nil then
		r2.requestSetNode(act.InstanceId, "WeatherValue", weatherValue)
	elseif requestType == 'local' then
		r2.requestSetLocalNode(act.InstanceId, "WeatherValue", weatherValue)
	elseif requestType == 'commit' then
		r2.requestCommitLocalNode(act.InstanceId, "WeatherValue")
	elseif requestType == 'cancel' then
		r2.requestRollbackLocalNode(act.InstanceId, "WeatherValue")
	end

	r2.requestEndAction()
end

----- MANUAL WEATHER ACTIVATION -------------------------------------------------------
function r2.ScenarioWindow:manualWeather()

	if not self.showedActId or not r2:getInstanceFromId(self.showedActId) then return end
	local act = r2:getInstanceFromId(self.showedActId)
	assert(act)

	self:setActNotes()

	local scenarioUI = self:getContainer()
	assert(scenarioUI)

	local actGr = scenarioUI:find("act_properties")
	assert(actGr)

	local manualButton = actGr:find("manual_weather"):find("toggle_butt")
	assert(manualButton)
	local manual=1
	if manualButton.pushed==true then manual=0 end
	
	r2.requestNewAction(i18n.get("uiR2EDChangeActWeatherManual"))
	r2.requestSetNode(act.InstanceId, "ManualWeather", manual)
	r2.requestEndAction()
end

--------------------- set act notes ----------------------------------------------------
function r2.ScenarioWindow:setActNotes()

	if not self.showedActId or not r2:getInstanceFromId(self.showedActId) then return end
	local act = r2:getInstanceFromId(self.showedActId)
	assert(act)

	local scenarioUI = self:getContainer()
	assert(scenarioUI)

	local actGr = scenarioUI:find("act_properties")
	assert(actGr)

	local preActNotes = actGr:find("pre_act_notes"):find("small_description").uc_input_string:toUtf8()
	local actNotes = actGr:find("act_notes"):find("small_description").uc_input_string:toUtf8()

	if act==nil then act = r2:getCurrentAct() end
	if actNotes~=act.ShortDescription then
		r2.requestSetNode(act.InstanceId, "ShortDescription", actNotes)
	end

	if preActNotes~=act.PreActDescription then
		r2.requestSetNode(act.InstanceId, "PreActDescription", preActNotes)
	end
end


--------------------- update act properties in scenario window -------------------------

function r2.ScenarioWindow:showActProperties()

	local actNb = self:getActComboBox().selection + 1
	local actTable = self:getActTable()
	if actTable[actNb]==nil then return end
	local act = actTable[actNb].Act
	if act==nil then act=r2:getCurrentAct() end

	self:setActNotes()
	self.showedActId = act.InstanceId
	
	self:updateActProperties()

	local goButton = self:getContainer():find("go_to_act")	
	assert(goButton)
	goButton.active = (act~=r2:getCurrentAct())

	local deleteButton = self:getContainer():find("delete_act")	
	assert(deleteButton)
	deleteButton.active = (act==r2:getCurrentAct())
end

function r2.ScenarioWindow:updateActProperties()

	if not self.showedActId or not r2:getInstanceFromId(self.showedActId) then return end
	local act = r2:getInstanceFromId(self.showedActId)
	assert(act)
	assert((not act:isBaseAct()))

	act:updateWeather()
	
	local scenarioUI = self:getContainer()
	assert(scenarioUI)

	local actGr = scenarioUI:find("act_properties")
	assert(actGr)

	-- location name
	local location = r2:getInstanceFromId(act.LocationId)
	if location==nil then return end
	local locationName = actGr:find("location_name")
	assert(locationName)
	local uc_location = ucstring()
	uc_location:fromUtf8(location.Name)
	locationName.uc_hardtext = uc_location

	-- season
	local seasons = {
					 ["Spring"] = "uiR2EDSpring", 
					 ["Summer"] = "uiR2EDSummer", 
					 ["Autumn"] = "uiR2EDAutumn", 
					 ["Winter"] = "uiR2EDWinter" 
					}			  
	local season = actGr:find("season_name")
	assert(season)
	local uc_season = ucstring()
	local seasonStr = seasons[location.Season]
	uc_season:fromUtf8(i18n.get(seasonStr):toUtf8())
	season.uc_hardtext = uc_season

	-- manual weather
	local weatherGr = actGr:find("weather")
	assert(weatherGr)
	local weatherManual = weatherGr:find("manual_weather")
	assert(weatherManual)
	weatherManual:find("toggle_butt").pushed = (act.ManualWeather == 0)

	local weatherSlider = actGr:find("weather_slider")
	assert(weatherSlider)
	weatherSlider.active = (act.ManualWeather==1)

	-- weather value
	local weatherSlider = weatherGr:find("slider")
	assert(weatherSlider)
	weatherSlider.value = act.WeatherValue

	-- notes
	do
		local notesGr = actGr:find("act_notes")
		assert(notesGr)
		local notesAct = notesGr:find("small_description")
		assert(notesAct)
		local uc_notes = ucstring()
		uc_notes:fromUtf8(act.ShortDescription)
		notesAct.uc_input_string = uc_notes
	end
	
	-- pre act notes
	do
		local notesGr = actGr:find("pre_act_notes")
		assert(notesGr)
		local notesAct = notesGr:find("small_description")
		assert(notesAct)
		local uc_notes = ucstring()
		uc_notes:fromUtf8(act.PreActDescription)
		notesAct.uc_input_string = uc_notes
	end


	-- budget
	if act==r2:getCurrentAct() then
		local entityBudgetText = scenarioUI:find("entity_budget_text")
		local macroBudgetText = scenarioUI:find("macro_budget_text")
		local ucBudget = ucstring()
		ucBudget:fromUtf8(i18n.get("uiR2EDMacroComponentsInAct"):toUtf8() .. act:getName())
		macroBudgetText.uc_hardtext = ucBudget
		ucBudget:fromUtf8(i18n.get("uiR2EDEntitiesInAct"):toUtf8() .. act:getName())
		entityBudgetText.uc_hardtext = ucBudget
	end
end


--------------------- update scenario properties in scenario window -------------------------
function r2.ScenarioWindow:updateScenarioProperties()

	self.lockComboBoxes = true

	local scenario = r2.Scenario

	local scenarioUI = self:getContainer()
	assert(scenarioUI)

	local scenarioGr = scenarioUI:find("scenario_properties")
	assert(scenarioGr)

	-- scenario level
	local levelCB = scenarioGr:find("level_combo_box")
	assert(levelCB)
	levelCB.selection = r2.Scenario.Description.LevelId

	do
		local cb = scenarioGr:find("locked_combo_box")
		assert(cb)
		if r2.Scenario.Description.OtherCharAccess then
			local access = r2.Scenario.Description.OtherCharAccess
			cb.selection_text = i18n.get("uiR2ED".. access):toUtf8()
		else
			cb.selection_text = i18n.get("uiR2EDFull"):toUtf8() 
		end
		
	end

	if r2.Scenario.Description.NevraxScenario  then
		local ui = getUI("ui:interface:r2ed_palette")	
		assert(ui)
		local widget = ui:find("nevrax_scenario")
		assert(widget)
		local ok = r2.Scenario.Description.NevraxScenario == "1"
		widget:find("toggle_butt").pushed = not ok
	end

	if r2.Scenario.Description.TrialAllowed  then
		local ui = getUI("ui:interface:r2ed_palette")	
		assert(ui)
		local widget = ui:find("trial_allowed_scenario")
		assert(widget)
		local ok =  r2.Scenario.Description.TrialAllowed == "1"
		widget:find("toggle_butt").pushed = not ok
	end
	
	if r2.Scenario.Description.MissionTag then
		local ui = getUI("ui:interface:r2ed_palette")	
		assert(ui)
		local widget = ui:find("mission_tag")
		assert(widget)
		local uc = ucstring()
		uc:fromUtf8(scenario.Description.MissionTag)
		widget:find("mission_tag_eb").eb.uc_input_string = uc
	end

	-- scenario rules
	local rulesCB = scenarioGr:find("rules_combo_box")
	if r2.Scenario.AccessRules == "strict" then
		rulesCB.selection = 0
	else
		rulesCB.selection = 1
	end

	-- scenario language
	local languageCB = scenarioGr:find("language_combo_box")
	assert(languageCB)
	languageCB.selection_text = i18n.get("uiR2ED"..r2.Scenario.Language):toUtf8()

	-- scenario type
	local typeCB = scenarioGr:find("type_combo_box")
	assert(typeCB)
	-- special case for old enums	
	if r2.Scenario.Type =="Combat" then		
		typeCB.selection_text = i18n.get("uiR2EDso_hack_slash"):toUtf8()
	elseif r2.Scenario.Type =="Roleplay" then		
		typeCB.selection_text = i18n.get("uiR2EDso_story_telling"):toUtf8()
	else
		typeCB.selection_text = i18n.get("uiR2ED"..r2.Scenario.Type):toUtf8()
	end

	-- scenario title
	local ucTitle = ucstring()
	ucTitle:fromUtf8(scenario.Description.Title)
	getUI("ui:interface:r2ed_scenario"):find("title_eb").eb.uc_input_string	= ucTitle

	-- scenario notes
	local notesGr = scenarioGr:find("scenario_notes")
	assert(notesGr)
	local notesScenario = notesGr:find("small_description")
	assert(notesScenario)
	local uc_notes = ucstring()
	uc_notes:fromUtf8(scenario.Description.ShortDescription)
	notesScenario.uc_input_string = uc_notes
	
	-- scenario name
	local scenarioName = scenarioUI:find("scenario_name_text")
	local uc_name = ucstring()
	uc_name:fromUtf8(i18n.get("uiR2EDScenarioFilename"):toUtf8() .. " " ..ucstring(scenario:getName()):toUtf8())
	scenarioName.uc_hardtext = uc_name

	self.lockComboBoxes = false
end

--------------------- set scenario notes ----------------------------------------------------
function r2.ScenarioWindow:setScenarioNotes()

	local scenarioUI = self:getContainer()
	assert(scenarioUI)

	local scenarioGr = scenarioUI:find("scenario_properties")
	assert(scenarioGr)

	local scenarioNotes = scenarioGr:find("scenario_notes"):find("small_description").uc_input_string:toUtf8()

	if r2.Scenario and scenarioNotes~= r2.Scenario.Description.ShortDescription then
		r2.requestNewAction(i18n.get("uiR2EDChangeScenarioDescription"))
		r2.requestSetNode(r2.Scenario.Description.InstanceId, "ShortDescription", scenarioNotes)
		r2.requestEndAction()
	end
end

--------------------- set scenario level ----------------------------------------------------
function r2.ScenarioWindow:scenarioLevel()		
	if self.lockComboBoxes then return end		
	
	self:setScenarioNotes()
	self:onTitleChanged()

	local scenarioWnd = self:getContainer()	
	local levelCB = scenarioWnd:find("level_combo_box")	
	r2.requestNewAction(i18n.get("uiR2EDChangeScenarioLevel"))	
	r2.requestSetNode(r2.Scenario.Description.InstanceId, "LevelId", levelCB.selection)	
	r2.requestEndAction()	
end

--------------------- set scenario rules ----------------------------------------------------
function r2.ScenarioWindow:scenarioRules()	
	if self.lockComboBoxes then return end

	self:setScenarioNotes()
	self:onTitleChanged()

	local scenarioWnd = self:getContainer()	
	local rulesCB = scenarioWnd:find("rules_combo_box")	
	local rules = {[0]="strict", [1]="liberal"}	
	r2.requestNewAction(i18n.get("uiR2EDChangeScenarioRules"))
	r2.requestSetNode(r2.Scenario.InstanceId, "AccessRules", rules[rulesCB.selection])
	r2.requestEndAction()
end

------------- set scenario language ----------------------------------------------------------
function r2.ScenarioWindow:scenarioLanguage()

	if self.lockComboBoxes then return end	

	self:setScenarioNotes()
	self:onTitleChanged()

	local comboB = getUICaller()
	local language = self.Languages[comboB.selection_text]

	r2.requestNewAction(i18n.get("uiR2EDChangeScenarioLanguage"))
	r2.requestSetNode(r2.Scenario.InstanceId, "Language", language)
	r2.requestEndAction()
end

------------- set scenario type ---------------------------------------------------------------
function r2.ScenarioWindow:scenarioType()

	if self.lockComboBoxes then return end	

	self:setScenarioNotes()
	self:onTitleChanged()

	local comboB = getUICaller()
	local type = self.Types[comboB.selection_text]

	r2.requestNewAction(i18n.get("uiR2EDChangeScenarioType"))
	r2.requestSetNode(r2.Scenario.InstanceId, "Type", type)
	r2.requestEndAction()
end

------------- set scenario type ---------------------------------------------------------------
function r2.ScenarioWindow:scenarioEditionLocked()

	if self.lockComboBoxes then return end	

	self:setScenarioNotes()
	self:onTitleChanged()

	local comboB = getUICaller()
	
	local type = self.Locked[comboB.selection_text]
	-- RoS not usefull anymore
	if  type == "RoSOnly" and  config.R2EDExtendedDebug ~= 1 then
		type = "Full"
		self.lockComboBoxes = true
		comboB.selection_text = i18n.get("uiR2EDFull")
		self.lockComboBoxes = false
	end
	r2.requestNewAction(i18n.get("uiR2EDChangeScenarioEditionLock"))
	r2.requestSetNode(r2.Scenario.Description.InstanceId, "OtherCharAccess", type)
	r2.requestEndAction()
end
--------------------------------------------------------------------------
-- the scenario title was changed by the user
function r2.ScenarioWindow:onTitleChanged()
	if self.lockComboBoxes then return end	
	local newTitle = getUI("ui:interface:r2ed_scenario"):find("title_eb").eb.uc_input_string:toUtf8()
	if newTitle ~= r2.Scenario.Description.Title then
		r2.requestNewAction(i18n.get("uiR2EDChangeScenarioTitle"))	
		r2.requestSetNode(r2.Scenario.Description.InstanceId, "Title", newTitle)
		r2.requestEndAction()
	end
end

----------------------- cancel focus on text of all edit box when select a new tab --------------
function r2.ScenarioWindow:cancelFocusOnText()
	local editBox = getUICaller()
	assert(editBox)
	editBox:cancelFocusOnText()
end
  	 


--------------------- set nevrax scenario ----------------------------------------------------
function r2.ScenarioWindow:nevraxScenario()
	if self.lockComboBoxes then return end		
	if config.R2EDExtendedDebug ~= 1 then return end

	local scenarioUI = getUI("ui:interface:r2ed_palette")	
	assert(scenarioUI)

	local widget = scenarioUI:find("nevrax_scenario")
	assert(widget)
	

	local val = "0"
	if widget:find("toggle_butt").pushed then val = "0" else val = "1" end

	self:setScenarioNotes()
	self:onTitleChanged()


	r2.requestNewAction(i18n.get("uiR2EDChangeScenarioNevraxTag"))	
	r2.requestSetNode(r2.Scenario.Description.InstanceId, "NevraxScenario", val)	
	r2.requestEndAction()

	
end

--------------------- set nevrax scenario ----------------------------------------------------
function r2.ScenarioWindow:trialAllowed()
	if self.lockComboBoxes then return end		
	if config.R2EDExtendedDebug ~= 1 then return end

	local ui = getUI("ui:interface:r2ed_palette")	
	assert(ui)

	local widget = ui:find("trial_allowed_scenario")
	assert(widget)
	
	local val = "0"
	if widget:find("toggle_butt").pushed then val = "0" else val = "1" end

	self:setScenarioNotes()
	self:onTitleChanged()

	r2.requestNewAction(i18n.get("uiR2EDChangeTrialAllowedTag"))	
	r2.requestSetNode(r2.Scenario.Description.InstanceId, "TrialAllowed", val  )	
	r2.requestEndAction()

end

--------------------------------------------------------------------------
-- the scenario title was changed by the user
function r2.ScenarioWindow:onScenarioMissionTagChanged()
	if self.lockComboBoxes then return end		
	if config.R2EDExtendedDebug ~= 1 then return end
	local newTag = getUI("ui:interface:r2ed_palette"):find("mission_tag"):find("mission_tag_eb").eb.uc_input_string:toUtf8()
	
	self:setScenarioNotes()
	self:onTitleChanged()

	if newTag ~= r2.Scenario.Description.MissionTag then
		r2.requestNewAction(i18n.get("uiR2EDChangeMissionTag"))	
		r2.requestSetNode(r2.Scenario.Description.InstanceId, "MissionTag", newTag)
		r2.requestEndAction()
	end
end
 
