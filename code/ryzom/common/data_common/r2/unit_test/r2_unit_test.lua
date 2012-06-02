--BaseY = -32594
BaseX = 21570
BaseY = -1363

UnitTest = {}


UnitTest.testAddSecondaryActUi = function()
	function Form_onOk(form)
		local act =r2.newComponent("Act")
		local features = act.Features
		local tmpDefault = r2.newComponent("DefaultFeature")
		table.insert(features, tmpDefault)
		r2.requestInsertNode(r2.ScenarioInstanceId, "Acts", -1, "", act)
	end

	function Form_onCancel(form)
	
	end
	r2:doForm("CreateNewAdventureActDescription", {}, Form_onOk, Form_onCancel)
end


UnitTest.testLoadAnimationScenarioUi = function()
	
	local function onOk(form)
		if (form.LoadScenario_Name ~= nil and type(form.LoadScenario_Name) == "string") then						
			if form.LoadScenario_Name == "" then
				local ui = r2:getForm("LoadScenario")
				ui.active = true		
				ui.Env.Choice = nil
				r2.CurrentForm = ui 

				messageBox(i18n.get("uiR2EDInvalidName"))
				return
			end

			local filename = form.LoadScenario_Name	
			if  string.find(filename, '\.r2', -3) == nil then

				local ui = r2:getForm("LoadScenario")
				ui.active = true		
				ui.Env.Choice = nil
				r2.CurrentForm = ui 

				messageBox(i18n.get("uiR2EDInvalidName"))
				return
			end
			filename = form.Path .. filename



			local header = r2.getFileHeader(filename)
			local mainland  = not header or header.NevraxScenario ~= "1" or header.TrialAllowed ~= "1" 
			local trial = isPlayerFreeTrial() 
			if mainland and getDbProp("SERVER:USER:IS_NEWBIE") == 1 and trial then
				local ui = r2:getForm("LoadScenario")
				ui.active = true		
				ui.Env.Choice = nil
				r2.CurrentForm = ui 
				r2.onSystemMessageReceived("ERR", "", "uiRingLoadingNotARoSScenario")
				return
			end

	
			local ok, errStr = r2.RingAccess.loadAnimation(filename)
			if not ok then
				local ui = r2:getForm("LoadScenario")
				ui.active = true		
				ui.Env.Choice = nil
				r2.CurrentForm = ui 

				messageBox(i18n.get("uiR2EDInvalidScenario"))

			end

		end		
	end

	local function onCancel(data, form)
		local ui = r2:getForm("LoadScenario")
		ui.active = true
		ui.Env.Choice = nil
		r2.CurrentForm = ui 

	end
	
	local str = r2:getStartingAnimationFilename();
	local ok, errStr = r2.RingAccess.loadAnimation(str)

	if ok and string.len(str) ~= 0 then
		local header = r2.getFileHeader(str)
		local mainland  = not header or header.NevraxScenario ~= "1" or header.TrialAllowed ~= "1" 
		local newbie = getDbProp("SERVER:USER:IS_NEWBIE") == 1
		if mainland and isPlayerFreeTrial() and newbie then
			r2.onSystemMessageReceived("ERR", "", "uiRingLoadingNotARoSScenario")
			ok = false
		end
	end

	if not ok then
		r2:doForm("LoadScenario", {}, onOk, onCancel)
		if string.len(str) ~= 0 then
			messageBox(errStr)
		end

	end

end


UnitTest.testWaitAnimationScenarioLoadingUi = function()
	
--	local ui = getUI("ui:interface:r2ed_animation_waiting")
--	ui.active = true
---	setTopWindow(ui)
		
end


UnitTest.testLoadScenarioUi = function()
	
	local function onCancel(data, form)
	end

	local function onOk(form)

		if (form.LoadScenario_Name ~= nil and type(form.LoadScenario_Name) == "string") then						
			if form.LoadScenario_Name == "" then
				messageBox(i18n.get("uiR2EDLoadScenario_InvalidFileName"))
				return
			end

			local ucName = ucstring()
			ucName:fromUtf8(form.LoadScenario_Name)
			local filename = tostring(ucName)	
			if string.find(filename, '\.r2', -3) == nil then
				messageBox(i18n.get("uiR2EDLoadScenario_InvalidFileName"))
				return
			end

--			if  string.find(filename, '\.r2', -3) == nil then
--				filename = form.Name .. ".r2"
--			end
			
			local path = form.Path
			local extendedFileName = filename
			if path and path ~= "" then
				extendedFileName = path..filename
			end

			if form.LoadScenario_Level == "" 
					and form.LoadScenario_Rules == ""  
					and  config.R2EDExtendedDebug == 1 then
				messageBox(i18n.get("uiR2EDLoadScenario_LoadScenarioWithoutHeader"))
			elseif form.RingAccess and form.RingAccess == 0 then	
				local ui = r2:getForm("LoadScenario")
				ui.active = true		
				r2.CurrentForm = ui 
				messageBox(i18n.get("uiR2EDLoadScenario_No"))
				return
			end



			local header = r2.getFileHeader(extendedFileName)
			if header then
				local locked = header and header.OtherCharAccess == "RunOnly"

				if locked and header.ModifierMD5 and string.len(header.ModifierMD5) > 0 then
					if not r2.hasCharacterSameCharacterIdMd5(header.ModifierMD5) then
						if config.R2EDExtendedDebug ~=1  then
							r2.onSystemMessageReceived("ERR", "", "uiR2EDLoadingLockedScenario")
						else
							local ui = r2:getForm("LoadScenario")
							ui.active = true		
							r2.CurrentForm = ui
							r2.onSystemMessageReceived("ERR", "", "uiR2EDLoadingLockedScenarioOverride")
							return
						end
					else

					end
				end
			end

			local ok = r2.load(extendedFileName)
			if not ok then
				messageBox(i18n.get("uiR2EDInvalidScenario"))
			else
				r2.acts.deleteOldScenario = true
			end				
		end		
	end

	
	
	r2:doForm("LoadScenario", {}, onOk, onCancel)
