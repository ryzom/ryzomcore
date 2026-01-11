
-- SEX -----------------------------------------------------------------------------------
r2.male = "male"
r2.female = "female"
r2.sexSelection = "male"

-- RACE ----------------------------------------------------------------------------------
r2.raceSelection = "Fyros"
r2.fyros  = "Fyros"
r2.matis  = "Matis"
r2.tryker = "Tryker"
r2.zorai  = "Zorai"

r2.NPCNames = {} -- All available names
r2.NPCAllNames = {} -- All Names (even those who are not available)

-- CHANGE VIEW ---------------------------------------------------------------------------
r2.faceView = false
r2.camPosY		= -16.4      
r2.camZoomPosY	= -3.9
r2.camTgtZ		= -0.79    
r2.camTgtZZorai	= -0.75    
r2.camPosYZorai	= -18.3  
r2.distlimitmin = 9.0
r2.distlimitmax = -r2.camPosY
r2.distlimitminZoom = 2.5

-- FACE MORPH EDIT TEXT ------------------------------------------------------------------
r2.raceMorphEditText = {}
r2.raceMorphEditText[r2.fyros]	= { [1]="Mouth level",	[2]="Eyes", 
									[3]="Nose level",	[4]="Mouth width",
									[5]="Nose length",	[6]="Nose width",
									[7]="Brown width",	[8]="Chin"}

r2.raceMorphEditText[r2.matis]	= { [1]="Eyes level",	[2]="Nose level", 
									[3]="Nose length",	[4]="Nose width",
									[5]="Mouth width",	[6]="Mouth level",
									[7]="Ear size",		[8]="Chin"}

r2.raceMorphEditText[r2.tryker] = { [1]="Eyes level",	[2]="Nose level", 
									[3]="Nose length",	[4]="Nose width",
									[5]="Mouth width",	[6]="Mouth level",
									[7]="Ear size",		[8]="Chin"}

r2.raceMorphEditText[r2.zorai]	= { [1]="Eyes size",	[2]="bottom horn 1", 
									[3]="bottom horn 2",[4]="middle horn 1",
									[5]="middle horn 2",[6]="top horn 1",
									[7]="top horn 2",	[8]=""}

r2.tableBodySets = {}
r2.tableFaceSets = {}
r2.tableFaceMorph = {}

r2.raceSheetNameMatch = 
{
	["basic_fyros_male.creature"] = "Fyros",
	["basic_fyros_female.creature"] = "Fyros",
	["basic_matis_male.creature"] = "Matis",
	["basic_matis_female.creature"] = "Matis",
	["basic_tryker_male.creature"] = "Tryker",
	["basic_tryker_female.creature"] = "Tryker",
	["basic_zorai_male.creature"] = "Zorai",
	["basic_zorai_female.creature"] = "Zorai"
}

r2.raceSheetClient = 
{
	male  = {Fyros="basic_fyros_male.creature",
		     Matis="basic_matis_male.creature",
		     Tryker="basic_tryker_male.creature",
		     Zorai="basic_zorai_male.creature"},

	female= {Fyros="basic_fyros_female.creature",
		     Matis="basic_matis_female.creature",
		     Tryker="basic_tryker_female.creature",
		     Zorai="basic_zorai_female.creature"}
}


-- ITEM INDEX / SLIDER VALUE FACES SETS -------------------------------------------------- 
r2.sliderValueToItemIndex = {}
r2.sliderValueToItemIndex[r2.fyros]  = {[0] = "fy_cheveux_shave01.sitem",	[1] = "fy_cheveux_short01.sitem",
										[2] = "fy_cheveux_short02.sitem",	[3] = "fy_cheveux_medium01.sitem",
										[4] = "fy_cheveux_medium02.sitem",	[5] = "fy_cheveux_medium03.sitem",
										[6] = "fy_cheveux_long01.sitem"}

r2.sliderValueToItemIndex[r2.matis]  = {[0] = "ma_cheveux_shave01.sitem",	[1] = "ma_cheveux_short01.sitem",
										[2] = "ma_cheveux_short02.sitem",	[3] = "ma_cheveux_medium01.sitem",
										[4] = "ma_cheveux_medium02.sitem",	[5] = "ma_cheveux_long01.sitem",
										[6] = "ma_cheveux_long02.sitem"}

r2.sliderValueToItemIndex[r2.tryker] = {[0] = "tr_cheveux_shave01.sitem",	[1] = "tr_cheveux_short01.sitem",
										[2] = "tr_cheveux_short02.sitem",	[3] = "tr_cheveux_short03.sitem",
										[4] = "tr_cheveux_medium01.sitem",	[5] = "tr_cheveux_medium02.sitem",
										[6] = "tr_cheveux_long01.sitem"}

r2.sliderValueToItemIndex[r2.zorai]  = {[0] = "zo_cheveux_shave01.sitem",	[1] = "zo_cheveux_short01.sitem",
										[2] = "zo_cheveux_medium01.sitem",	[3] = "zo_cheveux_medium02.sitem",
										[4] = "zo_cheveux_medium03.sitem",	[5] = "zo_cheveux_long01.sitem",
										[6] = "zo_cheveux_long02.sitem"}

r2.itemIndexToSliderValue = {}

r2.hasHelmet = false

r2.hairType = {}

r2.selectEquipmentSet = false

-----------------------------------------------------------------------
-- Current fonction to change an attribute in a npc
-- default is to do a 'requestSetNode'
function r2:setNpcAttribute(instanceId, propName, propValue)
	r2.requestSetNode(instanceId, propName, propValue)	
end



-- INIT ITEM INDEX TO SLIDER VALUE TABLE ------------------------------------------------- 
function r2:initItemIndexToSliderValueTable()

	r2.itemIndexToSliderValue={}
	local races = {r2.fyros, r2.matis, r2.tryker, r2.zorai}

	for key1, value1 in pairs(races) do
		r2.itemIndexToSliderValue[value1] = {}

		for key2, value2 in pairs(r2.sliderValueToItemIndex[value1]) do
			local itemIndex = getSheetId(value2)
			r2.itemIndexToSliderValue[value1][itemIndex] = key2
		end
	end
end


local body_sliders = {GabaritHeight		="slider_height", 
					  GabaritTorsoWidth	="slider_torso", 
					  GabaritArmsWidth	="slider_arms",
					  GabaritLegsWidth	="slider_legs",
					  GabaritBreastSize	="slider_breast",

					  HairType			="slider_haircut",
					  HairColor			="slider_hair_color",
					  Tattoo			="slider_tattoos",
					  EyesColor			="slider_eye_color",
					
					  MorphTarget1		="slider_morph_target1",
					  MorphTarget2		="slider_morph_target2",
					  MorphTarget3		="slider_morph_target3",
					  MorphTarget4		="slider_morph_target4",
					  MorphTarget5		="slider_morph_target5",
					  MorphTarget6		="slider_morph_target6",
					  MorphTarget7		="slider_morph_target7",
					  MorphTarget8		="slider_morph_target8"}

r2.sheetTypeCB = {}


-- share npc displayer between all instances
local npcCustomPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

local oldCustomNPCOnAttrModified = npcCustomPropertySheetDisplayerTable.onAttrModified


------------------------------------------------
function npcCustomPropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function npcCustomPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function npcCustomPropertySheetDisplayerTable:onPreHrcMove(instance)
	r2:npcPropertySheetDisplayer():onPreHrcMove(instance)	
end
------------------------------------------------
function npcCustomPropertySheetDisplayerTable:onPostHrcMove(instance)
	r2:npcPropertySheetDisplayer():onPostHrcMove(instance)			
end
------------------------------------------------
function npcCustomPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
r2.customizationOpen = false
function npcCustomPropertySheetDisplayerTable:onSelect(instance, isSelected)
	r2:npcPropertySheetDisplayer():onSelect(instance, isSelected)
	
	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	if not isSelected then
		r2.customizationOpen = npcUI.active
		npcUI.active = false
	else
		r2.raceSelection = getR2PlayerRace(instance.SheetClient)

		if r2.customizationOpen == true then
			r2.customizationOpen = false
			npcCustomPropertySheetDisplayerTable:updateAll(instance)
		end
	end
end

------------------------------------------------
function npcCustomPropertySheetDisplayerTable:updateAll(instance)
	-- nico patch :
	-- replace the r2:setNpcProperty function, so that it does call 'requestSetNode', but rather forward call
	-- to the displayer directly...
	local function bypassRequestNode(this, instanceId, propName, propValue)		
		local instance = r2:getInstanceFromId(instanceId)
		instance.DisplayerProperties:onAttrModified(instance, propName)
	end
	local oldSetNpcAttribute = r2.setNpcAttribute
	r2.setNpcAttribute = bypassRequestNode
	local ok, msg = pcall(self.updateAllPrivate, self, instance)
	if not ok then
		debugInfo(msg)
	end	
	r2.setNpcAttribute = oldSetNpcAttribute
end

------------------------------------------------
function npcCustomPropertySheetDisplayerTable:updateAllPrivate(instance)
	
	-- if attribute is visible in the generic property sheet, then update it
	local propDesc = instance:getPropDesc(attributeName)		
	if propDesc and propDesc.Visible ~= false then -- NB : "visible" may be nil, which default to 'true'...
		-- property visible in the generic property sheet so update it
		--debugInfo("Updating in generic property sheet : " .. attributeName)
		oldCustomNPCOnAttrModified(self, instance, attributeName)
	end

	assert(instance)	
	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local propWnd = r2:getPropertySheet(instance)	
	if propWnd.active then
		local nameEBox = propWnd:find("Name"):find("eb")
		local name = nameEBox.uc_input_string:toUtf8()
		r2:setNpcAttribute(instance.InstanceId, "Name", name)
	end	

	npcUI.active = true
				
	r2:updateNPCView(instance)
	r2.hairType = {}

	-- update sex
	local sexCB = npcUI:find("sex").combo_box
	assert(sexCB)

	if isR2PlayerMale(instance.SheetClient) then
		r2.sexSelection = r2.male
	else 
		r2.sexSelection = r2.female	
	end
	sexCB.Env.locked = true
	sexCB.selection_text = r2.sexSelection
	sexCB.Env.locked = false

	-- update race in combo box
	local raceCB = npcUI:find("race").combo_box
	assert(raceCB)
	r2.raceSelection = getR2PlayerRace(instance.SheetClient)
	local sliderHairType = r2.itemIndexToSliderValue[r2.raceSelection][instance.HairType]	
	r2.hasHelmet = (sliderHairType == nil)	
	
	if not r2.hasHelmet then
		r2.hairType.hairCut = sliderHairType
		r2.hairType.hairColor = instance.HairColor
	else
		r2.hairType.hairCut = 0
		r2.hairType.hairColor = 0
	end
	raceCB.Env.locked = true
	raceCB.selection_text = r2.raceSelection
	raceCB.Env.locked = false
	
	-- update name
	local editName = npcUI:find("name").edit_box_group
	assert(editName)
	editName.uc_input_string = instance:getDisplayName()
	npcUI.uc_title = concatUCString(i18n.get("uiRE2DPropertiesOf"), instance:getDisplayName())

	-- update notes
	local editNotes = npcUI:find("small_description")
	assert(editNotes)
	if instance.Notes ~= nil then
		editNotes.input_string = instance.Notes
	else
		editNotes.input_string = ""
	end

	-- update avoidable equipment
	r2:updateEquipment(instance, false)

	-- update link color
	local toggleB = npcUI:find("color_link").toggle_butt
	assert(toggleB)
	local link = false
	if instance.LinkColor==1 then link=true end
	toggleB.pushed = not link

	-- update weapons
	comboBox = npcUI:find("hands"):find("combo_box")

	local handsLevel = r2:getHandsLevel(instance.SheetModel, instance.Sheet)
	local rightHandIndex = instance.WeaponRightHand
	local leftHandIndex = instance.WeaponLeftHand

	local handsKey = rightHandIndex..":"..leftHandIndex..":"..handsLevel
	local weaponsName = r2.itemIndexEquipmentToSelectionText[instance.Equipment][comboBox.Env.nameComboBox][handsKey] 
	comboBox.Env.locked = true
	comboBox.selection_text = weaponsName
	comboBox.Env.locked = false

	-- camera target is out of 3D character
	r2:updateRaceView()
	r2.faceView = false

	-- update body sliders
	for key, value in pairs(body_sliders) do 
		local slider = npcUI:find(value):find("slider")
		assert(slider)
		if value == "slider_haircut" and not r2.hasHelmet then
			slider.value = r2.itemIndexToSliderValue[r2.raceSelection][instance["HairType"] ]
		else
			slider.value = instance[key]
		end
	end

	-- empty body/face/morph sets
	local bodyCB = npcUI:find("body_sets").combo_box
	assert(bodyCB)
	bodyCB.view_text = r2.emptyComboLine

	local faceCB = npcUI:find("face_sets").combo_box
	assert(faceCB)
	faceCB.view_text = r2.emptyComboLine

	local morphCB = npcUI:find("face_morph").combo_box
	assert(morphCB)
	morphCB.view_text = r2.emptyComboLine

	r2:updateRaceUI()
