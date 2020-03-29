
--init fake r2 so that LD and ET scenarios index files can be loaded


GameR2Loading = 
{	
	CurrentFile = "",
	CurrentPath = "",
	RingAccess = false,
	bypassRingAccess = true
}




function GameR2Loading:getLoadingWindow()
	return getUI("ui:interface:ring_scenario_loading_window")
end

function GameR2Loading:getFileName()
	return self:getLoadingWindow():find("Name")
end

function GameR2Loading:clear()
	GameR2Loading.CurrentFile = ""
	GameR2Loading.CurrentPath = ""
	GameR2Loading.RingAccess = false
	self:getFileName().hardtext = ""
	self:getLoadingWindow().active = false
	
	local window = self:getLoadingWindow()
	--this part of the code correspond to method "updateSize" for lua defined form
	local rollouts = window:find("rollouts")
	local deltaH = 40								
	window:invalidateCoords()
	window:updateCoords()
	local newHReal = rollouts.h_real				
	-- must resize the parent
	local newH = newHReal + deltaH
	local yOffset = newH - window.h
	--propertySheet.h = newH
	window.y = window.y + yOffset / 2 
	window.pop_min_h = newH 
	window.pop_max_h = newH 
	window:invalidateCoords()
	window:updateCoords()
end

-------------------------------------------------------
-- Setters for scenario information from header
-------------------------------------------------------
function GameR2Loading:setScenarioTitle(header)
	local win = self:getLoadingWindow()
	assert(win)
	local titleWidget = win:find("Title")
	--local titleWidget = getUI("ui:interface:r2ed_form_LoadScenario:content:enclosing:rollouts:0:prop_table:r_Title:widget_group:scenario_title")
	if header.Title then
		titleWidget.hardtext = header.Title
	else
		titleWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioDesc(header)
	local win = self:getLoadingWindow()
	assert(win)
	local descWidget = win:find("Description")
	if header.ShortDescription then
		descWidget.hardtext = header.ShortDescription
	else
		descWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioRules(header)
	local win = self:getLoadingWindow()
	assert(win)
	local rulesWidget = win:find("Rules")
	if header.Rules then
		rulesWidget.hardtext = header.Rules
	else
		rulesWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioLevel(header)
	local win = self:getLoadingWindow()
	assert(win)
	local lvlWidget = win:find("Level")
	if header.Level then
		lvlWidget.hardtext = header.Level
	else
		lvlWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioLanguage(header)
	local win = self:getLoadingWindow()
	assert(win)
	local lvlWidget = win:find("Language")
	if header.Language then
		local language = "uiR2ED"..header.Language
		lvlWidget.hardtext = i18n.get(language):toUtf8()
	else
		lvlWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioRingAccess(header)
	local win = self:getLoadingWindow()
	assert(win)
	local accessWidget = win:find("RingPointsLevel")
	--
	--Bypassed ring access while not properly working when entering the ring from ryzom
	if self.bypassRingAccess == true then
		accessWidget.active = false
		local accessWidgetTitle = win:find("RingPointsLevel_Caption")
		accessWidgetTitle.active = false	
		return
	end
	------------------------
	--------------------------

	local okButton = win:find("validate")

	if getDbProp("SERVER:USER:IS_NEWBIE") == 1 and isPlayerFreeTrial() and not header.OtherCharAccess or header.OtherCharAccess ~= "RoSOnly" then
		accessWidget.hardtext = i18n.get("uiR2EDLoadScenario_No"):toUtf8()
		okButton.frozen = true
	end

	if header.RingPointLevel then
		self.RingAccess = game.checkRingAccess(header.RingPointLevel)
		if self.RingAccess == true then

			accessWidget.hardtext = i18n.get("uiR2EDLoadScenario_Yes"):toUtf8()
		else
			accessWidget.hardtext = i18n.get("uiR2EDLoadScenario_No"):toUtf8()
		end
		
		okButton.frozen = not self.RingAccess
	else
		accessWidget.hardtext = ""
		okButton.frozen = false
	end