end

function UnitTest.saveScenario(name, overwrite)

	if (name ~= nil and type(name) == "string")
	then			
		if name == "" then
			messageBox(i18n.get("uiR2EDInvalidName"))
			return
		end			
		if  string.find(name, '\.r2', -3) == nil then
			name = name .. ".r2"
		end
		-- update scenario name with the new name
		if name ~= r2.Scenario.Ghost_Name then
			r2.requestNewAction(i18n.get("uiR2EDChangeScenarioName"))
			r2.requestSetNode(r2.Scenario.InstanceId, "Ghost_Name", nlfile.getFilename(name))
			r2.requestEndAction() -- force a flush
		end
		local extendedFilename = r2.getScenarioSavePath()..name

		--
		if overwrite ~= true then
			local file = io.open(extendedFilename, "r")
			if file ~= nil then
				io.close(file)		
				validMessageBox(concatUCString(ucstring(name), i18n.get("uiR2EDConfirmOverWrite")), "lua", "UnitTest.saveScenario('" .. name .. "', true )", "", "", "ui:interface")
				return
			end
		end
		
		local ok, ret = pcall(r2.Version.save, extendedFilename)		
		local errorMsg = ret
		if ok and ret then				
			displaySystemInfo(concatUCString(i18n.get("uiR2EDSaveSuccessFull"), ucstring(name)), "BC")
		else
			displaySystemInfo(concatUCString(i18n.get("uiR2EDSaveFailure"), ucstring(name)), "BC")				
		end
	end
end

UnitTest.testSaveScenarioUi = function()
	local function onOk(form)
		local smallName = form.Name
		uc=ucstring()
		uc:fromUtf8(smallName)
		local str = uc:toString()
		UnitTest.saveScenario(str, false)		
	end

	local function onCancel(form)
	
	end
	r2:doForm("SaveScenario", {}, onOk, onCancel)
end


UnitTest.tesConnectAdventure = function()
	local function onOk(form)
		if (form.AdventureId ~= nil and type(form.AdventureId) == "number")
		then			
			r2.requestMapConnection(form.AdventureId)
		else
			debugInfo("R2Lua can't connect adventure")
			
		end
	end

	local function onCancel(form)
	
	end
	r2:doForm("ConnectAdventure", {}, onOk, onCancel)
end


UnitTest.createRoad = function(x,y,name)
	local road = r2.newComponent("Road")
	
		local function wp(x, y, z)
			local wayPoint = r2.newComponent("WayPoint")
			local pos = r2.newComponent("Position")
			pos.x = x
			pos.y = y
			pos.z = z
			wayPoint.Position = pos
			return wayPoint
		end
		road.Base = "palette.geom.road"
		road.Name = name
		local tmpPositions = road.Points
		table.insert(tmpPositions, wp(x - 5, y, 0))
		table.insert(tmpPositions, wp(x + 5,  y, 0))
		table.insert(tmpPositions, wp(x, y - 5, 0))
	

	return road
end


--- UserComponent	


r2.getUserComponentSourceExtension = function () return "lua" end
r2.getUserComponentExtension  = function () return "lua" end
r2.getUserComponentExamplesDirectory= function () return "examples/user_components" end
r2.getUserComponentBinairyDirectory = function () return "ring_features" end

r2.UserComponentsManager = {}


local UserComponentsManager = r2.UserComponentsManager




function UserComponentsManager:newPackage(identifiant)
	local package = {}
	package.Name = "UnnamedPackage"
	package.Description = "Undescribed Package"
	package.Components = {}
		
	if (not identifiant) then
		package.GlobalNamespace = true
	else
		package.FileInfo  = clone(UserComponentsManager.CurrentFileInfo)
		
	end

	function package:fullname(name)
		if package.GlobalNamespace then
			return name
		else			
			return name .. "_" .. package.MD5
		end
	end

	function package:newComponent(name)
		if not package.GlobalNamespace then
			if r2.Scenario.UserComponents.UserComponents[self.FileInfo.Md5]  then
				r2.requestInsertNode(r2.Scenario.InstanceId, "UserComponents", -1, self.Md5, self.FileInfo.Package)
			end
		end
		return r2.newComponent(fullname(name))
	end

	function package:getUniqId()
		return self:fullname(self.Name)
	end
			
	return clone(package)
	
end