end

------------------------------------------------
function r2:initTypeUI(instance)

	local propertiesUI = getUI(r2:getDefaultPropertySheetUIPath(instance.Class))
	assert(propertiesUI)

	-- update avoidable types
	local typeCB = propertiesUI:find("TypeNPC")
	assert(typeCB)

	typeCB:resetTexts()

	local currentType = ""

	-- search for current ecosystem et level selected in palette UI
	local levelMin, levelMax, ecosystem = r2:getPaletteAccessibilityFactors()

	local place = 0
	if ecosystem and levelMin and levelMax then

		-- parent palette element in palette UI (type node)
		local typePalette = r2.PaletteIdToType[instance.Base]
		r2.sheetTypeCB = {}
		
		-- for each element of this type, check if must be added in combo box (only if also showed in palette UI)
		-- then the same test is used than in r2:setupPaletteAccessibleContent
		for typeKey, typeValue in pairs(typePalette) do

			local typeTranslation = typeValue.Translation
			local typeElement = r2.getPaletteElement(typeValue.Id)
			
			local show = false
			local levelElt = defaulting(typeElement.Level, 1)
			local ecosystemElt = defaulting(typeElement.Ecosystem, "")
			if string.match(typeElement.SheetClient, "object.*") then
				show = true
			elseif levelElt >= levelMin and levelElt <= levelMax and (ecosystemElt == "" or string.match(ecosystemElt, ecosystem)) then
				show = true					
			end

			if show then

				r2.sheetTypeCB[typeCB:getNumTexts()] = typeValue
				
				typeCB:addText(i18n.get(typeTranslation))
				
				if instance.Base==typeValue.Id then
					currentType = typeValue.Translation
				end

				if currentType=="" then place = place+1 end
			end
		end

		if tostring(instance.TypeNPC)=="-1" then
			-- TypeNPC
			r2:setNpcAttribute(instance.InstanceId, "TypeNPC", place)
		end
	end
end

------------------------------------------------

