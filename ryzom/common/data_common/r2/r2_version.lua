
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
		r2.Version.updateScenarioVersions()
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
				rules=i18n.get("uiR2EDliberal")
			elseif rules == "strict" then
				rules=i18n.get("uiR2EDstrict")
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


---------------------------------------------------------------------------------------------------------
-- VERSION UPDATE
--
-- A scenario made with an earlier version of the editor is brought up to date as plain data, before the
-- editor builds anything from it:
--   * a scenario read from a file is updated between the reading and the upload: the client calls
--     r2.Version.updateLoadedScenario with the scenario as a table;
--   * a scenario that comes from the server in an earlier version (an edition session the server kept)
--     is updated by r2.Version.updateVersion: a copy of it as a table is updated, and uploaded in place
--     of the scenario the server has.
-- No update step changes the scenario the editor shows.
--
-- The data has the form r2.newComponent gives a component: a table with named attributes, lists that
-- start at 1, references as r2.RefId values. An attribute a component does not have reads nil there: no
-- default value of its class answers.
--
-- A class brings its components up to date in its updateVersion function:
--     updateVersion = function(this, scenarioValue, currentValue, update)
-- 'this' is the component, scenarioValue the version the scenario has for the class and currentValue
-- the version of the class in this editor. A class that defines no updateVersion has the one of its
-- base class, which is then also called for the base class with the versions of the base class.
-- 'update' is the update in progress. Every change to the scenario goes through its functions (set,
-- erase, insert, remove, removeNode), which is how the update knows whether the scenario changed; note
-- tells the author something. The function returns true when the component is at currentValue.

local Update = {}
Update.__index = Update

-- Takes a table of the scenario and what it holds into the update, as part of 'parent'
function Update:index(node, parent)
	if type(node) ~= "table" or r2.isRefId(node) then return end
	self._Parents[node] = parent
	self._Removed[node] = nil
	local instanceId = node.InstanceId
	if type(instanceId) == "string" then
		self._Nodes[instanceId] = node
		-- (string.find: the editor has a string.match of its own, which tells whether a text matches)
		local first, last, prefix, number = string.find(instanceId, "^(.*)_(%d+)$")
		if first ~= nil and prefix == self._IdPrefix and tonumber(number) > self._LastId then
			self._LastId = tonumber(number)
		end
	end
	for k, v in pairs(node) do
		self:index(v, node)
	end
end

-- Takes a table and what it holds out of the update
function Update:forget(node)
	if type(node) ~= "table" or r2.isRefId(node) then return end
	self._Parents[node] = nil
	self._Removed[node] = true
	local instanceId = node.InstanceId
	if type(instanceId) == "string" and self._Nodes[instanceId] == node then
		self._Nodes[instanceId] = nil
	end
	for k, v in pairs(node) do
		self:forget(v)
	end
end

-- Sets an attribute of a component
function Update:set(node, name, value)
	assert(type(node) == "table" and name ~= nil and value ~= nil)
	local oldValue = node[name]
	if type(value) ~= "table" and type(oldValue) ~= "table" and oldValue == value then return end
	if r2.isRefId(value) and r2.isRefId(oldValue) and oldValue == value then return end
	self:forget(oldValue)
	node[name] = value
	self:index(value, node)
	self.Changed = true
end

-- Removes an attribute of a component
function Update:erase(node, name)
	assert(type(node) == "table" and name ~= nil)
	local oldValue = node[name]
	if oldValue == nil then return end
	self:forget(oldValue)
	node[name] = nil
	self.Changed = true
end

-- Adds a value to a list, at its end or at 'position' (the first position is 1)
function Update:insert(list, value, position)
	assert(type(list) == "table" and value ~= nil)
	if position ~= nil then
		table.insert(list, position, value)
	else
		table.insert(list, value)
	end
	self:index(value, list)
	self.Changed = true
end

-- Removes a value from a list. Returns false when the list does not hold it.
function Update:remove(list, value)
	assert(type(list) == "table" and value ~= nil)
	for position = 1, table.getn(list) do
		if rawequal(list[position], value) then
			self:forget(value)
			table.remove(list, position)
			self.Changed = true
			return true
		end
	end
	return false
end

