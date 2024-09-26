r2.acts = {

	newLocation = "new location",
	enlargingFactor = 2,
	islands = {},
	ecoSystemName = "r2_desert",
	islandButtonColors = 
	{
		["r2_desert"] = "157 110 74 255", 
		["r2_jungle"] = "47 110 54 255",
		["r2_forest"] = "74 52 0 255",
		["r2_lakes"] = "95 93 67 255", 
		["r2_roots"] = "66 52 41 255",
	},

	islandSelectedButtonColors = 
	{
		["r2_desert"] = "152 60 39", 
		["r2_jungle"] = "18 156 51",   
		["r2_forest"] = "133 36 13", 
		["r2_lakes"] = "109 149 106", 
		["r2_roots"] = "73 148 122",
	},

	islandOverButtonColors = 
	{
		["r2_desert"] = "127 48 33",
		["r2_jungle"] =  "0 51 20", 
		["r2_forest"] = "90 28 0", 
		["r2_lakes"] = "55 53 37",
		["r2_roots"] = "36 22 11", 
	},

	ecosystemNames = 
	{
		["r2_desert"] =	i18n.get("uiR2EDEcosystemDesert"):toUtf8(),
		["r2_jungle"] =	i18n.get("uiR2EDEcosystemJungle"):toUtf8(), 
		["r2_forest"] =	i18n.get("uiR2EDEcosystemForest"):toUtf8(), 
		["r2_lakes"]  =	i18n.get("uiR2EDEcosystemLacustre"):toUtf8(),
		["r2_roots"]  =	i18n.get("uiR2EDEcosystemPrimeRoots"):toUtf8(), 
	},

	selectedIslandButtonId = nil,
	selectedIslandName = nil,

	createNewScenario = true,
	deleteOldScenario = false,
	createNewLocation = true,

	currentScenario = 
	{
		name="",
		level = 0,
		rules = "strict",
		notes = ""
	},

	currentAct = 
	{
		name="",
		weather = 0,
		manualWeather = true,
		notes = "",
	},

	currentLocation = 
	{
		name="",
		islandName = "",
		instanceId = "",
		season = "spring",
		manualSeason = true,
		notes = "",
		entryPoint = "",
	},
}

-------------------------- to sort islands in function of their translated name -----------
function r2.acts:getIslandNb(islandName)

	local islandTrans = i18n.get(islandName):toUtf8()
	local islandNb = string.sub(islandTrans, -5)
	local endLen = 6
	if string.sub(islandNb, 1, 1) ~= " " then
		islandNb = string.sub(islandNb, 3)
		endLen = endLen-1
	else
		islandNb = string.sub(islandNb, 2)
	end

	-- outland test
	local outland = string.sub(islandTrans, 1, -endLen)
	outland = string.sub(outland, -7)
	if outland=="Outland" then return nil end

	islandNb = string.sub(islandNb, 1, -2)
	return tonumber(islandNb)
end