r2.bodyAttributesCB = {
					["GabaritHeight"] =		{comboBox="body_sets",	updateComboBox="r2ed_add_body_sets",	tableSets="tableBodySets"},
					["GabaritTorsoWidth"] = {comboBox="body_sets",	updateComboBox="r2ed_add_body_sets",	tableSets="tableBodySets"},
					["GabaritArmsWidth"] =	{comboBox="body_sets",	updateComboBox="r2ed_add_body_sets",	tableSets="tableBodySets"},
					["GabaritLegsWidth"] =	{comboBox="body_sets",	updateComboBox="r2ed_add_body_sets",	tableSets="tableBodySets"},
					["GabaritBreastSize"] = {comboBox="body_sets",	updateComboBox="r2ed_add_body_sets",	tableSets="tableBodySets"},
					["EyesColor"] =			{comboBox="face_sets",	updateComboBox="r2ed_add_face_sets",	tableSets="tableFaceSets"},
					["Tattoo"] =			{comboBox="face_sets",	updateComboBox="r2ed_add_face_sets",	tableSets="tableFaceSets"},
					["HairType"] =			{comboBox="face_sets",	updateComboBox="r2ed_add_face_sets",	tableSets="tableFaceSets"},
					["HairColor"] =			{comboBox="face_sets",	updateComboBox="r2ed_add_face_sets",	tableSets="tableFaceSets"},
					["MorphTarget1"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget2"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget3"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget4"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget5"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget6"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget7"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					["MorphTarget8"] =		{comboBox="face_morph", updateComboBox="r2ed_add_face_morph",	tableSets="tableFaceMorph"},
					}



function npcCustomPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
		

	r2:npcPropertySheetDisplayer():onAttrModified(instance, attributeName)

	if r2:getSelectedInstance()==nil or r2:getSelectedInstance().InstanceId ~= instance.InstanceId then
		return 
	end

	r2:updateNPCView(instance)

	local npcUI = getUI("ui:interface:r2ed_npc")

	local comboBox

	if attributeName == "HairType" then
		local sliderHairType = r2.itemIndexToSliderValue[r2.raceSelection][instance.HairType]	
		r2.hasHelmet = (sliderHairType == nil)
	end

	-- SEX AND RACE
	if attributeName == "SheetClient" then

		-- sex
		comboBox = npcUI:find("sex").combo_box

		local isFemale = string.find(instance[attributeName], "female")
		if isFemale ~= nil then
			r2.sexSelection = r2.female
		else
			r2.sexSelection = r2.male
		end
		comboBox.Env.locked = true
		comboBox.selection_text = r2.sexSelection
		comboBox.Env.locked = false
		
		-- race
		local comboBox = npcUI:find("race").combo_box
		r2.raceSelection = r2.raceSheetNameMatch[instance[attributeName]]
		comboBox.Env.locked = true
		comboBox.selection_text = r2.raceSelection
		comboBox.Env.locked = false

		r2:updateRaceView()
		r2:updateFaceView() 

		r2:updateRaceUI()

		return 
	end


	-- NAME
	if attributeName == "Name" then
		local editName = npcUI:find("name").edit_box_group
		assert(editName)
		editName.uc_input_string = instance:getDisplayName()
		npcUI.uc_title = concatUCString(i18n.get("uiRE2DPropertiesOf"), instance:getDisplayName())
		editName:cancelFocusOnText()
		return 
	end

	-- NOTES
	if attributeName == "Notes" then
		local editNotes= npcUI:find("small_description")
		assert(editNotes)
		editNotes.input_string = instance[attributeName]
		editNotes:cancelFocusOnText()
		return 
	end


	-- EQUIPMENT PIECE
	local slider
	for cbbName, v in pairs(r2.equipmentEnv) do
		if v.propName == attributeName  then
			
			comboBox = npcUI:find(cbbName):find("combo_box")
			assert(comboBox)

			local CBText = r2.itemIndexEquipmentToSelectionText[instance.Equipment][cbbName][instance[attributeName]]	
			if CBText==nil then CBText= r2.noPiece end
			if attributeName == "HairType" then				
				slider = npcUI:find("slider_haircut")
				assert(slider)					
				slider.active = (CBText == r2.noPiece)
				if slider.active then
					local value = r2.itemIndexToSliderValue[r2.raceSelection][instance["HairType"] ]
					if value then
						slider:find("slider").value = value
					end
				end

				slider = npcUI:find("slider_hair_color")
				assert(slider)					
				slider.active = (CBText == r2.noPiece)
				if slider.active then
					slider:find("slider").value = instance.HairColor
				end
			end
				
			comboBox.Env.locked = true
			comboBox.selection_text = CBText
			comboBox.Env.locked = false

			-- if equipment is 'no one', color slider must be hidden
			slider = npcUI:find(cbbName):find("slider")
			assert(slider)
			local line = npcUI:find(cbbName):find("line_slider")
			assert(line)
			slider.active = (CBText ~= r2.noPiece)
			line.active = (CBText ~= r2.noPiece)
			if slider.active then
				slider.value = instance[v.propColorName]	
			end

			return
		end

		if v.propColorName == attributeName  then			
			if attributeName == "HairColor" then				
				if not r2.hasHelmet then					
					slider = npcUI:find("slider_hair_color"):find("slider")					
					assert(slider)					
					slider.value = instance[attributeName]					
				end				
			else				
				slider = npcUI:find(cbbName):find("slider")
				assert(slider)
				slider.value = instance[attributeName]				
			end

			for k, v1 in pairs(r2.equipementComboB) do
				slider = npcUI:find(v1):find("slider")
				local comboBox = npcUI:find(cbbName):find("combo_box")
				local propName = r2.equipmentEnv[v1].propName
				local comboText 
				if propName=="HairType" then
					if r2.hasHelmet then
						comboText = r2.itemIndexEquipmentToSelectionText[instance.Equipment][v1][instance[propName]]
					else
						comboText = r2.noPiece
					end
				else
					comboText = r2.itemIndexEquipmentToSelectionText[instance.Equipment][v1][instance[propName]]
				end
			end
			return 
		end
	end

	-- EQUIPMENT
	if attributeName == "Equipment" then
		r2:updateEquipment(instance, false)
	end

	-- LINK COLOR
	if attributeName == "LinkColor" then
		local toggleB = npcUI:find("color_link").toggle_butt
		assert(toggleB)
		local link = false
		if instance.LinkColor==1 then link=true end
		toggleB.pushed = not link
	end

	-- BODY SETS / FACE SETS/ FACE MORPH
	local sliderUIPath = body_sliders[attributeName]
	if sliderUIPath then
		r2:updateBodyAttribute(attributeName, instance[attributeName])
		return
	end

	-- FUNCTION
	if attributeName == "Function" then
		local functionCB = npcUI:find("fonction").combo_box
		assert(functionCB)
		functionCB.Env.locked = true
		functionCB.selection_text = instance[attributeName]
		functionCB.Env.locked = false
		return
	end

	-- SHEET
	if attributeName == "Sheet" then

		-- update weapons
		local weaponsCB = npcUI:find("hands"):find("combo_box")
		assert(weaponsCB)

		local handsLevel = r2:getHandsLevel(instance.SheetModel, instance.Sheet)
		local rightHandIndex = instance.WeaponRightHand
		local leftHandIndex = instance.WeaponLeftHand

		local handsKey = rightHandIndex..":"..leftHandIndex..":"..handsLevel
		local weaponsName = r2.itemIndexEquipmentToSelectionText[instance.Equipment][weaponsCB.Env.nameComboBox][handsKey] 
		if weaponsName then
			weaponsCB.Env.locked = true
			weaponsCB.selection_text = weaponsName
			weaponsCB.Env.locked = false
		end

		return
	end

	-- PROFILE
	if attributeName == "Profile" then
		local profileCB = npcUI:find("speed_aggro").profile.combo_box
		assert(profileCB)
		profileCB.Env.locked = true
		profileCB.selection_text = r2.profileCB[instance[attributeName]]
		profileCB.Env.locked = false
		return
	end
end	

------------------------------------------------------------------------------------------- 
function r2:npcCustomPropertySheetDisplayer()	
	return npcCustomPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end


-- share npc displayer between all instances
local npcPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

local oldNPCOnAttrModified = npcPropertySheetDisplayerTable.onAttrModified

------------------------------------------------
function npcPropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function npcPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function npcPropertySheetDisplayerTable:onPreHrcMove(instance)	
end
------------------------------------------------
function npcPropertySheetDisplayerTable:onPostHrcMove(instance)	
	
	local selectedInstance = r2:getSelectedInstance()
	if selectedInstance and selectedInstance==instance then
		
		r2.miniActivities:openEditor()	

		if r2.activities.isInitialized then
			r2.activities.isInitialized = false
			r2.activities:initEditorAfterFirstCall()
		end	
	end

	instance:updatePermanentStatutIcon()
	r2.events:updateElementsUI()
end
------------------------------------------------
function npcPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function npcPropertySheetDisplayerTable:onSelect(instance, isSelected)

	r2:activeLogicEntityPropertySheetDisplayer():onSelect(instance, isSelected)

	r2:initTypeUI(instance)
end

------------------------------------------------
function npcPropertySheetDisplayerTable:onAttrModified(instance, attributeName)	

	-- if attribute is visible in the generic property sheet, then update it
	local propDesc = instance:getPropDesc(attributeName)		
	if propDesc and propDesc.Visible ~= false then -- NB : "visible" may be nil, which default to 'true'...
		-- property visible in the generic property sheet so update it
		oldNPCOnAttrModified(self, instance, attributeName)
	end

	r2:activeLogicEntityPropertySheetDisplayer():onAttrModified(instance, attributeName)
end	

------------------------------------------------------------------------------------------- 
function r2:npcPropertySheetDisplayer()	
	return npcPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end

------------------ UPDATE NPC EDITOR -------------------------------------------------------
function r2:updateNPCView(instance)

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
		
	local char3D = npcUI:find("char3d").char
	assert(char3D)
	
	local slotEntity = instance.DisplayerVisual.slotEntity
	assert(slotEntity)

	if slotEntity > -1 then
		char3D:setupCharacter3D(slotEntity)
	end

	-- disable char LOD interpolation
	char3D:enableLOD(false)
end

------------------ BODY --------------------------------------------------------------------

r2.bodyAttNb = 5
r2.bodyAttributes  = {[1] = "GabaritHeight",		[2] = "GabaritTorsoWidth", 
					  [3] = "GabaritArmsWidth",		[4] = "GabaritLegsWidth",
					  [5] = "GabaritBreastSize"}

r2.bodySliders	   = {[1] = "slider_height",		[2] = "slider_torso", 
					  [3] = "slider_arms",			[4] = "slider_legs",
					  [5] = "slider_breast"}


r2.bodyKeys		   = {[1] = "GabaritHeight",				[2] = "GabaritTorsoWidth", 
					  [3] = "GabaritArmsWidth",			[4] = "GabaritLegsWidth",
					  [5] = "GabaritBreastSize"}


------------------ FACE ---------------------------------------------------------------------

r2.faceAttNb = 4
r2.faceAttributes  = {[1] = "HairType",				[2] = "HairColor",
					  [3] = "Tattoo",				[4] = "EyesColor"}

r2.faceSliders	   = {[1] = "slider_haircut",		[2] = "slider_hair_color", 
					  [3] = "slider_tattoos",		[4] = "slider_eye_color"}

r2.faceKeys		   = {[1] = "HairType",				[2] = "HairColor", 
					  [3] = "Tattoo",				[4] = "EyesColor"}	
					  
					  
------------------ MORPH --------------------------------------------------------------------

r2.morphAttNb = 8
r2.morphAttributes = {[1] = "MorphTarget1",			[2] = "MorphTarget2",
					  [3] = "MorphTarget3",			[4] = "MorphTarget4",
					  [5] = "MorphTarget5",			[6] = "MorphTarget6",
					  [7] = "MorphTarget7",			[8] = "MorphTarget8"}

r2.morphSliders	   = {[1] = "slider_morph_target1",	[2] = "slider_morph_target2", 
					  [3] = "slider_morph_target3",	[4] = "slider_morph_target4",
					  [5] = "slider_morph_target5",	[6] = "slider_morph_target6", 
					  [7] = "slider_morph_target7",	[8] = "slider_morph_target8"}

r2.morphKeys	   = {[1] = "MorphTarget1",			[2] = "MorphTarget2", 
					  [3] = "MorphTarget3",			[4] = "MorphTarget4",
					  [5] = "MorphTarget5",			[6] = "MorphTarget6", 
					  [7] = "MorphTarget7",			[8] = "MorphTarget8"}

r2.emptyComboLine = ""


-- INIT NPC EDITOR ---------------------------------------------------------------------------

function r2:initNpcEditor()

	local npcUI = getUI("ui:interface:r2ed_npc")

	-- equipment color sliders
	local levelDesignEnabled = getClientCfgVar("LevelDesignEnabled")
	local maxVal = 5
	if tonumber(levelDesignEnabled)==1 then maxVal = 7 end

	for i=1, r2.equipmentAttNb do 
		local slider = npcUI:find(r2.equipementComboB[i]):find("slider")
		assert(slider)
		slider.max = maxVal
	end
	
	-- race
	local raceCB = npcUI:find("race").combo_box
	assert(raceCB)
	raceCB:resetTexts()
	raceCB:addText(ucstring("Fyros"))
	raceCB:addText(ucstring("Matis"))
	raceCB:addText(ucstring("Tryker"))
	raceCB:addText(ucstring("Zorai"))

	-- sex
	local sexCB = npcUI:find("sex").combo_box
	assert(sexCB)
	sexCB:resetTexts()
	sexCB:addText(ucstring(r2.female))
	sexCB:addText(ucstring(r2.male))

	-- Equipment
	r2:initEquipmentEnv()

	r2:loadTables()

	-- body sets
	local bodySetsCB = npcUI:find("body_sets").combo_box
	assert(bodySetsCB)
	local updateBodySetsCB = getUI("ui:interface:r2ed_add_body_sets"):find("update_sets").combo_box
	assert(updateBodySetsCB)
	bodySetsCB:resetTexts()
	updateBodySetsCB:resetTexts()
	
	for k, v in pairs(r2.tableBodySets) do
		bodySetsCB:addText(ucstring(k))
		updateBodySetsCB:addText(ucstring(k))
	end

	-- face sets
	local faceSetsCB = npcUI:find("face_sets").combo_box
	assert(faceSetsCB)
	local updateFaceSetsCB = getUI("ui:interface:r2ed_add_face_sets"):find("update_sets").combo_box
	assert(updateFaceSetsCB)
	faceSetsCB:resetTexts()
	updateFaceSetsCB:resetTexts()
	
	for k, v in pairs(r2.tableFaceSets) do
		faceSetsCB:addText(ucstring(k))
		updateFaceSetsCB:addText(ucstring(k))
	end

	-- face morph
	local faceMorphCB = npcUI:find("face_morph").combo_box
	assert(faceMorphCB)
	local updateFaceMorphCB = getUI("ui:interface:r2ed_add_face_morph"):find("update_sets").combo_box
	assert(updateFaceMorphCB)
	faceMorphCB:resetTexts()
	updateFaceMorphCB:resetTexts()
	
	for k, v in pairs(r2.tableFaceMorph) do
		faceMorphCB:addText(ucstring(k))
		updateFaceMorphCB:addText(ucstring(k))
	end

	bodySetsCB.view_text = r2.emptyComboLine
	updateBodySetsCB.view_text = r2.emptyComboLine
	faceSetsCB.view_text = r2.emptyComboLine
	updateFaceSetsCB.view_text = r2.emptyComboLine
	faceMorphCB.view_text = r2.emptyComboLine
	updateFaceMorphCB.view_text = r2.emptyComboLine

	r2:initItemIndexToSliderValueTable()
end

-- SAVE TABLE ------------------------------------------------------------------------------

function r2:saveTable(fileName, t)
	io.output(io.open(fileName,"w"))
	r2:serialize(t)
	io.close()
end

function r2:serialize(o)

	if type(o) == "number" then
		io.write(o)
	elseif type(o) == "string" then
		--io.write(string.format('%q', o))
		io.write("["..o.."]")
	elseif type(o) == "table" then
		io.write("{\n")
		for k,v in pairs(o) do
			r2:serialize(k)
			io.write(" = ")
			r2:serialize(v)
			io.write("\n")
		end
		io.write("}")
	end
end

-- LOAD TABLE ------------------------------------------------------------------------------

function r2:loadTables()

	r2.equipmentSets = r2:loadTable("save\\equipmentSets.txt")
	r2.tableBodySets = r2:loadTable("save\\tableBodySets.txt")
	r2.tableFaceSets = r2:loadTable("save\\tableFaceSets.txt")
	r2.tableFaceMorph = r2:loadTable("save\\tableFaceMorph.txt")

	r2:loadRaceSexNames(r2.fyros, r2.male)
	r2:loadRaceSexNames(r2.fyros, r2.female)
	r2:loadRaceSexNames(r2.matis, r2.male)
	r2:loadRaceSexNames(r2.matis, r2.female)
	r2:loadRaceSexNames(r2.tryker, r2.male)
	r2:loadRaceSexNames(r2.tryker, r2.female)
	r2:loadRaceSexNames(r2.zorai, r2.male)
	r2:loadRaceSexNames(r2.zorai, r2.female) 
end

function r2:loadTable(fileName)

	function loadTableR(file)

		local resultT = {}
		local s, e, l, line, key, value
		local line = file:read("*l")

		while line ~= "}" do

			s, e = string.find(line, " = ")

			key = string.sub(line, 1, s-1)
			value = string.sub(line, e+1)

			-- KEY
			-- string key
			if string.sub(key, 1, 1) == "[" then  
				l = string.len(key)
				key= string.sub(key, 2, l-1)
			-- number key
			else
				key = tonumber(key)
			end

			-- VALUE
			-- string value
			if string.sub(value, 1, 1) == "[" then  
				l = string.len(value)
				value= string.sub(value, 2, l-1)
			-- number value
			elseif value == "{" then
				value = loadTableR(file)
			else
				value = tonumber(value)
			end

			resultT[key] = value
			line = file:read("*l")
		end
		return resultT
	end

	local file = io.open(fileName, "r")

	if file == nil then return {} end

	if file:read("*l") ~= "{" then
		io.close(file)
		return {}
	end

	local resultT = loadTableR(file)
	io.close(file)
	return resultT
end


function r2:loadRaceSexNamesImpl(race, sex)
	local fileName = fileLookup(race.."_" .. sex .. ".txt")
	if fileName == "" then return end
	local resultT = {}
	local count = 1
	for line in io.lines(fileName) do 
		resultT[count] = line
		count = count + 1
	end
	return resultT
end

function r2:loadRaceSexNames(race, sex)
	local resultT = r2:loadRaceSexNamesImpl(race, sex)
	r2.NPCNames[race .. "_" .. sex] = resultT
end

function r2:loadAllRaceSexNames(race, sex)
	local resultT = r2:loadRaceSexNamesImpl(race, sex)
	r2.NPCAllNames[race .. "_" .. sex] = resultT
end

function r2:getAllNames()
	if (table.getn(r2.NPCAllNames) == 0) then
		r2:loadAllRaceSexNames(r2.fyros, r2.male)
		r2:loadAllRaceSexNames(r2.fyros, r2.female)
		r2:loadAllRaceSexNames(r2.matis, r2.male)
		r2:loadAllRaceSexNames(r2.matis, r2.female)
		r2:loadAllRaceSexNames(r2.tryker, r2.male)
		r2:loadAllRaceSexNames(r2.tryker, r2.female)
		r2:loadAllRaceSexNames(r2.zorai, r2.male)
		r2:loadAllRaceSexNames(r2.zorai, r2.female)	
	end

	return r2.NPCAllNames
end


-- UPDATE RACE VIEW ---------------------------------------------------------------------------

function r2:updateRaceView()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local scene3D = npcUI:find("char3d")
	assert(scene3D)

	-- camera target is out of 3D character
	local camera = scene3D.cam
	assert(camera)
	if r2.raceSelection == r2.zorai then
		camera.tgtz = r2.camTgtZZorai
		camera.posy = r2.camPosYZorai
		r2.distlimitmax = -r2.camPosYZorai
		scene3D.distlimitmax = r2.distlimitmax
	else
		camera.tgtz = r2.camTgtZ
		camera.posy = r2.camPosY
		r2.distlimitmax = -r2.camPosY
	end	
end

-- CHANGE VIEW --------------------------------------------------------------------------------

function r2:changeView()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local scene3D = npcUI:find("char3d")
	assert(scene3D)

	local camera = scene3D.cam
	assert(camera)
	
	if r2.faceView then
		scene3D.distlimitmin = r2.distlimitmin
		scene3D.distlimitmax = r2.distlimitmax
		if r2.raceSelection == r2.zorai then
			camera.tgtz = r2.camTgtZZorai
			camera.posy = r2.camPosYZorai
		else
			camera.tgtz = r2.camTgtZ
			camera.posy = r2.camPosY
		end	
		r2.faceView = false
	else
		scene3D.distlimitmin = r2.distlimitminZoom
		scene3D.distlimitmax = -r2.camZoomPosY
		local char3D = scene3D.char
		assert(char3D)
		local headZ = char3D.headz
		camera.tgtz = headZ + 0.07
		camera.posy = r2.camZoomPosY
		r2.faceView = true
	end
end


-----------------------------------------------------------------------------------------------
-- PROFILE ------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------

-- UPDATE NAME --------------------------------------------------------------------------------
function r2:updateName()

	local selection = r2:getSelectedInstance()
	if selection == nil then return end

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local editName  = npcUI:find("name").edit_box_group
	assert(editName)

	local newString = editName.uc_input_string:toUtf8()
	local oldString = defaulting(selection.Name, "")
	if newString ~= oldString then
		r2.requestNewAction(i18n.get("uiR2EDUpdateNpcNameAction"))
		r2:setNpcAttribute(selection.InstanceId, "Name", newString)
	end

	return editName.uc_input_string:toUtf8()
end

-- UPDATE NOTES --------------------------------------------------------------------------------
function r2:updateNotes()

	local selection = r2:getSelectedInstance()
	if selection == nil then return end

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local editNotes = npcUI:find("small_description")
	assert(editNotes)
	
	local newString = editNotes.uc_input_string:toUtf8()
	local oldString = defaulting(selection.Notes, "")
	if newString ~= oldString then
		r2.requestNewAction(i18n.get("uiR2EDUpdateNpcNotesAction"))
		r2:setNpcAttribute(selection.InstanceId, "Notes", newString)
	end
end

-- UPDATE RACE --------------------------------------------------------------------------------
function r2:updateRace(race)

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	local name = selection.Name
	if not race and not getUICaller().Env.locked then
		r2:updateNotes()
		name = r2:updateName()
	end

	r2.requestNewAction(i18n.get("uiR2EDUpdateNpcRaceAction"))

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local sliderValue 
	if r2.hasHelmet then
		sliderValue = r2.hairType.hairCut
	else 
		sliderValue = r2.itemIndexToSliderValue[r2.raceSelection][selection.HairType]
	end

	local oldRaceSelection = r2.raceSelection
	if race==nil then
		race = getUICaller().selection_text
	end
	r2.raceSelection = race
	
	-- requestSetNode
	local sheetClientName = r2.raceSheetClient[r2.sexSelection][r2.raceSelection]
	if not getUICaller().Env.locked then

		r2:setNpcAttribute(selection.InstanceId, "SheetClient", sheetClientName)

		-- hairType item index of new race
		if not r2.hasHelmet then
			local itemIndex = getSheetId(r2.sliderValueToItemIndex[r2.raceSelection][sliderValue])
			r2:setNpcAttribute(selection.InstanceId, "HairType", itemIndex)
		end

		if r2:isRandomizedNPCName(name, oldRaceSelection, r2.sexSelection) then
			local name = r2:randomNPCName(r2.raceSelection, r2.sexSelection)
			r2:setNpcAttribute(selection.InstanceId, "Name", name)
		end
	end

	r2.requestEndAction()
end

function r2:updateRaceUI()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	-- no eyes color and morph8 for Zorai
	local eyesColor = npcUI:find("slider_eye_color")
	assert(eyesColor)
	local sliderMorph8 = npcUI:find("slider_morph_target8")
	assert(sliderMorph8)
	if r2.raceSelection == r2.zorai then
		eyesColor.active = false
		sliderMorph8.active = false
	else
		eyesColor.active = true
		sliderMorph8.active = true
	end

	-- update morph sliders labels
	for key, value in pairs(r2.morphSliders) do
		local sliderMorph = npcUI:find(value)
		assert(sliderMorph)
		sliderMorph:find("slider_text_place"):find("slider_text").hardtext = r2.raceMorphEditText[r2.raceSelection][key]	
	end
end

-- UPDATE SEX ---------------------------------------------------------------------------------
function r2:updateSex()

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	local name = selection.Name
	if not getUICaller().Env.locked then
		r2:updateNotes()
		name = r2:updateName()
	end

	r2.requestNewAction(i18n.get("uiR2EDUpdateNpcSexAction"))

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local oldSexSelection = r2.sexSelection
	r2.sexSelection = getUICaller().selection_text

	-- update sex
	local breastGroup = npcUI:find("slider_breast")
	assert(breastGroup)
	if r2.sexSelection == r2.male then
		breastGroup.active = false
	else
		breastGroup.active = true
	end

	if not getUICaller().Env.locked then
		-- requestSetNode
		local sheetClientName = r2.raceSheetClient[r2.sexSelection][r2.raceSelection]
		r2:setNpcAttribute(selection.InstanceId, "SheetClient", sheetClientName)

		if r2:isRandomizedNPCName(name, r2.raceSelection, oldSexSelection) then
			local name = r2:randomNPCName(r2.raceSelection, r2.sexSelection)
			r2:setNpcAttribute(selection.InstanceId, "Name", name)
		end
	end

	r2.requestEndAction()
end

-- UPDATE SEX ---------------------------------------------------------------------------------
function r2:updateType()
	
	local selection = r2:getSelectedInstance()
	if selection == nil then return end

	local oldTypeNPC = selection.TypeNPC

	local typeNPC = getUICaller().selection
	local base = r2.sheetTypeCB[typeNPC].Id
	local typeElement = r2.getPaletteElement(base)

	r2.requestNewAction(i18n.get("uiR2EDUpdateNpcTypeAction"))

	-- TypeNPC
	r2:setNpcAttribute(selection.InstanceId, "TypeNPC", typeNPC)

	-- RingAccess
	r2:setNpcAttribute(selection.InstanceId, "RingAccess", typeElement.RingAccess)

	if selection:isKindOf("NpcCustom") then
		-- SheetModel
		r2:setNpcAttribute(selection.InstanceId, "SheetModel", typeElement.SheetModel)

		-- Sheet
		local level = r2:getLevel(selection)+1
		local sheet = typeElement.Sheet
		
		local s,e = string.find(sheet, ".creature")
		local firstPart = string.sub(sheet, 1, s-2)
		local newSheet = firstPart .. level .. ".creature"

		r2:setNpcAttribute(selection.InstanceId, "Sheet", newSheet)

		-- Base
		r2:setNpcAttribute(selection.InstanceId, "Base", base)

		-- Type
		r2:setNpcAttribute(selection.InstanceId, "Type", typeElement.Type)

		-- Equipment	
		local tableEquipment = r2.equipmentPalette[selection.Equipment]
		local newTableEquipment = r2.equipmentPalette[typeElement.Equipment]

		local chestNb = r2:getPieceEquipmentNumber(tableEquipment.chest_plate, selection.JacketModel)
		local jacketModel = r2:getPieceEquipmentFromNumber(newTableEquipment.chest_plate, chestNb)

		local legsNb = r2:getPieceEquipmentNumber(tableEquipment.legs, selection.TrouserModel)
		local trouserModel = r2:getPieceEquipmentFromNumber(newTableEquipment.legs, legsNb)

		local bootsNb = r2:getPieceEquipmentNumber(tableEquipment.boots, selection.FeetModel)
		local feetModel = r2:getPieceEquipmentFromNumber(newTableEquipment.boots, bootsNb)

		local glovesNb = r2:getPieceEquipmentNumber(tableEquipment.gloves, selection.HandsModel)
		local handsModel = r2:getPieceEquipmentFromNumber(newTableEquipment.gloves, glovesNb)

		local armsNb = r2:getPieceEquipmentNumber(tableEquipment.arms_guard, selection.ArmModel)
		local armModel = r2:getPieceEquipmentFromNumber(newTableEquipment.arms_guard, armsNb)

		local weaponRHNb = r2:getPieceEquipmentNumber(tableEquipment.hands, selection.WeaponRightHand)
		local weaponRightHand = r2:getPieceEquipmentFromNumber(newTableEquipment.hands, weaponRHNb, "right")

		local weaponLHNb = r2:getPieceEquipmentNumber(tableEquipment.hands, selection.WeaponLeftHand)
		local weaponLeftHand = r2:getPieceEquipmentFromNumber(newTableEquipment.hands, weaponLHNb, "left")

		local helmetNb = r2:getPieceEquipmentNumber(tableEquipment.helmet, selection.HairType)
		
		r2:setNpcAttribute(selection.InstanceId, "Equipment", typeElement.Equipment)
		r2:setNpcAttribute(selection.InstanceId, "JacketModel", jacketModel)
		r2:setNpcAttribute(selection.InstanceId, "TrouserModel", trouserModel)
		r2:setNpcAttribute(selection.InstanceId, "FeetModel", feetModel)
		r2:setNpcAttribute(selection.InstanceId, "HandsModel", handsModel)
		r2:setNpcAttribute(selection.InstanceId, "ArmModel", armModel)
		r2:setNpcAttribute(selection.InstanceId, "WeaponRightHand", weaponRightHand)
		r2:setNpcAttribute(selection.InstanceId, "WeaponLeftHand", weaponLeftHand)

		if helmetNb>=0 then
			local helmetType = r2:getPieceEquipmentFromNumber(newTableEquipment.helmet, helmetNb)
			if helmetType>0 then
				r2:setNpcAttribute(selection.InstanceId, "HairType", helmetType)
			elseif helmetType==0 then
				local itemIndex = getSheetId(r2.sliderValueToItemIndex[r2.raceSelection][0]) 
				r2:setNpcAttribute(selection.InstanceId, "HairType", itemIndex)
				--r2:setNpcAttribute(selection.InstanceId, "HairColor", r2.hairType.hairColor)
			end
		end

	else
		-- Base
		r2:setNpcAttribute(selection.InstanceId, "Base", base)

		-- SheetClient
		r2:setNpcAttribute(selection.InstanceId, "SheetClient", typeElement.SheetClient)

		-- Name
		local oldName = i18n.get(r2.sheetTypeCB[oldTypeNPC].Translation):toUtf8()
		if selection.Class == "Npc" and string.find(selection.Name, oldName) then
			local ucNewName =  getUICaller():getText(typeNPC)
			local newName = r2:genInstanceName(ucNewName):toUtf8()
			r2:setNpcAttribute(selection.InstanceId, "Name", newName)

		elseif selection.Name == oldName then
			r2:setNpcAttribute(selection.InstanceId, "Name", getUICaller().selection_text)
		end
	end

	r2.requestEndAction()
end

-- UPDATE FUCNTION ----------------------------------------------------------------------------
function r2:updateFunction()

	if getUICaller().Env.locked then
		return
	end

	local selection = r2:getSelectedInstance()
	assert(selection)

	r2.requestNewAction(i18n.get("uiR2EDUpdateNpcFunctionAction"))
	r2:setNpcAttribute(selection.InstanceId, "Function", getUICaller().selection_text)
end

-- UPDATE LEVEL ------------------------------------------------------------------------------
r2.updateLevel = function(value)

	local selection = r2:getSelectedInstance()
	assert(selection)

	local level = tonumber(value)+1
	local sheet = selection.Sheet
	
	local s,e = string.find(sheet, ".creature")
	local firstPart = string.sub(sheet, 1, s-2)
	local newSheet = firstPart .. level .. ".creature"

	r2.requestNewAction(i18n.get("uiR2EDUpdateNpcLevelAction"))
	r2:setNpcAttribute(selection.InstanceId, "Sheet", newSheet)
end

function r2:getHandsLevel(sheetModel, sheet)

	local s,e = string.find(sheetModel, "$hands")
	local base = string.sub(sheetModel, 1, s-1)
	local endSheetModel = string.sub(sheetModel, e+1)
	s,e = string.find(endSheetModel, "$level")
	local titi = string.sub(endSheetModel, 1, s-1)

	s,e = string.find(sheet, base)
	local startHands = string.sub(sheet, e+1)
	s,e = string.find(startHands, titi)

	return  string.sub(startHands, 1, s-1)
end

function r2:getLevel(instance)

	local sheet = instance.Sheet
	local s,e = string.find(sheet, ".creature")

	return tonumber(string.sub(sheet, s-1, s-1))-1
end

-----------------------------------------------------------------------------------------------
-- BODY ---------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------

-- UPDATE FACE VIEW ---------------------------------------------------------------------------

function r2:updateFaceView()
	
	if r2.faceView then
		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)
		local camera = npcUI:find("char3d").cam 
		assert(camera)
		local char3D = npcUI:find("char3d").char
		assert(char3D)
		local headZ = char3D.headz
		camera.tgtz = headZ + 0.07
		camera.posy = r2.camZoomPosY
	end