function UserComponentsManager:newComponent()
	local component = {
		BaseClass="LogicEntity",
		Name = "Unknown",
		Menu="ui:interface:r2ed_feature_menu", 
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		Text = "uiR2EDtooltipCreateUnkownComponent",
		Tooltip = "uiR2EDtooltipCreateUnkownComponent",
		Icon = "r2ed_feature_timer.tga",
		IconOver = "r2ed_feature_timer_over.tga",
		IconPushed= "r2ed_feature_timer_pushed.tga",
		Parameters = {},
		ApplicableActions = {},
		Events = {},
		Conditions = {},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		Prop = 	{ }
	}
	function component:registerMenu(logicEntityMenu)
		logicEntityMenu:addLine(ucstring(self.Name), "lua", "", self.Name)
	end
	
	function component:createImpl()

		if not r2:checkAiQuota() then return end


		component:create()
	end

	function component:create()
	end

	-----------------------------------------------------------------------------------------------		
	-- from base class
	component.getParentTreeNode = function(this)
		return this:getFeatureParentTreeNode()
	end
	---------------------------------------------------------------------------------------------------------
	-- from base class			
	component.appendInstancesByType = function(this, destTable, kind)
		assert(type(kind) == "string")
		--this:delegate():appendInstancesByType(destTable, kind)
		r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
		for k, component in specPairs(this.Components) do
			component:appendInstancesByType(destTable, kind)
		end
	end
	---------------------------------------------------------------------------------------------------------
	-- from base class
	component.getSelectBarSons = function(this)
		return Components
	end
	---------------------------------------------------------------------------------------------------------
	-- from base class		
	component.canHaveSelectBarSons = function(this)
		return false;
	end
	
	function component:onPostCreate() end	

	function component:translator(context)
		r2.Translator.createAiGroup(self, context)
	end
		
	function component:pretranslate(context)
		r2.Translator.translateAiGroup(self, context)
	end

	return clone(component)
end

function UserComponentsManager:addUserComponentIntoPalette(componentId, package, component)

	local  theText, theTooltip, theIcon, theIconOver, theIconPushed = component.Text, component.Tooltip, component.Icon, component.IconOver, component.IconPushed
	if not theText then theText = "uiR2EDtooltipCreateUnkownComponent" end
	if not theTooltip then theTooltip = "uiR2EDtooltipCreateUnkownComponent" end
	if not theIcon then theIcon = "r2ed_feature_timer.tga" end
	if not theIconOver then theIconOver = "r2ed_feature_timer_over.tga" end
	if not theIconPushed then theIconPushed = "r2ed_feature_timer_pushed.tga" end

	local palette = getUI("ui:interface:r2ed_palette")
	assert(palette)
	local menu = palette:find("sbtree_features")
	assert(menu)	
	menu = menu:find("feature_list")
	assert(menu)

	local theParent = "parent"
	
	if componentId  == 1 then
		theParent = "parent"
	else
		theParent = "r2_user_component" .. tostring(componentId - 1)
	end
	
	local instance = createGroupInstance("named_r2ed_tool", menu.id , { 
		id="r2_user_component"..tostring(componentId),
		tooltip=theTooltip,
		onclick_l="lua",
		posparent="parent",
		posref="TL TL", x=tostring(componentId * 4), 
		y=tostring(1 + componentId * -64), 
		icon = theIcon,
		icon_over = theIconOver,
		icon_pushed = theIconPushed,
		text=theText,
		params_l="r2.UserComponentsManager.Packages['".. package:getUniqId() .. "'].Components['".. component.Name.."']:create()"
		})
		debugInfo("r2.UserComponentsManager.Packages['".. package:getUniqId() .. "'].Components['".. component.Name.."']:createImpl()")
	assert(instance)	
	menu:addGroup(instance)

end


function UserComponentsManager:updateUserComponentsUi()
	local index = 0

	local k, package = next(self.Packages)
	while k do
		local kC, component = next(package.Components)
		while kC do
			UserComponentsManager:addUserComponentIntoPalette(index, package, component)
			index = index + 1

			kC, component = next(package.Components, kC)				
		end

		k, package = next(self.Packages, k)
	end
end


function UserComponentsManager:updateUserComponents()
	local index = 0
	local k, package = next(self.Packages)
	while k do
		
		local kC, component = next(package.Components)
		while kC do
			r2.registerComponent(component, package)
			kC, component = next(package.Components, kC)				
		end

		k, package = next(self.Packages, k)
	end
	self:updateUserComponentsUi();
end


function UserComponentsManager:registerPackageComponents(package)
	local kC, component = next(package.Components)
	while kC do
		r2.registerComponent(component, package)
		kC, component = next(package.Components, kC)				
	end

end


function UserComponentsManager:addUserComponent(filename)
	local package = self:loadPackage(filename)
	assert(package)
	self.Packages[package:getUniqId()] = package
	self:registerPackageComponents(package)
	UserComponentsManager:updateUserComponentsUi()

end

function UserComponentsManager:registerFileInfo(fileinfo)
	self.CurrentFileInfo = fileinfo
end


function UserComponentsManager:loadPackage(filename)

	self.CurrentPackage = nil
	self.CurrentFileInfo = nil




	local str = r2.readUserComponentFile(filename)
	if not str then
		debugInfo("Error in package '" .. filename .. "'")
		return nil
	end

	local f, msg = loadstring(str)
	if not f then
		debugInfo("Syntax error in package '" .. filename .. "': '" .. msg .. "'")
		return nil
	end

	local ok, msg = pcall(f)
	if not ok then 
		debugInfo("Loading error in package '" .. filename .. "': '" .. msg .. "'")
		return nil
	end

	if not self.CurrentPackage then
		debugInfo("No Package information in '"..filename.."'")
		return nil
	end

	r2.updateUserComponentsInfo(filename, self.CurrentPackage.Name, self.CurrentPackage.Description, 
		self.CurrentFileInfo.TimeStamp, self.CurrentFileInfo.MD5)
	self.CurrentPackage.fileinfo = clone(self.CurrentFileInfo)
	return self.CurrentPackage

end