--------------------------------------------------------------------------------------
--------------------------- init location editor -------------------------------------
function r2.acts:initActsEditor()

	-- create scenario/act window
	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	self.islands = {}
	local islands = getCompleteIslands()

	for islandName, islandAtts in pairs(islands) do
		local ecoSystemName = islandAtts.continent
		if self.islands[ecoSystemName] == nil then
			self.islands[ecoSystemName] = {}
		end

		local islandNb = self:getIslandNb(islandName)
		if islandNb then
			self.islands[ecoSystemName][islandNb] = {["name"] =islandName, ["table"] = islandAtts}
		end
	end

	local ecoList = actsUI:find("ecosystem_list_2")
	assert(ecoList)
	ecoList:clear()

	local ecoButtonsGr = actsUI:find("choose_ecosystem")
	assert(ecoButtonsGr)

	local buttonDim = 78
	local maxWLine = actsUI:find("choose_island_2"):find("back_gr").w_real - 1 
	local nbButtonsInLine = math.floor(maxWLine/(buttonDim+2))
	local addW = math.floor((maxWLine - buttonDim*nbButtonsInLine)/nbButtonsInLine)

	-- for each ecosystem group, create all islands buttons
	for ecoSysName, ecoSysTable in pairs(self.islands) do

		local ecoGr = createGroupInstance("template_lines", ecoList.id, {id=ecoSysName})
		assert(ecoGr)
		ecoList:addChild(ecoGr)
		ecoGr.active = false

		local maxPackage = 0
		local islandsNb = 0
		for k, v in pairs(ecoSysTable) do 
			local package = r2.getIslandRingAccess(v.name)
			if r2.RingAccess.testAccess(package) then
				local package = tonumber(string.sub(package, 2, 2))
				if package>maxPackage then maxPackage = package end
				islandsNb=islandsNb+1 
			end
		end
		local nbLines = math.ceil(islandsNb/nbButtonsInLine)

		local ecoButton = ecoButtonsGr:find(ecoSysName):find("eco_button")
		assert(ecoButton)

		local uc_package = ucstring()
		uc_package:fromUtf8(i18n.get("uiR2EDEcosystemPackage"):toUtf8().." : "..maxPackage)
		ecoButton.tooltip = uc_package

		for i=0, nbLines-1 do
			local lineGr = createGroupInstance("template_line", ecoGr.id, {id="line"..i, h=buttonDim})
			assert(lineGr)
			ecoGr:addChild(lineGr)
		end	

		if ecoGr.childrenNb >0 then
			local currentLine = ecoGr:getChild(0)
			assert(currentLine)

			local currentEltInLine = 0
			local currentLineNb = 0

			for islandNb, islandAttrs in pairs(ecoSysTable) do

				local islandName = islandAttrs.name
				local islandTable = islandAttrs.table

				local textureName = islandName.."_sp_little.tga"
				local ringAccess = r2.RingAccess.testAccess( r2.getIslandRingAccess(islandName) )
				if fileExists(textureName) and ringAccess then

					local tooltip = islandName 

					local islandW = islandTable.xmax - islandTable.xmin
					local islandH = islandTable.ymax - islandTable.ymin
					local maxDim = math.max(islandW, islandH)
					local ratio = 64/maxDim
					local width = math.floor(islandW*ratio)
					local height = math.floor(islandH*ratio)

					local maxDim = buttonDim - 20                      -- buttonDim - 2*(8+2)				
					local w_button_texture 
					local h_button_texture 
					local x_button_texture 
					local y_button_texture 
					local scale = "false"
					
					scale, w_button_texture, h_button_texture, width, height = 
						self:textureRedimension(textureName, maxDim, maxDim, width, height)

					scale = tostring(scale)

					-- center button island
					x_button_texture =((maxDim-width)/2 + 10)			--   (8 + (maxDim-width)/2 + 2)
					y_button_texture =-((maxDim-height)/2 + 10)			--   (-((maxDim-height)/2 + 8 + 2))

					local tmplParams = 
					{
						id=islandName,
						posparent="parent",
						posref="TL TL",
						sizeref= "",
						h = buttonDim+2,
						w = buttonDim+addW, 

						x_button="0", 
						y_button="0",	
						w_button = -addW, 
						h_button = "-2",

						icon = textureName,
						tooltip = tooltip,
						w_button_texture = w_button_texture,
						h_button_texture = h_button_texture,
						x_button_texture = x_button_texture,
						y_button_texture = y_button_texture,
						scale = scale,
						color= self.islandButtonColors[ecoSysName],
						back_color= self.islandButtonColors[ecoSysName],
						selected_color = self.islandSelectedButtonColors[ecoSysName].." 255",
						col_over = self.islandOverButtonColors[ecoSysName].." 80",
						group_params_l="r2.acts:openIslandCardFromButton('"..ecoSysName.."', '" ..islandName.."')",
						params_l="r2.acts:openIslandCardFromButton('"..ecoSysName.."', '" ..islandName.."')",
					}
					local buttonIsland = createGroupInstance("act_button_template", currentLine.id, tmplParams)
					
					if buttonIsland then
						currentLine:addChild(buttonIsland)	
						buttonIsland.Env.Name = islandName
					end

					currentEltInLine = currentEltInLine+1
					if currentEltInLine==nbButtonsInLine then
						currentLineNb = currentLineNb+1
						if currentLineNb < ecoGr.childrenNb then
							currentLine = ecoGr:getChild(currentLineNb)
						end
						currentEltInLine = 0
					end
				end
			end
		end
	end

	local newLocationMode = actsUI:find("new_location_mode_2")
	assert(newLocationMode)
	newLocationMode.Env.Name = self.newLocation

	local ringLevel = actsUI:find("ring_level")
	ringLevel.hardtext = "Ring level :    " .. r2.getCharacterRingAccess()
end

--------------------------------------------------------------------------------------
--------------------------- open scenario editor -------------------------------------
function r2.acts:openScenarioActEditor(newScenario, noCancelOption, rebuildFirstAct)
	
	setKeyboardContext("r2ed_scenario_creation")

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local locationEditor = actsUI:find("location")
	assert(locationEditor)

	local prevLocationEditor = actsUI:find("previous_location")
	assert(prevLocationEditor)

	actsUI.active = true
	setTopWindow(actsUI)

	self.createNewScenario = newScenario
	locationEditor.active = (newScenario==true or rebuildFirstAct==true)
	prevLocationEditor.active = not locationEditor.active
	self.createNewLocation = true

	local islandCancel = actsUI:find("island_cancel")
	assert(islandCancel)
	islandCancel.active = (noCancelOption~=true)

	local islandPrecedent = actsUI:find("island_precedent")
	assert(islandPrecedent)
	islandPrecedent.active = (self.createNewScenario~=true and rebuildFirstAct~=true)
	
	-----------------------------------------
	-- init scenario/act/location properties
	self.currentScenario.level = 0
	self.currentScenario.rules = "strict"

	self.currentAct.weather = math.random(0,1022) 
	self.currentAct.manualWeather = true
	self.currentLocation.manualSeason = true

	local seasonNb = math.random(1,4) 
	local seasons = {[1]="Spring", [2]="Summer", [3]="Autumn", [4]="Winter"}
	self.currentLocation.season = seasons[seasonNb]

	-- location season
	local seasonGr = actsUI:find("island_season")
	assert(seasonGr)
	local seasonButton = seasonGr:find(self.currentLocation.season)
	assert(seasonButton)
	self:selectButtonTemplate(seasonButton)

	local seasonManual = seasonGr:find("manual_season_2")
	assert(seasonManual)
	seasonManual:find("toggle_butt").pushed = not self.currentLocation.manualSeason

	-- act and scenario names
	if self.createNewScenario or rebuildFirstAct then
		self.currentAct.name = i18n.get("uiR2EDDefaultActTitle"):toUtf8() .. " 1"
	else
		local actNb = r2.Scenario.Acts.Size 
		self.currentAct.name = i18n.get("uiR2EDDefaultActTitle"):toUtf8() .. " " .. actNb 
	end
	
	if self.createNewScenario == true then
		self.currentScenario.name = i18n.get("uiR2EDNewScenario"):toUtf8()
	else
		r2.ScenarioWindow:setActNotes()

		-- select "new island" mode
		local newLocationMode = actsUI:find("new_location_mode_2")
		assert(newLocationMode)
		newLocationMode:find("button").active = false
		newLocationMode:find("selected_button").active = true
		self.currentLocation.islandName = ""
		self.currentLocation.instanceId = ""
			
		-- clean list of old previous locations
		local locationList = actsUI:find("location_list")
		assert(locationList)
		locationList:clear()

		self:openPreviousIslandsActs()

		local scrollPreviousLoc = actsUI:find("scroll_previous_islands")
		assert(scrollPreviousLoc)
		scrollPreviousLoc.trackPos = scrollPreviousLoc.h_real
	end

		self:openEcosystemIslands("r2_desert")		