end


-- UPDATE BODY ATTRIBUTE ----------------------------------------------------------------------
function r2:changeBodyAttribute(attributeName, requestType)

	local value = getUICaller().value 
	local sliderValue = nil
	if attributeName == "HairType" then
		r2.hairType.hairCut = value
		sliderValue = getSheetId(r2.sliderValueToItemIndex[r2.raceSelection][value])
	elseif attributeName == "HairColor" then
		r2.hairType.hairColor = value
	end

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	r2.requestNewAction(i18n.get("uiR2EDChangeNpcBodyAttributeAction"))

	if sliderValue == nil then
		sliderValue = value
	end

	if requestType == 'local' then
		r2.requestSetLocalNode(selection.InstanceId, attributeName, sliderValue)
	elseif requestType == 'commit' then
		r2.requestCommitLocalNode(selection.InstanceId, attributeName)
	elseif requestType == 'cancel' then
		r2.requestRollbackLocalNode(selection.InstanceId, attributeName)
	end

	r2.requestEndAction()
end

-- UPDATE BODY ATTRIBUTE FROM ON ATTRIBUTE MODIFIED -----------------------------------------------
function r2:updateBodyAttribute(attributeName, value)

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	if attributeName == "HairType" then
		value = r2.itemIndexToSliderValue[r2.raceSelection][value]
	end

	local slider = npcUI:find(body_sliders[attributeName]):find("slider")
	assert(slider)
	slider.value = value
	
	if attributeName == "GabaritHeight" then r2:updateFaceView() end

	if not r2:matchWithBodyAttributesSet(attributeName, value) then
		local comboBox = npcUI:find(r2.bodyAttributesCB[attributeName].comboBox).combo_box
		assert(comboBox)
		comboBox.view_text = r2.emptyComboLine

		local updateComboBox = "ui:interface:"..r2.bodyAttributesCB[attributeName].updateComboBox
		local updateCB = getUI(updateComboBox):find("update_sets").combo_box
		assert(updateCB)
		updateCB.view_text = r2.emptyComboLine
	end
end

function r2:matchWithBodyAttributesSet(attributeName, value)

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local comboBox = npcUI:find(r2.bodyAttributesCB[attributeName].comboBox).combo_box
	assert(comboBox)

	local tableSets = r2[r2.bodyAttributesCB[attributeName].tableSets]
	if tableSets == nil then return false end
	
	local setValues = tableSets[tostring(comboBox.selection_text)] 
	if setValues == nil then return false end

	return (setValues[attributeName] == value)
end