-- Removes a component from the list or the component that holds it
function Update:removeNode(node)
	local parent = self._Parents[node]
	if parent == nil then return false end
	if self:remove(parent, node) then return true end
	for name, value in pairs(parent) do
		if rawequal(value, node) then
			self:erase(parent, name)
			return true
		end
	end
	return false
end

-- The list or the component that holds a table of the scenario
function Update:getParent(node)
	return self._Parents[node]
end

-- The name or the position under which a table of the scenario is held, nil for the scenario itself
function Update:getKey(node)
	local parent = self._Parents[node]
	if parent == nil then return nil end
	for k, v in pairs(parent) do
		if rawequal(v, node) then return k end
	end
	return nil
end

-- The component a table of the scenario is part of: for a component in a list of a component, the
-- latter
function Update:getOwner(node)
	local parent = self._Parents[node]
	while parent ~= nil and parent.InstanceId == nil do
		parent = self._Parents[parent]
	end
	return parent
end

-- The component with an id, nil when the scenario has none. The id can be a reference.
function Update:getNode(instanceId)
	if instanceId == nil then return nil end
	return self._Nodes[tostring(instanceId)]
end

-- The text of an attribute that holds an id or a reference, "" when the component has none
function Update:getId(value)
	if value == nil then return "" end
	return tostring(value)
end

-- A new component of a class, as r2.newComponent gives it, with ids that are free in the scenario
function Update:newComponent(className)
	local component = r2.newComponent(className)
	if component == nil then error("no component of the class '" .. tostring(className) .. "' can be made") end
	local function renumber(node)
		if type(node) ~= "table" or r2.isRefId(node) then return end
		if node.InstanceId ~= nil then
			self._LastId = self._LastId + 1
			node.InstanceId = self._IdPrefix .. "_" .. tostring(self._LastId)
		end
		local names = {}
		for k, v in pairs(node) do
			if type(k) == "string" and type(v) == "table" then table.insert(names, k) end
		end
		table.sort(names)
		for i = 1, table.getn(names) do renumber(node[names[i]]) end
		for i = 1, table.getn(node) do renumber(node[i]) end
	end
	renumber(component)
	return component
end

-- Something to tell the author: the id of a translated text, and what takes the place of %1, %2 ...
-- in it
function Update:note(key, ...)
	table.insert(self.Notes, { Key = key, Args = { ... } })
end

-- Something to tell the author once for several components: each call adds one to the number the note
-- of 'key' has in the place of %1. What follows 'key' takes the places from %2 on, as the first call
-- gives it. These notes come after the others, in the order of their keys.
function Update:count(key, ...)
	local counted = self._Counted[key]
	if counted == nil then
		counted = { Count = 0, Args = { ... } }
		self._Counted[key] = counted
	end
	counted.Count = counted.Count + 1
end

-- Makes the notes of what Update:count counted
function Update:addCountedNotes()
	local keys = {}
	for key, counted in pairs(self._Counted) do table.insert(keys, key) end
	table.sort(keys)
	for i = 1, table.getn(keys) do
		local counted = self._Counted[keys[i]]
		local args = { counted.Count }
		for j = 1, table.getn(counted.Args) do table.insert(args, counted.Args[j]) end
		table.insert(self.Notes, { Key = keys[i], Args = args })
	end
	self._Counted = {}
end

-- Calls 'visitor' with every table of the scenario from 'node' down, 'node' first, in the order of the
-- update: the attributes of a table by their names, then its list. A visitor that returns false keeps
-- the walk out of what the table holds. The visitor does not change the scenario: what it finds is
-- changed after the walk.
function Update:walk(node, visitor)
	if type(node) ~= "table" or r2.isRefId(node) then return end
	if visitor(node) == false then return end
	local names = {}
	for k, v in pairs(node) do
		if type(k) == "string" and type(v) == "table" then table.insert(names, k) end
	end
	table.sort(names)
	for i = 1, table.getn(names) do self:walk(node[names[i]], visitor) end
	for i = 1, table.getn(node) do self:walk(node[i], visitor) end
end