end

function r2.acts:backPreviousLocations()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local newLocationMode = actsUI:find("new_location_mode_2")
	assert(newLocationMode)

	self:locationIsland(newLocationMode:find("button"))

	local locationEditor = actsUI:find("location")
	assert(locationEditor)

	local prevLocationEditor = actsUI:find("previous_location")
	assert(prevLocationEditor)

	locationEditor.active = false
	prevLocationEditor.active = true
end

-------------------------------------------------------------------------------------------------------
function r2.acts:openPreviousIslandsActs()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local locationList = actsUI:find("location_list")
	assert(locationList)

	local enlargingFactor = self.enlargingFactor 

	local locationActs = {}
	for key, act in specPairs(r2.Scenario.Acts) do
		local locationId = act.LocationId
		if locationActs[locationId]==nil then locationActs[locationId]={} end
		table.insert(locationActs[locationId], act.InstanceId)
	end

	for key, location in specPairs(r2.Scenario.Locations) do
		
		local islandName = location.IslandName

		local textureName = islandName .. "_sp.tga"

		local actsUI = getUI("ui:interface:r2ed_acts")
		assert(actsUI)

		local islandTable
		local ecoSysName = nil
		for ecoSystem, ecoSysTable in pairs(self.islands) do
			local islandNb = self:getIslandNb(islandName)
			if ecoSysTable[islandNb] and ecoSysTable[islandNb].name == islandName then
				ecoSysName = ecoSystem
				islandTable = ecoSysTable[islandNb].table
				break
			end
		end

		if ecoSysName then

			local backTexture = ecoSysName .. "_back.tga"

			local islandW, islandH = (islandTable.xmax - islandTable.xmin)*enlargingFactor, (islandTable.ymax - islandTable.ymin)*enlargingFactor

			-- resize island button to see all island whitout deformation
			local totalDim = 200
			local maxDim = totalDim-2*8
			local w_button_texture 
			local h_button_texture 
			local x_button_texture 
			local y_button_texture 
			local scale = "false"
			
			local initW, initH = islandW, islandH

			scale, w_button_texture, h_button_texture, islandW, islandH = 
				self:textureRedimension(textureName, maxDim, maxDim, islandW, islandH)

			scale = tostring(scale)

			-- center button island
			x_button_texture = (maxDim-islandW)/2 + 8				--   8 + (maxDim-width)/2 + 2 
			y_button_texture = -((maxDim-islandH)/2 + 8)			--   -((maxDim-height)/2 + 8 + 2)

			local h, h_button, y_button, h_text, y_text
			local marge=15
			if locationList.childrenNb == 0 then
				h=totalDim
				h_button="0"
				y_button="0"
				h_text="0"
				y_text="0"
			else
				h=totalDim+marge
				h_button=-marge
				y_button=-marge
				h_text=-marge
				y_text=-marge
			end

			local tmplParams = 
			{
				id=islandName,
				posparent="parent",
				posref="BR TR",
				sizeref="w",
				w="0",
				h=h,
				x="0",
				y="0",

				sizeref_act="h",
				w_act = totalDim,
				h_act="0",
				x_act="0",
				y_act="0",
				y_button=y_button, 	
				h_button=h_button,

				icon = textureName,
				w_button_texture = w_button_texture,
				h_button_texture = h_button_texture,
				x_button_texture = x_button_texture,
				y_button_texture = y_button_texture,
				scale = scale,
				color= self.islandButtonColors[ecoSysName],
				back_color= "255 255 255 255",
				selected_color = self.islandSelectedButtonColors[ecoSysName].." 255",
				col_over = self.islandSelectedButtonColors[ecoSysName].." 80",
				group_params_l="r2.acts:locationIsland(getUICaller())",
				params_l="r2.acts:locationIsland()",
				back_texture=backTexture,

				x_text=marge, 
				w_text= -(totalDim + marge + 15), 
				h_text=h_text,
				y_text=y_text,
			}
			local buttonIsland = createGroupInstance("previous_loc_template", locationList.id, tmplParams)
			
			if buttonIsland then
				locationList:addChild(buttonIsland)	
				buttonIsland.Env.InstanceId = location.InstanceId

				local uc_island = ucstring()
				uc_island:fromUtf8(location.Name)
				buttonIsland:find("button"):find("center_button").tooltip = uc_island
				buttonIsland:find("selected_button"):find("center_button").tooltip = uc_island

				-- init text
				local textList = buttonIsland:find("text_list")
				assert(textList)

				local prevText = textList:find("previous_text")
				assert(prevText)

				local text = "'" .. location.Name .. "' Location used in acts :\n"
				prevText:addColoredTextChild(text, 200, 0, 0, 255)

				local acts = locationActs[location.InstanceId]
				for k, actId in pairs(acts) do
					local act = r2:getInstanceFromId(actId)
					local act_text = act.Name 
					if act.ShortDescription~="" then 
						act_text = act_text .. " : "
					end

					prevText:addColoredTextChild(act_text, 200, 120, 0, 255)

					if act.ShortDescription~="" then
						act_text = act.ShortDescription
						local uc_act = ucstring()
						uc_act:fromUtf8(act_text)
						prevText:addTextChild(uc_act)
					end
				end

				-- init scroll target
				local scroll = textList:find("scroll_previous")
				assert(scroll)
				local list = textList:find("list")
				assert(list)
				scroll:setTarget(list.id)
			end

		end
	end
