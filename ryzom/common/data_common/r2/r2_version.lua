
r2.Version = {}

local version = r2.Version


function r2.Version.checkVersion(scenarioVersionList, currentVersionList)
	local undef = {}
	local older = {}
	local newer = {}
	local ok = true

	local k,v = next(scenarioVersionList, nil)
	while k do
		if (v ~= currentVersionList[k]) then
			ok = false
			if currentVersionList[k] == nil then
				table.insert(undef, k)
			elseif v < currentVersionList[k] then
				table.insert(older, k)
			else
				table.insert(newer, k)
			end
		end
		k,v = next(scenarioVersionList, k)
	end
	return ok, undef, older, newer
	
end

function r2.Version.getUndefComponent(scenarioVersionList, currentVersionList)
	local undef = {}
	local k,v = next(scenarioVersionList, nil)
	while k do
		if (v ~= currentVersionList[k] and currentVersionList[k] == nil ) then
			table.insert(undef, k)
		end
		k,v = next(scenarioVersionList, k)
	end
	return true
	
end

local levelToString = 
{
	[0] = "20",
	[1] = "50",
	[2] = "100",
	[3] = "150",
	[4] = "200",
	[5] = "250"
}

function r2.Version.save(filename)

	local scenario = r2.Scenario

	if scenario then
		local scenarioList = r2.Version.getCurrentVersionList()
		local update = false

		local k,v = next(scenarioList, nil)
		while k do
			if (r2.Scenario.Versions[k] ~= v) then update = true end			
			k,v = next(scenarioList, k)
		end

		k,v = next(r2.Scenario.Versions, nil)
		while k do
			if (scenarioList[k] ~= v) then update = true end			
			k,v = next(r2.Scenario.Versions, k)
		end

		if update then
			r2.requestSetGhostNode(scenario.InstanceId, "Versions", scenarioList)		
		end
		local accessList = {}


		local ok, level, err = r2.RingAccess.verifyScenario()
		if not r2.RingAccess.LoadAnimation and not r2.getIsAnimationSession() then	
			r2.updateScenarioAck(ok, level, err.What)
		end
		accessList = r2.RingAccess.getAccessListAsString(level)
		

		local values = {}



		local date = os.date()

		local firstLocationName = "" 
		local shortDescription = ""
		local title = ""
		local name = ""
		local userName = r2:getUserEntityName()
		local modifierMD5 = r2.getCharIdMd5()
		local creatorName = ""
		local rules = ""
		local level = ""
		local language = ""
		local type = ""
		local creatorMD5 =""
		local createUserName = userName
		local createDate = date
		local otherCharAccess = "Full"
		local nevraxScenario = "0"
		local trialAllowed = "0"
		local scenarioTag = ""


		if r2.Scenario and r2.Scenario.Locations 
			and table.getn(r2.Scenario.Locations) > 0 and r2.Scenario.Locations[0].IslandName then
			firstLocationName = r2.Scenario.Locations[0].IslandName
		end

		if r2.Scenario and r2.Scenario.Description then
			--shortDescription =string.gsub(r2.Scenario.Description.ShortDescription, "\n", "\\n")
			shortDescription = r2.Scenario.Description.ShortDescription
			level = string.gsub(r2.Scenario.Description.LevelId, "\n", "\\n")
			level = levelToString[tonumber(level)]
			rules = string.gsub(r2.Scenario.AccessRules, "\n", "\\n")
			if rules=="liberal" then 
				rules=i18n.get("uiR2EDliberal"):toUtf8()
			elseif rules == "strict" then
				rules=i18n.get("uiR2EDstrict"):toUtf8()
			end
			title = string.gsub(r2.Scenario.Description.Title, "\n", "\\n")
			language = r2.Scenario.Language 
			type = r2.Scenario.Type  			
			name = r2.Scenario.Name
			if r2.Scenario.Ghost_Name then
				name = r2.Scenario.Ghost_Name
			end
			if r2.Scenario.Description.Creator then
				createUserName = r2.Scenario.Description.Creator 
			end
			if r2.Scenario.Description.CreatorMD5 then
				creatorMD5 = r2.Scenario.Description.CreatorMD5
			end
			if r2.Scenario.Description.CreationDate then
				createDate = r2.Scenario.Description.CreationDate 
			end			
			if table.getn(r2.Scenario.Locations) > 0 then
				initialIslandLocation = r2.Scenario.Locations[0].IslandName
				initialEntryPoint = r2.Scenario.Locations[0].EntryPoint
				initialSeason =  r2.Scenario.Locations[0].Season
			end

			if r2.Scenario.Description.OtherCharAccess then
				otherCharAccess = r2.Scenario.Description.OtherCharAccess
				if otherCharAccess == "RoSOnly" then
					if config.R2EDExtendedDebug ~= 1 then					
						if filename == "data/r2_buffer.dat" then
							return false
						else
							r2.onSystemMessageReceived("BC_ML", "", "uiR2EDErrorRefuseToSaveRoS")
							r2.requestNewAction(i18n.get("uiR2EDUpdatingScenarioToDefaultAccess"))
							r2.requestSetNode(r2.Scenario.Description.InstanceId, "OtherCharAccess", "Full")
							r2.requestEndAction()						
							return false
						end
					else
						r2.onSystemMessageReceived("BC_ML", "", "Updating the system of Trial limitation")
						r2.requestNewAction(i18n.get("uiR2EDUpdatingScenarioAccess"))
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "OtherCharAccess", "Full")
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "NevraxScenario", "1")
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "TrialAllowed", "1")
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "ScenarioTag", "")
						r2.requestEndAction()
						return false
					end


				end
			end

			if r2.Scenario.Description.NevraxScenario == tostring(1) then
				nevraxScenario = r2.Scenario.Description.NevraxScenario
				trialAllowed = r2.Scenario.Description.TrialAllowed
				scenarioTag = r2.Scenario.Description.ScenarioTag
				userName = "Ring(Nevrax)"
				createUserName = userName

				if config.R2EDExtendedDebug ~= 1  then
					
					if filename == "data/r2_buffer.dat" then
						return false
					else
						r2.onSystemMessageReceived("BC_ML", "", "uiR2EDErrorRefuseToSaveRoS")
						r2.requestNewAction(i18n.get("uiR2EDUpdatingScenarioAccess"))
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "NevraxScenario", "0")
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "TrialAllowed", "0")
						r2.requestSetNode(r2.Scenario.Description.InstanceId, "ScenarioTag", "")
						r2.requestEndAction()
						
						return false
					end


				end
			end

		end

		table.insert(values, {Title = title})
		table.insert(values, {Name = name})
		table.insert(values, {ShortDescription = shortDescription})
		table.insert(values, {FirstLocation = firstLocationName})
		table.insert(values, {RingPointLevel = accessList})
		table.insert(values, {CreateBy = createUserName})
		table.insert(values, {CreatorMD5 = creatorMD5})
		table.insert(values, {CreationDate = createDate})
		table.insert(values, {ModifiedBy = userName})
		table.insert(values, {ModifierMD5 = modifierMD5})
		table.insert(values, {OtherCharAccess = otherCharAccess})

		table.insert(values, {ModificationDate = date})
		table.insert(values, {Rules = rules})
		table.insert(values, {Level = level})
		table.insert(values, {Type = type})
		table.insert(values, {Language = language})
		table.insert(values, {InitialIsland = initialIslandLocation})
		table.insert(values, {InitialEntryPoint = initialEntryPoint})
		table.insert(values, {InitialSeason = initialSeason})
		if nevraxScenario == tostring(1) then
			table.insert(values, {NevraxScenario = nevraxScenario})
			table.insert(values, {TrialAllowed = trialAllowed})
			table.insert(values, {ScenarioTag = scenarioTag})
		end

		
		r2.save(filename, values)
	end
	return true
