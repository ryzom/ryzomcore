-- Host side of the headless scenario chain (ryzom_r2_scenario_chain).
--
-- The host runs this script before the editor's scripts and before it reduces the debug library to what
-- the client gives its scripts, so the functions the error handler needs are kept here as locals. The
-- functions below are called later, by name, with values as arguments.
--
-- They do what the client does around the editor's scripts, and name the place they follow:
-- CComLuaModule::loadFromBuffer gives a scenario it has read from a file to
-- r2.Version.updateLoadedScenario, as a table, before the scenario is uploaded or shown;
-- CEditor::scenarioUpdated ends in r2.onScenarioUpdated (r2_features.lua), which sets r2.Scenario, runs
-- r2.Version.updateVersion and selects an act before it turns to the interface;
-- CClientEditionModule::requestStartScenario runs r2.Version.save, and startingScenario runs
-- r2.Translator.initStartingActIndex, reads the saved scenario back and translates it.

local getinfo, getlocal, traceback = debug.getinfo, debug.getlocal, debug.traceback
local rawXpcall = xpcall

r2Headless = {}

-- r2.loadPalette (r2_palette.lua) gives the equipment of the player races to r2.addR2PlayerEquipment, a
-- function of r2_ui_displayer_npc.lua. The client has that script when it loads the palette. Here
-- r2_core.lua loads the palette, and the interface scripts come after it: the calls are kept, and made
-- when the script is there.
local pendingEquipment = {}