end

--------------------------------------------------------------------------------------
--------------------------- texture button redimension -------------------------------
function r2.acts:textureRedimension(textureName, maxW, maxH, islandW, islandH)

	local scale = false
	local w, h

	if maxW<islandW or maxH<islandH then
		local ratioW = islandW/maxW
		local ratioH = islandH/maxH
		local maxRatio = math.max(ratioW, ratioH)
		
		local textureW, textureH = getTextureSize(textureName)
		local newW = math.floor(textureW/maxRatio)
		local newH = math.floor(textureH/maxRatio)

		scale = true
		w = newW 
		h = newH 

		islandW = math.floor(islandW/maxRatio)
		islandH = math.floor(islandH/maxRatio)
	else
		w = islandW 
		h = islandH 	
	end

	return scale, w, h, islandW, islandH
end

--------------------------------------------------------------------------------------
--------------------------- open islands list of an ecosystem ------------------------
function r2.acts:openEcosystemIslands(ecoSystemName)

	self.ecoSystemName = ecoSystemName

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	-- ecosystem buttons
	local ecoSystemList = actsUI:find("ecosystem_list")
	assert(ecoSystemList)

	local selectedEcoSystemGr = ecoSystemList:find(ecoSystemName)
	assert(selectedEcoSystemGr)

	local selectedIslandsLists = selectedEcoSystemGr:find("two_lists")
	assert(selectedIslandsLists)

	-- ecosystem islands lists
	local ecoSystemList2 = actsUI:find("ecosystem_list_2")
	assert(ecoSystemList2)

	local selectedEcoSystemGr2 = ecoSystemList2:find(ecoSystemName)
	assert(selectedEcoSystemGr2)

	if selectedEcoSystemGr2.active then 
		local ecoButton = selectedEcoSystemGr:find("eco_button")
		assert(ecoButton)
		ecoButton.pushed = true
		--return 
	end

	local index = ecoSystemList2:getElementIndex(selectedEcoSystemGr2)
	for i=0, index do
		ecoSystemList2:upChild(selectedEcoSystemGr2)
	end

	-- all islands list inactive instead of selected ecosystem (inverse)
	for i=0, ecoSystemList2.childrenNb-1 do
		local ecosystemGr2 = ecoSystemList2:getChild(i)
		assert(ecosystemGr2)
		ecosystemGr2.active = (ecosystemGr2.id == selectedEcoSystemGr2.id)
	end

	for i=0, ecoSystemList.childrenNb-1 do

		local ecosystemGr = ecoSystemList:getChild(i)
		assert(ecosystemGr)

		local islandsList = ecosystemGr:find("two_lists")
		assert(islandsList)

		local ecoButton = ecosystemGr:find("eco_button")
		assert(ecoButton)

		ecoButton.pushed = (islandsList.id == selectedIslandsLists.id)

		ecosystemGr.child_resize_h = false
		ecosystemGr.h = 32 
		if islandsList.id == selectedIslandsLists.id then  
			ecosystemGr.h = 38
		end
	end

	-- open first island image
	local selectedIslandsList = selectedEcoSystemGr2:find("line0")
	
	local islandCard = actsUI:find("island_current_image")
	assert(islandCard)

	local islandOk = actsUI:find("island_ok")
	assert(islandOk)

	if selectedIslandsList and (selectedIslandsList.childrenNb > 0) then
		
		-- color
		local ecoColor = self.islandSelectedButtonColors[self.ecoSystemName].." 255"
		local ecoColorBack = self.islandSelectedButtonColors[self.ecoSystemName].." 100"

		islandCard.active = true

		islandCard:find("bl").color = ecoColor
		islandCard:find("tl").color = ecoColor
		islandCard:find("tr").color = ecoColor
		islandCard:find("br").color = ecoColor
		islandCard:find("bottom").color = ecoColor
		islandCard:find("top").color = ecoColor
		islandCard:find("left").color = ecoColor
		islandCard:find("right").color = ecoColor
		islandCard:find("bg").color = ecoColorBack

		islandOk.active = true

		local firstIsland = selectedIslandsList:getChild(0)
		assert(firstIsland)
		self:openIslandCardFromButton(self.ecoSystemName, firstIsland.Env.Name)
	else
		local islandBitmap = actsUI:find("island_bitmap")
		assert(islandBitmap)

		islandBitmap.texture = ""

		islandCard.active = false

		islandOk.active = false
	end

	local scrollIslands = actsUI:find("scroll_islands_2")
	assert(scrollIslands)
	scrollIslands.trackPos = scrollIslands.h_real

	-- "choose island" title
	local title = actsUI:find("choose_island"):find("choose_island_title")
	assert(title)
	-- doesn't work in all language local titleText = " " .. i18n.get("uiR2EDChooseIsland"):toUtf8() .." " .. self.ecosystemNames[self.ecoSystemName] .. " " .. i18n.get("uiR2EDEcosystem"):toUtf8() .. " "
	local titleText = " " .. i18n.get("uiR2EDChooseIsland"):toUtf8() .. self.ecosystemNames[self.ecoSystemName] .. " "
	local uc_title = ucstring()
	uc_title:fromUtf8(titleText)
	title.uc_hardtext = uc_title