function UserComponentsManager:init()
	self.UserComponentDirectory = "./ring_features"
	self.Packages = {}
	self.CurrentPackage = nil
	-- r2.requestSetNode(r2.Scenario.InstanceId, "UserComponents", {}) -- TODO to remove
	self.Instanece = self	
end


function UserComponentsManager:addUserComponentIntoScenario(filename)
	local package= self:loadPackage(filename)
	if not package then return end
	local uniqId = package:getUniqId()
-- TODO
	if r2.Scenario.UserComponents[ uniqId ] then
		debugInfo("The User defined Component '".. package.Name .. "' is already present in the current scenario.")
		return
	else
		r2.requestInsertNode(r2.Scenario.InstanceId, "UserComponents", -1, uniqId, uniqId)
		-- TODO: requestUpdateComponent
		--		UserComponentsManager:addUserComponent(filename)
		r2.registerUserComponent(filename)
	end	
end


function UserComponentsManager:addUserComponentUi()
end


function UserComponentsManager:test()
--	UserComponentsManager:addUserComponent("./ring_features/r2_features_zone_triggers.lua.r2c")
end


function UserComponentsManager:registerPackage(package)
	assert(package)
	self.CurrentPackage = package
end

function UserComponentsManager:registerComponent(package, component)
	if not package then return end
	if not component then return end
	package.Components[package:fullname(component.Name)] = component
end

function UserComponentsManager:compileUserComponent(filename)
	local component = self:loadPackage(filename)
	if not component then
		debugInfo("Can not compile user component.")
		return
	end
	component = nil
	r2.saveUserComponentFile(filename, true)
end




UserComponentsManager:init()

function UserComponentsManager:createUserComponentUi(otherDirectory)
	local directory = otherDirectory
	if (directory == nil) then  directory= r2.getUserComponentExamplesDirectory() end
	local extension = r2.getUserComponentSourceExtension()
	
	local function onOk(form)
		local filename = directory .."/".. form.Name
		self:compileUserComponent(filename)
	end
	local function onCancel() end

	debugInfo(extension)
	r2:doForm("LoadUserComponent", {Directory=directory, Extension=extension}, onOk, onCancel)
end


function UserComponentsManager:importUserComponentUi(otherDirectory)

	local directory = otherDirectory
	if (directory == nil) then  directory= r2.getUserComponentBinairyDirectory() end
	local extension = r2.getUserComponentExtension()
	
	local function onOk(form)
		local filename = directory.."/".. form.Name
		UserComponentsManager:addUserComponentIntoScenario(filename)
	end
	local function onCancel() end
	r2:doForm("LoadUserComponent", {Directory=self.UserComponentDirectory, Extension=extension}, onOk, onCancel)
	
end


-----------------------------------------------------------------------------------------------------------------------------------------
-- Ring Access
-- 
r2.RingAccess = { }

local RingAccess = r2.RingAccess



function RingAccess.testAccess(access)
	-- always true in local mode
	if config.Local == 1 then
		return true
	end
	--
	local t = string.sub(access, 1, 1)
	local v = tonumber(string.sub(access, 2))

	local charAccessMap = r2.getRingAccessAsMap(r2.getCharacterRingAccess())
	if charAccessMap[t] == nil then return false end
	if charAccessMap[t] < v then return false end
	return true

end

function RingAccess.updateLevel(access, level, allowed)
	if string.len(access) == 0 then 
		return true, level
	end

	local t = string.sub(access, 1, 1)
	local v = tonumber(string.sub(access, 2, string.len(access)))
	if level[t] == nil or level[t] < v then
		level[t] = v		
	end
	if allowed[t] == nil or allowed[t] < v then
		return false, level 
	end
	return true, level
end

function RingAccess.errorMessageIsland(islandName, accessWanted, accessCharacter)
	local accessWantedMap =  r2.getRingAccessAsMap(accessWanted)
	local accessCharacterMap = accessCharacter

	local k, v = next(accessWantedMap, nil);
	assert( k)
	local charLevel = accessCharacterMap[k]
	if charLevel == nil then charLevel = 0 end

	return RingAccess.errorMessageImpl("InvalidIslandLevel", islandName, k, v, charLevel) 
	
end


function RingAccess.errorMessageBot(entityName, accessWanted, accessCharacter)
	local accessWantedMap =  r2.getRingAccessAsMap(accessWanted)
	local accessCharacterMap = accessCharacter

	local k, v = next(accessWantedMap, nil);
	assert( k)
	local charLevel = accessCharacterMap[k]
	if charLevel == nil then charLevel = 0 end

	return RingAccess.errorMessageImpl("InvalidBotLevel", entityName, k, v, charLevel) 
	
end


function RingAccess.errorMessageGeneric( accessWanted, accessCharacter)
	local accessWantedMap =  r2.getRingAccessAsMap(accessWanted)
	local accessCharacterMap = accessCharacter

	local k, v = next(accessWantedMap, nil);
	assert( k)
	local charLevel = accessCharacterMap[k]
	if charLevel == nil then charLevel = 0 end

	return RingAccess.errorMessageImpl("InvalidLevel", nil, k, v, charLevel) 
	
end