-- RANDOM BODY  ------------------------------------------------------------------------------
function r2:randomBody()

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	r2.requestNewAction(i18n.get("uiR2EDRandomNpcBodyAction"))

	local randomValue
	for i = 1,r2.bodyAttNb do
		randomValue = math.random(0,14)
		r2:setNpcAttribute(selection.InstanceId, r2.bodyAttributes[i], randomValue)
	end

	r2.requestEndAction()
end


-- RANDOM BODY SETS ---------------------------------------------------------------------------
function r2:randomSets(sets)

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local setsCombo = getUI("ui:interface:r2ed_npc"):find(sets).combo_box
	assert(setsCombo)

	if setsCombo.selectionNb > 0 then
		local randomSelection = math.random(1, setsCombo.selectionNb)
		if setsCombo.selectionNb > 1 then
			while randomSelection == (setsCombo.selection+1) do
				randomSelection = math.random(1, setsCombo.selectionNb)
			end
		end
		setsCombo.selection = randomSelection-1
	end
end

-- ADD BODY SETS -----------------------------------------------------------------------------



function r2:addBodySets()
	
	local addBodySets = getUI("ui:interface:r2ed_add_body_sets")
	assert(addBodySets)
	local editText = addBodySets:find("new_sets").edit_box_group
	assert(editText)
	if editText.input_string~="" then
		local updateBodySetsCombo = addBodySets:find("update_sets").combo_box
		assert(updateBodySetsCombo)

		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)
		local bodySetsCombo = npcUI:find("body_sets").combo_box
		assert(bodySetsCombo)

		bodySetsCombo:addText(editText.uc_input_string)
		updateBodySetsCombo:addText(editText.uc_input_string)

		-- add new sets in table
		local bodySets = npcUI:find("body_sets_scroll_target")
		assert(bodySets)
		r2.tableBodySets[editText.input_string] = {}

		local sliderValue
		for i = 1,r2.bodyAttNb do
			sliderValue = bodySets:find(r2.bodySliders[i]):find("slider").value 
			r2.tableBodySets[editText.input_string][r2.bodyKeys[i]] = sliderValue
		end
		r2:saveTable("save\\tableBodySets.txt", r2.tableBodySets)

		-- update set of main window combo box
		bodySetsCombo.selection_text = editText.input_string
		updateBodySetsCombo.selection_text = editText.input_string

		editText.input_string = ""
	end
end

-- UPDATE BODY SETS --------------------------------------------------------------------------
function r2:updateBodySets()

	local updateBodySetsCombo = getUI("ui:interface:r2ed_add_body_sets"):find("update_sets").combo_box
	assert(updateBodySetsCombo)
	local bodySetsName = updateBodySetsCombo.selection_text

	-- update sets in table
	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local bodySets = npcUI:find("body_sets_scroll_target")
	assert(bodySets)

	local sliderValue
	for i = 1,r2.bodyAttNb do
		sliderValue = bodySets:find(r2.bodySliders[i]):find("slider").value 
		r2.tableBodySets[bodySetsName][r2.bodyKeys[i]] = sliderValue
	end
	r2:saveTable("save\\tableBodySets.txt", r2.tableBodySets)

	-- update set of main window combo box
	local bodySetsCombo = npcUI:find("body_sets").combo_box
	assert(bodySetsCombo)
	bodySetsCombo.selection_text = bodySetsName
end

-- REMOVE BODY SETS ---------------------------------------------------------------------------
function r2:removeBodySets()

	local updateBodySetsCombo = getUI("ui:interface:r2ed_add_body_sets"):find("update_sets").combo_box
	assert(updateBodySetsCombo)
	local bodySetsName = updateBodySetsCombo.selection_text

	-- delete sets in table
	r2.tableBodySets[bodySetsName] = nil
	r2:saveTable("save\\tableBodySets.txt", r2.tableBodySets)

	-- delete sets in combo box
	updateBodySetsCombo:removeSelection(updateBodySetsCombo.selection)
	updateBodySetsCombo.view_text = r2.emptyComboLine
	
	local bodySetsCombo = getUI("ui:interface:r2ed_npc"):find("body_sets").combo_box
	assert(bodySetsCombo)
	bodySetsCombo:removeText(bodySetsName)
	bodySetsCombo.view_text = r2.emptyComboLine
end

-- SELECT BODY SETS ---------------------------------------------------------------------------
function r2:selectBodySets()

	local bodySetsCombo = getUI("ui:interface:r2ed_npc"):find("body_sets").combo_box
	assert(bodySetsCombo)

	local selection = r2:getSelectedInstance()

	if bodySetsCombo.view_text == r2.emptyComboLine then 
		return
	else
		r2.requestNewAction(i18n.get("uiR2EDSelectNpcBodySetsAction"))

		local bodySetsValue = r2.tableBodySets[bodySetsCombo.selection_text] 
		if bodySetsValue == nil then return end

		-- update sliders values
		for i = 1,r2.bodyAttNb do
			r2:setNpcAttribute(selection.InstanceId, r2.bodyAttributes[i], bodySetsValue[r2.bodyKeys[i]])
		end

		r2.requestEndAction()
	end

	local bodySetsPopup = getUI("ui:interface:r2ed_add_body_sets"):find("update_sets").combo_box
	assert(bodySetsPopup)
	bodySetsPopup.selection_text = bodySetsCombo.selection_text
end


----------------------------------------------------------------------------------------------
-- RANDOM FACE  ------------------------------------------------------------------------------
function r2:randomFace()

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	r2.requestNewAction(i18n.get("uiR2EDRandomNpcFaceAction"))

	local randomValue
	for i = 1,r2.faceAttNb do

		if r2.faceAttributes[i] == "HairColor" then
			randomValue = math.random(0,5)
		elseif r2.faceAttributes[i] == "Tattoo" then
			randomValue = math.random(0,31)
		elseif r2.faceAttributes[i] == "HairType" then
			randomValue = math.random(0,6)
		else
			randomValue = math.random(0,7)
		end

		if r2.faceAttributes[i] == "HairType" then
			if not r2.hasHelmet then
				local itemIndex = getSheetId(r2.sliderValueToItemIndex[r2.raceSelection][randomValue])
				r2:setNpcAttribute(selection.InstanceId, r2.faceAttributes[i], itemIndex)
			end
		elseif not (r2.faceAttributes[i] == "HairColor" and r2.hasHelmet) then
			r2:setNpcAttribute(selection.InstanceId, r2.faceAttributes[i], randomValue)
		end
	end

	r2.requestEndAction()
end

-- ADD FACE SETS -----------------------------------------------------------------------------

function r2:addFaceSets()

	local addFaceSets = getUI("ui:interface:r2ed_add_face_sets")
	assert(addFaceSets)
	local editText = addFaceSets:find("new_sets").edit_box_group
	assert(editText)
	if editText.input_string~="" then
		local updateFaceSetsCombo = addFaceSets:find("update_sets").combo_box
		assert(updateFaceSetsCombo)

		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)
		local faceSetsCombo = npcUI:find("face_sets").combo_box
		assert(faceSetsCombo)

		faceSetsCombo:addText(editText.uc_input_string)
		updateFaceSetsCombo:addText(editText.uc_input_string)

		-- add new sets in table
		local faceSets = npcUI:find("face_sets_scroll_target")
		assert(faceSets)
		r2.tableFaceSets[editText.input_string] = {}

		local sliderValue
		for i = 1,r2.faceAttNb do
			sliderValue = faceSets:find(r2.faceSliders[i]):find("slider").value 
			r2.tableFaceSets[editText.input_string][r2.faceKeys[i] ] = sliderValue
		end
		r2:saveTable("save\\tableFaceSets.txt", r2.tableFaceSets)

		-- update set of main window combo box
		faceSetsCombo.selection_text = editText.input_string
		updateFaceSetsCombo.selection_text = editText.input_string

		editText.input_string = ""
	end
end

-- UPDATE FACE SETS --------------------------------------------------------------------------
function r2:updateFaceSets()

	local updateFaceSetsCombo = getUI("ui:interface:r2ed_add_face_sets"):find("update_sets").combo_box
	assert(updateFaceSetsCombo)
	local faceSetsName = updateFaceSetsCombo.selection_text

	-- update sets in table
	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local faceSets = npcUI:find("face_sets_scroll_target")
	assert(faceSets)

	local sliderValue
	for i = 1,r2.faceAttNb do
		sliderValue = faceSets:find(r2.faceSliders[i]):find("slider").value 
		r2.tableFaceSets[faceSetsName][r2.faceKeys[i] ] = sliderValue
	end
	r2:saveTable("save\\tableFaceSets.txt", r2.tableFaceSets)

	-- update set of main window combo box
	local faceSetsCombo = npcUI:find("face_sets").combo_box
	assert(faceSetsCombo)
	faceSetsCombo.selection_text = faceSetsName
end

-- REMOVE FACE SETS ---------------------------------------------------------------------------
function r2:removeFaceSets()

	local updateFaceSetsCombo = getUI("ui:interface:r2ed_add_face_sets"):find("update_sets").combo_box
	assert(updateFaceSetsCombo)
	local faceSetsName = updateFaceSetsCombo.selection_text

	-- delete sets in table
	r2.tableFaceSets[faceSetsName] = nil
	r2:saveTable("save\\tableFaceSets.txt", r2.tableFaceSets)

	-- delete sets in combo box
	updateFaceSetsCombo:removeSelection(updateFaceSetsCombo.selection)
	
	local faceSetsCombo = getUI("ui:interface:r2ed_npc"):find("face_sets").combo_box
	assert(faceSetsCombo)
	faceSetsCombo:removeText(faceSetsName)
	faceSetsCombo.view_text = r2.emptyComboLine
end

-- SELECT FACE SETS ---------------------------------------------------------------------------
function r2:selectFaceSets()

	local faceSetsCombo = getUI("ui:interface:r2ed_npc"):find("face_sets").combo_box
	assert(faceSetsCombo)

	local selection = r2:getSelectedInstance()	

	if faceSetsCombo.view_text == r2.emptyComboLine then 
		return
	else
		r2.requestNewAction(i18n.get("uiR2EDSelectNpcFaceSetsAction"))

		local faceSetsValue = r2.tableFaceSets[faceSetsCombo.selection_text] 
		if faceSetsValue == nil then return end

		-- update sliders values
		for i = 1,r2.faceAttNb do
			local requestSetNode = true
			local requestValue = faceSetsValue[r2.faceKeys[i] ]

			if r2.faceAttributes[i] == "HairType" then
				if not r2.hasHelmet then
					requestValue = getSheetId(r2.sliderValueToItemIndex[r2.raceSelection][faceSetsValue[r2.faceKeys[i]]])
				else
					r2.hairType.hairCut = faceSetsValue[r2.faceKeys[i] ]
					requestSetNode = false
				end
			elseif r2.faceAttributes[i] == "HairColor" and r2.hasHelmet then
				r2.hairType.hairColor = faceSetsValue[r2.faceKeys[i] ]
				requestSetNode = false
			end

			if requestSetNode then
				r2:setNpcAttribute(selection.InstanceId, r2.faceAttributes[i], requestValue)
			end
		end

		r2.requestEndAction()
	end

	local faceSetsPopup = getUI("ui:interface:r2ed_add_face_sets"):find("update_sets").combo_box
	assert(faceSetsPopup)
	faceSetsPopup.selection_text = faceSetsCombo.selection_text
end

----------------------------------------------------------------------------------------------
-- RANDOM FACE MORPH -------------------------------------------------------------------------
function r2:randomFaceMorph()
	
	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	r2.requestNewAction(i18n.get("uiR2EDRandomNPCFaceMorphAction"))

	local randomValue
	for i = 1,r2.morphAttNb do

		randomValue = math.random(0,7)
		r2:setNpcAttribute(selection.InstanceId, r2.morphAttributes[i], randomValue)
	end

	r2.requestEndAction()
end

-- ADD FACE MORPH ---------------------------------------------------------------------------

function r2:addFaceMorph()

	local addFaceMorph = getUI("ui:interface:r2ed_add_face_morph")
	assert(addFaceMorph)
	local editText = addFaceMorph:find("new_sets").edit_box_group
	assert(editText)
	if editText.input_string~="" then
		local updateFaceMorphCombo = addFaceMorph:find("update_sets").combo_box
		assert(updateFaceMorphCombo)

		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)
		local faceMorphCombo = npcUI:find("face_morph").combo_box
		assert(faceMorphCombo)

		faceMorphCombo:addText(editText.uc_input_string)
		updateFaceMorphCombo:addText(editText.uc_input_string)

		-- add new sets in table
		local faceMorph = npcUI:find("face_morph_scroll_target")
		assert(faceMorph)
		r2.tableFaceMorph[editText.input_string] = {}

		local sliderValue
		for i = 1,r2.morphAttNb do
			sliderValue = faceMorph:find(r2.morphSliders[i]):find("slider").value 
			r2.tableFaceMorph[editText.input_string][r2.morphKeys[i] ] = sliderValue
		end
		r2:saveTable("save\\tableFaceMorph.txt", r2.tableFaceMorph)

		-- update set of main window combo box
		faceMorphCombo.selection_text = editText.input_string
		updateFaceMorphCombo.selection_text = editText.input_string
				
		editText.input_string = ""
	end