end

--------------------------------------------------------------------------------------
--------------------------- open island card and images ------------------------------
function r2.acts:openIslandCardFromButton(ecosystem, islandName)

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local ecosystemList = actsUI:find("ecosystem_list_2")
	assert(ecosystemList)

	local ecosystemGr = ecosystemList:find(ecosystem)
	assert(ecosystemGr)

	local islandButton = ecosystemGr:find(islandName)
	assert(islandButton)

	-- unselect last selection
	if self.selectedIslandButtonId~=nil then
		local lastSelectedIsland = getUI(self.selectedIslandButtonId)
		if lastSelectedIsland ~= nil then
			lastSelectedIsland:find("button").active = true
			lastSelectedIsland:find("selected_button").active = false
		end
	end

	-- select button
	islandButton:find("button").active = false
	islandButton:find("selected_button").active = true

	self.selectedIslandButtonId = islandButton.id

	self.selectedIslandName = islandButton.Env.Name

	self.currentLocation.islandName = self.selectedIslandName

	self:openIslandCard(self.selectedIslandName)
end

--------------------------------------------------------------------------------------
--------------------------- open island card -----------------------------------------
function r2.acts:openIslandCard(islandName)

	local enlargingFactor = self.enlargingFactor

	local textureName = islandName .. "_sp.tga"

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local islandBitmap = actsUI:find("island_bitmap")
	assert(islandBitmap)

	-- load texture
	islandBitmap.texture = textureName

	-- card title
	local title = actsUI:find("island_images"):find("card_title")
	assert(title)
	-- doesn't work in all language local titleText = " " .. i18n.get("uiR2EDZoomOn"):toUtf8() .." '" .. i18n.get(islandName):toUtf8() .. "' " .. i18n.get("uiR2EDIsland"):toUtf8() .. " "
	local titleText = " " .. i18n.get("uiR2EDZoomOn"):toUtf8() .. i18n.get(islandName):toUtf8() .. " "
	local uc_title = ucstring()
	uc_title:fromUtf8(titleText)
	title.uc_hardtext = uc_title

	local islandTable = self.islands[self.ecoSystemName][self:getIslandNb(islandName)].table
	local islandW, islandH = (islandTable.xmax - islandTable.xmin)*enlargingFactor, (islandTable.ymax - islandTable.ymin)*enlargingFactor
	local islandCard = actsUI:find("island_current_image")
	assert(islandCard)

	-- resize island button to see all island whitout deformation
	local maxW, maxH = islandCard.w_real-12, islandCard.h_real-12
	
	local initW, initH = islandW, islandH
	islandBitmap.scale, islandBitmap.w, islandBitmap.h, islandW, islandH = 
		self:textureRedimension(textureName, maxW, maxH, islandW, islandH)

	local cardButton = islandCard:find("selected_button")
	assert(cardButton)

	cardButton.w = - (maxW - islandW)
	cardButton.h = - (maxH - islandH)
	cardButton.x = (maxW - islandW)/2
	cardButton.y = - (maxH - islandH)/2

	-- center button island
	islandBitmap.x = 6
	islandBitmap.y = -6

	-- entry points
	local entryNb = 0
	for location, entry in pairs(islandTable.entrypoints) do

		local entryPoinButton = actsUI:find("entrypoint"..entryNb):find("flag")
		entryPoinButton.active = true
		entryPoinButton.x = (enlargingFactor*(entry.x - islandTable.xmin)/initW)*islandW
		entryPoinButton.y = (enlargingFactor*(entry.y  - islandTable.ymax)/initH)*islandH

		entryPoinButton.tooltip = i18n.get(location)
		entryPoinButton.parent.Env.Name = location
		if entryNb == 0 then
			entryPoinButton.texture = "r2ed_entry_point_pushed.tga"
			self.currentLocation.entryPoint = location
		else
			entryPoinButton.texture = "r2ed_entry_point.tga"
		end

		entryNb = entryNb+1
		if entryNb==9 then break end
	end

	if entryNb < 9 then
		for i=entryNb, 9 do
			local entryPointGr = actsUI:find("entrypoint"..i)
			assert(entryPointGr)
			local entryPoinButton = entryPointGr:find("flag")
			entryPoinButton.active = false
		end
	end

	-- enlarge selected ecosystem button 
	local ecoButton = actsUI:find(self.ecoSystemName)
	assert(ecoButton)

	local ecoList = ecoButton.parent
	assert(ecoList)
	
	for i=0, ecoList.childrenNb-1 do
		local button = ecoList:getChild(i)
		if button == ecoButton then 
			button:find("eco_button").wmin = 164 
		else 
			button:find("eco_button").wmin = 160 
		end
	end

	actsUI:invalidateCoords()
	actsUI:updateCoords()