local function newUpdate(data)
	local update = setmetatable({}, Update)
	update.Scenario = data
	update.Changed = false
	update.Notes = {}
	update._Counted = {}
	update._Parents = {}
	update._Nodes = {}
	update._Removed = {}
	-- the ids of this client are "<prefix>_<number>"
	local probe = r2.newComponent("Position")
	local first, last, prefix, number = string.find(tostring(probe.InstanceId), "^(.*)_(%d+)$")
	if first == nil then
		error("the id '" .. tostring(probe.InstanceId) .. "' of a new component is not a prefix and a number")
	end
	update._IdPrefix = prefix
	update._LastId = tonumber(number)
	update:index(data, nil)
	return update
end

-- The components of a scenario, each after the components it holds. The attributes are taken in the
-- order of their names, so that the same scenario is always updated in the same order.
local function collectNodes(node, nodes)
	if type(node) ~= "table" or r2.isRefId(node) then return end
	local names = {}
	for k, v in pairs(node) do
		if type(k) == "string" and type(v) == "table" then table.insert(names, k) end
	end
	table.sort(names)
	for i = 1, table.getn(names) do collectNodes(node[names[i]], nodes) end
	for i = 1, table.getn(node) do collectNodes(node[i], nodes) end
	if node.InstanceId ~= nil then table.insert(nodes, node) end
end

-- The classes of earlier editors that this editor has under another name, with the names those editors
-- gave their actions and what each action is now: { type, value }
local renamedClasses = {
	TimerFeature = {
		Class = "Timer",
		Actions = {
			["Desactivate"] = { "Deactivate" },
			["Add 10 Seconds"] = { "add seconds", "10" },
			["Add 1 minute"] = { "add seconds", "60" },
			["Sub 10 seconds"] = { "sub seconds", "10" },
			["Sub 1 min"] = { "sub seconds", "60" },
		},
	},
	BanditCampFeature = {
		Class = "BanditCamp",
		Actions = {
			["Desactivate"] = { "deactivate" },
		},
	},
}