end

-- UPDATE FACE MORPH ------------------------------------------------------------------------
function r2:updateFaceMorph()

	local updateFaceMorphCombo = getUI("ui:interface:r2ed_add_face_morph"):find("update_sets").combo_box
	assert(updateFaceMorphCombo)
	local faceMorphName = updateFaceMorphCombo.selection_text

	-- update sets in table
	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local faceMorph = npcUI:find("face_morph_scroll_target")
	assert(faceMorph)

	local sliderValue
	for i = 1,r2.morphAttNb do
		sliderValue = faceMorph:find(r2.morphSliders[i]):find("slider").value 
		r2.tableFaceMorph[faceMorphName][r2.morphKeys[i] ] = sliderValue
	end
	r2:saveTable("save\\tableFaceMorph.txt", r2.tableFaceMorph)

	-- update set of main window combo box
	local faceMorphCombo = npcUI:find("face_morph").combo_box
	assert(faceMorphCombo)
	faceMorphCombo.selection_text = faceMorphName
end

-- REMOVE FACE MORPH ---------------------------------------------------------------------------
function r2:removeFaceMorph()

	local updateFaceMorphCombo = getUI("ui:interface:r2ed_add_face_morph"):find("update_sets").combo_box
	assert(updateFaceMorphCombo)
	local faceMorphName = updateFaceMorphCombo.selection_text

	-- delete sets in table
	r2.tableFaceMorph[faceMorphName] = nil
	r2:saveTable("save\\tableFaceMorph.txt", r2.tableFaceMorph)

	-- delete sets in combo box
	updateFaceMorphCombo:removeSelection(updateFaceMorphCombo.selection)
	updateFaceMorphCombo.view_text = r2.emptyComboLine
	
	local faceMorphCombo = getUI("ui:interface:r2ed_npc"):find("face_morph").combo_box
	assert(faceMorphCombo)
	faceMorphCombo:removeText(faceMorphName)
	faceMorphCombo.view_text = r2.emptyComboLine
end

-- SELECT FACE MORPH ---------------------------------------------------------------------------
function r2:selectMorphSets()

	local faceMorphCombo = getUI("ui:interface:r2ed_npc"):find("face_morph").combo_box
	assert(faceMorphCombo)

	local selection = r2:getSelectedInstance()	

	if faceMorphCombo.view_text == r2.emptyComboLine then 
		return
	else
		r2.requestNewAction(i18n.get("uiR2EDSelectNPCMorphSetsAction"))

		local faceMorphValue = r2.tableFaceMorph[faceMorphCombo.selection_text] 
		if faceMorphValue == nil then return end

		-- update sliders values
		for i = 1,r2.morphAttNb do
			r2:setNpcAttribute(selection.InstanceId, r2.morphAttributes[i], faceMorphValue[r2.morphKeys[i]])
		end

		r2.requestEndAction()
	end

	local faceMorphPopup = getUI("ui:interface:r2ed_add_face_morph"):find("update_sets").combo_box
	assert(faceMorphPopup)
	faceMorphPopup.selection_text = faceMorphCombo.selection_text
end


-- CLOSE MODAL SETS WINDOW ----------------------------------------------------------------------

function r2:closeModalBodySets()
	r2:closeModal("ui:interface:r2ed_add_body_sets")
end

function r2:closeModalFaceSets()
	r2:closeModal("ui:interface:r2ed_add_face_sets")
end

function r2:closeModalFaceMorph()
	r2:closeModal("ui:interface:r2ed_add_face_morph")
end

function r2:closeModal(window)

	local modalWindow = getUI(window)
	assert(modalWindow)
	
	local editText = modalWindow:find("new_sets").edit_box_group
	assert(editText)
	editText.input_string = ""

	modalWindow.active = false
end


-----------------------------------------------------------------------------------------------
-- GROUP --------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------

r2.profile = {Civil="normal", Bandit="bandit", Guard = "guard"}
r2.profileCB = {normal="Civil", bandit="Bandit", guard="Guard"}

-- UPDATE PROFILE -----------------------------------------------------------------------------
function r2:updateProfile()

	if getUICaller().Env.locked then
		return
	end

	local selection = r2:getSelectedInstance()
	assert(selection)

	r2.requestNewAction(i18n.get("uiR2EDUpdateNPCProfileAction"))
	r2:setNpcAttribute(selection.InstanceId, "Profile", r2.profile[getUICaller().selection_text])
end

-----------------------------------------------------------------------------------------------
-- EQUIPMENT ----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------

r2.equipmentPalette = {}
r2.itemIndexEquipmentToSelectionText = {}
r2.equipmentEnv = {}
r2.noPiece = i18n.get("uiR2EdNoPiece"):toUtf8()

r2.equipmentAttNb = 6

r2.colorAttributes  =	  {[1] = "JacketColor",		[2] = "TrouserColor", 
						   [3] = "FeetColor",		[4] = "HandsColor",
						   [5] = "ArmColor"}

r2.equipementComboB		= {[1] = "chest_plate",		[2] = "legs", 
						   [3] = "boots",			[4] = "gloves",
					       [5] = "arms_guard",		[6] = "helmet",
						  }

r2.equipmentKeys		= r2.equipementComboB

r2.equipmentSets = {}

r2.linkColorB = false
r2.linkedColor = 0

function r2:initEquipmentEnv()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	
	local nameComboBox = "helmet"
	local comboBox = npcUI:find(nameComboBox).label_combo_box.combo_box
	local slider = npcUI:find(nameComboBox):find("slider")
	comboBox.Env.nameComboBox = nameComboBox
	slider.name = nameComboBox
	r2.equipmentEnv[nameComboBox] = {propName = "HairType", propColorName = "HairColor"}

	nameComboBox = "chest_plate"
	comboBox = npcUI:find(nameComboBox).label_combo_box.combo_box
	slider = npcUI:find(nameComboBox):find("slider")
	comboBox.Env.nameComboBox = nameComboBox
	slider.name = nameComboBox
	r2.equipmentEnv[nameComboBox] = {propName = "JacketModel", propColorName = "JacketColor"}

	nameComboBox = "legs"
	comboBox = npcUI:find(nameComboBox).label_combo_box.combo_box
	slider = npcUI:find(nameComboBox):find("slider")
	comboBox.Env.nameComboBox = nameComboBox
	slider.name = nameComboBox
	r2.equipmentEnv[nameComboBox] = {propName = "TrouserModel", propColorName = "TrouserColor"}

	nameComboBox = "boots"
	comboBox = npcUI:find(nameComboBox).label_combo_box.combo_box
	slider = npcUI:find(nameComboBox):find("slider")
	comboBox.Env.nameComboBox = nameComboBox
	slider.name = nameComboBox
	r2.equipmentEnv[nameComboBox] = {propName = "FeetModel", propColorName = "FeetColor"}

	nameComboBox = "gloves"
	comboBox = npcUI:find(nameComboBox).label_combo_box.combo_box
	slider = npcUI:find(nameComboBox):find("slider")
	comboBox.Env.nameComboBox = nameComboBox
	slider.name = nameComboBox
	r2.equipmentEnv[nameComboBox] = {propName = "HandsModel", propColorName = "HandsColor"}

	nameComboBox = "arms_guard"
	comboBox = npcUI:find(nameComboBox).label_combo_box.combo_box
	slider = npcUI:find(nameComboBox):find("slider")
	comboBox.Env.nameComboBox = nameComboBox
	slider.name= nameComboBox
	r2.equipmentEnv[nameComboBox] = {propName = "ArmModel", propColorName = "ArmColor"}

	nameComboBox = "hands"
	comboBox = npcUI:find(nameComboBox).combo_box
	comboBox.Env.nameComboBox = nameComboBox

	for equId, equTable in pairs(r2.equipmentPalette) do
		r2.itemIndexEquipmentToSelectionText[equId] = {}
		for comboName, comboList in pairs(equTable) do
			r2.itemIndexEquipmentToSelectionText[equId][comboName] = {}
			if tostring(comboName) ~= "hands" then
				for k, v in pairs(comboList) do
					local itemIndex = getSheetId(v.itemFile)
					r2.itemIndexEquipmentToSelectionText[equId][comboName][itemIndex] = v.trad
				end
			else
				for k, v in pairs(comboList) do
					local rightHandIndex = getSheetId(v.rightHand)
					local leftHandIndex = getSheetId(v.leftHand)
					local handsLevel = v.handsLevel
					local handsKey = rightHandIndex..":"..leftHandIndex..":"..handsLevel
					r2.itemIndexEquipmentToSelectionText[equId][comboName][handsKey] = v.trad

				end
			end
			r2.itemIndexEquipmentToSelectionText[equId][comboName][0] = r2.noPiece
		end
	end
end

-----------------------------------------------------------------------------------------------

function r2.addR2PlayerEquipment(paletteElt, equipmentTable)

	local tempTable = {}

	for equType, v in pairs(equipmentTable) do

		tempTable[equType] = {}
		for equName, equFile in pairs(v) do
			local tradEquName = i18n.get(equName):toUtf8()
			if tostring(equType) ~= "hands" then
				if equName == "uiR2EDequipment_none" then
					table.insert(tempTable[equType], 1, {["trad"]=tradEquName, ["itemFile"]=equFile})
				else
					table.insert(tempTable[equType], {["trad"]=tradEquName, ["itemFile"]=equFile})
				end
			else
				local s, e = string.find(equFile, ":")
				if s==nil or e==nil then
					debugInfo("Palette problem with equipment : "..paletteElt.Equipment)
				end
				local rightHand = string.sub(equFile, 1, e-1)

				equFile = string.sub(equFile, e+1)
				s, e = string.find(equFile, ":")
				if s==nil or e==nil then
					debugInfo("Palette probleme with equipment : "..paletteElt.Equipment)
				end
				local leftHand = string.sub(equFile, 1, e-1)

				equFile = string.sub(equFile, e+1)
				local handsLevel = equFile
				table.insert(tempTable[equType], {
													["trad"]=tradEquName, 
													["rightHand"]=rightHand, 
													["leftHand"]=leftHand, 
													["handsLevel"]=handsLevel
												 })
			end
		end
	end

	r2.equipmentPalette[paletteElt.Equipment] = tempTable
end

-----------------------------------------------------------------------------------------------

function r2:updateEquipment(instance, init)

	local equipmentId = instance.Equipment

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local comboBox 

	-- update current equipment 
	if r2.equipmentPalette[equipmentId] then

		for k, v in pairs(r2.equipmentPalette[equipmentId]) do
			comboBox = npcUI:find(k):find("combo_box")
			assert(comboBox)
			
			comboBox:resetTexts()
			
			for k1, v1 in pairs(v) do
				comboBox:addText(ucstring(v1.trad))
			end	
		end

		if init then r2.hasHelmet = false end
	end

	-- update equipment sets
	comboBox = npcUI:find("equipment_sets").combo_box
	assert(comboBox)

	local comboBoxPopup = getUI("ui:interface:r2ed_add_equipment_set"):find("update_sets").combo_box
	assert(comboBoxPopup)
			
	comboBox:resetTexts()
	comboBoxPopup:resetTexts()

	if r2.equipmentSets[equipmentId] then
		for k1, v1 in pairs(r2.equipmentSets[equipmentId]) do
			comboBox:addText(ucstring(k1))
			comboBoxPopup:addText(ucstring(k1))
		end
	end
	comboBox.view_text = r2.emptyComboLine
	comboBoxPopup.view_text = r2.emptyComboLine

	local toggleB = npcUI:find("color_link").toggle_butt
	assert(toggleB)
	toggleB.pushed = true
	r2:linkColor()

	-- update selection texts
	local comboBox, slider
	local helmetB = true
	for k, v in pairs(r2.equipmentEnv) do
		comboBox = npcUI:find(k):find("combo_box")
		assert(comboBox)
		slider = npcUI:find(k):find("slider")
		local line = npcUI:find(k):find("line_slider")

		local CBText = r2.itemIndexEquipmentToSelectionText[instance.Equipment][comboBox.Env.nameComboBox][instance[v.propName] ]
		if CBText==nil then CBText=r2.noPiece end
		comboBox.Env.locked = true
		comboBox.selection_text = CBText
		comboBox.Env.locked = false

		line.active = (CBText~=r2.noPiece)
		slider.active = (CBText~=r2.noPiece)
		if slider.active then
			slider.value = instance[v.propColorName]
		end

		if v.propName == "HairType" then
			slider = npcUI:find("slider_haircut")
			assert(slider)					
			slider.active = (CBText == r2.noPiece)
			if slider.active then
				local value = r2.itemIndexToSliderValue[r2.raceSelection][instance.HairType]
				if value then
					slider:find("slider").value = value
				end
			end

			slider = npcUI:find("slider_hair_color")
			assert(slider)					
			slider.active = (CBText == r2.noPiece)
			if slider.active then
				slider:find("slider").value = instance.HairColor
			end
		end
	end