end

--------------------------------------------------------------------------------------
--------------------------- select an entry point ---------------------------------------
function r2.acts:selectEntryPoint()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	for i=0, 9 do
		local entryPoinButton = actsUI:find("entrypoint"..i):find("flag")
		assert(entryPoinButton)

		if entryPoinButton == getUICaller() then
			getUICaller().texture = "r2ed_entry_point_pushed.tga"
			self.currentLocation.entryPoint = getUICaller().parent.Env.Name
		else
			entryPoinButton.texture = "r2ed_entry_point.tga"
		end
	end 
end

--------------------------------------------------------------------------------------
--------------------------- choose a name for location -------------------------------
function r2.acts:chooseLocationName()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local chooseNamePopup = getUI("ui:interface:r2ed_choose_location_name")
	assert(chooseNamePopup)

	local editText = chooseNamePopup:find("edit_box_group")
	assert(editText)

	local name = editText.uc_input_string:toUtf8()

	-- check location name duplication
	if not self.createNewScenario then
		for i=0, r2.Scenario.Locations.Size-1 do
			local location = r2.Scenario.Locations[i]
			if location.Name == name then
				messageBox(i18n.get("uiR2EDLocationNameDuplicated"))
				return
			end
		end
	end
	
	if name~="" then

		self.currentLocation.name = name

		chooseNamePopup.active = false
		actsUI.active = false

		disableModalWindow()
		setKeyboardContext("r2ed")

		if self.createNewScenario==true then
			self:createScenario()
		else
			self:createAct()
		end
	else
		editText:setFocusOnText()
	end
end

--------------------------------------------------------------------------------------
--------------------------- choose a name for scenario/act ---------------------------
function r2.acts:chooseScenarioActName()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local chooseNamePopup = getUI("ui:interface:r2ed_choose_scenario_act_name")
	assert(chooseNamePopup)

	-- act name
	local actGr = chooseNamePopup:find("act_name")
	assert(actGr)
	local actEditText = actGr:find("edit_box_group")
	assert(actEditText)
	local actName = actEditText.uc_input_string:toUtf8()
	local scenarioName = ""

	local scenarioEditText
	if self.createNewScenario == true then
		-- scenario name
		local scenarioGr = chooseNamePopup:find("scenario_name")
		assert(scenarioGr)

		scenarioEditText = scenarioGr:find("edit_box_group")
		assert(scenarioEditText)
		
		scenarioName = scenarioEditText.uc_input_string:toUtf8()
	end

	if (self.createNewScenario and scenarioName~="" and actName~="") or (not self.createNewScenario and actName~="") then
		chooseNamePopup.active = false
		disableModalWindow()

		self.currentScenario.name = scenarioName
		self.currentAct.name = actName

		if self.createNewLocation == true then
			local locationGr = actsUI:find("location")
			assert(locationGr)

			local scenarioActGr = actsUI:find("scenario_act")
			assert(scenarioActGr)

			scenarioActGr.active = false
			locationGr.active = true

			self:openEcosystemIslands("r2_desert")	
		else
			actsUI.active = false
			if self.createNewScenario==true then
				self:createScenario()
			else
				self:createAct()
			end
			setKeyboardContext("r2ed")
		end

	elseif self.createNewScenario and scenarioName==""then 
		scenarioEditText:setFocusOnText()

	elseif actName=="" then
		actEditText:setFocusOnText()	   
	end
end