-- The components of classes this editor does not have, before anything goes by the class of a
-- component.
--   TimerFeature        becomes the Timer of this editor, which has the same attributes
--   BanditCampFeature   becomes the BanditCamp of this editor; its cycle was the time the bandits sit
--                       and the time they wander, half of it each
-- A converted component keeps its id, so what names it still does; the actions it had under another
-- name get the name they have now.
-- A component of any other such class that stands in a list is removed with what it holds, and the
-- author is told. That is what becomes of the classes of the first editors that nothing replaces
-- (Event, EventCondition and ScriptAction: the events of activity steps, which no editor since has
-- run), and of the classes of features this editor is not set to load
-- (config.R2EDLoadDynamicFeatures). What named a removed component goes too: an action on it, a
-- handler of an event of it, and a handler with a condition on it, which can not be tested any more
-- and must not run without it; an attribute that held its id is emptied.
-- A component of such a class that another component holds by name (its behavior, its position) can
-- not be taken out: the component that holds it would lack a part the editor reads everywhere. The
-- update fails then, and nothing is changed.
-- 'steps' gets what was done, as the steps of the update: "Class>NewClass" and "Class>removed".
-- Returns nil, or what the update fails on: { Class, Attribute, Holder }, the class this editor does
-- not have, and the name under which a component of the class Holder holds it.
local function updateObsoleteClasses(update, steps)
	local data = update.Scenario
	local converted = {}
	local removed = {}
	local removedIds = {}		-- the ids of the removed components and of what they hold
	local removedCounts = {}	-- class > number of components
	local failure = nil
	update:walk(data, function(node)
		local className = node.Class
		-- (a table without the name of a class is no component of a class the editor lacks)
		if type(className) ~= "string" or className == "" or r2.Classes[className] ~= nil then return true end
		-- a scenario that is itself of no class of this editor fails the update
		if rawequal(node, data) then return true end
		if renamedClasses[className] ~= nil then
			table.insert(converted, node)
			return true
		end
		local key = update:getKey(node)
		if type(key) ~= "number" then
			if failure == nil then
				-- What holds it is the nearest component with a class. A table without one on the
				-- way is named in the attribute, as "Part.Shape", or "Parts[2].Shape" for the second
				-- table of a list.
				local attribute = tostring(key)
				local holder = update:getParent(node)
				while type(holder.Class) ~= "string" or holder.Class == "" do
					local holderKey = update:getKey(holder)
					if holderKey == nil then break end
					local separator = "."
					if string.sub(attribute, 1, 1) == "[" then separator = "" end
					if type(holderKey) == "number" then
						attribute = "[" .. tostring(holderKey) .. "]" .. separator .. attribute
					else
						attribute = tostring(holderKey) .. separator .. attribute
					end
					holder = update:getParent(holder)
				end
				-- (the table that holds everything is the scenario, whatever it says of its class)
				local holderClass = holder.Class
				if type(holderClass) ~= "string" or holderClass == "" then holderClass = "Scenario" end
				failure = { Class = className, Attribute = attribute, Holder = holderClass }
			end
			return false
		end
		table.insert(removed, node)
		removedCounts[className] = defaulting(removedCounts[className], 0) + 1
		update:walk(node, function(held)
			if held.InstanceId ~= nil then removedIds[tostring(held.InstanceId)] = true end
			return true
		end)
		return false
	end)
	if failure ~= nil then return failure end
	if table.getn(converted) == 0 and table.getn(removed) == 0 then return nil end

	local convertedActions = {}	-- id of a converted component > the actions of its earlier class
	local uncycledCount = 0		-- the bandit camps that had no cycle
	for i = 1, table.getn(converted) do
		local node = converted[i]
		local className = node.Class
		local renamedClass = renamedClasses[className]
		convertedActions[tostring(node.InstanceId)] = renamedClass.Actions
		if className == "TimerFeature" then
			update:set(node, "Base", r2.Translator.getDebugBase("palette.entities.botobjects.timer"))
		elseif className == "BanditCampFeature" then
			local cycle = tonumber(node.Cycle)
			if cycle ~= nil and cycle > 0 then
				local duration = math.max(10, math.ceil(cycle / 2))
				if node.SitDuration == nil then update:set(node, "SitDuration", duration) end
				if node.WanderDuration == nil then update:set(node, "WanderDuration", duration) end
			elseif cycle == 0 then
				-- without a cycle the bandits stayed where the camp started them; the property sheet
				-- of the camp of this editor has no duration below 10, and the camp gets those of a
				-- new camp
				uncycledCount = uncycledCount + 1
			end
			update:erase(node, "Cycle")
			-- "desactivation" is the name the class gave the event "deactivation"
			local behavior = node.Behavior
			if type(behavior) == "table" and type(behavior.Actions) == "table" then
				for k, handler in pairs(behavior.Actions) do
					local event = handler.Event
					if type(event) == "table" and event.Type == "desactivation" then
						update:set(event, "Type", "deactivation")
					end
				end
			end
		end
		update:set(node, "Class", renamedClass.Class)
		local stepName = className .. ">" .. renamedClass.Class
		steps[stepName] = defaulting(steps[stepName], 0) + 1
	end

	for i = 1, table.getn(removed) do
		update:removeNode(removed[i])
	end

	-- what names a removed component, and the actions of a converted one
	local erased = {}		-- action steps and handlers
	local untestable = {}	-- handlers with a condition on a removed component
	local emptied = {}		-- { Node, Key, Value }: attributes and entries of lists that hold an id
	local renamed = {}		-- { Action, To }
	update:walk(data, function(node)
		local className = node.Class
		if className == "ActionStep" or className == "ConditionStep" then
			local logic = node.Action
			if className == "ConditionStep" then logic = node.Condition end
			local entityId = update:getId(node.Entity)
			local valueId = ""
			if type(logic) == "table" then valueId = update:getId(logic.Value) end
			if removedIds[entityId] or removedIds[valueId] then
				local handler = update:getOwner(node)
				if className == "ConditionStep" and handler ~= nil and handler.Class == "LogicEntityAction" then
					table.insert(untestable, handler)
				else
					table.insert(erased, node)
				end
				return false
			end
			local actions = convertedActions[entityId]
			if className == "ActionStep" and actions ~= nil and type(logic) == "table" and actions[logic.Type] ~= nil then
				table.insert(renamed, { Action = logic, To = actions[logic.Type] })
			end
		elseif className == "LogicEntityAction" then
			local event = node.Event
			if type(event) == "table" and removedIds[update:getId(event.Value)] then
				table.insert(erased, node)
				return false
			end
		end
		for k, v in pairs(node) do
			if k ~= "InstanceId" and (type(v) == "string" or r2.isRefId(v)) and removedIds[tostring(v)] then
				table.insert(emptied, { Node = node, Key = k, Value = v })
			end
		end
		return true
	end)
	-- (a handler can be there for several of its conditions, and what it holds for itself)
	local erasedCount = 0
	for i = 1, table.getn(untestable) do
		if update:removeNode(untestable[i]) then erasedCount = erasedCount + 1 end
	end
	for i = 1, table.getn(erased) do
		if update:removeNode(erased[i]) then erasedCount = erasedCount + 1 end
	end
	local emptiedCount = 0
	for i = 1, table.getn(emptied) do
		local entry = emptied[i]
		if update._Removed[entry.Node] then
			-- gone with a handler that was removed above
		else
			if type(entry.Key) == "number" then
				update:remove(entry.Node, entry.Value)
			elseif r2.isRefId(entry.Value) then
				update:set(entry.Node, entry.Key, r2.RefId(""))
			else
				update:set(entry.Node, entry.Key, "")
			end
			emptiedCount = emptiedCount + 1
		end
	end
	for i = 1, table.getn(renamed) do
		local action = renamed[i].Action
		local to = renamed[i].To
		update:set(action, "Type", to[1])
		if to[2] ~= nil then update:set(action, "ValueString", to[2]) end
	end

	if table.getn(converted) > 0 then
		update:note("uiR2EDVersionNoteConvertedClasses", table.getn(converted))
	end
	if uncycledCount > 0 then
		update:note("uiR2EDVersionNoteBanditCampCycle", uncycledCount)
	end
	if table.getn(removed) > 0 then
		local classNames = {}
		for className, count in pairs(removedCounts) do
			table.insert(classNames, className .. " (" .. tostring(count) .. ")")
			steps[className .. ">removed"] = count
		end
		table.sort(classNames)
		update:note("uiR2EDVersionNoteRemovedClasses", table.concat(classNames, ", "))
		local referenceCount = erasedCount + emptiedCount
		if referenceCount > 0 then
			update:note("uiR2EDVersionNoteRemovedClassReferences", referenceCount)
		end
	end
	return nil
