r2.registerHighLevel = function()
	local classMapDescriptionVersion = 0
	
	local classMapDescription =
	{
		BaseClass="BaseClass",
		Name = "MapDescription",
		DisplayerUI = "",
		ersion = classMapDescriptionVersion,
		Prop =
		{			
			{Name="Title", Type="String", Category="uiR2EDRollout_Scenario"},
			{Name="LevelId", Type="String"},
			{Name="ShortDescription", Type="String", Category="uiREDRollout_"},
			{Name="OptimalNumberOfPlayer", Type="Number", Category="uiREDRollout_", Min="1", Max="10", Default="1"},
			{Name="Creator", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText"},
			{Name="CreatorMD5", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText"},

			{Name="CreationDate", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText"},
			{Name="OtherCharAccess", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText", DefaultValue="Full", DefaultInBase = 1 },
			{Name="NevraxScenario", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText", DefaultValue="0", DefaultInBase = 1 },
			{Name="TrialAllowed", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText", DefaultValue="0", DefaultInBase = 1 },
			{Name="ScenarioTag", Type="String", Category="uiREDRollout_", WidgetStyle = "StaticText", DefaultValue="", DefaultInBase = 1 },
		},
		
	
	}	


	local classScenarioVersionName = getClientCfgVar("BuildName")
	local classScenarioVersion = 6

	-- The island of a scenario saved before a scenario had Locations. Such a scenario names its island by
	-- a number in Description.LocationId: the position of the island in the list of the editor of that
	-- time, which had the desert islands only. The first list had 28 islands, and the update step of
	-- version 1 brings its numbers to those of the second, which has the 33 islands uiR2_Deserts01 to
	-- uiR2_Deserts33 in order. Returns nil for a number that is not of that list.
	local function getIslandFromOldLocationId(locationId)
		if type(locationId) == "number" and locationId >= 0 and locationId <= 32 and math.floor(locationId) == locationId then
			return string.format("uiR2_Deserts%02d", locationId + 1)
		end
		return nil
	end

	-- The island the content of a scenario stands on: the one whose rectangle holds the most positions of
	-- the scenario. A position that is relative to a parent falls in no island and does not count.
	-- Returns nil when nothing stands on an island.
	local function getIslandFromPositions(scenario)
		local islands = getCompleteIslands()
		local counts = {}
		local function visit(node)
			if not r2.isTable(node) then return end
			local position = node.Position
			if r2.isTable(position) and type(position.x) == "number" and type(position.y) == "number" then
				for islandName, island in pairs(islands) do
					if position.x >= island.xmin and position.x <= island.xmax
						and position.y >= island.ymin and position.y <= island.ymax then
						counts[islandName] = defaulting(counts[islandName], 0) + 1
					end
				end
			end
			local k, v = next(node, nil)
			while k do
				visit(v)
				k, v = next(node, k)
			end
		end
		visit(scenario.Acts)
		local bestIsland = nil
		local bestCount = 0
		for islandName, count in pairs(counts) do
			if count > bestCount or (count == bestCount and islandName < bestIsland) then
				bestIsland = islandName
				bestCount = count
			end
		end
		return bestIsland
	end

	-- Gives a scenario without a location the Location of its island, and the acts their LocationId.
	-- A scenario saved before the Location class has one island for all its acts.
	local function updateLocations(this, update)
		local locations = this.Locations
		local firstLocationId = nil
		if type(locations) == "table" and table.getn(locations) > 0 then
			firstLocationId = locations[1].InstanceId
		else
			local locationNumber = nil
			if type(this.Description) == "table" then locationNumber = this.Description.LocationId end
			local islandFromId = getIslandFromOldLocationId(locationNumber)
			local islandFromPositions = getIslandFromPositions(this)
			local islandName = islandFromPositions
			if islandName == nil then islandName = islandFromId end
			if islandName == nil then
				-- nothing tells the island: the scenario gets the first one, and its author has to choose
				islandName = "uiR2_Deserts01"
				update:note("uiR2EDVersionNoteIslandUnknown", tostring(locationNumber), islandName)
			elseif islandFromId ~= nil and islandFromId ~= islandName then
				update:note("uiR2EDVersionNoteIslandMismatch", islandFromId, islandName)
			end
			local location = update:newComponent("Location")
			location.IslandName = islandName
			location.EntryPoint = "uiR2EntryPoint01"
			location.Name = tostring(i18n.get(islandName))
			-- the season of the server: "Automatic" says so to the translator, to the server and to the
			-- editor, which all go by the name of the season and not by ManualSeason
			location.ManualSeason = 0
			location.Season = "Automatic"
			if type(locations) ~= "table" then
				update:set(this, "Locations", {})
			end
			update:insert(this.Locations, location)
			firstLocationId = location.InstanceId
		end

		-- the base act is on every island and names none; another act names its location
		local acts = this.Acts
		for actIndex = 1, table.getn(acts) do
			local act = acts[actIndex]
			if actIndex == 1 then
				if act.LocationId == nil then
					update:set(act, "LocationId", "")
				end
			else
				-- (the id as text: a file can hold it as a reference)
				local actLocationId = update:getId(act.LocationId)
				local known = false
				if actLocationId ~= "" then
					for locationIndex = 1, table.getn(this.Locations) do
						if update:getId(this.Locations[locationIndex].InstanceId) == actLocationId then known = true end
					end
				end
				if not known then
					update:set(act, "LocationId", firstLocationId)
				end
			end
		end
	end

	-- A scenario with one act has the permanent act only, which then holds all there is: the first act
	-- of a scenario is the permanent one in every version. A test starts in an act that is not the
	-- permanent one, so the scenario gets an empty first act after it, as a new scenario has
	-- (r2.acts:createScenario).
	local function addFirstAct(this, update)
		if table.getn(this.Acts) ~= 1 then return end
		local act = update:newComponent("Act")
		act.Name = tostring(i18n.get("uiR2EDDefaultActTitle")) .. " 1"
		table.insert(act.Features, update:newComponent("DefaultFeature"))
		update:insert(this.Acts, act)
		update:note("uiR2EDVersionNoteFirstAct")
	end

	-- Removes the groups that have no member left. Earlier versions of the editor kept them; a group is
	-- read through its first member.
	local function removeEmptyGroups(this, update)
		local emptyGroups = {}
		local function visit(node)
			if type(node) ~= "table" or r2.isRefId(node) then return end
			if node.Class == "NpcGrpFeature" and type(node.Components) == "table" and table.getn(node.Components) == 0 then
				table.insert(emptyGroups, node)
				return
			end
			for k, v in pairs(node) do
				visit(v)
			end
		end
		visit(this.Acts)
		for k, group in pairs(emptyGroups) do
			update:removeNode(group)
		end
		if table.getn(emptyGroups) > 0 then
			update:note("uiR2EDVersionNoteEmptyGroups", table.getn(emptyGroups))
		end
	end

	-- The position of a component in the world: its own, with those of the components that hold it as
	-- long as it inherits them
	local function getWorldPosition(node, update)
		local x, y, z = 0, 0, 0
		while node ~= nil do
			local position = node.Position
			if type(position) == "table" then
				x = x + defaulting(tonumber(position.x), 0)
				y = y + defaulting(tonumber(position.y), 0)
				z = z + defaulting(tonumber(position.z), 0)
			end
			if node.InheritPos == 0 then break end
			node = update:getOwner(node)
		end
		return x, y, z
	end

	-- A text in UTF-8 cut to 'count' characters at most
	local function cutText(text, count)
		local characters = 0
		for i = 1, string.len(text) do
			local byte = string.byte(text, i)
			-- (a byte from 128 to 191 goes on with the character before it)
			if byte < 128 or byte >= 192 then
				characters = characters + 1
				if characters > count then return string.sub(text, 1, i - 1) end
			end
		end
		return text
	end

	-- The chat sequences of the npcs become dialogs.
	-- The first editors kept the chat sequences of an npc in its behavior: an activity step or the action
	-- "begin chat sequence" started one, and the npc played one at a time. The editor has the dialog for
	-- that since, a component of the act, and nothing plays the sequences of an npc any more. Each
	-- sequence becomes a dialog in the act of its npc, and what named the sequence names the dialog:
	--   the chat of an activity step       a handler of the npc: "begin of activity step", "starts dialog"
	--   action "begin chat sequence"       action "starts dialog" of the dialog
	--   event "end of chat sequence"       event "end of dialog"; the handler moves to the dialog
	--   event "end of chat step"           event "end of chat"; the handler moves to the dialog
	--   condition "is in chat sequence"    condition "is in dialog" of the dialog
	--   condition "is in chat step"        condition "is in chat" of the dialog
	-- An npc played one sequence at a time: a sequence that started took the place of the one that
	-- played, without an event. So what starts a dialog first deactivates the other dialogs the
	-- sequences of that npc became, and activates its own. ("stops dialog" is not used for that: it
	-- raises "end of dialog", also for a dialog that does not play.)
	-- An activity step that lasted as long as its chat sequence (the time limit "Chat") lasts the sum of
	-- the times of the steps of the sequence, as it did, and the time its dialog takes besides to say its
	-- last line (see getDialogTicks), in whole seconds; it has no time limit when the sequence has no
	-- duration or is not in the scenario, which the translator of that time refused.
	-- A sequence without a step that nothing names is removed, and so is what names a sequence the
	-- scenario does not have: the chat of an activity step, an action, a handler of an event of the
	-- sequence, and a handler with a condition on it, which could never run.
	local function convertNpcChatSequences(this, update)
		local owners = {}			-- { Entity, Act }: the entities with chat sequences
		local activitySteps = {}	-- { Step, Behavior }: the activity steps with a chat
		local handlers = {}			-- the handlers of an event of a chat sequence
		local actionSteps = {}		-- the actions "begin chat sequence"
		local conditionSteps = {}	-- the conditions on a chat sequence
		local timeLimitSteps = {}	-- the activity steps that last as long as their chat

		for actIndex = 1, table.getn(this.Acts) do
			local act = this.Acts[actIndex]
			update:walk(act, function(node)
				local class = node.Class
				if class == nil then return true end
				local nodeBehavior = node.Behavior
				if type(nodeBehavior) == "table" and type(nodeBehavior.ChatSequences) == "table"
					and table.getn(nodeBehavior.ChatSequences) > 0 then
					table.insert(owners, { Entity = node, Act = act })
				end
				if class == "ActivityStep" then
					if node.TimeLimit == "Chat" then
						table.insert(timeLimitSteps, node)
					end
					if update:getId(node.Chat) ~= "" then
						-- the behavior the step is in, through its activity sequence
						local behavior = update:getOwner(node)
						while behavior ~= nil and behavior.Class ~= "Behavior" and behavior.Class ~= "LogicEntityBehavior" do
							behavior = update:getOwner(behavior)
						end
						table.insert(activitySteps, { Step = node, Behavior = behavior })
					end
				elseif class == "LogicEntityAction" then
					local event = node.Event
					if type(event) == "table" and (event.Type == "end of chat sequence" or event.Type == "end of chat step") then
						table.insert(handlers, node)
					end
				elseif class == "ActionStep" then
					local action = node.Action
					if type(action) == "table" and action.Type == "begin chat sequence" then
						table.insert(actionSteps, node)
					end
				elseif class == "ConditionStep" then
					local condition = node.Condition
					if type(condition) == "table" and (condition.Type == "is in chat sequence" or condition.Type == "is in chat step") then
						table.insert(conditionSteps, node)
					end
				end
				return true
			end)
		end
		if table.getn(owners) == 0 and table.getn(activitySteps) == 0 and table.getn(handlers) == 0
			and table.getn(actionSteps) == 0 and table.getn(conditionSteps) == 0
			and table.getn(timeLimitSteps) == 0 then
			return
		end

		-- what is named, and the sequences an activity step repeats
		local named = {}
		local repeated = {}
		for i = 1, table.getn(activitySteps) do
			local step = activitySteps[i].Step
			local chatId = update:getId(step.Chat)
			named[chatId] = true
			if step.Type == "Repeating" then repeated[chatId] = true end
		end
		for i = 1, table.getn(handlers) do named[update:getId(handlers[i].Event.Value)] = true end
		for i = 1, table.getn(actionSteps) do named[update:getId(actionSteps[i].Action.Value)] = true end
		for i = 1, table.getn(conditionSteps) do named[update:getId(conditionSteps[i].Condition.Value)] = true end

		-- an id that names no component of the scenario any more is left out
		local function getKnownId(value)
			local id = update:getId(value)
			if id ~= "" and update:getNode(id) == nil then return "" end
			return id
		end

		-- The name of a dialog: within the length the class gives a name, and no name a component of
		-- the scenario has already. A second component of a name gets a number after it, as the editor
		-- names its components.
		local nameLimit = nil
		for k, prop in pairs(r2.Classes.ChatSequence.Prop) do
			if prop.Name == "Name" then nameLimit = tonumber(prop.MaxNumChar) end
		end
		local usedNames = {}
		update:walk(this, function(node)
			-- (the sequences of the npcs go: their names are free)
			local owner = update:getOwner(node)
			if node.Class == "ChatSequence" and owner ~= nil and owner.Class == "Behavior" then return false end
			if node.InstanceId ~= nil and type(node.Name) == "string" then usedNames[node.Name] = true end
			return true
		end)
		local function getDialogName(name)
			local index = 0
			while true do
				local suffix = ""
				if index > 0 then suffix = " " .. tostring(index) end
				local candidate = name
				if nameLimit ~= nil then candidate = cutText(name, nameLimit - string.len(suffix)) end
				-- (a name that was cut does not end with a space)
				candidate = string.gsub(candidate, " +$", "") .. suffix
				if not usedNames[candidate] then
					usedNames[candidate] = true
					return candidate
				end
				index = index + 1
			end
		end

		-- The time a dialog takes besides the times of its steps, in ticks (tenths of a second), from the
		-- action that starts it to its last line and to its end. The translator gives the group of the
		-- dialog a chain of timers (r2.Translator.createAction):
		--   "dialog_starts"     10 to the first wait
		--   "chat_step_first"   10 * Time of the first step + 1 to the first step
		--   "chat_step"         10 * Time of the next step + 4 to the next step,
		--                       2 to the line of the step, 10 when the npc turns to someone first,
		--                       10 * 3 + 4 from the last step to the end of the dialog
		-- The AI service looks at the timers of a group of a scenario every 4 ticks
		-- (CSpawnGroupNpc::update), so each timer of the chain can come 3 ticks late.
		local function getDialogTicks(stepCount, lastStepTurns)
			local late = 3
			local lastStep = (10 + late) + (1 + late) + (stepCount - 1) * (4 + late)
			local line = 2
			if lastStepTurns then line = 10 end
			return lastStep + line + late, lastStep + 10 * 3 + 4 + late
		end
		-- the longest time limit of an activity step: what the hours, minutes and seconds of its
		-- editor go up to (r2.activities:initTimeMenu)
		local maxTimeLimit = 10 * 3600 + 59 * 60 + 59

		-- the dialogs
		local dialogs = {}			-- id of a sequence or of one of its steps > its dialog
		local stepIds = {}			-- id of a step of a sequence > id of the step of the dialog
		local sequenceSeconds = {}	-- id of a sequence > the sum of the times of its steps
		local sequenceTicks = {}	-- id of a sequence > { Line, End }: what getDialogTicks gives its dialog
		local ownerDialogs = {}		-- dialog > the dialogs the sequences of its npc became
		local convertedCount = 0
		local removedCount = 0
		for i = 1, table.getn(owners) do
			local entity = owners[i].Entity
			local act = owners[i].Act
			local sequences = entity.Behavior.ChatSequences
			local entityDialogs = {}
			local x, y, z = getWorldPosition(entity, update)
			for sequenceIndex = 1, table.getn(sequences) do
				local sequence = sequences[sequenceIndex]
				local steps = sequence.Components
				if type(steps) ~= "table" then steps = {} end
				local isNamed = named[update:getId(sequence.InstanceId)] == true
				local seconds = 0
				for stepIndex = 1, table.getn(steps) do
					if named[update:getId(steps[stepIndex].InstanceId)] then isNamed = true end
					-- (a time below 0 takes no time: its timer comes at once)
					seconds = seconds + math.max(0, defaulting(tonumber(steps[stepIndex].Time), 0))
				end
				sequenceSeconds[update:getId(sequence.InstanceId)] = seconds
				if table.getn(steps) == 0 and not isNamed then
					removedCount = removedCount + 1
				else
					-- as r2.dialogs:newSequenceInst makes a dialog
					local dialog = update:newComponent("ChatSequence")
					dialog.Base = r2.Translator.getDebugBase("palette.entities.botobjects.dialog")
					local sequenceName = tostring(defaulting(sequence.Name, ""))
					if sequenceName == "" then
						sequenceName = tostring(i18n.get("uiR2EDDialog")) .. " " .. tostring(sequenceIndex)
					end
					-- (an npc that has the name of its kind has no name of its own in the scenario)
					local entityName = tostring(defaulting(entity.Name, ""))
					if entityName ~= "" then
						dialog.Name = getDialogName(entityName .. " " .. sequenceName)
					else
						dialog.Name = getDialogName(sequenceName)
					end
					dialog.Position.x = x + sequenceIndex
					dialog.Position.y = y + 1
					dialog.Position.z = z
					-- it starts when something starts it, as the sequence did
					dialog.AutoStart = 0
					if repeated[update:getId(sequence.InstanceId)] then dialog.Repeating = 1 end
					local lastStepTurns = false
					for stepIndex = 1, table.getn(steps) do
						local oldStep = steps[stepIndex]
						local oldActions = oldStep.Actions
						if type(oldActions) ~= "table" then oldActions = {} end
						-- A step of a dialog has one action: the further actions of a step follow at once.
						-- An action whose npc turns to someone comes 10 ticks after its step, and the
						-- step after it must not come before that: it would take the place of the
						-- action, which would never be played (the translator keeps one step and one
						-- timer for the action of a dialog, "chat_step" and "chat_step_end"). A
						-- further action after such an action follows a second later, which is its
						-- step 14 ticks later; that second is one of the sequence then.
						local lastStepId = nil
						for actionIndex = 1, math.max(table.getn(oldActions), 1) do
							local step = update:newComponent("ChatStep")
							step.Name = tostring(defaulting(oldStep.Name, ""))
							step.Time = 0
							if actionIndex == 1 then
								step.Time = defaulting(tonumber(oldStep.Time), 0)
							elseif lastStepTurns then
								step.Time = 1
								seconds = seconds + 1
							end
							local action = update:newComponent("ChatAction")
							local oldAction = oldActions[actionIndex]
							if type(oldAction) == "table" then
								action.Who = r2.RefId(getKnownId(oldAction.Who))
								action.Says = update:getId(oldAction.Says)
								action.Emote = update:getId(oldAction.Emote)
								action.Facing = r2.RefId(getKnownId(oldAction.Facing))
							end
							-- (the translator turns an npc it knows to an entity it knows)
							lastStepTurns = tostring(action.Facing) ~= "" and tostring(action.Who) ~= ""
							table.insert(step.Actions, action)
							table.insert(dialog.Components, step)
							lastStepId = step.InstanceId
						end
						stepIds[update:getId(oldStep.InstanceId)] = lastStepId
						dialogs[update:getId(oldStep.InstanceId)] = dialog
					end
					sequenceSeconds[update:getId(sequence.InstanceId)] = seconds
					if table.getn(dialog.Components) > 0 then
						local lineTicks, endTicks = getDialogTicks(table.getn(dialog.Components), lastStepTurns)
						sequenceTicks[update:getId(sequence.InstanceId)] = { Line = lineTicks, End = endTicks }
					end
					dialogs[update:getId(sequence.InstanceId)] = dialog
					table.insert(entityDialogs, dialog)
					ownerDialogs[dialog] = entityDialogs
					if type(act.Features) ~= "table" then update:set(act, "Features", {}) end
					update:insert(act.Features, dialog)
					convertedCount = convertedCount + 1
				end
			end
			update:set(entity.Behavior, "ChatSequences", {})
		end

		-- an action of a dialog
		local function newDialogAction(dialog, actionType)
			local actionStep = update:newComponent("ActionStep")
			actionStep.Entity = r2.RefId(dialog.InstanceId)
			actionStep.Action.Type = actionType
			actionStep.Action.Value = r2.RefId("")
			return actionStep
		end
		-- What comes before the start of a dialog: the other dialogs of its npc are deactivated, which
		-- stops one that plays without an event of its end, and the dialog is activated again.
		local function newActionsBeforeStart(dialog)
			local actions = {}
			local entityDialogs = ownerDialogs[dialog]
			for k = 1, table.getn(entityDialogs) do
				if not rawequal(entityDialogs[k], dialog) then
					table.insert(actions, newDialogAction(entityDialogs[k], "deactivate"))
				end
			end
			if table.getn(actions) > 0 then
				table.insert(actions, newDialogAction(dialog, "activate"))
			end
			return actions
		end

		-- What named a sequence. What names a sequence that does not exist is removed: the author is told
		-- how many chats of activity steps, handlers, and actions of the handlers that stay.
		local lostCount = 0
		local function removeHandler(handler)
			if update:removeNode(handler) then
				lostCount = lostCount + 1
			end
		end
		local sequenceConditionCount = 0
		for i = 1, table.getn(conditionSteps) do
			local conditionStep = conditionSteps[i]
			local condition = conditionStep.Condition
			local valueId = update:getId(condition.Value)
			local dialog = dialogs[valueId]
			if update:getParent(conditionStep) == nil then
				-- removed with its handler, for another of its conditions
			elseif dialog ~= nil then
				update:set(conditionStep, "Entity", r2.RefId(dialog.InstanceId))
				if condition.Type == "is in chat step" and stepIds[valueId] ~= nil then
					update:set(condition, "Type", "is in chat")
					update:set(condition, "Value", r2.RefId(stepIds[valueId]))
				else
					update:set(condition, "Type", "is in dialog")
					update:set(condition, "Value", r2.RefId(""))
					sequenceConditionCount = sequenceConditionCount + 1
				end
			else
				-- the condition could not be met: its handler never ran, and goes with it
				local handler = update:getOwner(conditionStep)
				if handler ~= nil and handler.Class == "LogicEntityAction" then
					removeHandler(handler)
				elseif update:removeNode(conditionStep) then
					lostCount = lostCount + 1
				end
			end
		end
		local stepEventCount = 0
		local endHandlerDialogs = {}	-- the dialogs with a handler of their end
		local repeatedEndCount = 0		-- the handlers of the end of a dialog that repeats
		for i = 1, table.getn(handlers) do
			local handler = handlers[i]
			local event = handler.Event
			local valueId = update:getId(event.Value)
			local dialog = dialogs[valueId]
			if update:getParent(handler) == nil then
				-- removed above, with a condition that could not be met
			elseif dialog ~= nil then
				if event.Type == "end of chat step" and stepIds[valueId] ~= nil then
					update:set(event, "Type", "end of chat")
					update:set(event, "Value", r2.RefId(stepIds[valueId]))
					stepEventCount = stepEventCount + 1
				else
					update:set(event, "Type", "end of dialog")
					update:set(event, "Value", r2.RefId(""))
					endHandlerDialogs[dialog] = true
					-- The first editors did not all raise the event of the end of a sequence that
					-- repeats: the earliest raised none, and the translator of that time that is
					-- left in Logic.translateChatStep raises one at each pass. Its dialog raises
					-- one at the end of each pass, unless it is stopped before that end, as the
					-- next activity step does when it starts another dialog of the npc. The
					-- handler stays, and the author is told.
					if dialog.Repeating == 1 then repeatedEndCount = repeatedEndCount + 1 end
				end
				-- the event is one of the dialog now
				update:removeNode(handler)
				update:insert(dialog.Behavior.Actions, handler)
			else
				removeHandler(handler)
			end
		end

		-- The activity steps that last as long as their chat. The timer of an activity step is ten times
		-- its time limit and 1, in ticks, and starts with the dialog (Logic.getTimeLimit, and the handler
		-- "begin of activity step" made below). The step lasts until the dialog has said its last line: the
		-- next step, when it starts another dialog of the npc, stops this one, and a line that was not
		-- said yet would be lost. It lasts until the end of the dialog when a handler waits for that
		-- end, which a dialog that was stopped does not come to; not so for a dialog that repeats,
		-- whose step lasts until the last line of the first pass whatever handler it has.
		-- No time limit is longer than its editor can show.
		local unlimitedCount = 0	-- the steps without a time limit now, and those with the longest
		for i = 1, table.getn(timeLimitSteps) do
			local step = timeLimitSteps[i]
			local chatId = update:getId(step.Chat)
			local seconds = sequenceSeconds[chatId]
			local ticks = sequenceTicks[chatId]
			local dialog = dialogs[chatId]
			if seconds ~= nil and seconds > 0 and ticks ~= nil and dialog ~= nil then
				local dialogTicks = ticks.Line
				if endHandlerDialogs[dialog] and dialog.Repeating ~= 1 then dialogTicks = ticks.End end
				local limit = math.ceil((10 * seconds + dialogTicks) / 10)
				if not (limit <= maxTimeLimit) then
					limit = maxTimeLimit
					unlimitedCount = unlimitedCount + 1
				end
				update:set(step, "TimeLimit", "Few Sec")
				update:set(step, "TimeLimitValue", tostring(limit))
			else
				-- (as the editor sets a step without a time limit)
				update:set(step, "TimeLimit", "No Limit")
				update:set(step, "TimeLimitValue", "")
				unlimitedCount = unlimitedCount + 1
			end
		end

		for i = 1, table.getn(activitySteps) do
			local step = activitySteps[i].Step
			local behavior = activitySteps[i].Behavior
			local dialog = dialogs[update:getId(step.Chat)]
			if dialog ~= nil and behavior ~= nil then
				-- (as the editor makes a handler: without a name)
				local handler = update:newComponent("LogicEntityAction")
				handler.Event.Type = "begin of activity step"
				handler.Event.Value = r2.RefId(step.InstanceId)
				local actionsBefore = newActionsBeforeStart(dialog)
				for k = 1, table.getn(actionsBefore) do
					table.insert(handler.Actions, actionsBefore[k])
				end
				table.insert(handler.Actions, newDialogAction(dialog, "starts dialog"))
				if type(behavior.Actions) ~= "table" then update:set(behavior, "Actions", {}) end
				update:insert(behavior.Actions, handler)
			else
				lostCount = lostCount + 1
			end
			update:set(step, "Chat", r2.RefId(""))
		end
		for i = 1, table.getn(actionSteps) do
			local actionStep = actionSteps[i]
			local dialog = dialogs[update:getId(actionStep.Action.Value)]
			local actions = update:getParent(actionStep)
			if actions == nil then
				-- removed above, with its handler
			elseif dialog ~= nil then
				update:set(actionStep, "Entity", r2.RefId(dialog.InstanceId))
				update:set(actionStep.Action, "Type", "starts dialog")
				update:set(actionStep.Action, "Value", r2.RefId(""))
				local position = nil
				for k = 1, table.getn(actions) do
					if rawequal(actions[k], actionStep) then position = k end
				end
				if position ~= nil then
					local actionsBefore = newActionsBeforeStart(dialog)
					for k = 1, table.getn(actionsBefore) do
						update:insert(actions, actionsBefore[k], position + k - 1)
					end
				end
			elseif update:removeNode(actionStep) then
				lostCount = lostCount + 1
			end
		end

		if convertedCount > 0 then
			update:note("uiR2EDVersionNoteChatSequences", convertedCount)
		end
		if removedCount > 0 then
			update:note("uiR2EDVersionNoteEmptyChatSequences", removedCount)
		end
		if lostCount > 0 then
			update:note("uiR2EDVersionNoteLostChatSequences", lostCount)
		end
		if unlimitedCount > 0 then
			update:note("uiR2EDVersionNoteChatTimeLimits", unlimitedCount)
		end
		if sequenceConditionCount > 0 then
			update:note("uiR2EDVersionNoteChatSequenceConditions", sequenceConditionCount)
		end
		if stepEventCount > 0 then
			update:note("uiR2EDVersionNoteChatStepEvents", stepEventCount)
		end
		if repeatedEndCount > 0 then
			update:note("uiR2EDVersionNoteChatSequenceEnds", repeatedEndCount)
		end
	end

	-- The way an npc moves, as the first editors kept it: the behavior of the npc had the kind of
	-- movement in Type and the zone or the road in ZoneId. The activity sequences have replaced that, and
	-- the translator has nothing left to read the old form with. Such a behavior gets one activity
	-- sequence with one step that says the same, without a time limit.
	local function convertNpcMovements(this, update)
		local activities = {
			["wander"] = "Wander",
			["follow_route"] = "Follow Route",
			["patrol_route"] = "Patrol",
			["repeat_road"] = "Repeat Road",
		}
		local behaviors = {}
		update:walk(this.Acts, function(node)
			if node.Class == "Behavior" and activities[node.Type] ~= nil
				and (type(node.Activities) ~= "table" or table.getn(node.Activities) == 0) then
				table.insert(behaviors, node)
			end
			return true
		end)
		local lostCount = 0
		for i = 1, table.getn(behaviors) do
			local behavior = behaviors[i]
			local zoneId = update:getId(behavior.ZoneId)
			if zoneId ~= "" and update:getNode(zoneId) ~= nil then
				local step = update:newComponent("ActivityStep")
				step.Activity = activities[behavior.Type]
				step.ActivityZoneId = r2.RefId(zoneId)
				step.TimeLimit = "No Limit"
				step.TimeLimitValue = "0"
				local sequence = update:newComponent("ActivitySequence")
				table.insert(sequence.Components, step)
				if type(behavior.Activities) ~= "table" then update:set(behavior, "Activities", {}) end
				update:insert(behavior.Activities, sequence)
			else
				-- the zone or the road is not in the scenario: the npc stands still
				lostCount = lostCount + 1
			end
			update:set(behavior, "Type", "")
			update:set(behavior, "ZoneId", "")
		end
		if lostCount > 0 then
			update:note("uiR2EDVersionNoteLostMovements", lostCount)
		end
	end

	-- The entry points of the islands had names of their own in the first lists (uiR2SouthEntryPoint and
	-- so on); the list of this editor numbers them. The server starts a location whose entry point the
	-- island does not have at the first entry point of the island, which is, for all but one of the 28
	-- islands of the first list, the place the one named entry point of the island was. The location
	-- names that entry point now, so that the editor shows where the scenario starts. Nothing is done for
	-- an island the editor has no data of.
	local function updateEntryPointNames(this, update)
		if type(this.Locations) ~= "table" then return end
		local islands = getCompleteIslands()
		local count = 0
		for i = 1, table.getn(this.Locations) do
			local location = this.Locations[i]
			local entryPoint = location.EntryPoint
			local island = islands[location.IslandName]
			if type(entryPoint) == "string" and type(island) == "table" and type(island.entrypoints) == "table"
				and island.entrypoints[entryPoint] == nil then
				local firstName = nil
				for name, point in pairs(island.entrypoints) do
					if firstName == nil or name < firstName then firstName = name end
				end
				if firstName ~= nil then
					update:set(location, "EntryPoint", firstName)
					count = count + 1
				end
			end
		end
		if count > 0 then
			update:note("uiR2EDVersionNoteEntryPoints", count)
		end
	end

	local classScenario =
	{
		--BaseClass="BaseClass",
		BaseClass="LogicEntity",
		Name = "Scenario",
		InEventUI = true,
		Version = classScenarioVersion,
		VersionName =classScenarioVersionName,
		BuildPropertySheet = false,
		Prop = 
		{	
			{Name="AccessRules", Type="String", WidgetStyle = "StaticText"},
			{Name="Description", Type="MapDescription"},
			{Name="Acts", Type="Table"},
			{Name="Locations", Type="Table"},
			{Name="Texts", Type="TextManager"},
			{Name="VersionName", Type="String", DefaultValue=classScenarioVersionName, Visible=false }, -- just a string			
			{Name="Versions", Type="Table"}, 
			{Name="UserComponents", Type="Table"}, 
			{Name="PlotItems", Type="Table"},
			{Name="Language", Type="String", DefaultValue="en", Visible=false},
			{Name="Type", Type="String", DefaultValue="so_story_telling", Visible=false},
			--{Name="TestRefId", Type="RefId", Category="uiR2EDRollout_Test"},
			{Name="ScenarioScore", Type="Number", DefaultValue="0", Min="0", Visible=false},
			{Name="Score", Type="Number"},
		},
		Parameters = {},
		ApplicableActions = { 
			--"Start Scenario Timing", "Stop Scenario Timing", "add scenario points", "succeed scenario", "fail scenario"
		},
		Events = {
		--	"on scenario succeeded", "on scenario failed"
		},
		Conditions = {
		--	"is active", "is finished"
		},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		----------------------------------------------------------------------------------------------------
		isDisplayModeOptionAvailable = function(this) return false end,
		----------------------------------------------------------------------------------------------------
		updateVersion = function(this, scenarioValue, currentValue, update)
			local patchValue = scenarioValue
			local description = this.Description
			if patchValue < 1 then
				-- Patch only for old save (not the 0.0.3)
				if this.VersionName ~= "0.0.3" and type(description) == "table" then
					-- TODO use identifier instead
					local oldValue = description.LocationId
					local newValue = oldValue

					if type(oldValue) ~= "number" then
						-- no location number to bring to the second list
					elseif 0 == oldValue  then
						newValue = 1
					elseif 1 <= oldValue and oldValue <= 5 then
						newValue = oldValue + 2
					elseif 6 == oldValue then
						newValue =  9
					elseif 7 == oldValue then
						newValue =  11
					elseif 8 <= oldValue and oldValue <= 27 then
						newValue = oldValue + 5
					end

					if newValue ~= nil then
						update:set(description, "LocationId", newValue)
					end
				end
				patchValue = 1
			end

			-- plot item feature
			if patchValue < 2 then
				if this.PlotItems == nil then update:set(this, "PlotItems", {}) end
				if this.UserComponents == nil then update:set(this, "UserComponents", {}) end
				patchValue = 2
			end

			-- update of enum for scenario type
			if patchValue < 3 then
				if this.Type == nil or this.Type == "" then
					update:set(this, "Type", "so_story_telling")
				elseif this.Type == "Roleplay" then
					update:set(this, "Type", "so_story_telling")
				elseif this.Type == "Combat" then
					update:set(this, "Type", "so_hack_slash")
				end
				patchValue = 3
			end

			-- patch for level. The level is a number from 0 to 5, also where a file has it as text.
			if patchValue < 4 then
				if type(description) == "table" then
					local levelId = tonumber(description.LevelId)
					if levelId == nil or levelId < 0 or levelId > 5 then
						update:set(description, "LevelId", 0)
					end
				end
				patchValue = 4
			end

			-- version 5: the parts a scenario saved before they existed lacks. Earlier versions of the
			-- editor ran the steps above without their changes taking effect, and saved the scenario as
			-- updated: a scenario they updated says version 4 and lacks those parts too.
			if patchValue < 5 then
				-- the scenario is a logic entity
				if this.Behavior == nil then
					update:set(this, "Behavior", update:newComponent("LogicEntityBehavior"))
				end
				if this.PlotItems == nil then
					update:set(this, "PlotItems", {})
				end
				if this.UserComponents == nil then
					update:set(this, "UserComponents", {})
				end
				if this.Texts == nil then
					update:set(this, "Texts", update:newComponent("TextManager"))
				end
				if type(description) == "table" then
					local levelId = tonumber(description.LevelId)
					if levelId == nil or levelId < 0 or levelId > 5 then
						update:set(description, "LevelId", 0)
					end
				end
				if type(this.Acts) == "table" then
					removeEmptyGroups(this, update)
					addFirstAct(this, update)
					updateLocations(this, update)
				end
				patchValue = 5
			end

			-- version 6: what is left of the first editors. The chat sequences of the npcs become
			-- dialogs, the movement kept in the behavior of an npc becomes an activity sequence, and a
			-- location names an entry point its island has.
			if patchValue < 6 then
				if type(this.Acts) == "table" then
					convertNpcChatSequences(this, update)
					convertNpcMovements(this, update)
				end
				updateEntryPointNames(this, update)
				patchValue = 6
			end
			if patchValue == currentValue then return true end
			return false
		end,

		---------------------------------------------------------------------------------------------------------
		-- get name of tree icon 
		getContextualTreeIcon = function(this)
			return this:getTreeIcon()
		end,	
		
		-----------------------------------------------------------------------------
		getBaseAct = function(this)
			return this.Acts[0]
		end,

		getCurrentAct = function(this)
			return r2:getCurrentAct()
		end,
		-----------------------------------------------------------------------------
		-- from baseClass
		getDisplayName = function(this)
			return i18n.get("uiR2EDScenario")
		end,
		-----------------------------------------------------------------------------
		-- from baseClass
		isDeletable = function(this)
			return false
		end,
		-----------------------------------------------------------------------------
		--
		getBaseActLeftBudget = function(this)
			local maxValue = this:getMaxSecondaryActCost()
			return this.Description.MaxEntities - this:getBaseAct():getLocalCost() - maxValue
		end,
		-----------------------------------------------------------------------------
		-- Called by the C++ just before testing to see if the cost of this scenario is valid
		-- Should return true in this case
		-- If the scenario has a too high cost, then this function has the responsability to warn the player
		validateForTesting = function(this)
			if this:getMaxSecondaryActCost() < 0 then
				messageBox(i18n.get("uiR2EDScenarioBudgetExceeded"))
				return false
			else
				return true
			end
		end,
		-----------------------------------------------------------------------------
		--
		getMaxSecondaryActCost = function(this)
			local scenarioObj = r2.getScenarioObj()
			local max = 0
			local maxStatic = 0
			local first = true
			k,v = next(this.Acts,nil)
			while k~=nil do
				if first == true then
					first = false
				else
					if max < v:getLocalCost() then
							max = v:getLocalCost()
					end
					if maxStatic < v:getLocalStaticCost() then
						debugInfo("setting maxStatic")
						maxStatic = v:getLocalStaticCost()
					end
				end
				k,v=next(this.Acts,k)
			end

			return max, maxStatic
		end,
		-----------------------------------------------------------------------------
		-- returns a table with all objects of kind "kind" in the permanent act & current act
		getAllInstancesByType = function(this, kind)
			assert(type(kind) == "string")
			local result = {}			
			--this:delegate():appendInstancesByType(result, kind)
			r2.Classes.BaseClass.appendInstancesByType(this, result, kind)
			this.Acts[0]:appendInstancesByType(result, kind)
			local currAct = r2:getCurrentAct()
			if currAct ~= nil and currAct ~= this.Acts[0] then
				currAct:appendInstancesByType(result, kind)
			end
			return result				
		end,
		-----------------------------------------------------------------------------
		-- from baseClass
		completeSelectBarMenu = function(this, rootMenu)
			-- if all acts are not used, propose to create a new one
			--rootMenu:addSeparator()
			--r2:addMenuLine(rootMenu, this:getDisplayName(), "lua", "r2:setSelectedInstanceId('" .. this.InstanceId .."')", tostring(k), this:getSelectBarIcon(), 14)
			--rootMenu:addSeparator()
			--r2:addMenuLine(rootMenu, i18n.get("uiR2EDNewAct"), "lua", "r2.ScenarioWindow:newAct()", "new_act", "r2_icon_create.tga", 14)			
		end,
		-----------------------------------------------------------------------------
		-- from baseClass
		displayInSelectBar = function(this)
			return false -- don't display in the selection bar (only acts can be ...)
		end,
		--
		---------------------------------------------------------------------------------------------------------
		-- from base class		
		getFirstSelectBarSon = function(this)
			return r2:getCurrentAct():getFirstSelectBarSon()
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class		
		canHaveSelectBarSons = function(this)
			return false;
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class, update the item ui on first display
		onPostCreate = function(this)
			r2.PlotItemsPanel:reinit()
			r2.PlotItemDisplayerCommon:touch()
			if r2.Mode == "Edit" then
				-- if some element in the scenario are hidden, then display
				-- a message for the user to remember it
				-- local sons = {}
				-- this:getSons(sons)
				-- local showWarning = false
				-- for k, v in pairs(sons) do
				--	if type(v) == "userdata" and  v.isKindOf and v:isKindOf("WorldObject") and v.DisplayMode ~= 0 then
				--		showWarning = true
				--		break
				--	end
				-- end
				-- if showWarning then
				-- 	messageBox(i18n.get("uiR2EDDisplayModeMenuReminder"))
				-- end
				-- reset display modes
				r2.PrimDisplayFrozen = false
				r2.PrimDisplayVisible = true
				r2.PrimDisplayContextualVisibility = false
				r2.BotObjectsFrozen = false
				r2:setupFreezeBotObjectButton()
			end			
		end,

		onAttrModified = function(this, name)
			if this == r2.Scenario then
				if name ~= "Acts" then
					r2.ScenarioWindow:updateScenarioProperties()
				end

				if name=="Acts" and r2.Scenario:getCurrentAct()==r2.Scenario:getBaseAct() and r2.Scenario.Acts.Size>1 then
					r2.ScenarioWindow:setAct( r2.Scenario.Acts[1] )
				end
			end
		end,

		-----------------------------------------------------------------------------
		onErase = function(this)
			r2.acts.deleteOldScenario = false
		end,
		getName = function(this)
			if this.Ghost_Name then return this.Ghost_Name end
			return this.Name
		end,
	}

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function classScenario.initLogicEntitiesMenu(this, logicEntityMenu)
		local name = i18n.get("uiR2EDScenario")
		logicEntityMenu:addLine(name, "lua", "", "Scenario")
	end

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	function classScenario.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
		local name = r2.Scenario:getName()
		subMenu:addLine(name, "lua", calledFunction.."('".. r2.Scenario.InstanceId .."')", r2.Scenario.InstanceId)
	end

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function classScenario:getLogicTranslations()
		local logicTranslations = {
			["ApplicableActions"] = {
					--["Start Scenario Timing"]	= { menu=i18n.get( "uiR2AA0ScenarioStartTiming" ), 
					--						text=i18n.get( "uiR2AA1ScenarioStartTiming" )},
					--["Stop Scenario Timing"]	= { menu=i18n.get( "uiR2AA0ScenarioStopTiming" ), 
					--						text=i18n.get( "uiR2AA1ScenarioStopTiming" )},
					--["add scenario points"]	= { menu=i18n.get( "uiR2AA0ScenarioAddPoints" ), 
					--						text=i18n.get( "uiR2AA1ScenarioAddPoints" )},
					--["succeed scenario"]	= { menu=i18n.get( "uiR2AA0ScenarioSucceed" ), 
					--						text=i18n.get( "uiR2AA1ScenarioSucceed" )},
					--["fail scenario"]	= { menu=i18n.get( "uiR2AA0ScenarioFail" ), 
					--						text=i18n.get( "uiR2AA1ScenarioFail" )},
			},
			["Events"] = {	
				--["On Scenario Started"]	= { menu=i18n.get( "uiR2Event0ScenarioStart" ), 
				--							text=i18n.get( "uiR2Event1ScenarioStart" )},
				--["on scenario succeeded"]	= { menu=i18n.get( "uiR2Event0ScenarioSucceed" ), 
				--							text=i18n.get( "uiR2Event1ScenarioSucceed" )},
				--["on scenario failed"]	= { menu=i18n.get( "uiR2Event0ScenarioFailed" ), 
				--							text=i18n.get( "uiR2Event1ScenarioFailed" )},
			},
			["Conditions"] = {	
			}
		}
		return logicTranslations
	end
	
	function classScenario.initEventValuesMenu(this, menu, categoryEvent)	
		for ev=0,menu:getNumLine()-1 do

			local eventType = tostring(menu:getLineId(ev))

			if r2.events.eventTypeWithValue[eventType] == "Number" then
				menu:addSubMenu(ev)
				local subMenu = menu:getSubMenu(ev)
				local func = ""

				local lineNb = 0
				for i=1, 100, 20 do
					local lineStr = tostring(i).."/"..tostring(i+19)
					subMenu:addLine(lineStr, "", "", tostring(i))

					subMenu:addSubMenu(lineNb)
					local subMenu2= subMenu:getSubMenu(lineNb)
					for s=0, 19 do
						lineStr = tostring(i+s) 
						local func = "r2.events:setEventValue('','" .. categoryEvent .."','".. lineStr.."')"
						subMenu2:addLine(lineStr, "lua", func, lineStr)
					end
					lineNb = lineNb+1
				end
			end
		end
	end

	function classScenario.pretranslate(this, context)
		-- Nothing to do: Done by act[0]:pretranslate
	end

	function classScenario.translate(this, context)
		r2.Translator.translateAiGroup(this, context)
	end


	function classScenario.getLogicEvent(this, context, event)
		assert( event.Class == "LogicEntityAction") 

		local component =  this -- r2:getInstanceFromId(event.Entity)
		assert(component)
		local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
		assert(rtNpcGrp)

		local eventType = tostring(event.Event.Type)
		
		local eventHandler, firstCondition, lastCondition = nil, nil, nil
		
		local rtNpcGrp = r2.Translator.getRtGroup(context, r2.Scenario.Acts[0].InstanceId)
		assert(rtNpcGrp)

		if eventType == "On Scenario Started" then 
			return r2.Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)
		elseif eventType == "on scenario succeeded" then
			return r2.Translator.getComponentUserEvent(rtNpcGrp, 7)
		elseif eventType == "on scenario failed" then
			return r2.Translator.getComponentUserEvent(rtNpcGrp, 8)
		end
		
		return eventHandler, firstCondition, lastCondition
	end
	
	function classScenario.getLogicAction(entity, context, action)
		assert( action.Class == "ActionStep") 
		local component = r2:getInstanceFromId(action.Entity)
		assert(component)
		local rtNpcGrp = r2.Utils.getRtGroup(context, component.InstanceId)
		assert(rtNpcGrp)
		
		if (action.Action.Type == "Start Scenario Timing") then	
			local action = r2.Translator.createAction("start_scenario_timing")
			return action, action
		elseif (action.Action.Type == "Stop Scenario Timing") then	
			local action = r2.Translator.createAction("stop_scenario_timing")
			return action, action
		elseif (action.Action.Type == "add scenario points") then
			if not action.Action.ValueString then return end
			local points = tonumber(action.Action.ValueString)
			local baseAct = r2.Scenario:getBaseAct()
			local rtBaseActGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)

			local action = r2.Translator.createAction("add_scenario_points", rtBaseActGrp.Id, points)
			return action, action
		elseif action.Action.Type == "succeed scenario" then
			local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Success", 1)
			local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 7)
			local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
			assert(retAction)
			return retAction, retAction
		elseif action.Action.Type == "fail scenario" then
			local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Success", 0)
			local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 8)
			local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
			assert(retAction)
			return retAction, retAction
		end

		return r2.Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
	end

	function classScenario.getLogicCondition(this, context, condition)
		assert( condition.Class == "ConditionStep") 
		local component = r2:getInstanceFromId(condition.Entity)
		assert(component)
		local rtNpcGrp = r2.Utils.getRtGroup(context, component.InstanceId)
		assert(rtNpcGrp)

		return r2.Translator.getFeatureActivationCondition(condition, rtNpcGrp)
	end



	function classScenario.getActIndex(this, actInstanceId)
		local index = 0
		local k, v = next(this.Acts)
		while k do
			if tostring(v.InstanceId) == actInstanceId then
				return index
			end
			index = index + 1
			k, v = next(this.Acts, k)
		end
		return -1
	end


	-- maps each season found in the list box season to an enum to pass to setEditorSeason
	r2.ListBoxSeasonToEditorSeason = 
	{
		[0] = "Automatic",
		[1] = "Spring",
		[2] = "Summer",
		[3] = "Autumn",
		[4] = "Winter"
	}	


	local classActVersion = 7

	local classAct =
	{
		BaseClass="LogicEntity",
		Name = "Act",
		InEventUI = true,	
		Menu="ui:interface:r2ed_base_menu",
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "createActUIDisplayer",
		TreeIcon="r2ed_icon_act.tga",
		Version=classActVersion,
		Parameters = {},
		ApplicableActions = {
			"Start Act",
		},
		Events = {
			"On Act Started", 
			--"On Scenario Started", 
		},
		Conditions = {
		--	"is active", "is finished"
		},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},
		Prop = 
		{			
			{Name="Version", Type="Number", Visible=false, DefaultValue=tostring(classActVersion)},
			{Name="Features", Type="Table"},						
			{Name="Events",Type="Table"},

			-- following field are tmp for property sheet building testing
			-- {Name="PVP", Type="Number", WidgetStyle="Boolean", Category="uiR2EDRollout_Test" },
			-- {Name="Slider1", Type="Number", WidgetStyle="Slider", Category="uiR2EDRollout_Test"},
			-- {Name="Slider2", Type="Number", WidgetStyle="Slider", Category="uiR2EDRollout_Test"},
			-- {Name="ComboBox1", Type="Number", WidgetStyle="EnumDropDown",
			--	Enum= { "Toto", "Tata", "Titi" }
			--},
			-- {Name="ComboBox2", Type="Number", WidgetStyle="EnumDropDown",
			-- Enum= { "A", "B", "C" }
			-- },
			{Name="Title", Type="String", WidgetStyle = "StaticText"},
			{Name="Name", Type="String", MaxNumChar="25"},
			

			{Name="ActivitiesIds",Type="Table"},
			{Name="Counters",Type="Table"},
			{Name="ManualWeather", Type="Number", WidgetStyle="Boolean", DefaultValue="0",
			 Visible = function(act) return not act:isBaseAct() end,
			},
			{Name="WeatherValue", Type="Number", WidgetStyle="Slider", Min=0, Max=1022, -- The final value in the Rt datas is 0 for autiweather and 1-1023 for weather value ([1, 1023] range <=> [0, 1022])
			 Visible = function(act) return act.ManualWeather == 1 and not act:isBaseAct() end,
			},
			{Name="Season", Type="Number", WidgetStyle="EnumDropDown", DefaultValue="0", Visible=false,
			  Enum= { "uiR2EDSeasonAuto", "uiR2EDSpring", "uiR2EDSummer", "uiR2EDAutumn", "uiR2EDWinter" },			  
			},
			{Name="LocationId", Type="String", Visible=false},
			{Name="ShortDescription", Type="String", Visible=false},
			{Name="PreActDescription", Type="String", DefaultValue="", Visible=false, DefaultInBase=1},
		},

		updateVersion = function(this, scenarioValue, currentValue, update)
			local patchValue = scenarioValue
			if patchValue < 1 then
				if this.ManualWeather == nil then update:set(this, "ManualWeather", 0) end
				if this.WeatherValue == nil then update:set(this, "WeatherValue", 0) end
				patchValue = 1
			end

			if patchValue < 2 then
				if not this.Behavior then
					update:set(this, "Behavior", update:newComponent("LogicEntityBehavior"))
					if this.InheritPos == nil then update:set(this, "InheritPos", 1) end
					-- TODO Add position (0,0,0)
				end
				patchValue = 2

			end
			if patchValue < 3 then
				if not this.Name then
					update:set(this, "Name", defaulting(this.Title, ""))
				end
				patchValue = 3
			end
			if patchValue < 4 then
				patchValue = 4
			end
			-- version 5 : Remove the "Cost" field -> hold locally now
			if patchValue < 5 then
				update:erase(this, "Cost")
				update:erase(this, "StaticCost")
				patchValue =  5
			end

			if patchValue < 6 then
				update:erase(this, "ExportList")
				patchValue = 6
			end

			-- version 7: what the steps of versions 1 to 3 set. Earlier versions of the editor ran them
			-- without their changes taking effect, and saved the act as updated. (An act without
			-- ManualWeather or InheritPos gets the default of its class when it is read, which is the
			-- value those steps set.)
			if patchValue < 7 then
				if this.Behavior == nil then
					update:set(this, "Behavior", update:newComponent("LogicEntityBehavior"))
				end
				if this.WeatherValue == nil then
					update:set(this, "WeatherValue", 0)
				end
				if this.Name == nil then
					update:set(this, "Name", defaulting(this.Title, ""))
				end
				patchValue = 7
			end
			if patchValue == currentValue then return true end
			return false
		end,
		-----------------------------------------------------------------------------	
		canChangeDisplayMode = function(this)
			return false
		end,	
		-----------------------------------------------------------------------------
		onActChanged = function(this)
			assert(this)
			if this == r2:getCurrentAct() then					
				r2.acts:updatePaletteFromEcosystem()
				r2.ScenarioWindow:updateActProperties()	
			end
		end,
		-----------------------------------------------------------------------------
		onAttrModified = function(this, name)			
			if name == "Features" or Name == "Events" then
				-- ignore messages triggeered by sons for the update of the property window
				return
			end
			r2.ScenarioWindow:updateActProperties()
		end,
		-----------------------------------------------------------------------------
		onErase = function(this)

			this.User.Deleted = true

			if this.User.DeleteInProgress == true then return end

			this.User.DeleteInProgress = true
			this:setDeleteActionName()
			-- assume than on delete can only be called if this act is selected
			--assert(this:isSameObjectThan(r2:getCurrentAct()))

			-- update name of acts in act combo box 
			local afterDeletedAct = false
			for i=0, r2.Scenario.Acts.Size-1 do
				local act = r2.Scenario.Acts[i]
				if afterDeletedAct then
					r2.ActUIDisplayer:updateActName(act)
				elseif act==this then
					afterDeletedAct = true
				end
			end

			-- if Act[1] exists go to act1
			if not r2.acts.deleteOldScenario then
				if (table.getn(r2.Scenario.Acts) > 1) and (this~=r2.Scenario.Acts[1])  then
					r2.ScenarioWindow:setAct( r2.Scenario.Acts[1] )
				else
					r2:setCurrentActFromId(r2.Scenario:getBaseAct().InstanceId)

					if r2.logicComponents.undoRedoInstances[this.InstanceId] and (table.getn(r2.Scenario.Acts) <= 2) then
						r2.acts:openScenarioActEditor(false, true, true)
					end
				end
			end

			if r2.logicComponents.undoRedoInstances[this.InstanceId] then
				r2.logicComponents.undoRedoInstances[this.InstanceId] = nil
			end	
		end,

		-----------------------------------------------------------------------------
		updateWeather = function(this)					
			if this==r2:getCurrentAct() and this.WeatherValue and this.ManualWeather then
				setWeatherValue(this.ManualWeather == 0, this.WeatherValue / 1022)
			end
		end,
		-----------------------------------------------------------------------------
		accept = function(this, targetInstance)
			if targetInstance:isKindOf("BanditCampFeature") then
				return "Features"
			else
				return nil
			end			
		end,
		getSelectBarIcon = function(this)
			return "r2ed_icon_act.tga"
		end,
		-----------------------------------------------------------------------------
		isBaseAct = function(this)
			local parentScenario = this:getParentScenario()
			if not parentScenario then return false end
			return this:isSameObjectThan(parentScenario:getBaseAct())
			--return this == this:getParentScenario():getBaseAct()
		end,
		
		-----------------------------------------------------------------------------
		-- get the tree control where object other than botobject are inserted in this act
		getContentTree = function(this)			
			return this.User.ContentTree
		end,

		-----------------------------------------------------------------------------
		-- get the tree control where macro components are inserted in this act
		getMacroContentTree = function(this)	
			return this.User.MacroContentTree
		end,

		-----------------------------------------------------------------------------
		-- get the tree control where object other than botobject are inserted in this act
		getContentTreeNodes = function(this, nodeName)	
		
			local nodes = {}
			if this:isBaseAct() then
				local acts = this.Parent
				if acts.Size>1 then
					for i=1, acts.Size-1 do
						if acts[i]:getContentTree() then
							local node 
							if nodeName then
								node = acts[i]:getContentTree():getRootNode():getNodeFromId(nodeName)
							else
								node = acts[i]:getMacroContentTree():getRootNode()
							end
							nodes[acts[i].InstanceId] = node
						else
							return nil
						end
					end
				end
			else
				local node 
				if nodeName then
					node = this:getContentTree():getRootNode():getNodeFromId(nodeName)
				else
					node = this:getMacroContentTree():getRootNode()
				end
				nodes[this.InstanceId] = node
			end	
			return nodes
		end,

		-----------------------------------------------------------------------------
		-- get the default feature for this act
		getDefaultFeature = function(this)
			assert(this.Features[0]:isKindOf("DefaultFeature"))
			return this.Features[0]
		end,
		-----------------------------------------------------------------------------		
		hasScenarioCost = function(this)
			return true
		end,

		getName = function(this)
			assert(this)

			local name = this.Name
			local actNb = r2.logicComponents:searchElementIndex(this)-1

			local firstPart = i18n.get("uiR2EDDefaultActTitle").. actNb 
			local firstPartSpace = i18n.get("uiR2EDDefaultActTitle").. " " .. actNb 
			if name=="" then
				name = firstPartSpace
			elseif string.lower(name)==string.lower(firstPart) or string.lower(name)==string.lower(firstPartSpace) then
			else
				name = firstPartSpace .. ":" .. name
			end

			return name
		end,
		setName = function(this, value)
			assert(this)
			this.Name = value
		end,
		-----------------------------------------------------------------------------		
		-- from baseClass
		getDisplayName = function(this)
			if this:isBaseAct() then
				return i18n.get("uiR2EDBaseAct")
			end
			return this:getName()
		end,
		-----------------------------------------------------------------------------
		-- from baseClass
		isDeletable = function(this)
			return not this:isBaseAct()
		end,
		---------------------------------------------------------------------------------------------------------
		-- called when the instance is selected (default is no op)
		onSelect = function(this, selected)
			if selected and this ~= r2:getCurrentAct() then
				-- act was changed from the select bar, update editor state	
				r2.ScenarioWindow:setAct(this)
			end
		end,
		---------------------------------------------------------------------------------------------------------
		-- from baseClass : 
		-- special : the scenario is not displayed in the select bar, but serves as a 'root' for enumeration so
		-- we return it when acts menu is popped in the selectbar, so that acts can be enumerated
		getFirstSelectBarParent = function(this)
			return this.ParentInstance
		end,
		---------------------------------------------------------------------------------------------------------
		-- from baseClass :
		getSelectBarSons = function(this)
			return this.Features
		end,
		
		---------------------------------------------------------------------------------------------------------
		-- from BaseClass
		getParentAct = function(this)
			return this
		end	
	}

	function classAct.getActivitiesIds(this)
		local actActivitiesIds = {}
		local k, v = next(this.Features, nil)
		while k do
			local activitiesIds = {}
			if v.getActivitiesIds then
				activitiesIds = v:getActivitiesIds()
				table.merge(actActivitiesIds, activitiesIds)
			end
			k, v = next(this.Features, k)
		end
		return actActivitiesIds
	end

	function classAct.getWorldPos(this)
		return { x = 0, y = 0, z = 0 }
	end

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	function classAct.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)

		local empty = true
		local actsTable = r2.Scenario.Acts 

		for i=0, r2.Scenario.Acts.Size-1 do
			local act = r2.Scenario.Acts[i]
			if not act:isBaseAct() then
				subMenu:addLine(act.Name, "lua", calledFunction.."('".. act.InstanceId .."')", act.InstanceId)
				empty = false
			end
		end

		if empty==true then
			subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
		end
	end

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function classAct:getLogicTranslations()
		local logicTranslations = {
			["ApplicableActions"] = {
				["Start Act"]			= { menu=i18n.get( "uiR2AA0ActStart"	), 
											text=i18n.get( "uiR2AA1ActStart"	)},
			},
			["Events"] = {	
				["On Act Started"]		= { menu=i18n.get( "uiR2Event0ActStart"	), 
											text=i18n.get( "uiR2Event1ActStart"	)},
			},
			["Conditions"] = {	
			}
		}
		return logicTranslations
	end

	-----------------------------------------------------------------------------
	-- eval the used quota for this act alone
	function classAct.getUsedQuota(this)
		return this:getLocalCost()
	end	

	-----------------------------------------------------------------------------
	function classAct.appendInstancesByType(this, destTable, kind)				
		assert(type(kind) == "string")
		--this:delegate():appendInstancesByType(destTable, kind)
		r2.Classes.BaseClass.appendInstancesByType(this, destTable, kind)
		for key, feature in specPairs(this.Features) do	
			feature:appendInstancesByType(destTable, kind)
		end
	end
	-----------------------------------------------------------------------------
	function classAct.onDelete(this)
		if this.User.DeleteInProgress == true then return end

		-- assume than on delete can only be called if this act is selected
		assert(this:isSameObjectThan(r2:getCurrentAct()))

		r2.ScenarioWindow:deleteAct()	
	end

	-----------------------------------------------------------------------------
	-- return the left quota to add content into this act
	function classAct.getLeftQuota(this)
		if this:isBaseAct()
		then
			local maxValue, maxValue2 = this:getParentScenario():getMaxSecondaryActCost()
			local leftQuota = r2.getMaxNpcs() - this:getLocalCost() - maxValue
			local leftStaticQuota = r2.getMaxStaticObjects() - this:getLocalStaticCost() - maxValue2

			return leftQuota, leftStaticQuota
		else
			--return this:getParentScenario().Description.MaxEntities - this:getParentScenario():getBaseAct().Cost - this.Cost
			local cost =  r2.getMaxNpcs() - this:getParentScenario():getBaseAct():getLocalCost() - this:getLocalCost()
			local staticCost = r2.getMaxStaticObjects() - this:getParentScenario():getBaseAct():getLocalStaticCost() - this:getLocalStaticCost()

			return cost, staticCost 
		end
	end

	function classAct.getActId(this)
		assert(this)
		local parent = this:getParentScenario()
		local id = -1;
		local k,v = next(parent.Acts, nil)
		while k do
			id = id + 1
			if this:isSameObjectThan(v) then
				return id
			end
			k,v = next(parent.Acts, k)
		end
		return id -- -1
		
	end

	function classAct.getLogicAction(entity, context, action)
		
		assert( action.Class == "ActionStep") 
		local component = r2:getInstanceFromId(action.Entity)
		assert(component)
		local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
		assert(rtNpcGrp)
		local  firstAction, lastAction = nil, nil
		if action.Action.Type == "Start Act" then
			firstAction, lastAction = r2.Translator.createAction("dssStartAct", entity:getActId()) 
			assert(firstAction)
			assert(lastAction)
		end
		
		return firstAction, lastAction
	end


	function classAct.getLogicEvent(this, context, event)
		assert( event.Class == "LogicEntityAction") 

		local component =  this -- r2:getInstanceFromId(event.Entity)
		assert(component)
		local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
		assert(rtNpcGrp)

		local eventType = tostring(event.Event.Type)
		
		local eventHandler, lastCondition = nil, nil

		if eventType == "On Act Started" then 
			return r2.Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)
		end
		
		
		return eventHandler, firstCondition, lastCondition
	end

	function classAct.getLogicCondition(this, context, condition)
		return nil
	end


	----------------------------------------------------------------------------
	-- Create a controler for the current Act
	--
	function classAct.pretranslate(this, context)
		r2.Translator.createAiGroup(this, context)
	end

	function classAct.translate(this, context)
		entity = this
		
		if this:isBaseAct() then
			local baseAct = this:getParentScenario():getBaseAct()
			local rtNpcGrpBase = r2.Translator.getRtGroup(context, baseAct.InstanceId)
			local rtAction = r2.Translator.createAction("set_scenario_points", rtNpcGrpBase.Id )
			
			r2.Translator.translateAiGroupEvent("timer_t1_triggered", this, context, rtAction)
		end


		if not this:isBaseAct() then
			r2.Translator.translateAiGroup(entity, context)
			local rtNpcGrp = r2.Translator.getRtGroup(context, entity.InstanceId)
			local baseAct = this:getParentScenario():getBaseAct()
			local rtNpcGrpBase = r2.Translator.getRtGroup(context, baseAct.InstanceId)
			local index = context.Scenario:getActIndex(this.InstanceId)

			local rtAction = r2.Translator.createAction("act_starts", rtNpcGrp.Id, rtNpcGrpBase.Id, index)
			r2.Translator.translateAiGroupInitialState(entity, context, rtAction)
		end
	end
	
	local classLocationVersion = 2
	local classLocation =
	{
		Version = classLocationVersion,
		BaseClass="BaseClass",
		Name = "Location",
		Prop = 
		{			
			{Name="IslandName", Type="String"},
			{Name="ShortDescription", Type="String"},
			{Name="Time", Type="Number"},
			{Name="EntryPoint", Type="String"},
			{Name="Season", Type="String", DefaultValue=""},
			{Name="ManualSeason", Type="Number", WidgetStyle="Boolean", DefaultValue="0" }, 
		},

		onCreate = function(this)
			this:updateSeason()	
		end,

		onAttrModified = function(this, name)	
			if not r2:getCurrentAct() then return end
			if this.InstanceId == r2:getCurrentAct().LocationId then
				if name == "Season" then
					this:updateSeason()
				end
			end

		end,

		updateSeason = function(this)		
		
			-- change season in the editor
			-- effect may be seen only at the next teleport message if we're joining a session
			local season = this.Season
			if this.ManualSeason == 0 then season="Automatic" end
			--inspect(season)
			r2:setEditorSeason(season)				
		end,

		updateVersion = function(this, scenarioValue, currentValue, update)
			local patchValue = scenarioValue
			local seasonNames = { ["summer"] = "Summer", ["winter"] = "Winter", ["fall"] = "Autumn", ["spring"] = "Spring" }
			if patchValue < 1 then
				-- the season names of earlier versions, for a location with a season of its own
				if this.ManualSeason == 1 and seasonNames[this.Season] then
					update:set(this, "Season", seasonNames[this.Season])
				end
				patchValue = 1
			end
			-- version 2
			if patchValue < 2 then
				-- an entry point saved as a number is its position in the list of the island
				local entryPoint = this.EntryPoint
				if type(entryPoint) == "number" then
					update:set(this, "EntryPoint", string.format("uiR2EntryPoint%02d", math.floor(entryPoint) + 1))
				end
				-- the season names of earlier versions, also for a location without a season of its own
				if seasonNames[this.Season] then
					update:set(this, "Season", seasonNames[this.Season])
				end
				-- A location without a season has the season of the server, and says so by name: the
				-- server reads the name, and has none to read otherwise.
				if this.Season == nil or this.Season == "" then
					update:set(this, "Season", "Automatic")
				end
				patchValue = 2
			end
			if patchValue == currentValue then return true end
			return false
		end,

	}

	local classState =
	{
		Name="State",
		Prop=
		{
			{Name="InstanceId", Type="String"},
			{Name="Name", Type="String" },
			{Name="Behavior",Type="Behavior"}
		}	
	}
	
	r2.registerComponent(classMapDescription)
	r2.registerComponent(classScenario)
	r2.registerComponent(classAct)
	r2.registerComponent(classState)
	r2.registerComponent(classLocation)