end

-----------------------------------------------------------------------------------------------

function r2:updatePieceEquipment()

	if not r2.selectEquipmentSet then 
		r2.requestNewAction(i18n.get("uiR2EDUpdateEquipmentPieceAction"))
	end

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local equipmentType = getUICaller().selection_text
	local nameComboBox = getUICaller().Env.nameComboBox
	local itemIndex

	if equipmentType == r2.noPiece then
		if r2.equipmentEnv[nameComboBox].propName == "HairType" then
			local sliderHair = npcUI:find("slider_haircut")
			assert(sliderHair)
			local sliderHairColor = npcUI:find("slider_hair_color")
			assert(sliderHairColor)
			
			if not getUICaller().Env.locked and r2.hasHelmet then
				
				itemIndex = getSheetId(r2.sliderValueToItemIndex[r2.raceSelection][r2.hairType.hairCut])
				r2:setNpcAttribute(selection.InstanceId, r2.equipmentEnv[nameComboBox].propName, itemIndex)
				r2:setNpcAttribute(selection.InstanceId, r2.equipmentEnv[nameComboBox].propColorName, r2.hairType.hairColor)
			end
			
		else
			if not getUICaller().Env.locked then
				r2:setNpcAttribute(selection.InstanceId, r2.equipmentEnv[nameComboBox].propName, 0)
			end
		end
	else
		if r2.equipmentEnv[nameComboBox].propName == "HairType" then
			local sliderHair = npcUI:find("slider_haircut")
			assert(sliderHair)
			local sliderHairColor = npcUI:find("slider_hair_color")
			assert(sliderHairColor)
		end
		local itemFile = ""
		for k, v in pairs(r2.equipmentPalette[selection.Equipment][nameComboBox]) do
			if v.trad == equipmentType then itemFile = v.itemFile break end
		end

		itemIndex = getSheetId(itemFile)
		if not getUICaller().Env.locked then
			r2:setNpcAttribute(selection.InstanceId, r2.equipmentEnv[nameComboBox].propName, itemIndex)
		end

		if r2.linkColorB then
			r2:updateColor(npcUI:find(nameComboBox):find("slider"))
		end
	end

	if not r2.saveEquipmentSet and not r2:matchWithEquipmentSet(nameComboBox, equipmentType) then
		local updateEquSetCombo = getUI("ui:interface:r2ed_add_equipment_set"):find("update_sets").combo_box
		assert(updateEquSetCombo)
		updateEquSetCombo.view_text = r2.emptyComboLine

		local equSetCombo = npcUI:find("equipment_sets").combo_box
		assert(equSetCombo)
		equSetCombo.view_text = r2.emptyComboLine
	end
end

function r2:matchWithEquipmentSet(nameComboBox, equipmentType)

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local equSetCombo = npcUI:find("equipment_sets").combo_box
	assert(equSetCombo)

	local setName = equSetCombo.selection_text
	if setName==r2.emptyComboLine then return false end

	local selection = r2:getSelectedInstance()

	if r2.equipmentSets[selection.Equipment] == nil then return false end
	local equSetsValue = r2.equipmentSets[selection.Equipment][setName] 
	if equSetsValue == nil then return false end

	return (equSetsValue[nameComboBox].comboSelection == equipmentType)
end

function r2:updateWeapons()	

	local selection = r2:getSelectedInstance()
	local hands = getUICaller()
	if hands.Env.locked == true then return end

	r2.requestNewAction(i18n.get("uiR2EDUpdateNPCWeaponsAction"))

	local equipmentType = hands.selection_text

	local handsTable
	for k, v in pairs(r2.equipmentPalette[selection.Equipment][hands.Env.nameComboBox]) do
		if v.trad == equipmentType then handsTable = v break end
	end
	assert(handsTable)

	local rightHandFile = handsTable.rightHand
	local rightHandIndex = getSheetId(rightHandFile)
	local leftHandFile = handsTable.leftHand
	local leftHandIndex = getSheetId(leftHandFile)
	local handsLevel = handsTable.handsLevel

	local sheet = selection.Sheet
	local s, e = string.find(sheet, ".creature")
	local level = string.sub(sheet, s-1, s-1)

	local newSheet = selection.SheetModel
	
	newSheet = string.gsub(newSheet, "$hands", handsLevel)
	newSheet = string.gsub(newSheet, "$level", level)
	
	r2:setNpcAttribute(selection.InstanceId, "WeaponRightHand", rightHandIndex)
	r2:setNpcAttribute(selection.InstanceId, "WeaponLeftHand", leftHandIndex)
	r2:setNpcAttribute(selection.InstanceId, "Sheet", newSheet)

	r2.requestEndAction()
end

-----------------------------------------------------------------------------------------------

function r2:closeModalEquipment()
	r2:closeModal("ui:interface:r2ed_add_equipment_set")
end

-----------------------------------------------------------------------------------------------

function r2:addEquipmentSet()
	
	local selection = r2:getSelectedInstance()
	assert(selection)

	local addEquSet = getUI("ui:interface:r2ed_add_equipment_set")
	assert(addEquSet)
	local editText = addEquSet:find("new_sets").edit_box_group
	assert(editText)
	if editText.input_string~="" then
		local updateEquSetCombo = addEquSet:find("update_sets").combo_box
		assert(updateEquSetCombo)

		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)
		local equSetCombo = npcUI:find("equipment_sets").combo_box
		assert(equSetCombo)

		updateEquSetCombo:addText(editText.uc_input_string)
		equSetCombo:addText(editText.uc_input_string)

		if r2.equipmentSets[selection.Equipment] == nil then
			r2.equipmentSets[selection.Equipment] = {}
		end
		r2.equipmentSets[selection.Equipment][editText.input_string] = {}

		for i=1, r2.equipmentAttNb do 
			local comboBox = npcUI:find(r2.equipementComboB[i]):find("combo_box")
			local slider = npcUI:find(r2.equipementComboB[i]):find("slider")
			
			r2.equipmentSets[selection.Equipment][editText.input_string][r2.equipmentKeys[i] ] = {
				comboSelection = comboBox.selection_text, color = slider.value}
		end

		r2:saveTable("save\\equipmentSets.txt", r2.equipmentSets)

		equSetCombo.selection_text = editText.input_string
		
		editText.input_string=""
	end
end

-- SELECT EQUIPMENT SETS ---------------------------------------------------------------------------

r2.saveEquipmentSet = false

function r2:selectEquipmentSets()

	r2.selectEquipmentSet = true

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local equSetCombo = npcUI:find("equipment_sets").combo_box
	assert(equSetCombo)
	
	local selection = r2:getSelectedInstance()

	if equSetCombo.view_text == r2.emptyComboLine then 
		return
	else
		if r2.equipmentSets[selection.Equipment] == nil then return end
		local equSetsValue = r2.equipmentSets[selection.Equipment][equSetCombo.selection_text] 
		if equSetsValue == nil then return end

		r2.requestNewAction(i18n.get("uiR2EDSelectNPCEquipmentSetsAction"))
		r2.saveEquipmentSet = true

		-- update comboBoxes and sliders values
		for i = 1,r2.equipmentAttNb do

			local comboItemText = equSetsValue[r2.equipementComboB[i]].comboSelection
			local colorPiece = equSetsValue[r2.equipementComboB[i]].color

			local comboBox = npcUI:find(r2.equipementComboB[i]).label_combo_box.combo_box
			assert(comboBox)
			
			if comboItemText == r2.noPiece then
				local propName =  r2.equipmentEnv[r2.equipementComboB[i]].propName
				if not (propName == "HairType" and not r2.hasHelmet) then

					comboBox.selection_text = r2.noPiece

					if r2.equipmentEnv[r2.equipementComboB[i] ].propColorName ~= "HairColor" then
						r2:setNpcAttribute(selection.InstanceId, r2.equipmentEnv[r2.equipementComboB[i]].propColorName, 0)
						npcUI:find(r2.equipementComboB[i]):find("slider").value = 0
					end
				end
			else 
				comboBox.selection_text = equSetsValue[r2.equipmentKeys[i]].comboSelection
				
				r2:setNpcAttribute(selection.InstanceId, r2.equipmentEnv[r2.equipementComboB[i]].propColorName, colorPiece)
				npcUI:find(r2.equipementComboB[i]):find("slider").value = colorPiece
			end
		end

		local updateEquSetsCombo = getUI("ui:interface:r2ed_add_equipment_set"):find("update_sets").combo_box
		assert(updateEquSetsCombo)

		updateEquSetsCombo.selection_text = equSetCombo.selection_text
		
		r2.saveEquipmentSet = false
		r2.requestEndAction(i18n.get("uiR2EDSelectNPCEquipmentSetsAction"))
	end

	r2.selectEquipmentSet = false
end

-----------------------------------------------------------------------------------------------

function r2:updateEquipmentSet()

	if r2.saveEquipmentSet then return end

	local selection = r2:getSelectedInstance()

	local updateEquSetsCombo = getUI("ui:interface:r2ed_add_equipment_set"):find("update_sets").combo_box
	assert(updateEquSetsCombo)
	local equSetsName = updateEquSetsCombo.selection_text
	
	-- update sets in table
	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	
	local comboBSelection, slider
	for i = 1,r2.equipmentAttNb do
		comboBSelection = npcUI:find(r2.equipementComboB[i]):find("combo_box").selection_text
		r2.equipmentSets[selection.Equipment][equSetsName][r2.equipementComboB[i] ].comboSelection = comboBSelection
		slider = npcUI:find(r2.equipementComboB[i]):find("slider")
		
		r2.equipmentSets[selection.Equipment][equSetsName][r2.equipementComboB[i] ].color = slider.value
	end

	r2:saveTable("save\\equipmentSets.txt", r2.equipmentSets)

	local equSetCombo = npcUI:find("equipment_sets").combo_box
	assert(equSetCombo)
	equSetCombo.selection_text = updateEquSetsCombo.selection_text 
end

-----------------------------------------------------------------------------------------------

function r2:removeEquipment()

	local selection = r2:getSelectedInstance()

	local updateEquSetsCombo = getUI("ui:interface:r2ed_add_equipment_set"):find("update_sets").combo_box
	assert(updateEquSetsCombo)
	local equSetsName = updateEquSetsCombo.selection_text

	-- delete sets in table
	r2.equipmentSets[selection.Equipment][equSetsName] = nil
	r2:saveTable("save\\equipmentSets.txt", r2.equipmentSets)

	-- delete sets in combo box
	updateEquSetsCombo:removeSelection(updateEquSetsCombo.selection)
	
	local equSetsCombo = getUI("ui:interface:r2ed_npc"):find("equipment_sets").combo_box
	assert(equSetsCombo)
	equSetsCombo:removeText(equSetsName)
	equSetsCombo.view_text = r2.emptyComboLine
	
	updateEquSetsCombo.view_text = r2.emptyComboLine
end

-----------------------------------------------------------------------------------------------

function r2:randomEquipment()
	r2.requestNewAction(i18n.get("uiR2EDRandomNpcEquipmentAction"))
	r2.selectEquipmentSet = true

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	for i=1, r2.equipmentAttNb do -- no right or left hand
		local comboBox = npcUI:find(r2.equipementComboB[i]):find("combo_box")
		if comboBox.selectionNb > 0 then
			comboBox.selection = math.random(1, comboBox.selectionNb) - 1	
		end
	end

	r2.selectEquipmentSet = false
	r2.requestEndAction()
end

-----------------------------------------------------------------------------------------------

function r2:randomColor()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local levelDesignEnabled = getClientCfgVar("LevelDesignEnabled")
	local maxVal = 5
	if tonumber(levelDesignEnabled)==1 then maxVal = 7 end

	if r2.linkColorB then
		local randomColor = math.random(0, maxVal)
		while r2.linkedColor == randomColor do
			randomColor = math.random(0, maxVal)
		end
		r2.linkedColor = randomColor
	end
	
	for i=1, r2.equipmentAttNb do -- no right or left hand
		local slider = npcUI:find(r2.equipementComboB[i]):find("slider")
	
		if not r2.linkColorB then
			slider.value = math.random(0, maxVal)
		end	
	end
	r2:updateLinkColor()
end

---------RANDOM ALL NPC PROPERTIES -------------------------------------------------------------