end

-- Runs the update steps on a scenario held as a table. Returns a table:
--   Status   "current"    the scenario has the versions of this editor: nothing was done
--            "unchanged"  its versions are earlier and no step had anything to change: the table is
--                         as it was, but for its list of versions, which is the one of this editor
--            "changed"    steps changed it, and it has the versions of this editor now
--            "failed"     a step failed, or the scenario itself or a part a component holds by name
--                         is of a class this editor does not have: the table is not to be used
--   Notes    what the steps tell the author: { Key = id of a translated text, Args = { ... } }
--   Steps    the steps that ran: "Class:from>to" > number of components; for the components of a
--            class this editor does not have, "Class>NewClass" and "Class>removed"
--   Newer    the classes the scenario has in a later version than this editor: "Class:version" list
--   Versions the list of versions the scenario has now ("unchanged" and "changed")
--   Class, From, To, Reason    of a failure: the class whose step failed, its versions, and the error
--   Attribute, Holder          of a failure on a class this editor does not have, when a component of
--                              the class Holder holds it under the name Attribute
--   Error    true when the update itself raised an error, outside of a step
--
-- The version a scenario has for a class is the one its Versions table gives. The table lists the
-- classes that had a component when it was made, and never the classes those derive from. A class it
-- does not list, and every class of a scenario without the table, counts as of its first version: the
-- component can be of any version, since it was added after the table was made, by the editor that
-- had the scenario then. So every step looks at the component before it changes it, and leaves one
-- that is up to date as it is.
local function runUpdateSteps(data)
	assert(type(data) == "table")
	local result = { Status = "current", Notes = {}, Steps = {}, Newer = {} }
	local update = newUpdate(data)
	-- the classes this editor does not have go first: the steps go by the class of a component
	local failure = updateObsoleteClasses(update, result.Steps)
	if failure ~= nil then
		result.Status = "failed"
		result.Class = failure.Class
		result.Attribute = failure.Attribute
		result.Holder = failure.Holder
		result.Reason = "the '" .. failure.Attribute .. "' of a component of the class '" .. failure.Holder
			.. "' is of the class '" .. failure.Class .. "', which this editor does not have"
		return result
	end
	local currentVersionFullList = r2.Version.getCurrentVersionFullList()
	local scenarioVersionList = r2.Version.getScenarioVersionList(data)
	if not update.Changed and r2.Version.checkVersion(scenarioVersionList, currentVersionFullList) then
		return result
	end

	local nodes = {}
	collectNodes(data, nodes)
	local newer = {}
	for i = 1, table.getn(nodes) do
		local node = nodes[i]
		local className = node.Class
		while not update._Removed[node] and className ~= nil and className ~= "" do
			local class = r2.Classes[className]
			if class == nil then
				result.Status = "failed"
				result.Class = className
				result.Reason = "the scenario has a component of the class '" .. tostring(className) .. "', which this editor does not have"
				return result
			end
			local scenarioVersion = defaulting(scenarioVersionList[className], 0)
			local currentVersion = defaulting(currentVersionFullList[className], 0)
			if scenarioVersion < currentVersion then
				local stepName = className .. ":" .. tostring(scenarioVersion) .. ">" .. tostring(currentVersion)
				local ok, done = false, "the class has no update function"
				if class.updateVersion ~= nil then
					ok, done = pcall(class.updateVersion, node, scenarioVersion, currentVersion, update)
				end
				if not ok or not done then
					result.Status = "failed"
					result.Class = className
					result.From = scenarioVersion
					result.To = currentVersion
					result.Reason = "the update of a component of the class '" .. tostring(node.Class) .. "' failed in the step " .. stepName
					if not ok then result.Reason = result.Reason .. ": " .. tostring(done) end
					return result
				end
				result.Steps[stepName] = defaulting(result.Steps[stepName], 0) + 1
			elseif scenarioVersion > currentVersion then
				newer[className] = scenarioVersion
			end
			className = class.BaseClass
		end
	end

	for className, scenarioVersion in pairs(newer) do
		table.insert(result.Newer, className .. ":" .. tostring(scenarioVersion))
	end
	table.sort(result.Newer)
	update:addCountedNotes()
	result.Notes = update.Notes

	-- the scenario is at the versions of this editor now; a class it has in a later version keeps it
	local versions = {}
	r2.Version._getCurrentVersionListImpl(data, versions)
	for className, scenarioVersion in pairs(newer) do
		if versions[className] ~= nil then versions[className] = scenarioVersion end
	end
	update:forget(data.Versions)
	data.Versions = versions
	result.Versions = versions
	if not update.Changed then
		result.Status = "unchanged"
		return result
	end
	data.VersionName = r2.Classes["Scenario"].VersionName
	result.Status = "changed"
	return result