end

function GameR2Loading:setScenarioAuthor(header, rating)
	local win = self:getLoadingWindow()
	assert(win)
	local authorWidget = win:find("Author")
	--local ratingStr = " ("..tostring(rating)..")"
	if header.CreateBy then
		authorWidget.hardtext = header.CreateBy
	else
		authorWidget.hardtext = ""
	end
end


--RATINGS
function GameR2Loading:setScenarioFunRating(rating)
	local win = self:getLoadingWindow()
	assert(win)
	local funWidget = win:find("FunRating")
	if rating then
		funWidget.hardtext = tostring(rating)
	else
		funWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioDifficultyRating(rating)
	local win = self:getLoadingWindow()
	assert(win)
	local diffWidget = win:find("DifficultyRating")
	if rating then
		diffWidget.hardtext = tostring(rating)
	else
		diffWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioAccessibilityRating(rating)
	local win = self:getLoadingWindow()
	assert(win)
	local accessWidget = win:find("AccessibilityRating")
	if rating then
		accessWidget.hardtext = tostring(rating)
	else
		accessWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioOriginalityRating(rating)
	local win = self:getLoadingWindow()
	assert(win)
	local origWidget = win:find("OriginalityRating")
	if rating then
		origWidget.hardtext = tostring(rating)
	else
		origWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioDirectionRating(rating)
	local win = self:getLoadingWindow()
	assert(win)
	local directionWidget = win:find("DirectionRating")
	if rating then
		directionWidget.hardtext = tostring(rating)
	else
		directionWidget.hardtext = ""
	end
end

function GameR2Loading:setScenarioRRPTotal(rrpTotal)
	local win = self:getLoadingWindow()
	assert(win)
	local rrpWidget = win:find("RRPTotal")
	if rrpTotal then
		rrpWidget.hardtext = tostring(rrpTotal)
	else
		rrpWidget.hardtext = ""
	end
end


function GameR2Loading:initWindow()
	if not r2 then
		r2 = {}
	end

	local emptyHeader = {}
	self:setScenarioTitle(emptyHeader)
	self:setScenarioDesc(emptyHeader)
	self:setScenarioRules(emptyHeader)
	self:setScenarioLevel(emptyHeader)
	self:setScenarioLanguage(emptyHeader)
	self:setScenarioRingAccess(emptyHeader)
	self:setScenarioAuthor(emptyHeader, 0)

	--self:setScenarioFunRating()
	--self:setScenarioDifficultyRating()
	--self:setScenarioOriginalityRating()
	--self:setScenarioAccessibilityRating()
	--self:setScenarioDirectionRating()
	--self:setScenarioRRPTotal()
	
	self:getFileName().hardtext = ""

	
end