function RingAccess.errorMessageImpl(errorType, name, package, entityLevel, charLevel)

	local trad
	local entityName

	if errorType == "InvalidIslandLevel" then
		trad =  i18n.get("uiR2EDErrMessageNoEnoughRingPointFor".."Island")
		entityName = i18n.get(name):toUtf8()
	elseif errorType == "InvalidBotLevel" then
		trad =  i18n.get("uiR2EDErrMessageNoEnoughRingPointFor".."Bot")
		entityName = name
	elseif errorType =="InvalidLevel" then
		trad =  i18n.get("uiR2EDErrMessageNoEnoughRingPointFor".."Generic")
	else
		trad =  i18n.get("uiR2EDErrMessageDataCorrupted")
		package = nil
		entityLevel = nil
		chatLevel = nil
		entityName = nil
	end



	local str = trad:toUtf8()

	if package  and string.len(package) ~= 0 then
		local category = i18n.get(string.format("uiR2EDRingAccessCategory_%s", package))
		str=string.gsub (str, "<Package>", category:toUtf8())
	end
	if entityLevel then
		str=string.gsub (str, "<EntityLevel>", tostring(entityLevel))
	end
	if charLevel then
		str=string.gsub (str, "<CharLevel>", tostring(charLevel))
	end
	if entityName then
		str=string.gsub (str, "<EntityName>", entityName)
	end

	local err = {}
	err.Type = errorType
	err.Name = name
	err.Package = package
	err.CharLevel = charLevel
	err.EntityLevel = entityLevel
	err.What = str
	return err

end

function RingAccess.dumpRingAccessLocations(level, allowed, ok)
	local k1, v1 = next(r2.Scenario.Locations, nil)
	while k1 do

		local access = r2.getIslandRingAccess(v1.IslandName)
		local map = r2.getRingAccessAsMap(access)
		r2.print(access)

		local k, v = next(map, nil)
		if k then
			if not level[k] then level[k] = {} end
			if not level[k][v] then level[k][v] = {} end
			table.insert(level[k][v], { Ok=test, Type="Island", Name=v1.IslandName} )
		end


		 k1, v1 = next(r2.Scenario.Locations, k1)
	end
	return ok, level
end


function RingAccess.dumpRingAccessEntityRec(node, level, allowed, ok, err )		
	local isTree = false
	
	if type(node) == "table" then
		isTree = true
	elseif type(node) == "userdata" then
		local mt = getmetatable(node)
		if mt~= nil and mt.__next ~= nil then
			isTree = true
		end			
	end
	
	if isTree then
		if node.Class ~= nil then
			local access = ""
			local access2 = nil
			local type = "Unknown"
			if node.isKindOf and node:isKindOf("Npc") then
				access2 = node.RingAccess
				local sheet = ""
				if node.Sheet then sheet = node.Sheet end
				access = r2.getSheetRingAccess(node.SheetClient, sheet)
				type = "Bot"

				
			elseif node.RingAccess then
				local access = node.RingAccess
				type = "Component"
				

			end

			local map = r2.getRingAccessAsMap(access)
			local k, v = next(map, nil)
			if k then
				if not level[k] then level[k] = {} end
				if not level[k][v] then level[k][v] = {} end
				local obj = { Ok=test, Type=type, Name=node.Name, PaletteId=access2}
				table.insert(level[k][v], obj)
			end
		end

		local t = nil

		if node.Components then t = node.Components end
		if node.Acts then t = node.Acts end
		if node.Features then t = node.Features end

		if t then	
			local k, v = next(t, nil)
			while k do
				ok, level, err = RingAccess.dumpRingAccessEntityRec(v, level, allowed, ok, err)
				k, v = next(t, k)
			end
		end	
	end
	return ok, level, err
end

function RingAccess.dumpRingAccess()
	local level = {} -- will be filled

	local allowed = r2.getRingAccessAsMap(r2.getCharacterRingAccess())

	local ok = true
	local err= {}
	err.What = ""
	ok, level = RingAccess.dumpRingAccessLocations(level, allowed, ok, err)
	ok, level, err = RingAccess.dumpRingAccessEntityRec(r2.Scenario, level, allowed, ok, err)

	RingAccess.saveRingAccessLevel(level)
	return ok, level, err
end

function RingAccess.saveRingAccessLevel(data)
	local function dumpRec(item, f)
		local k,v = next(item)
		while k do
			f:write(k.."='"..v.."', ") 
			k,v = next(item, k)
		end
	end

	local filename = "scenario_level.txt"
	local f = io.open(filename, "w")
	assert(f)
	f:write("-- "..filename.." ".. os.date() .."'\n\n")
	f:write("r2.ScenarioRingAccess = {\n\n")


	local catId, cat = next(data, nil)
	while catId do
		f:write("\t"..catId.."= {\n")
		local levelId, level = next(cat, nil)
		while levelId do
			f:write("\t\t"..levelId.."= {\n")
			local itemId, item = next(level, nil)
			while itemId do
				f:write("\t\t\t{");
				dumpRec(item,f)
				f:write("}\n");
				itemId, item = next(level, itemId)
			end
			f:write("\t\t}\n")
			levelId, level = next(cat, levelId)
		end
		f:write("\t}\n")
		catId, cat = next(data, catId)
	end
	f:write("}\n")
	f:flush()
	f:close()

	messageBox("Ring access has been saved in \""..filename.."\"")


end

function RingAccess.verifyLocations(level, allowed, ok, err)
	local k, v = next(r2.Scenario.Locations, nil)
	while k do

		local access = r2.getIslandRingAccess(v.IslandName)
		local test, level = RingAccess.updateLevel(access, level, allowed)
		if not test then
			if ok then 
				ok = false
				err = RingAccess.errorMessageIsland(v.IslandName, access, allowed)
			end
		end
		 k, v = next(r2.Scenario.Locations, k)
	end	
	return ok, level, err
end