end

-- Runs the update steps on a scenario held as a table, and returns what runUpdateSteps returns. An
-- error outside of a step gives a failure as well.
function r2.Version.updateScenarioData(data)
	local ok, result = pcall(runUpdateSteps, data)
	if not ok then
		result = { Status = "failed", Notes = {}, Steps = {}, Newer = {}, Error = true, Reason = tostring(result) }
	end
	return result
end

-- A translated text with its arguments in the place of %1, %2 ... An argument that is itself the id of
-- a translated text (the name of an island) is shown translated.
local function getUpdateText(key, args)
	local text = tostring(i18n.get(key))
	if args == nil then return text end
	return (string.gsub(text, "%%(%d)", function(index)
		local value = args[tonumber(index)]
		if value == nil then return "" end
		if type(value) == "string" and string.sub(value, 1, 2) == "ui" and i18n.hasTranslation(value) then
			return tostring(i18n.get(value))
		end
		return tostring(value)
	end))
end

-- Tells the author that an update failed. 'place' says where the scenario came from, and with it what
-- becomes of the scenario: "File" (a file that is loaded: it is not loaded), "Test" (the scenario saved
-- for a test: the test does not start), "Session" (a scenario the server kept: it is shown as it is).
local function tellUpdateFailure(result, place)
	debugInfo(colorTag(255, 0, 0) .. "The scenario can not be brought up to date: " .. tostring(result.Reason))
	local text
	if result.Error then
		text = getUpdateText("uiR2EDVersionUpdateError" .. place)
	elseif result.From == nil and result.Holder ~= nil then
		text = getUpdateText("uiR2EDVersionUnknownClassIn" .. place, { tostring(result.Class), tostring(result.Attribute), tostring(result.Holder) })
	elseif result.From == nil then
		text = getUpdateText("uiR2EDVersionUnknownClass" .. place, { tostring(result.Class) })
	else
		text = getUpdateText("uiR2EDVersionUpdateFailed" .. place, { tostring(result.Class), tostring(result.From), tostring(result.To) })
	end
	displaySystemInfo(text, "BC")
	return text