function GameR2Loading:setCurrSelectedFile(filename, path)
	local window = self:getLoadingWindow()
	assert(window)
	
	if getUICaller().pushed then
		getUICaller().pushed = true
	end
	
	--formInstance.LastFileButton = getUICaller()
	--r2.CurrentForm.Env.FormInstance.Name = filename
	--GameR2Loading.Env.FormInstance.Name = filename

	local formName = window:find("Name")
	formName.hardtext = filename
	GameR2Loading.CurrentFile = filename
	
	GameR2Loading.CurrentPath = path
		
	local header = game.getFileHeader(path..filename)

	--tmp
	local authorRating = 42
	--debugInfo("### CHECKING BODYMD5 ###")
	--if header and header.BodyMD5 then
	--	debugInfo("BodyMD5: *" ..tostring(header.BodyMD5))
	--	game.getScenarioAverageScores(tostring(header.BodyMD5))
	--end
	--debugInfo("### ****** ###")
	
	if header then
		self:setScenarioTitle(header)
		self:setScenarioAuthor(header, authorRating)
		self:setScenarioDesc(header)
		self:setScenarioRules(header)
		self:setScenarioLevel(header)
		self:setScenarioLanguage(header)
		self:setScenarioRingAccess(header)

		--self:setScenarioFunRating(42)
		--self:setScenarioDifficultyRating(43)
		--self:setScenarioOriginalityRating(44)
		--self:setScenarioAccessibilityRating(45)
		--self:setScenarioDirectionRating(46)
		--self:setScenarioRRPTotal(47)
	end
	
	--this part of the code corresponds to method "updateSize" for lua defined form
	local rollouts = window:find("rollouts")
	local deltaH = 40								
	window:invalidateCoords()
	window:updateCoords()
	local newHReal = rollouts.h_real				
	-- must resize the parent
	local newH = newHReal + deltaH
	local yOffset = newH - window.h
	--propertySheet.h = newH
	window.y = window.y + yOffset / 2 
	window.pop_min_h = newH 
	window.pop_max_h = newH 
	window:invalidateCoords()
	window:updateCoords()
end


function GameR2Loading:buildScenarioTree()
	
	local window = self:getLoadingWindow()
	if not window then
		debugInfo("GameR2Loading:buildScenarioTree: failed to get ui")
		return  
	end
	
	local tree = window:find('tree_list')
	assert(tree)

	local rootNode = SNode()
	rootNode.Id = "root"
	rootNode.Text = "Scenarios"
	rootNode.Opened = false 
	--rootNode.Bitmap = "r2ed_icon_macro_components.tga"
	tree:setRootNode(rootNode)

	function addNodeToParent(name, id, parentId)
		local parentNode = rootNode:getNodeFromId(parentId)
		
		addedNode = SNode()
		--local text = i18n.hasTranslation(parentCategory)
		--if not text then text = parentCategory else text = i18n.get(parentCategory) end
		addedNode.Text = name
		addedNode.Id = id
		addedNode.Opened = false
		parentNode:addChild(addedNode)
	end
	
	function parseDirectory(nodeId, searchPath)
		local destNode = rootNode:getNodeFromId(nodeId)
		local autosaveNode = rootNode:getNodeFromId("autosave")
		local files = getPathContent(searchPath)
		table.sort(files)
		for k, v in pairs(files) do
			if string.lower(nlfile.getExtension(v)) == "r2" then			
				local shortFilename = nlfile.getFilename(v)
				local shortFileEscape=string.gsub(shortFilename, "'", "\\'")				
				local fileLeaf= SNode()
				fileLeaf.Id = shortFilename
				local len = string.len(shortFilename) - 3
				fileLeaf.Text = string.sub(shortFilename,0, len)
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "GameR2Loading:setCurrSelectedFile('" .. shortFileEscape .. "', '"..searchPath.."')"				
				
				--if autosave file, plug it in the autosave branch
				if string.find(shortFilename, "autosave") ~= nil then 
					autosaveNode:addChild(fileLeaf)
				else
					destNode:addChild(fileLeaf)
				end
			end
		end
	end
	
	function loadLDScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_leveldesign_scenarios.lua")
		if fileListOk then
			fileListOk()
		end
		local files = r2.LevelDesignScenarios
		for parentId, fileEntries in pairs(files) do
			local parentNode = rootNode:getNodeFromId(parentId)
			if not parentNode then
				addNodeToParent(parentId, parentId, "root")
				parentNode = rootNode:getNodeFromId(parentId)
			end
			for k, v in pairs(fileEntries) do
				local fileLeaf= SNode()
				fileLeaf.Id = v[1]
				local len = string.len(v[1]) - 3
				fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "GameR2Loading:setCurrSelectedFile('" .. v[1] .. "', './Examples/')"				
				parentNode:addChild(fileLeaf)
			end
		end 
	end
	
	function loadETScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_event_team_scenarios.lua") 
		if fileListOk then
			fileListOk()
		end
		local files = r2.EventTeamScenarios
		for parentId, fileEntries in pairs(files) do
			local parentNode = rootNode:getNodeFromId(parentId)
			if not parentNode then
				addNodeToParent(parentId, parentId, "root")
				parentNode = rootNode:getNodeFromId(parentId)
			end
			for k, v in pairs(fileEntries) do
				local fileLeaf= SNode()
				fileLeaf.Id = v[1]
				local len = string.len(v[1]) - 3
				fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "GameR2Loading:setCurrSelectedFile('" .. v[1] .. "', './Examples/')"				
				parentNode:addChild(fileLeaf)
			end
		end 
	end

	function loadNewbieScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_newbieland_scenarios.lua")
		if fileListOk then
			fileListOk()
		end
		local files = r2.NewbielandScenarios
		if files then
			for parentId, fileEntries in pairs(files) do
				local parentNode = rootNode:getNodeFromId(parentId)
				if not parentNode then
					addNodeToParent(parentId, parentId, "root")
					parentNode = rootNode:getNodeFromId(parentId)
				end
				for k, v in pairs(fileEntries) do
					local fileLeaf= SNode()
					fileLeaf.Id = v[1]
					local len = string.len(v[1]) - 3
					fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
					fileLeaf.AHName = "lua"
					fileLeaf.AHParams = "GameR2Loading:setCurrSelectedFile('" .. v[1] .. "', './Examples/')"				
					parentNode:addChild(fileLeaf)
				end
			end
		end
	end
	
	--LEVELDESIGN SCENARIOS
	--addNodeToParent("Examples", "examples", "root")
	--addNodeToParent("Simple", "simple", "examples")
	loadLDScenariosFromFile()
	
	--EVENTTEAM SCENARIOS
	loadETScenariosFromFile()
	
	--NEWBIELAND SCENARIOS
	loadNewbieScenariosFromFile()

	--MY SCENARIOS
	addNodeToParent("My Files", "my_files", "root")
	addNodeToParent("Autosave", "autosave", "my_files")
	addNodeToParent("My scenarios", "my_scenarios", "my_files")
	parseDirectory("my_scenarios", "./My_scenarios/")

	--Tmp branch for old scenarios
	addNodeToParent("Old scenarios", "old", "root")
	parseDirectory("old", "./")

	tree:forceRebuild()