--------------------------------------------------------------------------------------
--------------------------------- createScenario -------------------------------------
function r2.acts:createScenario()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	-- scenario/act/location notes
	local scenarioNotes, actNotes
	scenarioNotes = ""
	actNotes = ""
	
	r2.resetNameGiver()
	-- create scenario component
	local scenario= r2.newComponent("Scenario")
	scenario.Ghost_Name = self.currentScenario.name
	scenario.Description.ShortDescription = scenarioNotes
	if r2.Scenario and r2.Scenario.Language then
		scenario.Language = r2.Scenario.Language
	end
	scenario.AccessRules = self.currentScenario.rules
	scenario.Description.LevelId = self.currentScenario.level
	scenario.Description.CreationDate = os.date()
	scenario.Description.Creator = r2:getUserEntityName()
	scenario.Description.CreatorMD5 = r2.getCharIdMd5()


	
	-- create permanent act component
	local act =r2.newComponent("Act")
	local features = act.Features
	local tmpDefault = r2.newComponent("DefaultFeature")
	act.Name =  i18n.get("uiR2EDBaseAct"):toUtf8() 
	table.insert(features, tmpDefault)
	table.insert(scenario.Acts, act)

	-- create act 1 component
	act = r2.newComponent("Act")
	local features = act.Features
	local tmpDefault = r2.newComponent("DefaultFeature")
	r2.ActUIDisplayer.LastSelfCreatedActInstanceId = act.InstanceId
	--act.Name =  i18n.get("uiR2EDAct1"):toUtf8() .. ":" .. r2.currentAct.name
	act.Name =  self.currentAct.name
	act.WeatherValue = self.currentAct.weather
	local manualWeather = 0
	if self.currentAct.manualWeather == true then manualWeather = 1 end
	act.ManualWeather = manualWeather 
	act.ShortDescription = actNotes
	table.insert(features, tmpDefault)
	table.insert(scenario.Acts, act)

	-- create location
	local location = r2.newComponent("Location")
	location.Season = self.currentLocation.season
	location.IslandName = self.currentLocation.islandName  
	location.Name = self.currentLocation.name
	location.EntryPoint = self.currentLocation.entryPoint
	local manualSeason = 0
	if self.currentLocation.manualSeason == true then manualSeason = 1 end
	location.ManualSeason = manualSeason 
	table.insert(scenario.Locations, location)
	act.LocationId = location.InstanceId
	
	r2.requestCreateScenario(scenario)	
	r2:waitScenarioScreen()	

	self.deleteOldScenario = true
end

function r2.acts:createAct()

	r2.requestNewAction(i18n.get("uiR2EDNewActAction"))

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	-- act/location notes
	local actNotes = ""

	-- create act component
	local act =r2.newComponent("Act")
	local features = act.Features
	local tmpDefault = r2.newComponent("DefaultFeature")
	if (tmpDefault == nil) then
		debugWarning("Failed to create default feature");
		return
	end	
	r2.ActUIDisplayer.LastSelfCreatedActInstanceId = act.InstanceId
	local actNb = r2.Scenario.Acts.Size 
	--act.Name = i18n.get("uiR2EDDefaultActTitle"):toUtf8() .. actNb .. ":" .. r2.currentAct.name 
	act.Name = self.currentAct.name 
	act.WeatherValue = self.currentAct.weather
	local manualWeather = 0
	if self.currentAct.manualWeather == true then manualWeather = 1 end
	act.ManualWeather = manualWeather 
	act.ShortDescription = actNotes
	table.insert(features, tmpDefault)
	if (act == nil) then
		debugWarning("Failed to create additionnal 'Act'");
		return
	end	

	-- create location
	local location
	if self.createNewLocation then

		location = r2.newComponent("Location")
		location.Season = self.currentLocation.season
		location.IslandName = self.currentLocation.islandName  
		location.Name = self.currentLocation.name
		location.EntryPoint = self.currentLocation.entryPoint
		local manualSeason = 0
		if self.currentLocation.manualSeason == true then manualSeason = 1 end
		location.ManualSeason = manualSeason 
		act.LocationId = location.InstanceId

		r2.requestInsertNode(r2.Scenario.InstanceId, "Locations", -1, "", location)
	else
		act.LocationId = self.currentLocation.instanceId
	end

	r2.requestInsertNode(r2.Scenario.InstanceId, "Acts", -1, "", act)
end

--------------------------------------------------------------------------------------
--------------------------- choose location name popup -------------------------------
function r2.acts:openLocationName()

	local chooseNamePopup = getUI("ui:interface:r2ed_choose_location_name")
	assert(chooseNamePopup)

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	if not actsUI.active then return end

	local okButton = actsUI:find("island_ok")
	assert(okButton)

	enableModalWindow(okButton, chooseNamePopup.id)

	local editText = chooseNamePopup:find("edit_box_group")
	assert(editText)
	editText.uc_input_string = i18n.get(self.currentLocation.islandName)
	editText:setFocusOnText()
end

------------------------ SELECT BUTTON IN LIST --------------------------------
-- only one button can be selected --------------------------------------------
function r2.acts:selectButtonTemplate(buttonTemplate)

	local list = buttonTemplate.parent
	assert(list)

	for i=0,list.childrenNb-1 do
		local child = list:getChild(i)
		assert(child)
		child:find("button").active = true
		child:find("selected_button").active = false
	end

	buttonTemplate:find("button").active = false
	buttonTemplate:find("selected_button").active = true
end

function r2.acts:selectButton(button)

	local list = button.parent
	assert(list)

	for i=0,list.childrenNb-1 do
		local child = list:getChild(i)
		assert(child)
		child:find("button").pushed = false
	end

	button:find("button").pushed = true
end