end

-- Tells the author what an update did with the scenario
local function tellUpdateDone(result, place, args)
	for stepName, count in pairs(result.Steps) do
		debugInfo(colorTag(0, 255, 255) .. "Update step " .. stepName .. " on " .. tostring(count) .. " component(s)")
	end
	displaySystemInfo(getUpdateText("uiR2EDVersionUpdated" .. place, args), "BC")
	for k, note in pairs(result.Notes) do
		displaySystemInfo(getUpdateText(note.Key, note.Args), "BC")
	end
end

-- Tells the author what an update found and could change nothing about (a location on an island the
-- editor does not have): the notes alone, since the scenario is as it was
local function tellUpdateNotes(result)
	for k, note in pairs(result.Notes) do
		displaySystemInfo(getUpdateText(note.Key, note.Args), "BC")
	end
end

-- Tells the author that the scenario has classes in a later version than this editor
local function tellNewerClasses(result)
	if table.getn(result.Newer) > 0 then
		displaySystemInfo(getUpdateText("uiR2EDVersionNewer", { table.concat(result.Newer, ", ") }), "BC")
	end
end

-- Called by the client with a scenario it has read from a file, before it uploads or shows it. 'place'
-- is "File" for a file the author loads and "Test" for the scenario a test has saved and reads back.
-- Returns nil when the scenario is to be used as it is, the scenario to use in its place when the
-- update changed it or gave it its list of versions, and false when the update failed: the author is
-- told, and the scenario is not to be used.
function r2.Version.updateLoadedScenario(data, place)
	if place ~= "Test" then place = "File" end
	local result = r2.Version.updateScenarioData(data)
	r2.Version.LastUpdate = result
	if result.Status == "failed" then
		messageBox(tellUpdateFailure(result, place))
		return false
	end
	tellNewerClasses(result)
	if result.Status == "current" then return nil end
	if result.Status == "changed" then
		tellUpdateDone(result, place)
	else
		tellUpdateNotes(result)
	end
	return data
end

-- Whether the editor is in an edition session, where the server has a scenario of this editor's to
-- keep in step
local function isEditionSession()
	return r2.Mode == "Edit" or r2.Mode == "BackToEditing"
end

-- Called when the editor has a new scenario to show (r2.onScenarioUpdated). A scenario that came
-- from a file is at the versions of this editor here, and nothing is done. One the server kept from
-- an earlier editor is not: a copy of it is brought up to date and takes its place on the server,
-- and the editor shows that one when it comes back. Nothing is changed in the scenario the editor
-- has now. When no step has anything to change, only the list of versions goes to the server
-- (r2.requestSetScenarioVersions), so that the scenario is not looked at again each time it arrives.
-- (A scenario with a class of a later version than this editor is looked at each time: its list can
-- not say the versions of this editor.)
-- The upload can not come back as a scenario to update: the copy carries the versions of this editor.
-- When the server refuses it, the editor keeps the scenario as it is, which the client tells the
-- author; the next scenario the editor is given is looked at again.
-- Returns false when the scenario was sent to take the place of the one the editor has.
function r2.Version.updateVersion()
	local scenarioInstance = r2.Scenario
	if not scenarioInstance then return true end

	-- a first look at the scenario the editor has, which costs no copy
	local ok, versionOk = pcall(function()
		return r2.Version.checkVersion(r2.Version.getScenarioVersionList(), r2.Version.getCurrentVersionFullList())
	end)
	if ok and versionOk then
		return true
	end

	local result
	local data = nil
	if ok then
		data = r2.getScenarioObj(true)
		if type(data) ~= "table" then return true end
		result = r2.Version.updateScenarioData(data)
	else
		-- the list of versions of the scenario can not be read
		result = { Status = "failed", Notes = {}, Steps = {}, Newer = {}, Error = true, Reason = tostring(versionOk) }
	end
	r2.Version.LastUpdate = result
	if result.Status == "failed" then
		tellUpdateFailure(result, "Session")
		return true
	end
	if not isEditionSession() then
		-- only an edition session has a scenario to replace on the server
		if result.Status == "changed" then
			debugInfo(colorTag(255, 255, 0) .. "The scenario is of an earlier version and is not updated outside of an edition session")
		end
		return true
	end
	if result.Status ~= "changed" then
		tellUpdateNotes(result)
		if result.Versions ~= nil then
			r2.requestSetScenarioVersions(result.Versions)
		end
		return true
	end
	-- the scenario as the server has it, for its author
	r2.save("old_scenario.r2")
	tellNewerClasses(result)
	tellUpdateDone(result, "Session", { "old_scenario.r2" })
	r2.requestCreateScenario(data)
	return false