end


local function updateVersionImpl(nodeList, scenarioVersionList, currentVersionFullList)
	assert(nodeList)
	assert(scenarioVersionList)
	assert(currentVersionFullList)

	local k,v = next(nodeList, nil)
	while k do


		-- go up in the class hierarchy, calling each redefinition of 'onUpdate' function			
		-- when a version change is detected
		local currClassName = v.Class

		-- look at a update function for this component or one of it's ancester
		while currClassName ~= "" and currClassName ~= nil do				
			local currClass = r2.Classes[currClassName]
			if not currClass then
				debugInfo(colorTag(255, 0, 0) .. "Error can not update your scenario: the component"..currClassName.." seems to be obsolete")
				return false
			end
			local scenarioVersionNode = defaulting(scenarioVersionList[currClassName], 0)
			local currentVersionNode = defaulting(currentVersionFullList[currClassName], 0)
			if currentVersionNode ~= scenarioVersionNode then
				--
				if scenarioVersionNode == nil then
					debugInfo(colorTag(0, 255 ,255) .. "The component  (".. v.Class .. ") does not exist anymore")
					return false
				elseif currentVersionNode == nil then
					debugInfo(colorTag(0, 255 ,255) .. "The component  (".. v.Class .. ") does not exist anymore")
					return false
				else
					debugInfo(colorTag(0, 255 ,255) .. "Updating the component  " .. v.InstanceId .. "(".. v.Class .. "/"..currClassName..") from "..  scenarioVersionNode .. " to " .. currentVersionNode ..".")
				
					local updateFunc = currClass.updateVersion
					if not updateFunc then
						debugInfo( "Your scenario can not be updated. Because the component " .. v.Class .. " can not be updated (no update Function?).\n")
						return false
					end
					local ok1, ok2 = pcall(updateFunc, v, scenarioVersionNode, currentVersionNode)
					if not ok1  or not ok2 then 
						debugInfo( "Your scenario can not be updated. Because the component " .. v.Class .. " can not be updated.\n")
						if not ok1 then 
							debugInfo(ok2)
						end
						return false
					end
				
				end
			end
			currClassName = currClass.BaseClass
		end
		
		k,v = next(nodeList, k)
	end
	return true