--------------------------- CHOOSE LOCATION SEASON ---------------------------------------
function r2.acts:locationSeason(caller)

	local buttonTemplate
	if caller == nil then
		caller = getUICaller()
		buttonTemplate = caller.parent.parent
	else
		buttonTemplate = caller.parent	
	end

	self:selectButtonTemplate(buttonTemplate)

	local list = buttonTemplate.parent
	local seasons = {[0]="Spring", [1]="Summer", [2]="Autumn", [3]="Winter"}
	local seasonNb = list:getElementIndex(buttonTemplate)
	self.currentLocation.season = seasons[seasonNb]
end

----- MANUAL SEASON ACTIVATION -------------------------------------------------------
function r2.acts:manualSeason()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local manualButton = actsUI:find("manual_season_2"):find("toggle_butt")
	assert(manualButton)

	self.currentLocation.manualSeason = (manualButton.pushed==false)

	local seasonList = actsUI:find("season_list_2")
	assert(seasonList)

	seasonList.active = self.currentLocation.manualSeason
end

--- OPEN POPUP NAME FOR ACT/SCENARIO OR LOCATION -----------------------------------------
function r2.acts:openPopupName()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local createLocationGr = actsUI:find("location")
	assert(createLocationGr)
	local prevLocationGr = actsUI:find("previous_location")	
	assert(prevLocationGr)

	if prevLocationGr.active then
		self:createLocationOrCreateAct()
	else
		self:openLocationName()
	end
end


--- CANCEL ACT/SCENARIO CREATION --------------------------------------------------------
function r2.acts:cancelActCreation()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	actsUI.active =false

	setKeyboardContext("r2ed")
end


--------------------- update palette UI when change act ------------------------------
function r2.acts:updatePaletteFromEcosystem()

	local ecosystemTrees = 
				{
					["r2_desert"]	= "fytrees",
					["r2_forest"]	= "fotrees",
					["r2_jungle"]	= "jutrees",
					["r2_lakes"]	= "trtrees",
					["r2_roots"]	= "prtrees",
				}

	local currentAct = r2:getCurrentAct()
	assert(currentAct)

	local botObjectsPalette = {}
	if not r2.Palette.BotEntries then
		return  -- special case for the 'light' palette
	end

	botObjectsPalette = r2.Palette.BotEntries

	if not currentAct:isBaseAct() then
		local currentLocation = r2:getInstanceFromId(currentAct.LocationId)
		if currentLocation==nil then return end

		-- search for ecosystem of current location
		local islandEcosystem
		for ecoName, v in pairs(self.islands) do
			if self.islands[ecoName][self:getIslandNb(currentLocation.IslandName)].name==currentLocation.IslandName then
				islandEcosystem = ecoName
				break
			end
		end

		-- flag to display only vegetation of current ecosystem
		if islandEcosystem and ecosystemTrees[islandEcosystem] then
			for ecoName, v in pairs(ecosystemTrees) do
				botObjectsPalette[ecosystemTrees[ecoName]].Display = (ecoName==islandEcosystem)
			end
		end
	else
		for ecoName, v in pairs(ecosystemTrees) do
			botObjectsPalette[ecosystemTrees[ecoName]].Display = false
		end
	end

	r2:buildPaletteUI()	
end

----------------------------------------------------------------------------------------------
function r2.acts:locationIsland(caller)

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	local buttonTemplate
	if caller == nil then
		caller = getUICaller()
		buttonTemplate = caller.parent.parent
	else
		buttonTemplate = caller.parent	
	end

	if buttonTemplate.Env.Name == self.newLocation then

		-- select "create new location" mode
		buttonTemplate:find("button").active = false
		buttonTemplate:find("selected_button").active = true
		self.createNewLocation = true

		-- deselect all previous locations
		local prevLocationList = actsUI:find("location_list")
		assert(prevLocationList)
		for i=0,prevLocationList.childrenNb-1 do
			local child = prevLocationList:getChild(i)
			assert(child)
			child:find("button").active = true
			child:find("selected_button").active = false
		end
		self.currentLocation.islandName = ""
		self.currentLocation.intanceId = ""
	else
		-- select this previous location
		self:selectButtonTemplate(buttonTemplate.parent)
		self.currentLocation.instanceId = buttonTemplate.parent.Env.InstanceId

		-- deselect "create new location" mode
		local newLocationMode = actsUI:find("new_location_mode_2")
		assert(newLocationMode)
		newLocationMode:find("button").active = true
		newLocationMode:find("selected_button").active = false
		self.createNewLocation = false
	end
end

--------------------------------------------------------------------------------------
--------------------------- choose a name for scenario/act ---------------------------
function r2.acts:createLocationOrCreateAct()

	local actsUI = getUI("ui:interface:r2ed_acts")
	assert(actsUI)

	if self.createNewLocation then
		
		local createLocationGr = actsUI:find("location")
		assert(createLocationGr)
		local prevLocationGr = actsUI:find("previous_location")	
		assert(prevLocationGr)

		createLocationGr.active = true
		prevLocationGr.active = false

		self:openEcosystemIslands("r2_desert")
	else
		actsUI.active = false

		disableModalWindow()
		setKeyboardContext("r2ed")

		self:createAct()
	end
end