end




--r2.Features.Act = {}
--
--local feature = r2.Features.Act
--
--function mergeTraduction(localLogicEntityAttributes)
--	local k, v = next(localLogicEntityAttributes, nil)
--	while k do
--		local k2, v2 = next(v, nil)
--		while k2 do
--			if not r2.logicEntityAttributes[k][k2] then
--				r2.logicEntityAttributes[k][k2] = v2
--			end
--			k2, v2 = next(v, k2)
--		end
--		k, v = next(localLogicEntityAttributes, k)
--	end
--end
--
--
--function feature:init()
--	-- register trad
--	local localLogicEntityAttributes = {
--		["ApplicableActions"] = {	
--			["Start Act"]	= i18n.get("uiR2EdStartAct"), 
--		},
--		["Events"] = {	
--			["On Act Started"]		= i18n.get("uiR2EdOnActStarted"),
--		},
--		["Conditions"] = {	
--		}
--	}
--	mergeTraduction(localLogicEntityAttributes)
--end
--
--feature.init()
--
--r2.Features["Act"] = feature
--

-----------------------------------------------
-- Not Save to disc so not version number needed

r2.registerBasicBricks=function()
	local classRtScenario =
	{
		Name = "RtScenario",
		Prop =
		{
			{Name="Acts", Type="Table"},
			{Name="Texts", Type="RtTextManager"},
			{Name="PlotItems", Type="Table"},
			{Name="Locations", Type="Table"}
		}
	}

	r2.registerComponent(classRtScenario)

	local classRtAct =
	{	
		Name="RtAct",
		Prop=
		{
			{Name="Id", Type="String"},
			{Name="NpcGrps", Type="Table"},
			{Name="FaunaGrps", Type="Table"},
			{Name="AiStates", Type="Table"},
			{Name="Npcs", Type="Table"},
			{Name="Events",Type="Table"},
			{Name="Actions", Type="Table"},
			{Name="WeatherValue", Type="Number"},
			{Name="Name", Type="String"},
			{Name="IslandName", Type="String"},
			{Name="Season", Type="Number"},
			{Name="LocationId", Type="Number"},
			{Name="UserTriggers", Type="Table"}
		}
	}
	r2.registerComponent(classRtAct)

	local classRtLocation =
	{	
		Name="RtLocation",
		Prop=
		{
			{Name="Id", Type="String"},
			{Name="Island", Type="String"},
			{Name="EntryPoint", Type="String"},
			{Name="Season", Type="Number"},
		}
	}
	r2.registerComponent(classRtLocation)

	do
		local classRt =
		{	
			Name="RtUserTrigger",
			Prop=
			{
				{Name="Id", Type="String"},
				{Name="Name", Type="String"},
				{Name="Grp", Type="String"},
				{Name="TriggerId", Type="Number"},
			}
		}
		r2.registerComponent(classRt)

	end
	
	local classRtNpcGrp =
	{
		Name = "RtNpcGrp",
		Prop = 
		{
			{Name="Id", Type="String"},
			{Name="Name", Type="String"},
			{Name="Children", Type="Table"},
			{Name="AutoSpawn", Type="Number", DefaultValue="1"},
			{Name="BotChat_parameters", Type="String"},
			{Name="BotEquipment", Type="String"},
			{Name="BotSheetClient", Type="String"},
			{Name="BotVerticalPos", Type="String", DefaultValue="auto"},
			{Name="Count", Type="Number"},
			{Name="GrpKeywords", Type="String"},
			{Name="AiProfilParams", Type="String"},
			{Name="GrpParameters", Type="String"},

		}
			
	}

	r2.registerComponent(classRtNpcGrp)

	local classRtNpc =
	{
		Name = "RtNpc",
		Prop =
		{
			{Name="Id", Type="String", DefaultValue="" },
			{Name="Name", Type="String" },
			{Name="Children", Type="Table" },
			{Name="ChatParameters", Type="String" },
			{Name="Equipment", Type="Table" },
			{Name="IsStuck", Type="Number" },
			{Name="Keywords", Type="String" },
			{Name="Sheet", Type="String" },
			{Name="SheetClient", Type="String" },
			{Name="BotVerticalPos", Type="String", DefaultValue="auto" },
			{Name="Angle", Type="Number" },
			{Name="DmProperty", Type="Number"},
			{Name="Pt", Type="RtPosition" },
		}
	}

	r2.registerComponent(classRtNpc)
	
	local classRtPosition =
	{
		Name = "RtPosition",
		Prop =
		{
			{Name="x", Type="Number" },
			{Name="y", Type="Number" },
			{Name="z", Type="Number" },
		}
	}

	r2.registerComponent(classRtPosition)
	



	local classRtAiState  =
	{
		Name = "RtAiState",
		Prop = 
		{
			{Name="Id", Type="String", DefaultValue="" },
			{Name="Name", Type="String" },
			{Name="Children", Type="Table" },
			{Name="AiActivity", Type="String", DefaultValue="no_change" },
			{Name="AiMovement", Type="String", DefaultValue="" },
			{Name="AiProfileParams", Type="Number" },
			{Name="Keywords", Type="String" },
			{Name="VerticalPos", Type="String", DefaultValue="auto" },
			{Name="Pts", Type="Table" },
			{Name="Reactions",Type="Table"}
		}
	}



	r2.registerComponent(classRtAiState)

	local classRtNpcEventHandler =
	{
		Name = "RtNpcEventHandler",
		Prop =
		{
			{Name="Id", Type="String", DefaultValue="" },
			{Name="Name",Type="String"},
			{Name="Event", Type="String"},
			{Name="StatesByName",Type="String"},
			{Name="GroupsByName",Type="String"},
			{Name="ActionsId", Type="Table"}
		}
	}

	r2.registerComponent(classRtNpcEventHandler)

	local classRtNpcEventHandlerAction =
	{
		Name = "RtNpcEventHandlerAction",
		Prop =
		{
			{Name="Id", Type="String", DefaultValue="" },
			{Name="Action", Type="String"},
			{Name="Name", Type="String"},
			{Name="Parameters", Type="String"},
			{Name="Children",Type="Table"},
			{Name="Weight",Type="Number",DefaultValue="1"}
		} 
	}

	r2.registerComponent(classRtNpcEventHandlerAction)