end

-- The classes of the components of the scenario the editor has, with the versions of this editor,
-- without the components the editor adds for itself (the ghosts): a walk over the scenario as the
-- editor has it, which copies nothing.
local function collectSavedVersions(node, versionList)
	local nodeType = type(node)
	if nodeType == "string" or nodeType == "number" or nodeType == "boolean" or nodeType == "nil" then return end
	local className = node.Class
	if className ~= nil and className ~= "" then
		if node.Ghost == true then return end
		if versionList[className] == nil then
			local class = r2.Classes[className]
			if class ~= nil and class.Version then
				versionList[className] = class.Version
			else
				versionList[className] = 0
			end
		end
	end
	local k, v = next(node, nil)
	while k do
		collectSavedVersions(v, versionList)
		k, v = next(node, k)
	end
end

-- Gives the scenario the editor has the versions of the classes of this editor, before it is saved,
-- and gives the list to the server, which would otherwise keep the one it has: a request on the
-- scenario of the editor alone does not reach it.
-- A scenario of an earlier version that an update would change keeps the versions it has: the file
-- then says what its content is, and is updated when it is read.
function r2.Version.updateScenarioVersions()
	local scenario = r2.Scenario
	if not scenario then return end

	-- A first look at the scenario the editor has, which costs no copy: a scenario whose list names
	-- the classes of its components, and no other, at the versions of this editor is saved as it is.
	local savedList = {}
	collectSavedVersions(scenario, savedList)
	local savedVersions = scenario.Versions
	if savedVersions ~= nil then
		local same = true
		local k,v = next(savedList, nil)
		while k do
			if (savedVersions[k] ~= v) then same = false end
			k,v = next(savedList, k)
		end
		k,v = next(savedVersions, nil)
		while k do
			if (savedList[k] ~= v) then same = false end
			k,v = next(savedVersions, k)
		end
		if same then return end
	end

	-- the scenario as it is saved, without the components the editor adds for itself
	local data = r2.getScenarioObj(true)
	if type(data) ~= "table" then return end
	local list = {}
	r2.Version._getCurrentVersionListImpl(data, list)
	local update = false

	-- a scenario saved before the versions were kept has no Versions table
	local versions = data.Versions
	if type(versions) ~= "table" then
		update = true
	else
		local k,v = next(list, nil)
		while k do
			if (versions[k] ~= v) then update = true end
			k,v = next(list, k)
		end

		k,v = next(versions, nil)
		while k do
			if (list[k] ~= v) then update = true end
			k,v = next(versions, k)
		end
	end
	if not update then return end

	-- what an update makes of the scenario
	local result = r2.Version.updateScenarioData(data)
	if result.Status ~= "current" and result.Status ~= "unchanged" then return end
	-- (a class of a later version than this editor keeps the version the scenario has for it)
	if result.Versions ~= nil then list = result.Versions end
	if isEditionSession() then
		-- the scenario of the editor and the one of the server
		r2.requestSetScenarioVersions(list)
	else
		r2.requestSetGhostNode(scenario.InstanceId, "Versions", list)
	end
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
	-- (a table with an empty text for its class is of no class: the list does not name it)
	if node.Class and node.Class ~= "" and not versionList[node.Class] then
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
	if node.Class and node.Class ~= "" and not versionList[node.Class] then
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