function r2Headless.beforeCore()
	r2.addR2PlayerEquipment = function(paletteElt, equipmentTable)
		pendingEquipment[#pendingEquipment + 1] = { paletteElt, equipmentTable }
	end
end

function r2Headless.afterInterfaceScripts()
	for _, call in ipairs(pendingEquipment) do
		r2.addR2PlayerEquipment(call[1], call[2])
	end
	pendingEquipment = {}
end

-- What r2:buildPaletteUI (r2_ui_palette.lua) leaves in r2.PaletteIdToTranslation,
-- r2.PaletteIdToGroupTranslation and r2.PaletteIdToType when it builds the palette tree: the same walk
-- over r2.Palette.Entries and r2.Palette.BotEntries, without the tree
function r2Headless.fillPaletteTranslations()
	local function walkBranch(luaBranch, branchStrId)
		local branchText = i18n.get(branchStrId)
		for k, v in pairs(luaBranch) do
			if k ~= "instances" then
				if type(v) == "table" and v.Display ~= false then
					walkBranch(v, "uiR2ED" .. k)
				end
			else
				for instKey, instValue in pairs(v) do
					local text
					if instValue.DirectName ~= nil then
						text = instValue.DirectName
					else
						text = i18n.get(instValue.Translation)
					end
					r2.PaletteIdToTranslation[instValue.Id] = text
					r2.PaletteIdToGroupTranslation[instValue.Id] = branchText
					r2.PaletteIdToType[instValue.Id] = v
				end
			end
		end
	end
	walkBranch(r2.Palette.Entries, r2.Palette.StrId)
	walkBranch(r2.Palette.BotEntries, r2.Palette.StrId)
end

-- What the client does with a scenario it has read from a file, before it uploads or shows it
-- (CComLuaModule::updateLoadedScenario): the version update on the scenario as a table. 'place' is
-- "File" for a file that is loaded and "Test" for the scenario a test reads back. Returns what
-- r2.Version.updateLoadedScenario returned, and the table as the function left it.
function r2Headless.updateLoadedScenario(data, place)
	return r2.Version.updateLoadedScenario(data, place), data
end

-- The author adds a component to the first act after the base act, as the palette does for a feature:
-- one action with one insert request
function r2Headless.addFeature(component)
	local acts = r2.Scenario.Acts
	local act = acts[0]
	if acts.Size > 1 then act = acts[1] end
	r2.requestNewAction("add a component")
	r2.requestInsertNode(act.InstanceId, "Features", -1, "", component)
	r2.requestEndAction()
end

-- What the last version update came to (r2.Version.LastUpdate), as text: the status, the steps that ran
-- as "Class:from>to=components", the notes for the author as "key(argument, ...)" one to a line, the
-- classes of a later version, the reason of a failure, and whether the failure is about a class the
-- editor does not have ("class"), about a step ("step") or an error of the update itself ("error")
function r2Headless.lastUpdate()
	local result = r2.Version.LastUpdate
	if result == nil then return '', '', '', '', '', '' end
	local steps = {}
	for name, count in pairs(result.Steps) do
		steps[#steps + 1] = name .. '=' .. tostring(count)
	end
	table.sort(steps)
	local notes = {}
	for _, note in ipairs(result.Notes) do
		local arguments = {}
		for index, argument in ipairs(note.Args) do
			arguments[index] = tostring(argument)
		end
		notes[#notes + 1] = note.Key .. '(' .. table.concat(arguments, ', ') .. ')'
	end
	local kind = 'step'
	if result.Error then
		kind = 'error'
	elseif result.From == nil then
		kind = 'class'
	end
	return tostring(result.Status), table.concat(steps, ','), table.concat(notes, '\n'),
		table.concat(result.Newer, ','), tostring(result.Reason or ''), kind
end

-- The start of r2.onScenarioUpdated. The editor is in an edition session (CEditor::setMode).
function r2Headless.attach(instanceId)
	r2.Mode = 'Edit'
	r2.Scenario = r2:getInstanceFromId(instanceId)
	r2.ScenarioInstanceId = instanceId
	r2.Version.LastUpdate = nil
	return r2.Scenario ~= nil
end

-- Whether every class of the scenario is at the version of the editor, the scenario's version name, and
-- the classes that are undefined, older and newer, each as "Class:scenario version>editor version"
function r2Headless.versionState()
	local current = r2.Version.getCurrentVersionFullList()
	local scenario = r2.Version.getScenarioVersionList()
	local ok, undef, older, newer = r2.Version.checkVersion(scenario, current)
	local function describe(list)
		local parts = {}
		for _, name in ipairs(list) do
			parts[#parts + 1] = name .. ':' .. tostring(scenario[name]) .. '>' .. tostring(current[name])
		end
		table.sort(parts)
		return table.concat(parts, ',')
	end
	return ok, tostring(r2.Scenario.VersionName), describe(undef), describe(older), describe(newer)
end

function r2Headless.upgrade()
	return r2.Version.updateVersion()
end

-- The act r2.onScenarioUpdated selects after the version update (r2_features.lua, from "local currentAct"
-- to the two User fields), which the translator reads back to choose the act a test starts in
function r2Headless.selectAct(startingActIndexText)
	local scenario = r2.Scenario
	local startingActIndex = tonumber(startingActIndexText)
	local currentAct = nil
	assert(scenario.Acts)
	if startingActIndex < table.getn(scenario.Acts) then
		r2.DefaultActInstanceId = scenario.Acts[startingActIndex].InstanceId
		if scenario.Acts[startingActIndex].Features.Size > 0 then
			r2.DefaultFeatureInstanceId = scenario.Acts[startingActIndex].Features[0].InstanceId
		end
		currentAct = scenario.Acts[startingActIndex]
	else
		r2.DefaultActInstanceId = scenario.Acts[0].InstanceId
		if scenario.Acts[0].Features.Size > 0 then
			r2.DefaultFeatureInstanceId = scenario.Acts[0].Features[0].InstanceId
		end
		currentAct = scenario.Acts[0]
	end
	if currentAct ~= nil then
		r2.Scenario.User.SelectedActInstanceId = tostring(currentAct.InstanceId)
		r2.Scenario.User.SelectedLocationInstanceId = tostring(currentAct.LocationId)
	end
end

-- What r2.Version.save (r2_version.lua) does to the scenario before it writes the file, with the
-- configuration and the file name of a started test ("save/r2_buffer.dat", no extended debug). It
-- starts with r2.Version.updateScenarioVersions, which is called here as it is. A scenario with the
-- "RoSOnly" access or marked as a Nevrax scenario is not saved: the function changes those properties
-- and returns. The rest of it builds the file header and asks the ring access of the character, which
-- the host does not have.
-- Returns "saved", or why the function returns before it writes.
function r2Headless.save()
	local scenario = r2.Scenario
	r2.Version.updateScenarioVersions()
	local description = scenario.Description
	if description then
		if description.OtherCharAccess == "RoSOnly" then
			r2.onSystemMessageReceived("BC_ML", "", "uiR2EDErrorRefuseToSaveRoS")
			r2.requestNewAction(i18n.get("uiR2EDUpdatingScenarioToDefaultAccess"))
			r2.requestSetNode(description.InstanceId, "OtherCharAccess", "Full")
			r2.requestEndAction()
			return "OtherCharAccess is RoSOnly"
		end
		if description.NevraxScenario == tostring(1) then
			r2.onSystemMessageReceived("BC_ML", "", "uiR2EDErrorRefuseToSaveRoS")
			r2.requestNewAction(i18n.get("uiR2EDUpdatingScenarioAccess"))
			r2.requestSetNode(description.InstanceId, "NevraxScenario", "0")
			r2.requestSetNode(description.InstanceId, "TrialAllowed", "0")
			r2.requestSetNode(description.InstanceId, "ScenarioTag", "")
			r2.requestEndAction()
			return "NevraxScenario is 1"
		end
	end
	return "saved"
end

function r2Headless.initStartingActIndex()
	r2.Translator.initStartingActIndex()
end

-- Message handler for the translation: the traceback, then a "context:" line with what the translator was
-- working on when the error came. It reads the locals of the script functions on the stack: the class of
-- the entities (target, actionEntity and so on) and the type of the event, action or condition in hand.
-- Those are names of the editor's vocabulary, not text of the scenario.
local function translateErrorHandler(message)
	local text = traceback(tostring(message), 2)
	local entityNames = { target = true, hlNpc = true, hlComponent = true, actionEntity = true, conditionEntity = true,
		component = true, entity = true, this = true, comp = true }
	local typedNames = { eventHandler = 'Event', event = 'Event', action = 'Action', condition = 'Condition' }
	local function read(f)
		local ok, value = rawXpcall(f, function() end)
		if ok then return value end
		return nil
	end
	local parts = {}
	local seen = {}
	for level = 2, 16 do
		local info = getinfo(level, 'S')
		if not info then break end
		if info.what ~= 'C' then
			local index = 1
			while true do
				local name, value = getlocal(level, index)
				if not name then break end
				if not seen[name] and r2.isTable(value) then
					if entityNames[name] then
						local class = read(function() return value.Class end)
						if type(class) == 'string' then
							seen[name] = true
							parts[#parts + 1] = name .. '=' .. class
						end
					elseif typedNames[name] then
						local field = typedNames[name]
						local kind = read(function() return value[field].Type end)
						if type(kind) == 'string' then
							seen[name] = true
							parts[#parts + 1] = name .. '=[' .. kind .. ']'
						end
					end
				end
				index = index + 1
			end
		end
	end
	if #parts > 0 then
		text = text .. '\ncontext: ' .. table.concat(parts, ' ')
	end
	return text
end

-- r2.doTranslateFeatures (r2_translator.lua) calls the protected function inside a pcall and words the
-- error again; the protected function is called here so that the error keeps its traceback
function r2Headless.translate()
	r2.LastTranslationErrorMsg = nil
	local ok, result = rawXpcall(r2.Translator.doTranslateFeaturesProtected, translateErrorHandler, r2.ScenarioInstanceId)
	if not ok then error(result, 0) end
	return result
end