local classRtFauna =
	{
		Name= "RtFauna",
		Prop=
		{
			{Name="Id", Type="String", DefaultValue=""},
			{Name="Name",Type="String"},
			{Name="Pts", Type="Table" },
			{Name="Children", Type="Table"}
		}
	}
	r2.registerComponent(classRtFauna)

	local classRtGroupFaunaEx =
	{
		Name="RtGroupFaunaEx",
		Prop=
		{
			{Name="Id",Type="String",DefaultValue=""},
			{Name="Name",Type="String"},
			{Name="FaunaType",Type="String",DefaultValue="HERBIVORE"},
			{Name="Children", Type="Table"}
		}
	}
	r2.registerComponent(classRtGroupFaunaEx)


	local classRtFaunaGenericPlace =
	{
		Name="RtFaunaGenericPlace",
		Prop=
		{
			{Name="Id",Type="String",DefaultValue=""},
			{Name="Name",Type="String"},
			{Name="FlagFood",Type="String",DefaultValue="true"},
			{Name="FlagRest",Type="String",DefaultValue="true"},
			{Name="FlagSpawn",Type="String",DefaultValue="true"},
			{Name="Index",Type="Number"},
			{Name="IndexNext",Type="Table"},
			{Name="Place",Type="Place"}
		}
	}
	r2.registerComponent(classRtFaunaGenericPlace)

	local classRtPopulation =
	{
		Name = "RtPopulation",
		Prop=
		{
			{Name="Id",Type="String",DefaultValue=""},
			{Name="Name",Type="String"},
			{Name="Children",Type="Table"}
		}
	}
	r2.registerComponent(classRtPopulation)

	local classRtPeople =
	{
		Name="RtPeople",
		Prop=
		{
			{Name="Name",Type="String"},
			{Name="Count",Type="Number"},
			{Name="CreatureCode",Type="String"}
		}
	}
	r2.registerComponent(classRtPeople)

	local classRtTextManager =
	{	
		Name="RtTextManager",
		Prop=
		{
			{Name="Id", Type="String"},
			{Name="Texts", Type="Table"},
		}
	}

	r2.registerComponent(classRtTextManager)

	local classRtEntryText =
	{
		Name="RtEntryText",
		Prop=
		{
			{Name="Id", Type="String"},
			{Name="Text", Type="String"}
		}
	}

	r2.registerComponent(classRtEntryText)


	-- Copy struct TMissionItem
	local classRtPlotItem =
	{
		Name = "RtPlotItem",
		Prop =
		{
			{Name="Id", Type="String"},
			{ Name="SheetId", Type="Number" },
			{ Name="Name", Type="String" },
			{ Name="Description", Type="String" },
			{ Name="Comment", Type="String" }
		}
	}
	r2.registerComponent(classRtPlotItem)

end
	
		