function r2:randomNPCSex(sheetClient)

	local race
	local s = string.find(sheetClient, "fyros")
	if s ~= nil then race = "Fyros" end

	s = string.find(sheetClient, "matis")
	if s ~= nil then race = "Matis" end

	s = string.find(sheetClient, "tryker")
	if s ~= nil then race = "Tryker" end

	s = string.find(sheetClient, "zorai")
	if s ~= nil then race = "Zorai" end

	local sex = (math.random(0,10)>5)
	if sex == true then 
		sex = r2.male 
	else 
		sex = r2.female 
	end

	return r2.raceSheetClient[sex][race]
end

function r2:randomNPCProperties(equipmentId, race)
	
	local result = {}

	-- random body
	result["GabaritHeight"] = math.random(0,14)
	result["GabaritTorsoWidth"] = math.random(0,14)
	result["GabaritArmsWidth"] = math.random(0,14)
	result["GabaritLegsWidth"] = math.random(0,14)
	result["GabaritBreastSize"] = math.random(0,14)

	-- random face
	result["HairColor"] = math.random(0,5)
	result["Tattoo"] = math.random(0,31)
	result["HairType"] = getSheetId(r2.sliderValueToItemIndex[race][math.random(0,6)])
	result["EyesColor"] = math.random(0,7)

	-- random face morph
	result["MorphTarget1"] = math.random(0,7)
	result["MorphTarget2"] = math.random(0,7)
	result["MorphTarget3"] = math.random(0,7)
	result["MorphTarget4"] = math.random(0,7)
	result["MorphTarget5"] = math.random(0,7)
	result["MorphTarget6"] = math.random(0,7)
	result["MorphTarget7"] = math.random(0,7)
	result["MorphTarget8"] = math.random(0,7)

	-- random equipment
	r2:randomNPCEquipment(equipmentId, result)

	return result
end

function r2:randomNPCEquipment(equipmentId, result)

	if result == nil then
		result = {}
	end

	local tableEquipment = r2.equipmentPalette[equipmentId]
	assert(tableEquipment)
	local hasHelmet = false
	local size = 0
	local hasNudeOption = false
	for k, v in pairs(tableEquipment.helmet) do
		size = size+1
		if k == r2.noPiece then
			hasNudeOption = true
		end	
	end
	if size > 0 and (not hasNudeOption or (math.random(0,10) > 5)) then
		local helmetType = r2:randomPieceEquipment(tableEquipment.helmet, false)
		if helmetType ~= 0 then
			result["HairType"] = helmetType
			hasHelmet = true
		end
	end
	result["JacketModel"] = r2:randomPieceEquipment(tableEquipment.chest_plate, false)
	result["TrouserModel"] = r2:randomPieceEquipment(tableEquipment.legs, false)
	result["FeetModel"] = r2:randomPieceEquipment(tableEquipment.boots, false)
	result["HandsModel"] = r2:randomPieceEquipment(tableEquipment.gloves, false)
	result["ArmModel"] = r2:randomPieceEquipment(tableEquipment.arms_guard, false)

	local weaponRH, weaponLH = r2:randomPieceEquipment(tableEquipment.hands, true)
	result["WeaponRightHand"] = weaponRH
	result["WeaponLeftHand"] = weaponLH

	-- random equipment color
	local levelDesignEnabled = getClientCfgVar("LevelDesignEnabled")
	local maxVal = 5
	if tonumber(levelDesignEnabled)==1 then maxVal = 7 end
	result["JacketColor"] = math.random(0, maxVal)
	result["ArmColor"] = math.random(0, maxVal)
	result["HandsColor"] = math.random(0, maxVal)
	result["TrouserColor"] = math.random(0, maxVal)
	result["FeetColor"] = math.random(0, maxVal)
	if hasHelmet then
		result["HairColor"] = math.random(0,maxVal)
	end

	return result
end

-- Same as randomNPCName but can return 2 times the same name
function r2:randomNPCName2(race, sex)
	local allNames = r2:getAllNames()
	local tableNames = allNames[race .."_" .. sex]
	if not tableNames or table.getn(tableNames) == 0 then return "" end
	local indexName = math.random(1, table.getn(tableNames))
	return tableNames[indexName]
end

function r2:randomNPCName(race, sex)

	-- random name
	local tableNames = r2.NPCNames[race .."_" .. sex]

	local name = ""
	if table.getn(tableNames) == 0 then
		r2:loadRaceSexNames(race, sex)
		tableNames = r2.NPCNames[race .."_" .. sex]
	end

	if table.getn(tableNames) > 0 then
		local indexName = math.random(1, table.getn(tableNames))
		name = tableNames[indexName]
		tableNames[indexName] = tableNames[table.getn(tableNames)]
		tableNames[table.getn(tableNames)] = nil
	end
	
	return  name
end

function r2:isRandomizedNPCName(name, race, sex)

	local tableNames = r2:loadRaceSexNamesImpl(race, sex)

	for k, vName in pairs(tableNames) do
		if vName==name then return true end
	end

	return false
end

function r2:searchSheet(equipmentId, weaponRH, weaponLH, sheet, sheetModel)

	local tableEquipment = r2.equipmentPalette[equipmentId]
	assert(tableEquipment)

	local hands = tableEquipment.hands

	for k, v in pairs(hands) do
		local rightHand = v.rightHand
		local leftHand = v.leftHand
		if weaponRH==getSheetId(rightHand) and weaponLH==getSheetId(leftHand) then
			local handsLevel = v.handsLevel

			local s, e = string.find(sheet, ".creature")
			local level = string.sub(sheet, s-1, s-1)

			local newSheet = sheetModel
			newSheet = string.gsub(newSheet, "$hands", handsLevel)
			newSheet = string.gsub(newSheet, "$level", level)
			return newSheet
		end
	end

	return ""
end


-- Return a random NPC (Warning names can be duplicates)
function r2:randomNPC(base)
	assert(base)

	local npc = r2.newComponent("NpcCustom")

	npc.Base = base					
	
	-- random equipment and random sex
	local sheetClient = r2.getPropertyValue(npc, "SheetClient")
	
	local sexSheetClient = r2:randomNPCSex(sheetClient)
	npc.SheetClient = sexSheetClient

	local raceSelection = getR2PlayerRace(npc.SheetClient)
	local sexSelection = nil
	if isR2PlayerMale(npc.SheetClient) then
		sexSelection = r2.male
	else 
		sexSelection = r2.female
	end

	local equipment = r2.getPropertyValue(npc, "Equipment")

	local results = r2:randomNPCProperties(equipment, raceSelection)
	local k,v = next(results, nil)
	while v do
		npc[k] = v
		k,v = next(results, k)
	end
	npc.Name = r2:randomNPCName2(raceSelection, sexSelection)

	return npc
end

function r2:randomPieceEquipment(pieces, isHands)

	local keyPlace = 1
	assert(pieces ~= nil)		
	local size = 0
	-- why table.getn(pieces) doesn't work???
	for k, v in pairs(pieces) do
		size = size + 1
	end

	if size > 1 then
		keyPlace = math.random(1,size)
	end

	local key, itemFileName
	local count = 1
	for k, v in pairs(pieces) do
		if count == keyPlace then
			key = k
			tempTable = v
			break
		end
		count = count + 1
	end
	if key == r2.noPiece or tempTable == nil then
		return 0
	elseif isHands==true then
		local rightHand = getSheetId(tempTable.rightHand)
		local leftHand = getSheetId(tempTable.leftHand)
		return rightHand, leftHand
	else
		return getSheetId(tempTable.itemFile)
	end
end

function r2:getPieceEquipmentNumber(pieces, piece)


	local pieceNb = 0
	for k, v in pairs(pieces) do
		
		if v.rightHand and v.leftHand then 

			if getSheetId(v.rightHand) == piece then
				return pieceNb
			elseif getSheetId(v.leftHand) == piece then
				return pieceNb
			end

		elseif getSheetId(v.itemFile) == piece then
			return pieceNb
		end
		pieceNb = pieceNb + 1
	end

	return -1
end

function r2:getPieceEquipmentFromNumber(pieces, number, handSide)


	local pieceNb = 0
	for k, v in pairs(pieces) do

		if pieceNb == number then
			if v.rightHand and v.leftHand then 
				if handSide == "right" then
					return getSheetId(v.rightHand)
				elseif handSide == "left" then
					return getSheetId(v.leftHand)
				end
			else
				return getSheetId(v.itemFile)
			end
		end
		pieceNb = pieceNb + 1
	end

	return -1
end

-----------------------------------------------------------------------------------------------

function r2:updatePieceEquipmentColor(requestType)

	if not r2.linkColorB then
		r2:updateColor(getUICaller(), requestType)	
	else
		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)
		
		r2.linkedColor = getUICaller().value
		r2:updateLinkColor(requestType)
	end
end

-----------------------------------------------------------------------------------------------
r2.updateLinkColors = false
function r2:updateLinkColor(requestType)

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	if not r2.linkColorAction then
		r2.requestNewAction(i18n.get("uiR2EDUpdateLinkedNPCColorAction"))
	end

	r2.updateLinkColors = true

	for i=1, r2.equipmentAttNb do
		r2:updateColor(npcUI:find(r2.equipementComboB[i]):find("slider"), requestType)
	end

	r2.updateLinkColors = false

	if not r2.linkColorAction then
		r2.requestEndAction()
	end
end

function r2:updateColor(slider, requestType)

	local selection = r2:getSelectedInstance()
	if selection == nil then
		debugInfo("No selection")
		return
	end

	if not r2.updateLinkColors then
		r2.requestNewAction(i18n.get("uiR2EDUpdateNPCColorAction"))
	end

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local nameComboBox = slider.name
	local equipmentType = npcUI:find(nameComboBox):find("combo_box").selection_text

	if equipmentType == r2.noPiece then
		slider.value = 0
	else
		if r2.linkColorB then
			slider.value = r2.linkedColor
		end

		local propColor = r2.equipmentEnv[nameComboBox].propColorName
		if requestType == nil then
			r2:setNpcAttribute(selection.InstanceId, propColor, slider.value)
		elseif requestType == 'local' then
			r2.requestSetLocalNode(selection.InstanceId, propColor, slider.value)
		elseif requestType == 'commit' then
			r2.requestCommitLocalNode(selection.InstanceId, propColor)
		elseif requestType == 'cancel' then
			r2.requestRollbackLocalNode(selection.InstanceId, propColor)
		else
			debugInfo("r2:updateColor : unknown request type")
		end
	end
	r2.requestEndAction()
end

-----------------------------------------------------------------------------------------------
r2.linkColorAction=false
function r2:linkColor()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)
	local toggleB = npcUI:find("color_link").toggle_butt
	assert(toggleB)
	r2.linkColorB = not toggleB.pushed 

	-- update LinkColor property
	local selection = r2:getSelectedInstance()
	if selection == nil then return end

	r2.linkColorAction = true
	r2.requestNewAction(i18n.get("uiR2EDLinkColorEquipment"))
	local link = 0
	if r2.linkColorB then link=1 end 
	r2:setNpcAttribute(selection.InstanceId, "LinkColor", link)

	-- update colors
	if not r2.linkColorB then
		r2.linkedColor = 0
	else
		local colors = {}
		local slider  

		for i=r2.equipmentAttNb, 1, -1 do -- no right or left hand

			slider = npcUI:find(r2.equipementComboB[i]):find("slider")

			local nameComboBox = slider.name
			local equipmentType = npcUI:find(nameComboBox):find("combo_box").selection_text

			if equipmentType ~= r2.noPiece then
				local value = slider.value
				if colors[value] == nil then colors[value] = 0 end
				colors[value] = colors[value] + 1
			end	
		end

		local colorMax = -1
		local colorMaxCount = 0

		for k, v in pairs(colors) do
			if v > colorMaxCount then
				colorMaxCount = v
				colorMax = k
			end
		end

		if colorMax~=-1 then
			if colorMaxCount==1 then colorMax = slider.value end

			r2.linkedColor = colorMax

			r2:updateLinkColor()
		end
	end

	r2.requestEndAction()
	r2.linkColorAction = false
end

---------PREVIEW -----------------------------------------------------------------------------

function r2:preview()

	local npcUI = getUI("ui:interface:r2ed_npc")
	assert(npcUI)

	local npcView = npcUI:find("npc_view")
	assert(npcView)

	local sep = npcUI:find("v_sep")
	assert(sep)

	if npcView.active then
		npcUI.pop_min_w = npcUI.pop_min_w - 300
		npcUI.pop_max_w = npcUI.pop_max_w - 300
		npcUI.x = npcUI.x + 300
	else
		npcUI.pop_min_w = npcUI.pop_min_w + 300
		npcUI.pop_max_w = npcUI.pop_max_w + 300
		npcUI.x = npcUI.x - 300
	end

	npcView.active = not npcView.active
	sep.active = not sep.active
end

function r2:isNPCPlant(sheetClient)
	return string.match(sheetClient, "cp[%w_]*%.creature")
end