function RingAccess.verifyEntityRec(node, level, allowed, ok, err )		
	local isTree = false
	
	if type(node) == "table" then
		isTree = true
	elseif type(node) == "userdata" then
		local mt = getmetatable(node)
		if mt~= nil and mt.__next ~= nil then
			isTree = true
		end			
	end
	
	if isTree then
		if node.Class ~= nil then
			if node.isKindOf and node:isKindOf("Npc") then
				local access = node.RingAccess
				local test, level = RingAccess.updateLevel(access, level, allowed)
				local sheet = ""
				if node.Sheet then sheet = node.Sheet end
				local access2 = r2.getSheetRingAccess(node.SheetClient, sheet)
				local test2, level = RingAccess.updateLevel(access2, level, allowed)
				if not test2  then 
					test = false
					access = access2
				end

				if not test and ok then
					ok = false
					err = RingAccess.errorMessageBot(node.Name, access, allowed)
				end

				
			elseif node.RingAccess then
				local test = RingAccess.updateLevel(node.RingAccess, level, allowed)
				if not test and  ok then
					ok = false
					err = RingAccess.errorMessageGeneric(node.access, allowed)
				end

			end
		end

		local t = nil

		if node.Components then t = node.Components end
		if node.Acts then t = node.Acts end
		if node.Features then t = node.Features end

		if t then	
			local k, v = next(t, nil)
			while k do
				ok, level, err = RingAccess.verifyEntityRec(v, level, allowed, ok, err)
				k, v = next(t, k)
			end
		end	
	end
	return ok, level, err
end

function RingAccess.verifyScenario()
	
	local level = {} -- will be filled

	local allowed = r2.getRingAccessAsMap(r2.getCharacterRingAccess())

	local ok = true
	local err= {}
	err.What = ""
	ok, level, err = RingAccess.verifyLocations(level, allowed, ok, err)
	ok, level, err = RingAccess.verifyEntityRec(r2.Scenario, level, allowed, ok, err)

	return ok, level, err
end

function RingAccess.verifyRtScenario(rtScenario)
	local ok, err = r2.verifyRtScenario(rtScenario)
	if not ok then
		err = RingAccess.errorMessageImpl(err.Type, err.EntityName, err.Package, err.EntityLevel, err.CharLevel)
	end
	return ok, err
end

function RingAccess.getAccessListAsString(list)
	local ret = ''
	local k,v = next(list)
	while k do
		if string.len(ret) ~= 0  then ret = ret ..':' end
		ret = ret .. k .. tostring(v)
		k, v= next(list, k)
	end
	return ret
end

function RingAccess.createAccessList()
	assert( config.R2EDExtendedDebug == 1) 
	function createAccessListRec(node, f)	
		if r2.isTable(node) then
			-- each palette entry			
			if node.Id ~= nil then
				local base = r2.getPaletteElement(node.Id)
				local access = r2.getPropertyValue(base, "RingAccess")
				local sheet =  r2.getPropertyValue(base, "Sheet")
				local sheetClient =  r2.getPropertyValue(base, "SheetClient")
				if not sheet then sheet = "" end
				if not sheetClient then sheetClient = "" end

				f:write(string.format("  <entityAccess name=\"%s\" package=\"%s\" sheetClient=\"%s\" sheetServer=\"%s\"/>\n", node.Id, access, sheetClient, sheet))
			else
				local k,v = next(node, nil)
				while k do
					createAccessListRec(v, f)
					k,v = next(node, k)
				end
			end
		end
	end
	
	local f = io.open("data_common/r2/r2_ring_access.xml", "w")
	assert(f)
	f:write("<?xml version=\"1.0\"?>\n\n")
	f:write("<!-- Do not modify this file!! He is generated by r2.RingAccess.createAccessList() with data from r2_palette.lua -->\n\n")
	f:write("<entitiesAccess>\n")
	createAccessListRec(r2.Palette, f)
	f:write("</entitiesAccess>\n")
	f:flush()
	f:close()

end

function RingAccess.loadAnimation(str)
	RingAccess.LoadAnimation = true
	local ok, msg = r2.loadAnimation(str)
	RingAccess.LoadAnimation = false
	return ok, msg
end



function r()
	resetR2UICache()
	runCommand('resetEditorAndReloadUI')
--	UserComponentsManager:addUserComponentUi()
end

function t2()
--	UserComponentsManager:loadPackage('./ring_features/r2_features_zone_triggers.lua.r2c.gz')
end

function t3()
--	r2.registerUserComponent('./ring_features/r2_features_zone_triggers.lua.r2c.gz')
end


function t5()
	local toto = "&ezr_חא'_\\)d //:1' 2 יייא'..)א\/:*?\"<>|א)@4 58ftgsfdg\"\/:*?\"<>|"
	toto = string.gsub(toto, "[\\\/\:\*\?\"\<\>\|]", "_")
end


r2.UserComponentManager = r2.UserComponentsManager