end



function GameR2Loading:displayLoadingWindow()
	
	self:initWindow()
	local isInNoobLand = getDbProp("SERVER:USER:IS_NEWBIE")
	if isInNoobLand == 1 and isPlayerFreeTrial() then
		self:buildNewbielandScenarioTree()
	else
		self:buildScenarioTree()
	end
	local window = self:getLoadingWindow()
	if not window then
		debugInfo("GameR2Loading:DisplayLoadingWindow: failed to get ui")
		return  
	end
	--this part of the code correspond to method "updateSize" for lua defined form
	local rollouts = window:find("rollouts")
	local deltaH = 40								
	window:invalidateCoords()
	window:updateCoords()
	local newHReal = rollouts.h_real				
	-- must resize the parent
	local newH = newHReal + deltaH
	local yOffset = newH - window.h
	--propertySheet.h = newH
	window.y = window.y + yOffset / 2 
	window.pop_min_h = newH 
	window.pop_max_h = newH 
	window:invalidateCoords()
	window:updateCoords()
	window.active = true

	local scenarioControlWnd = getUI("ui:interface:r2ed_scenario_control")
	if scenarioControlWnd.active then
		scenarioControlWnd.active = false
	end
end


function GameR2Loading:validateLoading()
							
	if GameR2Loading.CurrentFile == "" then
		messageBox(i18n.get("uiR2EDLoadScenario_InvalidFileName"))
		return
	end
	
	local filename = GameR2Loading.CurrentFile	
	
	if  string.find(filename, '.r2', -3) == nil then
		messageBox(i18n.get("uiR2EDLoadScenario_InvalidFileName"))
		return
	end
	
	if GameR2Loading.CurrentPath == "" then
		messageBox(i18n.get("uiR2EDLoadScenario_InvalidFileName"))
		return
	end
	local path = GameR2Loading.CurrentPath
	
	local extendedFileName = filename
	if path and path ~= "" then
		extendedFileName = path..filename
	end
	
	if getDbProp("SERVER:USER:IS_NEWBIE")==0 then
		getUI("ui:interface:r2ed_scenario_control").active=true
		runAH(nil, "scenario_information", "ScenarioName="..extendedFileName)
	else
		runAH(nil, "init_scenario_control", "")
		runAH(nil, "scenario_information", "ScenarioName="..extendedFileName)
		runAH(nil, "load_scenario", "")
	end
	self:getLoadingWindow().active = false
	self:clear()