end

function r2.Version.updateVersion()
	if r2.Translator == nil then
		debugInfo("Syntax error")
		return false
	end
	local scenarioInstance = r2.Scenario

	if not scenarioInstance then return true end

	local currentVersionFullList = r2.Version.getCurrentVersionFullList()
	local scenarioVersionList = r2.Version.getScenarioVersionList()
	
	local versionOk, undef, older, newer = r2.Version.checkVersion(scenarioVersionList, currentVersionFullList)

	local modified = false
	if not versionOk then

		debugInfo(colorTag(0,255,25).."Begin Update")
		
		local scenarioVersionName = scenarioInstance.VersionName
		local currentVersionName = r2.Classes["Scenario"].VersionName

		if scenarioVersionName == nil then scenarioVersionName = "0.0.0" end
		if currentVersionName == nil then currentVersionName = "0.0.0" end
		
		local str = "Updating the scenario from '"..scenarioVersionName.."' to '".. currentVersionName .."'.\nThe obsolete scenario will be saved in 'old_scenario.r2'"
		printMsg(str)
		local nodeList = r2.Version.getScenarioNodes(scenarioInstance)
		assert(nodeList)

		do
			local k,v = next(undef, nil)
			while k do
				debugInfo(colorTag(0, 255 ,255) .. "The component ".. v .. " used in this scenario does not exist.")
				k,v = next(undef, k)
			end
		end

		do
			local k,v = next(older, nil)
			while k do
				debugInfo(colorTag(0, 255 ,255) .. "The component ".. v .. " is too old (we will try to update it)")
				k,v = next(older, k)
			end
		end

		do
			local k,v = next(newer, nil)
			while k do
				debugInfo(colorTag(0, 255 ,255) .. "The component ".. v .. " is too new (maybe wrong version?)")
				k,v = next(newer, k)
			end
		end
		if table.getn(undef) > 0 then
			debugInfo(colorTag(255, 0, 0) .. "Error can not update your scenario: the scenario containse components that are not defined")
			return true
		end
		local oldState = scenarioInstance.Ghost
		scenarioInstance.Ghost = false
		r2.save("old_scenario.r2")
		scenarioInstance = r2.Scenario

		scenarioInstance.Ghost = true

		local syntaxOk, ok = pcall(updateVersionImpl, nodeList, scenarioVersionList, currentVersionFullList)	
		if not syntaxOk then
			r2.print(ok)
			ok = false
		end
		if  ok then
		-- ok
			debugInfo(colorTag(0,255,25).."Update succced")
			local currentVersionList = r2.Version.getCurrentVersionList()
			r2.requestSetNode(scenarioInstance.InstanceId, "VersionName", currentVersionName)
			r2.requestSetNode(scenarioInstance.InstanceId, "Versions", currentVersionList)
			scenarioInstance.Ghost = oldState
			r2.requestUploadCurrentScenario()
			r2.clearActionHistoric() -- undo invalidated after a version update !!	
			modified = true
		else
			debugInfo(colorTag(255, 0, 0) .. "Errors occurs while updateing your scenario")
		
		end
		scenarioInstance.Ghost = oldState
		debugInfo(colorTag(0,255,25).."End Update")

	end	
	return not modified;