function r2:dumpDialogsAsText(filename, noMessage)
	
	local function dumpRec(entity, f)
		if entity:isKindOf("ChatSequence") then
			f:write("{\n\tName=[["..entity.Name.."]], Texts={\n")
			local stepId, step = next(entity.Components)
			while stepId do

				local who =""
				if tostring(step.Actions[0].Who) == "" then
					who = step.Actions[0].WhoNoEntity
				else
					local whoInst = r2:getInstanceFromId(step.Actions[0].Who)
					if whoInst then
						who = whoInst.Name
					end
				end
				local what = ""
				if tostring(step.Actions[0].Says) ~= "" then
					local whatInst =  r2:getInstanceFromId(step.Actions[0].Says)
					if whatInst then 
						local tmp = ucstring()
						tmp:fromUtf8(whatInst.Text)
						what = tmp:toString()
					end
				end
				id = tostring(step.Actions[0].Says)
				if id ~= "" then
					f:write("\t\t{Id=[["..id.."]], Who=[["..who.."]], Text=[["..what.."]], },\n")
				end
				stepId, step = next(entity.Components, stepId)				
			end

			f:write("\t}\n},\n")
		end
		if entity.Components then
			local componentId, comp = next(entity.Components, nil)
			while componentId do
				dumpRec(comp)
				componentId, comp = next(entity.Components, componentId)
			end
		end
	end
	
 	local allNames = r2:getAllNames()
	local function dumpBotnameRec(entity, f)
		assert(f)
		if not entity:isKindOf("Act") and not entity:isKindOf("Scenario") and entity.Name and entity.Name~="" then
			local mustPrint = true
			-- do not print DefaultName eg "Growling Gingo"
			if entity:isKindOf("Npc") then
				local basename = entity.Base
				if basename then basename = r2.PaletteIdToTranslation[ basename ] end
				if basename ~= nil and basename == entity.Name  then
					mustPrint = false
				end
				local found = false
				local types, names = next (allNames)
				while types  and  mustPrint do
					local index, name = next (names) 
					while index and  mustPrint do
						if entity.Name == name then mustPrint = false end 
						index, name = next (names, index) 
					end
					types, names = next (allNames, types)
				end
				local base = r2.getPaletteElement(entity.Base)
				if base and base.Name == entity.Name then mustPrint = false end
			end	
			if mustPrint then
				f:write("\t{ Id=[["..entity.InstanceId.."]], Name=[["..entity.Name.."]], },\n");			
			end
		end
		if entity.Components then
			local componentId, comp = next(entity.Components, nil)
			while componentId do
				dumpBotnameRec(comp, f)
				componentId, comp = next(entity.Components, componentId)
			end
		end
	end

	local previousEntity = nil	
	local function dumpVariousPropertyRec(entity, f)
		assert(f)

		local function dumpProperty(property, entity, f)
			if entity[property] and string.len( entity[property] ) > 0 then
				if previousEntity ~= entity.InstanceId then f:write("\n") end
				previousEntity = entity.InstanceId
				f:write("\t{\tId=[["..entity.InstanceId.."]], Property=[[".. property .. "]], Text=[[".. entity[property]  .. "]]},\n");	
			end
		end

		if  not r2.isTable(entity) then return end

		if entity.iKindOf and entity:isKindOf("Location") then
			dumpProperty("Name", entity, f)
		end

		if entity.iKindOf and entity:isKindOf("PlotItem") then
			dumpProperty("Name", entity, f)
			dumpProperty("Comment", entity, f)
			dumpProperty("Desc", entity, f)
		end

		-- Act or Scenenario description
		dumpProperty("Title", entity, f)
		dumpProperty("ShortDescription", entity, f)
		dumpProperty("PreActDescription", entity, f)

		-- Mission
		dumpProperty("BroadcastText", entity, f)
		dumpProperty("ContextualText", entity, f)
		dumpProperty("MissionSucceedText", entity, f)
		dumpProperty("MissionText", entity, f)
		dumpProperty("WaitValidationText", entity, f)
		


		local k,v = next(entity)
		while k do
			dumpVariousPropertyRec(v, f)
			k,v = next(entity, k)
		end
		
	end


	if not filename then filename = "scenario_texts.txt" end
	local f = io.open(filename, "w")
	do 

		assert(f)
		f:write("-- scenario_texts.txt '".. os.date() .."'\n\n")
		f:write("-- Here are the content of differents the dialogs\n")
		f:write("-- You must only translate the \"Text\" field (not the \"Who\" or \"Name\")\n")

		f:write("r2.DialogsAsText = {\n")


		local actId, act = next(r2.Scenario.Acts, nil)
		while actId do
			local featureId, feature = next(act.Features, nil)
			while featureId do
				dumpRec(feature, f)
				featureId, feature = next(act.Features, featureId)
			end
			actId, act = next(r2.Scenario.Acts, actId)
		end
		f:write("}\n\n\n")
	end

	do 

		assert(f)
		f:write("\n-- Here are the content of different component name of the scenario\n")
		f:write("-- Generic Fauana name and default botname are not present\n")
		f:write("-- You only have to translate the \"Name\" Field\n")
		f:write("r2.BotnameAsText = {\n\n")


		local actId, act = next(r2.Scenario.Acts, nil)
		while actId do
			local featureId, feature = next(act.Features, nil)
			while featureId do
				dumpBotnameRec(feature, f)
				featureId, feature = next(act.Features, featureId)
			end
			actId, act = next(r2.Scenario.Acts, actId)
		end
		f:write("}\n\n\n")
	end

	do 

		assert(f)
		f:write("\n-- Here are the content of different field of the scenario like plot item/act description\n")
		f:write("-- or task description/task text\n")
		f:write("-- You only have to translate the \"Text\" Field\n")
		f:write("r2.DiversText = {\n\n")



		dumpVariousPropertyRec(r2.Scenario, f)
		
		f:write("}\n")
		f:flush()
		f:close()
	end
	if not noMessage then
		messageBox("Text content has been saved in \"scenario_text.txt\"")
	end
end