end

function GameR2Loading:cancelLoading()
	self:getLoadingWindow().active = false
	self:clear()
	runAH(nil, "open_ring_sessions", "")
end

function GameR2Loading:getTreeTooltip()
	local tree = getUI("ui:interface:ring_scenario_loading_window"):find('tree_list')

	local isInNoobLand = getDbProp("SERVER:USER:IS_NEWBIE")
	if isInNoobLand == 1 and isPlayerFreeTrial() then
		tree.tooltip = i18n.get("uiR2EdPropertyToolTip_LoadScenario_LoadScenario_NewbieTreeInfo"):toUtf8()
	else
		tree.tooltip = i18n.get("uiR2EdPropertyToolTip_LoadScenario_LoadScenario_TreeInfo"):toUtf8()
	end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
------ NEWBIELAND LOADING WINDOW
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------


function GameR2Loading:buildNewbielandScenarioTree()
	
	local window = self:getLoadingWindow()
	if not window then
		debugInfo("GameR2Loading:buildScenarioTree: failed to get ui")
		return  
	end
	
	local tree = window:find('tree_list')
	assert(tree)

	local rootNode = SNode()
	rootNode.Id = "root"
	rootNode.Text = "Scenarios"
	rootNode.Opened = false 
	--rootNode.Bitmap = "r2ed_icon_macro_components.tga"
	tree:setRootNode(rootNode)

	function addNodeToParent(name, id, parentId)
		local parentNode = rootNode:getNodeFromId(parentId)
		
		addedNode = SNode()
		--local text = i18n.hasTranslation(parentCategory)
		--if not text then text = parentCategory else text = i18n.get(parentCategory) end
		addedNode.Text = name
		addedNode.Id = id
		addedNode.Opened = false
		parentNode:addChild(addedNode)
	end
		
	function loadNewbieScenariosFromFile()
		--if using a bnp, use doFile method instead
		local fileListOk = loadfile("examples/r2_newbieland_scenarios.lua")
		if fileListOk then
			fileListOk()
		end
		local files = r2.NewbielandScenarios
		for parentId, fileEntries in pairs(files) do
			local parentNode = rootNode:getNodeFromId(parentId)
			if not parentNode then
				addNodeToParent(parentId, parentId, "root")
				parentNode = rootNode:getNodeFromId(parentId)
			end
			for k, v in pairs(fileEntries) do
				local fileLeaf= SNode()
				fileLeaf.Id = v[1]
				local len = string.len(v[1]) - 3
				fileLeaf.Text = string.gsub(string.sub(v[1],0, len), "_", " ")
				fileLeaf.AHName = "lua"
				fileLeaf.AHParams = "GameR2Loading:setCurrSelectedFile('" .. v[1] .. "', './Examples/')"				
				parentNode:addChild(fileLeaf)
			end
		end 
	end
	
	
	--NEWBIELAND SCENARIOS
	loadNewbieScenariosFromFile()

	tree:forceRebuild()
end