end




-- return Scenario Nodes - leaf first
function r2.Version.getScenarioNodes(node, nodeListParam)
	assert(node)
	if nodeListParam then nodeList = nodeListParam else nodeList = {} end
	
	if  not r2.isTable(node)  then return nodeList end	
	local k,v = next(node, nil)
	while k do
		r2.Version.getScenarioNodes(v, nodeList)		
		k,v = next(node, k)
	end
	if node.InstanceId then
		table.insert(nodeList, node)
		if not node.Class then assert(nil) end
	end
	return nodeList
end


-- get the Current Version of sceanrio
function r2.Version.getCurrentVersionList()
	local scenarionInstance = r2.Scenario
	local versionList = {}
	r2.Version._getCurrentVersionListImpl(scenarionInstance, versionList)
	return versionList;
end

function r2.Version.getCurrentVersionFullList()
	local versionList = {}
	local classes = r2.Classes
	local k, v = next(classes, nil)
	while k do
		if v.Version then 
			versionList[k] = v.Version 
		else
			versionList[k] = 0
		end
		k, v = next(classes, k)
	end
	return versionList
end

function r2.Version._getCurrentVersionListImpl(node, versionList)
	if ( type (node) == "string" or type(node) == "number")  then return end
	local k,v = next(node, nil)
	if node.Class and not versionList[node.Class] then
		if r2.Classes[node.Class] and r2.Classes[node.Class].Version then
			versionList[node.Class] = r2.Classes[node.Class].Version
		else
			versionList[node.Class] = 0
		end
	end
	while k do
		r2.Version._getCurrentVersionListImpl(v, versionList);
		k,v = next(node, k)
	end
end


function r2.Version.getScenarioVersionList(scenarioNode)
	local scenario = scenarioNode
	if not scenario then scenario = r2.Scenario end
	assert(scenario)
	local versions = scenario.Versions
	local versionList = {}
	if versions == nil then
		return r2.Version._getScenarioVersionListImpl(scenario, versionList)
	else
		local k,v = next(versions, nil)
		while k do
			versionList[k] = v
			k,v = next(versions, k)
		end
	end
	return versionList	

end

function r2.Version._getScenarioVersionListImpl(node, versionList)
	if ( type (node) == "string" or type(node) == "number")  then return end
	local k,v = next(node, nil)
	if node.Class and not versionList[node.Class] then
		versionList[node.Class] = 0
	end	
	while k do
		r2.Version._getScenarioVersionListImpl(v, versionList);
		k,v = next(node, k)
	end
	return versionList
end

-- Version 0.0.0 -> prior to 7 dec 2005

-- version 0.0.1 prior to 7 dec 2005
-- Act.Version = 1
-- Act.ManualWeather
-- Act.WeatherValue

-- version 0.0.2 19 dec 2005
-- Act.Version = 2
-- Act.Behavior (Because Act changes type from BaseType to LogicAction
--
-- version 0.0.3 1 janv 2006
-- ActivityStep.Version=1
-- "Inactive" -> "Stand Still"