function r2:updateDialogsFromText(filename, noMessage)
	if not filename then filename = "dialog.txt" end
	local f = io.open(filename, "r")
	if not f then 
		if not noMessage then
			messageBox("Can not open \""..filename.."\"")	
		else
			ld.log("Can not open \""..filename.."\"")
		end
		return
	end
	assert(f)
	if f ~= nil then
		local content = f:read("*all") 

		io.close(f)		

		local fun, msg = loadstring(content)
		if not fun then
			local err = "Syntax error in \""..filename.."\":"..msg
			if not noMessage then
				messageBox(err)	
			else
				ld.log(err)
			end
			return
		end
		r2.DialogsAsText = {}
		r2.BotnameAsText = {}
		r2.DiversText = {}
		fun()

		local dialogId, dialog = next(r2.DialogsAsText, nil)
		while dialogId do
			local textId, text = next(dialog.Texts, nil)
			while textId do
				local t = tostring(text.Text)
				local uc=ucstring(t)
				local utf8=uc:toUtf8()
				r2.requestSetNode(tostring(text.Id), "Text", utf8)
				textId, text = next(dialog.Texts, textId)
			end
			dialogId, dialog = next(r2.DialogsAsText, dialogId)
		end

		do
			local id, value = next(r2.BotnameAsText, nil)
			while id do
				local t = tostring(value.Name)
				local uc=ucstring(t)
				local utf8=uc:toUtf8()
				r2.requestSetNode(tostring(value.Id), "Name", utf8)
				id, value = next(r2.BotnameAsText, id)
			end
		end

		do
			local id, value = next(r2.DiversText, nil)
			while id do
				local t = tostring(value.Text)
				local uc=ucstring(t)
				local utf8=uc:toUtf8()
				r2.requestSetNode(tostring(value.Id), value.Property, utf8)
				id, value = next(r2.DiversText, id)
			end
		end
	end
	if not noMessage then
		messageBox("Dialogs content has been update with text from \""..filename.."\"")	
	end
	return true
end


ld = {}

ld.WaitingCommands = {}

function ld.runScript(scriptName)
	table.insert(ld.WaitingCommands, {'runScript', scriptName})
end
function ld.translate(textFilename, fileToSave)
	table.insert(ld.WaitingCommands, {'translate', textFilename, fileToSave})
end

function ld.saveTexts(fileToSave)
	table.insert(ld.WaitingCommands, {'saveTexts', fileToSave})
end

function ld.loadScena(fileToLoad)
	table.insert(ld.WaitingCommands, {'loadScena', fileToLoad})
end

function ld.log(toto)
	debugInfo("ld: "..toto)
	local f = io.open("ld.log", "a")
	f:write(tostring(os.date()).." "..toto.."\n")
	f:flush()
	f:close()
end
function ld.update()
	local command = nil
	if ld.lock == 1 then  return end
	if table.getn( ld.WaitingCommands ) > 0 then
		command=table.remove(ld.WaitingCommands, 1)

	else
		return
	end

	if command[1] == 'saveTexts' then
		ld.log("ld.saveTexts " .. command[2])
		r2:dumpDialogsAsText(command[2], true)
	elseif command[1] == 'loadScena' then
		ld.log("ld.loadScena " .. command[2])
		ld.lock = 1
		r2.load(command[2 ])
	elseif command[1] == 'translate' then
		ld.log("ld.translate " .. command[2] .. " " .. command[3])
		r2.requestNewAction("Update")
		r2:updateDialogsFromText(command[2], true)
		r2.requestEndAction()
		r2.requestNewAction("Save")
		r2.Version.save(command[3])	
		r2.requestEndAction()
	elseif command[1] == 'runScript' then
		debugInfo("ld.runScript " ..  command[2])
		dofile(command[2])
	else
		ld.log("Unknown command:" .. command[1])
	
	end
	ld.log( tostring(table.getn(ld.WaitingCommands)) .. " tasks restantes")
	if table.getn(ld.WaitingCommands) == 0 then
		ld.log("")
	end
end

function createVisitZone()
	r2.Features['VisitZone'].Components.VisitZone:create()
end

function createAmbush()
	r2.Features['Ambush'].Components.Ambush:createProtected()
end

function createManHunt()
	r2.Features['ManHuntFeature'].Components.ManHunt:create()
end

function createTimedSpawner()
	r2.Features['TimedSpawner'].Components.TimedSpawner:create()
end

function createRandomChest()
	r2.Features['RandomChest'].Components.RandomChest:create()
end
function createDelivery()
	r2.Features['DeliveryTask'].Components.DeliveryTask:create()
end

function createKillNpc()
	r2.Features['KillNpc'].Components.KillNpc:create()
end

function createTargetMob()
	r2.Features['TargetMob'].Components.TargetMob:create()
end
function testIsInPalette()
	if r2.isInPalette("palette.entities.creatures.ccegf4") then
		debugInfo("Machin is in palette")
	else
		debugInfo("Machin is NOT in palette bordel")
	end
end

function createHiddenChest()
	r2.Features['HiddenChest'].Components.HiddenChest:create()
end

function createHunt()
	r2.Features['HuntTask'].Components.HuntTask:create()
end


function testFiles()
	local files = {"r2_translator.lua", "r2_logic.lua", "r2_unit_test.lua", "r2_features.lua", "r2_components.lua", "r2_version.lua"}
	local k,v = next(files)
	while k do
		local f = fileLookup(v)
		local fun= loadfile(f)
		if not fun then
			if f == "" then
				debugInfo("Error: "..v.." is an unknown file.")
			else
				debugInfo("Error: "..f.." has syntax errors.")
			end
		end
		k,v = next(files, k)
	end
end
