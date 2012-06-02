

if r2.Translator == nil
then
	r2.Translator={}
end

local Translator = r2.Translator

Translator.PredatorEnemyFaction = "Player|guard|bandit|herbivore|karavan";

-- Namespace global
function printMsg(str)	
	messageBox(str)	
	debugInfo(colorTag(255,255,0)..str)
	local ucStringMsg = ucstring()
	ucStringMsg:fromUtf8(str)	
	displaySystemInfo(ucStringMsg, "BC")
	messageBox(str)
end

function printError( str)
	local msg = "Translation WRN:"
	debugInfo(colorTag(255,0,0)..msg..str)
--	local ucStringMsg = ucstring()
--	ucStringMsg:fromUtf8(str)
--	displaySystemInfo(ucStringMsg, "BC")
	--messageBox(str)
	assert(nil)
end

r2.Translator.MultilineBc = {}

function Translator.updateEachSecond()
	if table.getn( Translator.MultilineBc ) > 0 then
		local msg=table.remove(Translator.MultilineBc, 1)
		if msg then 
			local ucStringMsg = ucstring()
			ucStringMsg:fromUtf8(msg)	
			displaySystemInfo(ucStringMsg, "BC")
		end
	end
end

function printMsgML(str)
	local strs = r2.split(str, "\n")
	for k,v in pairs(strs) do
		table.insert(Translator.MultilineBc, v)
	end
end
--local devMode = false
local devMode =   config.R2EDExtendedDebug
local dataDevMode = false

function printWarning( str)
	local msg = "Translation Error:"..str
	debugInfo(colorTag(255,0,0)..msg)
	-- msg will be displayed when client is back to edition

 	-- Just report the last error
 	if (r2.LastTranslationErrorMsg  == nil) then
		r2.LastTranslationErrorMsg = str
		local ucStringMsg = ucstring("Translation Error")
		-- ucStringMsg:fromUtf8(r2.LastTranslationErrorMsg)
		displaySystemInfo(ucStringMsg, "BC")
	 	messageBox(str)
	end
	if devMode then
		assert(nil)
	else
		error(str) -- force to exit current translation
	end
end

function BOMB_IF(condition, str)
	if ( not condition) then
		printWarning(str)
	end
end

-- Namespace r2
function r2:getScenarioId()
	local str = r2:getNamespace()
	str = string.gsub(str, "r2_", "")
	str = string.gsub(str, "_", "")
	local  sessionId = tonumber(str)
	return sessionId
end


function r2:getActId(actIn)
	assert(actIn)
	local index = -1
	local actId, act = next(r2.Scenario.Acts)
	while actId do
		index = index + 1
		if (tostring(act.InstanceId) == tostring(actIn.InstanceId)) then return index end
		actId, act = next(r2.Scenario.Acts, actId)

	end
	assert(nil)
	return -1
end


-- Namespace Translator
function Translator.getRtGroup(context, instanceId)
	assert(context.RtAct)
	assert( context ~= nil and type(context)=="table")
	assert( instanceId ~= nil and type(instanceId) == "string")

	if context.RtGroups[instanceId]==nil
	then
		context.RtGroups[instanceId] = r2.newComponent("RtNpcGrp")
		context.RtGroups[instanceId].Name = context.RtGroups[instanceId].Id
		table.insert(context.RtAct.NpcGrps, context.RtGroups[instanceId])
	end
	return context.RtGroups[instanceId]
end

function Translator.getRtStatesNames(context, instanceId)
	local rtNpcGrp = Translator.getRtGroup(context, instanceId)

	local statesNames = context.GroupStatesName[rtNpcGrp.Name]
	return statesNames
end

r2.doTranslateFeatures = function(scenario)
	return Translator.doTranslateFeatures(scenario.InstanceId)
end

r2.translateFeature = function(context)
	local component = context.Feature
	if (component == nil)  then return end

	if component.translate ~= nil then
		component:translate(context)
	end
end


r2.translateFeatures = function(scenario)

	local rtScenario = r2.doTranslateFeatures(scenario)
	r2.requestUpdateRtScenario(rtScenario)
end



-- creat a context (mainly RtComponent)
Translator.createContext = function(scenario)
	local context = {}
	context.Scenario = scenario
	context.RtScenario = r2.newComponent("RtScenario")
	context.TextTranslateId={}
	context.Feature=scenario -- obsloete to remove
	context.GroupStatesName = {}
	context.GroupTriggeredActions = {}
	context.Events = {}
	context.ActivityStates = {}
	context.RtGrps={}
	context.RtGroups={}
	context.CounterNames={}
	context.RtCounters = {}
	--context.EasterEggUniqId = {} -- RtGrpId to uniqId
	context.ActIdToRtAct  = {}
	context.InteractingSceneryObjects = {}
	return context
end

-- return the equipment of a rtNpc by looking at the visual properties of an hlNpc
Translator.translateEquipment = function(hlNpc)
	local instanceId = hlNpc.InstanceId
	local equipment = ""
	local instance = r2:getInstanceFromId(instanceId)

	if instance:isKindOf("NpcCustom") then
		equipment = equipment..r2.getVisualPropertiesFromInstanceId(instanceId);
	end

	if equipment == nil then
		equipment = ""
	end

	local isFauna = hlNpc.IsFauna
	if isFauna ~= nil and isFauna == 1 then
		-- Npc with default name use default translation system name
		local basename = hlNpc.Base
		if basename then basename = r2.PaletteIdToTranslation[ basename ] end
		if basename == nil or basename ~= hlNpc.Name  then
			equipment = equipment .. "FAUNA_BOT_USE_BOTNAME\n"
		end
	end
	return equipment
end

-- get an rtNpc aiActivity from an hlNpc eg "civil" -> "normal"
Translator.getAiActivity = function(hlNpc)
	assert(hlNpc and type(hlNpc) == "userdata")
	local aiActivity = hlNpc.AiActivity
	local profile =  hlNpc.Profile
	local str = "no_change"

	if profile then return "" end

	if profile  ~= nil then
		if profile == "bandit" then str = "bandit" end
		if profile == "guard"  then str = "guard" end
		if profile == "civil"  then str = "normal" end
	elseif aiActivity  ~= nil then
		str = aiActivity
	end

	return str;
end

--  get rtNpc from hlNpc
Translator.translateNpc = function(hlNpc, context)

	local function findInTable(instanceId)
		for k, v in pairs(context.InteractingSceneryObjects) do
			if v == instanceId then return true end
		end
		return false
	end
	
	
	assert(hlNpc and type(hlNpc) == "userdata")
	local RtNpc = r2.newComponent("RtNpc")
	

	RtNpc.Name = hlNpc.Name
	
	
	RtNpc.SheetClient = hlNpc.SheetClient
	
	
	RtNpc.Sheet = hlNpc.Sheet
	if RtNpc.Sheet == nil then RtNpc.Sheet = "" end

	

	if hlNpc:isBotObject() and context and context.InteractingSceneryObjects
		and findInTable(hlNpc.InstanceId) then
			RtNpc.IsStuck = 0
			RtNpc.Sheet = "object_chest_wisdom_std_sel.creature"
	else
		RtNpc.IsStuck = hlNpc.IsStuck
	end

	RtNpc.Pt = r2.getWorldPos(hlNpc)
	RtNpc.Angle = hlNpc.Angle
	RtNpc.Equipment = Translator.translateEquipment(hlNpc)

	local animProp = 0
	if not hlNpc:getParentAct():isBaseAct() then
		animProp = animProp + 1 -- TODO test if default feature
	end

	if hlNpc.IsBotObject ~= 1 then
		animProp = animProp + 2 -- Living Creature
	end

	if hlNpc.IsBotObject ~= 1 and hlNpc.IsPlant ~= 1 then
		animProp = animProp + 4 -- Controlable creature
	end

	if hlNpc.IsBotObject ~= 1 and hlNpc.IsNpc == 1 then
		animProp = animProp + 8 -- Creature that talk
	end
	RtNpc.DmProperty = animProp
	return RtNpc
end

-- Behavior: the behavior of the npc (or the leader of an group)
-- translate the activitySequences of an npc/group
Translator.translateActivities = function (context, hlComponent, behavior, rtNpcGrp, aiActivity)

	assert(context)
	assert(hlComponent)
	assert(behavior)
	assert(rtNpcGrp)
	assert(aiActivity)

	local initFun = Logic.initGroupActivitiesTranslation
	local translateActivitySequence = Logic.translateActivitySequence
	--all the states names of all the sequences of this group
	local statesNames=""
	local first = true
	--for each group's activity
	local activityIndex = 1

	local firstState =""
	--creation of the group's initial state

	local leader = hlComponent
	if hlComponent:isKindOf("NpcGrpFeature") then
		if table.getn(hlComponent.Components) >= 0 then
			leader = hlComponent.Components[0]
		else
			leader = nil
		end
	end

	if table.getn(behavior.Activities) == 0 then


	

		-- create initial and only state
		local aiState = r2.newComponent("RtAiState")
		statesNames = aiState.Id
		r2.Utils.setState(context, behavior, aiState)
		aiState.Name = hlComponent.InstanceId..".init"
		aiState.AiActivity = aiActivity
		table.insert(context.RtAct.AiStates, aiState)
		table.insert(aiState.Children, rtNpcGrp.Id)
		firstState = aiState.Id
	

	else

		local k, v  = next(behavior.Activities, nil)
		while k do
		
			if (v.Class == "ActivitySequence") then
				initFun(context, hlComponent, v, first, activityIndex, aiActivity, rtNpcGrp)

				if first then
					firstState = Logic.StatesByName
				end
				--translate the activity
	 			translateActivitySequence(context, hlComponent, v, activityIndex, rtNpcGrp)
				statesNames = statesNames..Logic.StatesByName.."\n"
				activityIndex = activityIndex + 1
				first = false
			else
				error("Error while translating '" .. hlComponent.Name .. "' its  " .. tostring(nbActivity) .." ActiviySequence  contains an element of type " .. v.Class)
			end
			k, v = next(behavior.Activities, k)
	
		end
	end


	if leader and not leader:isBotObject() then
		local category = leader.SubCategory
		local aggro = leader.Aggro
		if not category then 
			category = leader.Category
		end

		if category then
			local event = r2.Translator.createEvent("start_of_state", firstState, rtNpcGrp.Id)
			table.insert(context.RtAct.Events, event)
			local action = r2.Translator.createAction("bot_init", rtNpcGrp.Id, category, aggro, leader.BotAttackable, leader.PlayerAttackable)
			table.insert(context.RtAct.Actions, action)
			table.insert(event.ActionsId, action.Id)				
		end
	end

	context.GroupStatesName[rtNpcGrp.Name] = statesNames

end



function Translator.initializeFactions(context, leader, rtGrp, aiStateName )
	if leader and not leader:isBotObject() then
		local category = leader.SubCategory
		local aggro = leader.Aggro
		if not category then 
			category = leader.Category
		end

		if category then
			local event = r2.Translator.createEvent("start_of_state", aiStateName, rtGrp.Id)
			table.insert(context.RtAct.Events, event)
			local action = r2.Translator.createAction("faction_init", rtGrp.Id, category, aggro, leader.BotAttackable, leader.PlayerAttackable)
			table.insert(context.RtAct.Actions, action)
			table.insert(event.ActionsId, action.Id)				
		end
	end
end

-- translate an eventHandler defined in the behavior of an npc / group
-- used to implement translateEventHandlers
Translator.translateEventHandler = function(context, hlNpc, eventHandler, rtNpcGrp)

	local getName = function(object, optionalName)
		if optionalName and object.optionalName then return "'"..object.optionalName.."'" end
		if object.Name then return "'"..object.Name.."'" end
		if object.InstanceId then return "("..object.InstanceId..")" end
		return "??"
	end

	local event = nil
	local firstCondition = nil
	local lastCondition = nil

	local target = nil


	target = hlNpc

	if not target then
		printWarning("Error in component '" .. eventHandler.Name.."'")
	end

	if tostring(eventHandler.Event.Type) == "" then return nil end

	if not target.getLogicEvent then
		local eventName =  eventHandler.Name
		printWarning("The component '" .. target.Name .. "' seem to not be able to handle events '")

	end

	event, firstCondition, lastCondition = target.getLogicEvent(target, context, eventHandler)


	if  not event	then
		printWarning("Error in '"..  target.Name.. "' the Event Handler '".. eventHandler:getName() .. "' don't seem to work because the event '"..eventHandler.Event.Type.."' don't seem to be implemented." )

	end



	local kCondition, condition = next(eventHandler.Conditions, nil)
	while kCondition do


		local conditionEntity = r2:getInstanceFromId(condition.Entity)
		if condition.Condition.Type ~= "" and conditionEntity then
			assert(conditionEntity)
			local firstCondition2, lastCondition2 = conditionEntity:getLogicCondition(context, condition)

			if not firstCondition2 or not lastCondition2 then
				printWarning("Unknown Condition '".. condition.Condition.Type .. "' in EventHandler ".. eventHandler:getName().." in component " .. getName(target))
				return nil
			end

	
	
			if not firstCondition then
				firstCondition = firstCondition2
				lastCondition = lastCondition2
			else
				table.insert(lastCondition.Children, firstCondition2)	
				lastCondition = lastCondition2
			end
		end

		kCondition, condition = next(eventHandler.Conditions, kCondition)
	end


	local firstAction = nil
	local lastAction = nil

	if eventHandler.Actions.Size > 0 then

		local multiAction = nil
		if eventHandler.Actions.Size > 1 then
			multiAction = Translator.createAction("multi_actions")
		end

		local kAction, action = next(eventHandler.Actions, nil)
		while kAction do
			local actionEntity = r2:getInstanceFromId(action.Entity)

			if action.Action.Type ~= "" and actionEntity then
				local firstAction2, lastAction2 = actionEntity:getLogicAction(context, action)

				if not firstAction2 or not lastAction2 then
					printWarning("Unknown Action '".. action.Action.Type .. "' in EventHandler ".. eventHandler:getName().." in component " .. getName(target))
				end

				if multiAction then
					table.insert(multiAction.Children, firstAction2)
				else
					firstAction = firstAction2
				end
			end

			kAction, action = next(eventHandler.Actions, kAction)
		end

		if eventHandler.Actions.Size > 1 then
			firstAction = multiAction
		end
	end

	-- if there is actions then the first executed npc_event_handler_action are the dynamic_if from  [firstCondition, lastCondition]
	if lastCondition then
		table.insert(lastCondition.Children, firstAction)
		firstAction = firstCondition
	end

	if event and firstAction then

		local actInstanceId = eventHandler:getLogicActInstanceId()
		
		if (tostring(actInstanceId) == "") then
			debugInfo("Invalid Multi act action:"..eventHandler:getName())
			return
		end


		
		local rtAct2 = context.ActIdToRtAct[actInstanceId]
		local rtAct = context.RtAct
		if rtAct2 ~= rtAct then
			local baseAct = context.Scenario:getBaseAct()
			local index = context.Scenario:getActIndex(actInstanceId)
			if index == -1 then
				printWarning("Invalid Scenario")
			end
			local rtNpcGrpBase = r2.Translator.getRtGroup(context, baseAct.InstanceId)
			
			local action = Translator.createAction("test_act", rtNpcGrpBase.Id , index)
			table.insert(action.Children, firstAction)
			firstAction = action
		end

		-- insert a npc_event_handler
		table.insert(rtAct.Events, event) -- TODO use eventHandler->

		-- insert a npc_event_handler_action
		table.insert(event.ActionsId, firstAction.Id)
		table.insert(rtAct.Actions, firstAction)
	end
end


-- translates eventHandlers of a npc/group (eventHandlers are defined in beahvior)
Translator.translateEventHandlers = function(context, hlNpc, eventHandlers, rtNpcGrp)
	assert(rtNpcGrp)
	assert(context)
	if (eventHandlers ~= nil) then
		local k, v = next (eventHandlers, nil)
		while k do
			local caller = nil
			if devMode then
				caller = function (...) arg[1](arg[2], arg[3], arg[4], arg[5]) return true end
		
			else
				caller = pcall			
			end
			if not caller(Translator.translateEventHandler, context, hlNpc, v, rtNpcGrp) then
				local eventType = v.Event.Type
				if eventType == nil then eventType = "" end
				local componentName = hlNpc.Name
				if componentName == nil then componentName = "" end
				printWarning("Error in event handler '"..eventType.."' In component "..componentName)
			end
			k, v = next (eventHandlers, k)
		end	
	end
end





-- translate a scenario
-- scenarioInstanceId the instanceId of the scenario that will be translated to rtData
-- returns rtScenario or nil
Translator.doTranslateFeatures = function(scenarioInstanceId)
	local ok
	local result

	r2.LastTranslationErrorMsg = nil
	ok, result = pcall(Translator.doTranslateFeaturesProtected, scenarioInstanceId)
	if not ok then
		printWarning(result)
	end

	return result
end

function Translator.initStartingActIndex(startingAct)
	local startingAct = r2.Scenario.User.SelectedActInstanceId
	
	local acts = r2.Scenario.Acts


	local actId, act = next(acts, nil)
	local actIndex = 0
	while (actId ~= nil)
	do
		if startingAct and tostring(act.InstanceId) == startingAct then
			r2.setStartingActIndex(actIndex )
		else 
			r2.setStartingActIndex(1)
		end
		actIndex = actIndex + 1
		actId, act = next(acts, actId)
	end
end
Translator.doTranslateFeaturesProtected = function(scenarioInstanceId)
	local scenario = r2:getInstanceFromId(scenarioInstanceId)
	assert(scenario) -- something is broken elsewhere
	assert( r2.Features ~= nil )
	local acts = scenario.Acts
	local context = Translator.createContext(scenario)
	local cost = 0
	local rtScenario = context.RtScenario
	-----------------------------
	--elements counting
	local maxSecondaryActCost = 0
	local baseActCost = 0
	local first=true



	--
	-- Recursive method that call a specific function (ie createGhostComponent, pretranslate) on a component
	-- and every components it contains. Each component level is treated, so that the function needs to be called
	-- only once on the toplevel feature.
	--
	local function recursiveFunctionCall(f,components, param)
		if (components == nil) then
			return
		end
		local k, v = next(components, nil)
		while k do
			if v[f] then
				v[f](v, param)		
			end
			if v.Components then
				recursiveFunctionCall(f, v.Components, param)
			end
			if v.SubComponents then
				recursiveFunctionCall(f, v.SubComponents, param)
			end
	
			k, v = next(components, k)
		end
	end

	local function recursiveTranslate(components, context)
		if (components == nil) then
			return
		end
		local k, v = next(components, nil)
		while k do
			context.Feature= v
			r2.translateFeature(context)
			if v.Components then
				recursiveTranslate(v.Components, context)
			end
			if v.SubComponents then
				recursiveTranslate(v.SubComponents, context)
			end
			k, v = next(components, k)
		end
	end

	local function recursivePretranslate2(components, context)
		if (components == nil) then
			return
		end
		local k, v = next(components, nil)
		while k do
			context.Feature= v
			if v.pretranslate2 then v.pretranslate2(v, context) end

			if v.Components then
				recursivePretranslate2(v.Components, context)
			end
			if v.SubComponents then
				recursivePretranslate2(v.SubComponents, context)
			end
			k, v = next(components, k)
		end
	end


	-- Management of items (Copy from Edition Data to Rt Data)
	do
		local plotItemId, plotItem = next(scenario.PlotItems, nil)
		while plotItemId do

			assert(type(plotItem.SheetId) == "number")
			assert(type(plotItem.Name) == "string")
			assert(type(plotItem.Desc) == "string")
			assert(type(plotItem.Comment) == "string")

			assert(string.len(plotItem.Name) < 256)
			assert(string.len(plotItem.Desc) < 256)
			assert(string.len(plotItem.Comment) < 256)

			local rtPlotItem = r2.newComponent("RtPlotItem")
			rtPlotItem.SheetId = plotItem.SheetId
			
			rtPlotItem.Description = plotItem.Desc
			
			rtPlotItem.Name = plotItem.Name
			rtPlotItem.Comment = plotItem.Comment
			table.insert(rtScenario.PlotItems, rtPlotItem)
			plotItemId, plotItem = next(scenario.PlotItems, plotItemId)
		end

	end


	-- ghost
	do
		local actId, act = next(acts, nil)
		while (actId ~= nil) do	
			local features = act.Features
			recursiveFunctionCall("createGhostComponents", features, act)		

			actId, act = next(acts, actId)
		end
	end

	-- pre Translation
	do

		local actId, act = next(acts, nil)
		while (actId ~= nil) do

			local rtAct = r2.newComponent("RtAct")	
			if act.WeatherValue ~=nil and act.ManualWeather == 1 then
				rtAct.WeatherValue = 1 + act.WeatherValue
			else
				rtAct.WeatherValue = 0
			end
			rtAct.ActDescription = ""
			rtAct.PreActDescription = ""

			if act.ShortDescription then
				rtAct.ActDescription =  act.ShortDescription
			end
			if act.PreActDescription then
				rtAct.PreActDescription = act.PreActDescription
			end
			
	
			context.RtAct = rtAct

			context.ActIdToRtAct[act.InstanceId] = rtAct

			context.Act = act
			context.RtAct = rtAct
			local features = act.Features

			act:pretranslate(context)
			recursiveFunctionCall("pretranslate", features, context)

	

			actId, act = next(acts, actId)
		end
	end

	-----------------------------
	--texts translation
	context.Feature = scenario.Texts
	r2.Features["TextManager"].Translator(context)

	--for each act

	local actId, act = next(acts, nil)
	while (actId ~= nil) do
		cost= 0
		-- debugInfo("Act:: "..act.InstanceId)

		local rtAct = context.ActIdToRtAct[act.InstanceId]
		context.RtAct = rtAct
		context.Act = act
		table.insert(rtScenario.Acts, rtAct)


		local activitiesIds = act:getActivitiesIds()
		--creating states for all the activities of all the groups in this act
		local k, v = next(activitiesIds, nil)
		while k do
			local sequence = r2:getInstanceFromId(v)
			if sequence and sequence.Components
			then
				Logic.createActivityStates(context, sequence)
			end
			k, v = next(activitiesIds, k)
		end

		actId, act = next(acts, actId)
	end

	-- translate activities
	do

		local actId, act = next(acts, nil)
		while (actId ~= nil) do
			local rtAct = context.ActIdToRtAct[act.InstanceId]
			context.RtAct = rtAct
			context.Act = act
		
			local features = act.Features
			recursivePretranslate2(features, context)

	
			actId, act = next(acts, actId)
		end
	end

 	local first = true
	
	actId, act = next(acts, nil)
	while (actId ~= nil) do
		local rtAct = context.ActIdToRtAct[act.InstanceId]
		context.RtAct = rtAct
		context.Act = act

		
		local features = act.Features


		assert(features ~= nil or actId == "Keys")

		recursiveTranslate(features, context)

		context.Feature = act
		act:translate(context);
		-- scenario
		if first then
			first = false
			context.Feature = scenario
			scenario:translate(context);
		end

		actId, act = next(acts, actId)
	end




	-- Location Id
	local locationId, location = next(scenario.Locations)
	local locationIndex = 0
	local locationMap = {}

	locationMap[""] = 0

	while locationId do
		local rtLocation =r2.newComponent("RtLocation")
		rtLocation.Island = location.IslandName
		rtLocation.EntryPoint = location.EntryPoint
		local  enumToInt = {Automatic=0, Spring=1, Summer=2, Autumn=3, Winter=4}
		rtLocation.Season = enumToInt[ location.Season ]
		locationMap[location.InstanceId] = locationIndex

		table.insert(rtScenario.Locations, rtLocation)
		locationIndex = locationIndex + 1
		locationId, location = next(scenario.Locations, locationId)
	end

	local startingAct = r2.Scenario.User.SelectedActInstanceId


	-- Act Name, position
	local actId, act = next(acts, nil)
	local actIndex = 0
	while (actId ~= nil)
	do

		local rtAct = context.ActIdToRtAct[act.InstanceId]
		rtAct.Name = act.Name

		rtAct.LocationId = locationMap[ act.LocationId ]

		if startingAct and tostring(act.InstanceId) == startingAct then
			r2.setStartingActIndex(actIndex )
		else 
			r2.setStartingActIndex(1)
		end
		actIndex = actIndex + 1
		actId, act = next(acts, actId)
	end
	-- Ring accss
	if ( r2.getMustVerifyRingAccessWhileLoadingAnimation()) then
		do
			local ok, level, err = r2.RingAccess.verifyScenario()
			r2.updateScenarioAck(ok, level, err.What)
		end

		local ok, err = r2.RingAccess.verifyRtScenario(rtScenario)
		if not ok then
			printWarning(err.What)
		end
	end
	
--	inspect(rtScenario)
	return rtScenario
end

-- Returns a RtNpcEventHandlerAction if the action is allowed
--first parameter: action type
Translator.createAction = function(...)
	local debug=config.R2EDExtendedDebug
	local function header(toto)

		if debug then
			return "print(\"<"..toto..">\");\n"
		end
		return "//"..toto
	end

	local function footer(toto)

		if debug then
			return "print(\"</"..toto..">\");\n"
		end
		return ""
	end


	local action = r2.newComponent("RtNpcEventHandlerAction")
	local actionType = arg[1]
	action.Action = actionType
	action.Name = actionType

	if actionType == "test_act" then
		assert(type(arg[2])=="string")
		assert(type(arg[3])=="number")
		local rtGrpId = arg[2] -- scenario
		local actId = arg[3] -- actId

		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		
		action.Action = "dynamic_if"
		action.Parameters = prefix.."CurrentAct == "..tostring(actId)
		return action
	end

	if actionType == "wander_destination_reached" then
		assert(type(arg[2])=="string")
		assert(type(arg[3])=="string")
		assert(type(arg[4])=="number")
		assert(type(arg[5])=="number")

		local rtGrpId = arg[2] 
		local states = arg[3]  
		local nextStep = arg[4]  
		local time =  arg[5]  

		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		
		action.Action = "code"

		action.Parameters = 
			"v2 = ".. nextStep..";\n" ..
			"()setTimer("..1+ 10*time..", 0);\n"
		

		return action
	end
	if actionType == "next_road" then
		assert(type(arg[2])=="string")
		assert(type(arg[3])=="string")
		assert(type(arg[4])=="number")
		assert(type(arg[5])=="string")

		action.Action = "code"

		local paramCount = tonumber(arg[5])
		if paramCount == nil then
			paramCount = "0"
		end
		paramCount = tostring(paramCount)

		action.Parameters =
[[//next_road
if (  ParamRepeatCount == 0  || ParamGroup.RoadCountLimit < ParamRepeatCount - 1) {
	if ( ParamRepeatCount != 0) { ParamGroup.RoadCountLimit = ParamGroup.RoadCountLimit + 1; }
	()ParamGroup.postNextState("ParamState");
} else {
	ParamGroup.RoadCountLimit = 0;
	ParamGroup.v2 = ParamActivityIndex;
	()ParamGroup.setTimer(1, 0);
}

]]
		action.Parameters = string.gsub(action.Parameters, "ParamGroup", r2:getNamespace() .. tostring(arg[2]))
		action.Parameters = string.gsub(action.Parameters, "ParamState",  r2:getNamespace() .. tostring(arg[3]))
		action.Parameters = string.gsub(action.Parameters, "ParamActivityIndex", tostring(arg[4]))
		action.Parameters = string.gsub(action.Parameters, "ParamRepeatCount", paramCount)

			

		return action
	end


	if actionType == "trigger_zone_min_player" then
		assert(type(arg[2])=="string")
		assert(type(arg[3])=="string")
		assert(type(arg[4])=="number")
		local rtGrpId = arg[2] 
		local states = arg[3]  
		local nbMinPlayer = arg[4]


		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		
		action.Action = "dynamic_if"
		if nbMinPlayer == 0 then
			action.Parameters = prefix.."Active == 1 && "..prefix.."NbPlayer == 0"
		else
			action.Parameters = prefix.."Active == 1 && "..prefix.."NbPlayer >= "..tostring(nbMinPlayer)
		end

		return action
	end


	if actionType == "on_player_arrived_impl" then
		assert(arg[2])

		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end


			action.Parameters = header(actionType)..
			"if ( "..prefix.."Active == 1 )\n{\n"..
			"\tif ( "..prefix.."Cyclic == 1 )\n"..
			"\t{\n"..
				"\t\t"..prefix.."Enter = 0 ;\n" ..
				"\t\t()"..prefix.."setEvent(1);\n" ..
			"\t}\n"..
			"\telse if ( "..prefix.."Enter == 1 )\n"..
			"\t{\n"..
				"\t\t"..prefix.."Enter = 0;\n"..
				"\t\t".."()"..prefix.."setEvent(1);\n" ..		
			"\t}\n" ..
			"}\n"..
			 footer(actionType)

		action.Action = "code"
		return action
	end

	if actionType == "on_player_left_impl" then
		assert(arg[2])

		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end


			action.Parameters = header(actionType)..
			"if ( "..prefix.."Active == 1 )\n{\n"..
			"\tif ( "..prefix.."Cyclic == 1 )\n"..
			"\t{\n"..
				"\t\t"..prefix.."Leave = 0 ;\n" ..
				"\t\t()"..prefix.."setEvent(2);\n" ..
			"\t}\n"..
			"\telse if ( "..prefix.."Leave == 1 )\n"..
			"\t{\n"..
				"\t\t"..prefix.."Leave = 0;\n"..
				"\t\t".."()"..prefix.."setEvent(2);\n" ..		
			"\t}\n" ..
			"}"
			 ..footer(actionType)

		action.Action = "code"
		return action
	end
	if actionType == "trigger_zone_init" then
		assert(arg[2])
		assert(arg[3])
		assert(arg[4])
		local rtGrpId = arg[2]
		local auto = arg[3]
		local cyclic = arg[4]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end


		action.Parameters = header(actionType)..
		"\t"..prefix.."Active = ".. auto .." ;\n" ..
		"\t"..prefix.."Leave = ".. auto .." ;\n" ..
		"\t"..prefix.."Enter = ".. auto .." ;\n" ..
		"\t"..prefix.."Cyclic = "..tostring(cyclic).." ;\n" ..
		"if ("..prefix.."Active == 1)\n"..
		"{\n"..
		"\t()"..prefix.."setEvent(4);\n" ..
		"}\n"..
		 footer(actionType)

		action.Action = "code"
		return action
	end

	if actionType == "trigger_zone_activates" then
		assert(arg[2])

		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		
		action.Parameters = header(actionType)..
		"if (" ..prefix.."Active == 1)\n"..
		"{"..
		"\t ()" ..prefix.."setEvent(4);\n"..
		"}\n"..
		"else\n"..
		"{\n"..
		"\t"..prefix.."Active = 1 ;\n" ..
		"\t"..prefix.."Leave = 1 ;\n" ..
		"\t"..prefix.."Enter = 1 ;\n" ..
		"()"..prefix.."setEvent(4);\n" ..
		"}\n"..
		footer(actionType)

		action.Action = "code"
		return action
	end

	if actionType == "trigger_zone_deactivates" then
		assert(arg[2])

		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end


		action.Parameters = header(actionType)..
		"\t"..prefix.."Active = 0 ;\n" ..
		"\t"..prefix.."Leave = 0;\n" ..
		"\t"..prefix.."Enter = 0;\n" ..
		"()"..prefix.."setEvent(5);\n" ..
		footer(actionType)

		action.Action = "code"
		return action
	end

	if actionType == "act_starts" then
		assert(arg[2])
		assert(arg[3])
		assert( type(arg[4]) == "number")

		local rtGrpId = arg[2]
	 	local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end


		local rtGrpId2 = arg[3]
		local prefix2=""
		if rtGrpId2 and rtGrpId2 ~= "" then
			prefix2 = r2:getNamespace() .. rtGrpId2.."."
		end


		action.Parameters = header(actionType)..
			"()"..prefix.."setTimer(50,0);\n" .. -- act start in 1 second
			prefix2.."CurrentAct = " .. tostring( arg[4] ) .. ";\n" ..
			"if ( "..prefix2.."v0 == 0 )\n"  .. 
			"{\n"..
				"\t()"..prefix2.."setTimer(50,0);\n"..
				"\t"..prefix2.."v0 = 0;\n"..
				"\t()"..prefix2.."setTimer(150, 1);\n"..
				"\t"..prefix2.."ScenarioPoints = 0;\n"..
			"}\n"..
			
			 footer(actionType)

		action.Action = "code"
		return action
	end
	
	if actionType == "random_chest_activate" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		local eggId =tostring( tonumber(arg[3]))
		local actId =tostring( tonumber(arg[4]))
		local x = tostring(tonumber(arg[5]))
		local y = tostring(tonumber(arg[6]))
		local z = tostring(tonumber(arg[7]))
		local scenario = tostring( r2:getScenarioId())

		local item1Weight	= arg[8]
		local item1Id		= arg[9]
		local item1Qty		= arg[10]
		local item1Str		= ""
		if item1Id and item1Id ~= "" then
			item1Str		= tostring(item1Id)..":"..tostring(item1Qty)
		end

		local item2Weight	= arg[11]
		local item2Id		= arg[12]
		local item2Qty		= arg[13]
		local item2Str		= ""
		if item2Id and item2Id ~= "" then
			item2Str		= tostring(item2Id)..":"..tostring(item2Qty)
		end

		local item3Weight	= arg[14]
		local item3Id		= arg[15]
		local item3Qty		= arg[16]
		local item3Str		= ""
		if item3Id and item3Id ~= "" then
			item3Str		= tostring(item3Id)..":"..tostring(item3Qty)
		end

		local name = arg[17]

		local sum12			= tostring(item1Weight + item2Weight)
		local sum123			= tostring(item1Weight + item2Weight + item3Weight)

		action.Parameters = "//random_chest_activate\n"
			.."(" ..prefix.."r)rndm(0,100);\n"
			.."if (" ..prefix.. "r > 0 && "..prefix.."r <= "..tostring(item1Weight)..")\n"
			.."{\n\t"
			.."()"..prefix.."activateEasterEgg(" .. eggId .. ", " .. scenario .."," .. actId .. ", \"" .. item1Str.. "\", " .. x.. ", " .. y.. ", " .. z .. ", 0, \""..r2:getNamespace() .. rtGrpId.."\", \""..name.."\", \"\");\n"
			.."}\n"
			.."if (" ..prefix.. "r > "..item1Weight.." && "..prefix.."r <= "..sum12..")\n"
			.."{\n\t"
			.."()"..prefix.."activateEasterEgg(" .. eggId .. ", " .. scenario .."," .. actId .. ", \"" .. item2Str.. "\", " .. x.. ", " .. y.. ", " .. z .. ", 0, \""..r2:getNamespace() .. rtGrpId.."\", \""..name.."\", \"\");\n"
			.."}\n"
			.."if (" ..prefix.. "r > "..sum12.." && "..prefix.."r <= "..sum123..")\n"
			.."{\n\t"
			.."()"..prefix.."activateEasterEgg(" .. eggId .. ", " .. scenario .."," .. actId .. ", \"" .. item3Str.. "\", " .. x.. ", " .. y.. ", " .. z .. ", 0, \""..r2:getNamespace() .. rtGrpId.."\", \""..name.."\", \"\");\n"
			.."}\n"
			.."()"..prefix.."setEvent(4);\n"
		action.Action = "code"
		return action
	end



	if actionType == "easter_egg_activate" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local eggId =tostring( tonumber(arg[3]))
		local actId =tostring( tonumber(arg[4]))

		local items = tostring(arg[5])
		local x = tostring(tonumber(arg[6]))
		local y = tostring(tonumber(arg[7]))
		local z = tostring(tonumber(arg[8]))
		local heading = tostring(tonumber(arg[9]))
		local name = tostring(arg[10])
		if not name then name = "" end
		local look = arg[11]
		if not look then look = "" end
		local scenario =tostring( r2:getScenarioId())



		assert(eggId and scenario and items and x and y and z)
		if not heading then heading = tostring(0) end

		action.Parameters = "//"..actionType.."\n" ..
			"()"..prefix.."activateEasterEgg(" .. eggId .. ", " .. scenario .."," .. actId .. ", \"" .. items.. "\", " .. x.. ", " .. y.. ", " .. z .. "," .. heading .. ", \""..r2:getNamespace() .. rtGrpId.."\", \"".. name .."\", \"".. look .."\");\n"..
			"()"..prefix.."setEvent(4);\n"
		action.Action = "code"
		return action
	end

	if actionType == "easter_egg_deactivate" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local eggId =tostring( tonumber(arg[3]))
		local actId =tostring( tonumber(arg[4]))
		local scenario =tostring( r2:getScenarioId())
		assert(eggId and scenario)
		
		action.Parameters = "//"..actionType.."\n" ..
			"()"..prefix.."deactivateEasterEgg(" .. eggId .. ", " .. scenario.. "," .. actId..");\n"..
			"()"..prefix.."setEvent(5);\n"


		action.Action = "code"
		return action
	end

	if actionType == "dialog_starts" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		action.Parameters=
			"//"..actionType.."\n" ..
			""..prefix.."start=1;\n"  ..
			""..prefix.."v1=0;\n"  .. -- intial time before start of dialog is kind of long because we don't want in a start of state that the targeted npc don't exist
			"()"..prefix.."setTimer(10, ".. Logic.chatTimerId ..");\n" ..
			"()"..prefix.."setEvent(1);" .. "\t//start of dialog\n"

		action.Action = "code"
		return action
	end

	if actionType == "dialog_continues" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		action.Parameters=
			"//"..actionType.."\n" ..
			"if ("..prefix.."break == 1) {\n"  ..
			"\t()"..prefix.."setTimer(1, ".. Logic.chatTimerId ..");\n" ..
			"}\n"

		action.Action = "code"
		return action
	end


	if actionType == "chat_starts" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		assert(type(arg[3]) == "number")
		local index = tonumber(arg[3])
		action.Parameters=
			"//"..actionType.."\n" ..
			""..prefix.."start=1;\n"  ..
			""..prefix.."v1=".. tostring(index+1)..";\n"  ..
			"()"..prefix.."setTimer(1, ".. Logic.chatTimerId ..");\n"

		action.Action = "code"
		return action
	end


	if actionType == "dialog_stops" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		action.Parameters=
			"//"..actionType.."\n" ..
			""..prefix.."start=0;\n"  ..
			"()"..prefix.."setEvent(2);" .. "\t//end of dialog\n"

		action.Action = "code"
		return action
	end

	if actionType == "dialog_deactivate" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() ..rtGrpId.."."
		end
		action.Parameters = prefix.."start = 0;\n"
			..prefix.."Active = 0;\n"

		action.Action = "code"
		return action
	end

	if actionType == "dialog_init" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local mustRepeat = tonumber(arg[3])
		local autoStart = tonumber(arg[4])
		assert(mustRepeat)

		action.Parameters=
			"//"..actionType.."\n" ..
			""..prefix.."repeat=".. mustRepeat..";\n"..
			""..prefix.."AutoStart=".. autoStart..";\n"
			--.."()"..prefix.."setEvent(5); // spawned\n"

		action.Action = "code"
		return action
	end



	if actionType == "chat_step_first" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local initialWait = tonumber(arg[3])
		assert(initialWait)
		action.Parameters =
			"//"..actionType.."\n" ..
			prefix .. Logic.chatStepVar .. " = 1;\n" ..
			"()"..prefix.."setTimer("..tostring(initialWait*10+1) ..", ".. Logic.chatTimerId ..");\n" ..
			"()"..prefix.."setEvent(3);" .. "\t//start of chat\n"

		action.Action = "code"
		return action
	end



	if actionType == "chat_step_last" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId .. "."
		end
		local nbParam = arg[3]
		assert(nbParam and type(nbParam) == "number")

		action.Parameters =
			"//"..actionType.."\n" ..
			prefix..Logic.chatStepVar .. "="..tostring(1+nbParam)..";\n"  .. --set because of setEvent
			"()"..prefix.."setEvent(4);\n" ..
			"if ("..prefix.."repeat == 1) {\n" ..
			"\t"..prefix.."start=1;\n"  ..
			"\t"..prefix..Logic.chatStepVar .. "=0;\n"  ..
			"\t()"..prefix.."setTimer(4, ".. Logic.chatTimerId ..");\n"..
			"\t()"..prefix.."setEvent(2);" .. "\t//end of dialog\n" ..
			"\t()"..prefix.."setEvent(1);" .. "\t//start of dialog\n" ..
			"} else {\n" ..
			"\t"..prefix.."start=0;\n"  ..
			"\t()"..prefix.."setEvent(2);" ..  "\t//end of dialog\n" ..
			"}\n"

		action.Action = "code"
		return action
	end

	if actionType == "chat_step" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local param = arg[3]
		assert(type(param) == "table")


		--local m_action = action
		--m_action.Action = "multi_actions"

		local say =""
		local emote=""
		local facing=""

		local startChat = "()"..prefix.."setTimer(2, 0);\n \n" -- timer 0.4 in on seconde
		-- create facing action
		if param.Facing ~= "" and param.Facing ~= nil and param.Who ~=nil
		then
			facing = "//facing\n" ..
				"(@group1)"..r2:getNamespace()..param.WhoGrp..".context();\n"
				.. "(@group2)"..r2:getNamespace()..param.FacingGrp..".context();\n"
				.. "()"..r2:getNamespace()..rtGrpId..".facing(@group1,\""..param.Who.."\", @group2, \"".. param.Facing.."\");\n \n"
			startChat = "()"..prefix.."setTimer(10, 0);\n \n" -- timer 0.4 in on seconde

		end


		local mustBreak = prefix.."break = "..tostring(param.Break)..";\n" 
		if  param.Break == 0 then
			mustBreak = mustBreak..
				"()"..prefix.."setTimer(" .. tostring(4+10*tonumber(param.Time)).. ", ".. Logic.chatTimerId ..");\n\n"
		else
			mustBreak = mustBreak .."\n"
		end

		do
		--	local action = r2.newComponent("RtNpcEventHandlerAction")
			action.Action = "code"
			action.Parameters =
				"//"..actionType.." - ChatStep ".. tostring(param.Index).." \n" ..
				prefix.."step = " .. tostring(param.Index) ..";\n \n"..
				say..facing..emote..

				"//set next chat step\n" ..
				mustBreak ..
				prefix .. Logic.chatStepVar .. " =  " .. param.Index .. " + 1;\n \n" ..

				startChat..	
				"()"..prefix.."setTimer(25, 2);\n \n".. -- timer 0.9 in on seconde
				"//End of dialog\n"..
				"()"..prefix.."setEvent(4);\n \n"
		--	table.insert(m_action.Children, action)
		end



		return action

	end


	if actionType == "chat_step_end" then

		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local param = arg[3]
		assert(type(param) == "table")

		local baseActRtGrpId = arg[4]
		assert(type(baseActRtGrpId) == "string")

		if (table.getn(param.Emotes) == 0) then	return nil end


		local code =
		"// lauch emote at end of chat step\n \n" ..
		"if ("..prefix.."start == 1)\n" ..
		"{\n"..
		"\tswitch ( ".. prefix.."step )\n"..
		"\t{\n"

		local i = 0
		local n = table.getn(param.Emotes)
		while i ~=  n do
			i = i +1

	
			if  param.WhoNoEntitys[i]  == "_System" or param.WhoNoEntitys[i]  == "_DM" then
				local say = ""				
				who = "System"

				local msg = param.Says[i]
				if not msg then msg = "" end

				if param.WhoNoEntitys[i] == "_DM" then		

					say = "\t\t\t" .. "()".. r2:getNamespace() .. baseActRtGrpId .. ".dssMessage(  "..tostring(r2:getScenarioId()) .. ", "
						.."\"DM\", \"".. who.. "\", \"" .. msg .. "\");\n \n"

				else
					--avoid to display "system :" when a system msg is broadcasted
					who = ""
					say = "\t\t\t" .. "()".. r2:getNamespace() .. baseActRtGrpId .. ".dssMessage(  "..tostring(r2:getScenarioId()) .. ", "
						.."\"SYS\", \"".. who.. "\", \"" .. msg .. "\");\n \n"
				end

				code = code ..
					"\t\t"..	"case "..param.Indexs[i].." :\n" ..
					"\t\t"..	"{\n" ..
					say ..
					"\t\t"..	"}\n \n"
		
			elseif rtGrpId and param.Whos[i] and param.Whos[i] ~= "" and param.Grps[i] and param.Grps[i] ~= "" then
				local say = ""
				if param.Says[i] ~= nil and param.Says[i] ~= "" then
					say = "\t\t\t" .. "()".. r2:getNamespace() .. rtGrpId .. ".npcSay(@group,\"" .. param.Whos[i] .. "\", \"DSS_" .. tostring(r2:getScenarioId()) .. " " .. param.Says[i] .. "\");\n \n"
				end

				local emote = ""
				if param.Emotes[i] ~= "" and param.Emotes[i] ~= nil then
					local behaviorValue = r2.getEmoteBehaviorFromEmoteId(param.Emotes[i])
					emote =  "\t\t\t" .. "()"..r2:getNamespace()..rtGrpId..".emote(@group,\""..param.Whos[i].."\", \""..behaviorValue.."\");\n"
				end

		
	
				code = code ..
					"\t\t"..	"case "..param.Indexs[i].." :\n" ..
					"\t\t"..	"{\n" ..
					"\t\t\t" ..		 "(@group)".. r2:getNamespace() .. param.Grps[i] .. ".context();\n \n" ..
					say ..
					emote ..
					"\t\t"..	"}\n \n"
			end

		end

		code = code .. "\t}\n}"


		action.Action = "code"
		action.Parameters = code

		return action

	end
	
	--BROADCAST
	if actionType == "broadcast_msg" then
		local baseActRtGrpId = arg[2]
		assert(baseActRtGrpId)
		assert(type(baseActRtGrpId) == "string")
		
		local msg = arg[3]
		assert(msg)
		assert(type(msg) == "string")

		local who = ""

		action.Parameters = "()".. r2:getNamespace() .. baseActRtGrpId .. ".dssMessage(  "..tostring(r2:getScenarioId()) .. ", "
						.."\"SYS\", \"".. who.. "\", \"" .. msg .. "\");\n"

		action.Action = "code"
		return action
	end

	--QUEST ACTIONS
	if actionType == "validate_quest_step" then
		local questRtGrpId = arg[2]
		local prefix = ""
		if questRtGrpId and questRtGrpId ~= "" then
			prefix = r2:getNamespace() .. questRtGrpId.."."
		end
				
		local taskRtIds = arg[3]
		local nbTasks = table.getn(taskRtIds)
		
		action.Parameters = 
[[
	if (]] ..prefix.. [[v2 != 0)
	{
		switch(]] ..prefix.. [[v2)
		{
]]
		
		--the case (1) never happens : when the quest begins and when the first task is completed, the step index 
		--is incremented before the action "validate_quest_step"
		--
		local i
		for i = 2, nbTasks do
			action.Parameters = action.Parameters..
[[
		case ]] ..tostring(i).. [[ : 
		{
			if (]]..taskRtIds[i - 1]..[[Active == 1)
			{
				]]..taskRtIds[i - 1]..[[Active = 0;
				()]]..taskRtIds[i - 1]..[[setEvent(5);
			}
			()]] ..taskRtIds[i].. [[setEvent(7);
			]] ..taskRtIds[i].. [[Active = 1;
			()]] ..taskRtIds[i].. [[setEvent(4);
		}
]]
		end
		
		--last task
		action.Parameters = action.Parameters..
[[
	//default is only used by the last step of the quest
		case ]]..tostring(nbTasks + 1) ..[[ : 
		{
			if (]]..taskRtIds[nbTasks]..[[Active == 1)
			{
				]]..taskRtIds[nbTasks]..[[Active = 0;
				()]]..taskRtIds[nbTasks]..[[setEvent(5);
			}
			//if the quest is repeatable
			if (]] ..prefix.. [[v1 == 1)
			{	
				//resetting the index to 1 for first quest step
				]] ..prefix.. [[v2 = 1;
				//()]] ..taskRtIds[1].. [[setEvent(7);
				]] ..taskRtIds[1].. [[Active = 1;
				()]] ..taskRtIds[1].. [[setEvent(4);
			}
			else
			{
				]] ..prefix.. [[v2 = 0;	
			}
			()]] ..prefix.. [[setEvent(8);
		}
	} //!switch
} //!if
]]

		action.Action = "code"
		
		return action
		
	end


	if actionType == "increment_quest_step_index" then
		local currentNamespace = r2:getNamespace()
		
		local questRtGrpId = arg[2]
		local prefix = ""
		if questRtGrpId and questRtGrpId ~= "" then
			prefix = r2:getNamespace() .. questRtGrpId.."."
		end

		local currentTaskIndex = arg[3]

		action.Parameters = "if ("..prefix.."v2 == " ..tostring(currentTaskIndex)..")\n"
			.."{\n"
			.."\t "..prefix.."v2 = " ..prefix.."v2 + 1;\n"
			.."\t ()" ..prefix.."setEvent(9);\n"
			.."}"
			

		action.Action = "code"
		return action
	end
	

	if actionType == "request_item" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local grpName = r2:getNamespace() .. rtGrpId
	
		local items = tostring(arg[3])
		assert(items)

		local phrase = tostring(arg[4])
		assert(phrase)
		if phrase == "" then phrase = "Ok" end
		action.Parameters =
			"// request_item\n"
			.. "(@groupToNotify)".. grpName ..".context();\n"
			.."()receiveMissionItems(\"".. items.."\", \"".. phrase .."\", @groupToNotify);\n"


		action.Action = "code"
		return action

	end


	if actionType == "give_item" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local grpName = r2:getNamespace() .. rtGrpId
	
		local items = tostring(arg[3])
		assert(items)

		local phrase = tostring(arg[4])
		assert(phrase)
		if phrase == "" then phrase = "Ok" end
	
		action.Parameters =
			"// give_item\n"
			.. "(@groupToNotify)".. grpName ..".context();\n"
			.."()giveMissionItems(\"".. items.."\", \"".. phrase .."\", @groupToNotify);\n"


		
		action.Action = "code"
		return action

	end


	if actionType == "give_reward" then
		local rtGiverGrpId = arg[2]
		local rtGiverName = arg[3]
		local rtGiverGrpName = r2:getNamespace()..rtGiverGrpId

		local texts = arg[4]
	
		local rewardText = texts["rewardText"]
		local rareRewardText = texts["rareRewardText"]	
		local inventoryFullText = texts["inventoryFullText"]
		local notEnoughPointsText = texts["notEnoughPointsText"]

		local textsArgs = "\""..rewardText.."\", "
			.."\""..rareRewardText.. "\", "
			.."\""..inventoryFullText.. "\", "
			.."\""..notEnoughPointsText.."\""

		action.Parameters = "//Give reward (giver : '".. rtGiverName.."')\n"
			.."(@groupToNotify)".. rtGiverGrpName..".context();\n"
			.."()giveReward("..textsArgs..", @groupToNotify);\n"

		action.Action = "code"
		return action
	end

	if actionType == "teleport_near" then
		local rtGiverGrpId = arg[2]
		local rtGiverGrpName = r2:getNamespace()..rtGiverGrpId
		local uniqId = arg[3]
		local x = arg[4]
		local y = arg[5]
		local z = arg[6]

		action.Parameters = "//teleport Near\n"
			.."(@groupToNotify)".. rtGiverGrpName..".context();\n"
			.."()teleportNear("..tostring(x)..", "..tostring(y).. ", ".. tostring(z)..", @groupToNotify);\n"

		action.Action = "code"
		return action
	end

	if actionType == "talk_to" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local grpName = r2:getNamespace() .. rtGrpId
	
		local phrase = tostring(arg[3])
		assert(phrase)

		if phrase == "" then phrase = "Ok" end

		action.Parameters =
			"// talk_to\n"
			.. "(@groupToNotify)".. grpName ..".context();\n"
			.."()talkTo(\"".. phrase .."\", @groupToNotify);\n"


		action.Action = "code"
		return action

	end

	if actionType == "set_value" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local variable = tostring(arg[3])
		assert(variable)
		--local variableLen = string.len(variable)
		--assert(variableLen < 8)
		--variable = string.lower(variable)
		local i = 0;

		assert( type(arg[4]) == 'number' or type(arg[4]) == 'string')
		
	 	local value = arg[4]
		action.Parameters = prefix .. variable .. " = " .. value..";\n"
		action.Action = "code"
		return action
	end
	
	--called each time some scenario points
	if actionType == "add_scenario_points" then
		--scenario rtId
		local rtBaseActId = arg[2]
		local points = arg[3]
		local prefix = ""
		if rtBaseActId and rtBaseActId ~= "" then
			prefix = r2:getNamespace()..rtBaseActId.."."
		end

		action.Parameters = prefix.."ScenarioPoints = "..prefix.."ScenarioPoints + " ..tostring(points)..";\n"
							
		action.Action = "code"
		return action
	end
	

	--called every 30 seconds or so to avoid sending network msg each time some points are added
	if actionType == "set_scenario_points" then
		local rtScenarioId = tostring( r2:getScenarioId())
		local rtBaseActId = arg[2]

		local prefix = ""
		if rtBaseActId and rtBaseActId ~= "" then
			prefix = r2:getNamespace()..rtBaseActId.."." 
		end
		
		action.Parameters = "()setScenarioPoints("..rtScenarioId.. ", " ..prefix.."ScenarioPoints);\n"
			.."()"..prefix.."setTimer(300,1);\n"
		action.Action = "code"
		return action
	end

	if actionType == "start_scenario_timing" then
		local rtScenarioId = tostring( r2:getScenarioId())

		action.Parameters = "()startScenarioTiming("..rtScenarioId..");\n"
		action.Action = "code"
		return action
	end

	if actionType == "stop_scenario_timing" then
		local rtScenarioId = tostring( r2:getScenarioId())
		
		action.Parameters = "()endScenarioTiming("..rtScenarioId..");\n"
		action.Action = "code"
		return action
	end

	if actionType == "if_value_equal" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local variable = tostring(arg[3])
		assert(variable)
		local variableLen = string.len(variable)
		--variable = string.lower(variable)
		local i = 0;

		assert( type(arg[4]) == 'number')
		local value = arg[4]
		action.Parameters = prefix..variable .. " == " .. value

		if (arg[5] ~= nil ) then
			assert( type(arg[5]) == 'table')
			local value = arg[4]
			action.Parameters = prefix..variable .. " == " .. value
			table.insert(action.Children, arg[5])
		end


		action.Action = "dynamic_if"

		return action
	end
	
	-- "validate_task" is used when the player completed part of a mission but didn't come back to the mission giver
	if actionType == "validate_task" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		action.Parameters = "//validateTask \n" 
						.."if (" ..prefix.."Active == 1 && " ..prefix.."v2 == 1 )\n"
						.."{"
						.."\n\t" ..prefix.."v2 = 2;\n\t"
						.."()" ..prefix.."setEvent(8);\n"
						.."}"
		action.Action = "code"

		return action
	end
	
	-- "complete_mission" is used when the player comes back to the mission giver after having validated the mission.
	if actionType == "complete_mission" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		action.Parameters = "//complete_mission \n" 
						.."if (" ..prefix.."Active == 1)\n"
						.."{"
						.."\n\t" ..prefix.."v2 = 2;\n\t"
						.."()" ..prefix.."setEvent(9);\n"
						.."}"
		action.Action = "code"

		return action
	end

	if  string.find(actionType, "timer_") ~= nil
	then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local timer = tonumber(arg[3])
		assert(0 <= timer and timer <= 3)

		if actionType == "timer_trigger" then
			action.Parameters = "()"..prefix.."setTimer(4, "..timer..");\n"

		elseif actionType == "timer_disable" then
			action.Parameters = "()"..prefix.."timerDisable("..timer..");\n"
			..prefix.. "Active = 0;"
		elseif actionType == "timer_suspend" then
			action.Parameters = "()"..prefix.."timerSuspend("..timer..");\n"
		elseif actionType == "timer_resume" then
			action.Parameters = "()"..prefix.."timerResume("..timer..");\n"
		elseif actionType == "timer_enable" then
			printWarning("timerEnable is not implemented in AIS!")
			action.Parameters = "()"..prefix.."timerEnable("..timer..");\n" -- !!!NOT IMPLEMENTED IN AIS!!!
		elseif actionType == "timer_is_enable" then
			action.Parameters = "("..prefix.."is_enable"..")"..prefix.."timerIsEnabled("..timer..");\n"
		elseif actionType == "timer_is_suspended" then
			action.Parameters = "("..prefix.."is_suspended"..")"..prefix.."timerIsSuspended("..timer..");\n"
		elseif actionType == "timer_add" or actionType == "timer_sub" then
			local wait = tonumber(arg[4])
			assert(wait and 0<= wait)
			wait = wait*10 + 4
			if  actionType == "timer_sub" then
				action.Parameters = "()"..prefix.."timerAdd("..timer..", " .. -wait  .. ");\n"
			else
				action.Parameters = "()"..prefix.."timerAdd("..timer..", " ..  wait  .. ");\n"
			end
		elseif actionType == "timer_set" then
			local wait = tonumber(arg[4])
			assert(wait and 0<= wait)
			wait = wait*10 + 4
			action.Parameters = "()"..prefix.."setTimer("..wait..", " .. timer .. ");\n"
			..prefix.."Active = 1;\n"

		elseif actionType == "timer_set_daytime" then
			local hours = tonumber(arg[4])
			local minutes = tonumber(arg[5])
			assert(hours and 0<= hours and hours <= 23)
			assert(minutes and 0<= minutes and minutes <= 60)
			action.Parameters = "()"..prefix.."timerSetRyzomDaytime("..timer..", " .. hours .. ", "..minutes..");\n"
		else
			debugInfo(colorTag(255,0,0).."Unhandeld action '" ..actionType .."'")
			assert(nil)
		end
		action.Action = "code"
		return action
	end

	-------------------------------------
	-- Counter feature
	-------------------------------------
	if string.find(actionType, "counter_") ~= nil
	then
		local rtGrpId = arg[2]
		local prefix = ""

		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		--
		-- Action "Init": initializes the counter by storing the initial counter value in v1
		-- and the triggerValue in v2.
		--
		if actionType == "counter_init" then
			local value = arg[3]
			local triggerValue = arg[4]
			action.Parameters = prefix.. "v0 = 1;\n"
			..prefix.."v1 = "..value..";\n"
			..prefix.."v2 = "..triggerValue..";\n"
		end

		--
		-- Action "increment": checks wether the counter is active or not (var v0 used as a boolean),
		-- then increment the counter and enventually checks if the triggerValue (stored in v2) has
		-- been reached to trigger a userEvent.
		--
		if actionType == "counter_inc" then
			action.Name = "counter_inc"
			action.Parameters = "if (" ..prefix.. "v0 == 1)\n"
			.. "{\n\tif (" ..prefix.. "v1 >= 0)\n"
			.. "\t{\n"
			.. "\t\t" .. prefix.. "v1 = " ..prefix.. "v1 + 1;\n"
			.. "\t\tif (" ..prefix.. "v1 == " ..prefix.."v2)\n"
			.. "\t\t{\n"
			.. "\t\t\t" .. prefix.. "e=3;\n"
			.. "\t\t\t ()" ..prefix.. "setEvent(0);\n"
			.. "\t\t}\n"
			.."\t}\n}"
			--.. "()"..prefix..'debug("v0=");'.."\n"
			--.. "()"..prefix..'debug('..prefix..'v0'..');'.."\n"
			--.. "()"..prefix..'debug("v1=");'.."\n"
			--.. "()"..prefix..'debug('..prefix..'v1'..');'.."\n"
			--.. "()"..prefix..'debug("v2=");'.."\n"
			--.. "()"..prefix..'debug('..prefix..'v2'..');'.."\n"
		end

		--
		-- Action "decrement": works the same as increment (checks if the counter can be decremented)
		--
		if actionType == "counter_dec" then
			action.Name = "counter_dec"
			action.Parameters = "if (" ..prefix.. "v0 == 1)\n"
			.. "{\n\tif (" ..prefix.. "v1 > 0)\n"
			.. "\t{\n"
			.. "\t\t" .. prefix.. "v1 = " ..prefix.. "v1 - 1;\n"
			.. "\t\tif (" ..prefix.. "v1 == " ..prefix.. "v2)\n"
			.. "\t\t{ \n"
			.. "\t\t\t" .. prefix.. "e=3;\n"
			.. "\t\t\t()" ..prefix.. "setEvent(0);\n"
			.. "\t\t}\n"
			.."\t}\n}\n"
			.. "()"..prefix..'debug("v0=");'.."\n"
			.. "()"..prefix..'debug('..prefix..'v0'..');'.."\n"
			.. "()"..prefix..'debug("v1=");'.."\n"
			.. "()"..prefix..'debug('..prefix..'v1'..');'.."\n"
			.. "()"..prefix..'debug("v2=");'.."\n"
			.. "()"..prefix..'debug('..prefix..'v2'..');'.."\n"
		end

		if actionType == "counter_enable" then
			action.Name = "counter_enable"
			action.Parameters = prefix.."v0 = 1;\n"
		end

		if actionType == "counter_disable" then
			action.Name = "counter_disable"
			action.Parameters = prefix.."v0 = 0;\n"
		end

		if actionType == "counter_trigger" then
		end

		action.Action = "code"

		if actionType == "counter_is_enable" then
			action.Name =  "counter_is_enable"
			action.Action = "dynamic_if"
			action.Parameters = prefix.. "v0 == 1\n"
		end


		return action
	end

	-------------------------------------
	-- GiveItem Feature
	-------------------------------------

	if string.find(actionType, "giveItem_") ~= nil
	then
		local rtGrpId = arg[2]
		local prefix = ""

		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		if actionType == "giveItem_init" then
			local qty = arg[3]
			--local triggerValue = arg[4]
			action.Parameters = prefix.. "v0 = 1;\n"
			..prefix.."v1 = "..qty..";\n"
			--..prefix.."v2 = "..triggerValue..";\n"
		end

		if actionType == "giveItem_enable" then
			action.Name = "giveItem_enable"
			action.Parameters = prefix.."v0 = 1;\n"
		end

		if actionType == "giveItem_disable" then
			action.Name = "giveItem_disable"
			action.Parameters = prefix.."v0 = 0;\n"
		end

		action.Action = "code"

		if actionType == "giveItem_is_enable" then
			action.Name =  "giveItem_is_enable"
			action.Action = "dynamic_if"
			action.Parameters = prefix.. "v0 == 1\n"
		end

		return action

	end

	-------------------------------------
	-- RequestItem Feature
	-------------------------------------

	if string.find(actionType, "requestItem_") ~= nil
	then
		local rtGrpId = arg[2]
		local prefix = ""

		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		if actionType == "requestItem_init" then
			local qty = arg[3]
			--local triggerValue = arg[4]
			action.Parameters = prefix.. "v0 = 1;\n"
			..prefix.."v1 = "..qty..";\n"
			--..prefix.."v2 = "..triggerValue..";\n"
		end

		if actionType == "requestItem_enable" then
			action.Name = "requestItem_enable"
			action.Parameters = prefix.."v0 = 1;\n"
		end

		if actionType == "requestItem_disable" then
			action.Name = "requestItem_disable"
			action.Parameters = prefix.."v0 = 0;\n"
		end

		action.Action = "code"

		if actionType == "requestItem_is_enable" then
			action.Name =  "requestItem_is_enable"
			action.Action = "dynamic_if"
			action.Parameters = prefix.. "v0 == 1\n"
		end

		return action

	end

	if actionType == "bot_init" then
		function indent(s)
			s = "\t" .. string.gsub(s, "\n", "\n\t")
			return s
		end
		local rtGrpId = arg[2]
		local prefix = ""
			if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		

		assert(type(arg[3]) == "string")
		assert(type(arg[4]) == "number")
		assert(type(arg[5]) == "number")
		assert(type(arg[6]) == "number")

		local category = tostring(arg[3])
		local aggroDist = tonumber(arg[4])
		local botAttackable = tonumber(arg[5]) 
		local playerAttackable = tonumber(arg[6])


		local action = r2.Translator.createAction("faction_init", rtGrpId, category, aggroDist, botAttackable, playerAttackable)
		local code = action.Parameters
		code = 	indent(code)
		action.Parameters = "if ("..prefix.."factInit != 1)\n{\n"..code..prefix.."factInit = 1;\n}\n"

		return action
		
	end
	if actionType == "faction_init" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end
		

		assert(type(arg[3]) == "string")
		assert(type(arg[4]) == "number")
		assert(type(arg[5]) == "number")
		assert(type(arg[6]) == "number")

		local category = tostring(arg[3])
		local aggroDist = tonumber(arg[4])
		local botAttackable = tonumber(arg[5]) 
		local playerAttackable = tonumber(arg[6])
		
		
		local code =""

		code = code.."()"..prefix.."setActivity(\"faction\");\n" 

		if category == "Civil" then
			if botAttackable == 1 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"guard\");\n"	-- don't assist
			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"	-- don't assist
			end
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n" 

		elseif category == "Guard" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"Player\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n" 

			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"guard\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"Player|guard\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"bandit|carnivore|kitin\");\n" 

			end

		elseif category =="Karavan" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"Player\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n" 

			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"karavan\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"Player|karavan\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"bandit|carnivore|kitin|plant|kitinWorker\");\n" 
			end
		elseif category =="Kami" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"Player\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n" 

			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"kami\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"Player|kami\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"bandit|kitin|kitinWorker\");\n" 
			end
		elseif category == "Bandit" then
			if botAttackable == 0 then
				code = code.."()setFactionProp(\"faction\", \"civil\");\n"
				if playerAttackable == 1 then
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player\");\n"
					code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
					code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				end
			
			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"bandit\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"bandit\");\n"
				if playerAttackable == 1 then
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player|guard|karavan|kami\");\n"
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"guard|karavan|kami\");\n"
				end
			end
		elseif category == "Carnivore" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				if playerAttackable == 1 then
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player\");\n"
					code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
					code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"

				end
			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"carnivore\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				if playerAttackable == 1 then
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player|guard|bandit|herbivore|karavan\");\n"
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"guard|bandit|herbivore|karavan\");\n"
				end
			end
		elseif category == "Herbivore" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"

			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"herbivore\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"

			end
		elseif category == "Plant" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"plant\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"plant\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"

			end
		elseif category == "Degen" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"

			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"degen\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"degen\");\n"
				if playerAttackable == 1 then
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player|guard|bandit|plant|herbivore|carnivore|kitin|kitinWorker|kami|karavan\");\n"
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"guard|bandit|plant|herbivore|carnivore|kitin|kitinWorker|kami|karavan\");\n"
				end
			end

		elseif category =="WorkerKitin" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"

			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"kitinWorker\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"kitin|kitinWorker\");\n"
				code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"

			end
		elseif category =="SoldierKitin" then
			if botAttackable == 0 then
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"
				if playerAttackable == 1 then
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player\");\n"
					code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
					code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"

				end
			else
				code = code.."()"..prefix.."setFactionProp(\"faction\", \"kitin\");\n"
				code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"kitin|kitinWorker\");\n"
				if playerAttackable == 1 then 
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"Player|guard|bandit|karavan|kami\");\n" 
				else
					code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"guard|bandit|karavan|kami\");\n" 
				end
			end
		else
			code = code.."()"..prefix.."setFactionProp(\"faction\", \"civil\");\n"	-- don't assist
			code = code.."()"..prefix.."setFactionProp(\"ennemyFaction\", \"\");\n"
			code = code.."()"..prefix.."setFactionProp(\"friendFaction\", \"\");\n"
		end	
		code = code .. "()"..prefix.."setAggro("..tostring(aggroDist)..", 20);\n"
		action.Parameters = code
		action.Action = "code"
		return action
	end
	
	--set player_attackable 
	if actionType == "set_player_attackable" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() ..rtGrpId.."."
		end
		
		assert(type(arg[3]) == "number")

		local playerAttackable = arg[3]
		action.Parameters = "()"..prefix.."setPlayerAttackable("..playerAttackable..");"
		action.Action = "code"
		return action
	end

	--set bot_attackable 
	if actionType == "set_bot_attackable" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() ..rtGrpId.."."
		end
		
		assert(type(arg[3]) == "number")

		local botAttackable = arg[3]
		action.Parameters = "()"..prefix.."setBotAttackable("..botAttackable..");"
		action.Action = "code"
		return action
	end
	
	--make a npc run
	if actionType == "set_running_speed" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() ..rtGrpId.. "."
		end

		action.Parameters = "()"..prefix.."addPersistentProfileParameter(\"running\");"
		action.Action = "code"
		return action
	end

	--make a npc walk
	if actionType == "set_walking_speed" then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() ..rtGrpId.. "."
		end

		action.Parameters = "()"..prefix.."removePersistentProfileParameter(\"running\");"
		action.Action = "code"
		return action
	end

	if actionType == "generic_event_trigger"
	then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local eventId = tonumber(arg[3])
		assert(eventId and 0 <= eventId and eventId <= 9)

		action.Parameters = prefix.."e="..eventId..";\n" ..
			"()"..prefix.."setEvent(0);\n"

		action.Action = "code"
		return action
	end

	if actionType == "user_event_trigger"
	then
		local rtGrpId = arg[2]
		local prefix = ""
		if rtGrpId and rtGrpId ~= "" then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local eventId = tonumber(arg[3])
		assert(eventId and 0 <= eventId and eventId <= 9)

		action.Parameters = "()"..prefix.."setEvent(".. eventId..");\n"

		action.Action = "code"
		return action
	end



-- generci_event
--

	if actionType == "dssStartAct" then
		local actId = tonumber(arg[2])
		assert(actId)
		local sessionId = r2:getScenarioId()
		assert(sessionId)
		action.Action = "code"
		action.Parameters = "()dssStartAct(" .. sessionId .. ", " .. actId .. ");\n"
		return action, action
	end

--trigger_event action
	if string.find(actionType, "trigger_event_%d") ~= nil
	then
		action.Parameters = arg[2]
		return action
	end

--spawn/despawn action
	if (actionType == "null_action")
	then
		return action
	end

	if actionType == "spawn" then 
		local rtNpcGrpId = arg[2]
		action.Action = "code"
		local prefix = ""
		if rtNpcGrpId then
			prefix = r2:getNamespace() .. rtNpcGrpId.."."
		end
		action.Parameters = "()"..prefix.. actionType.."();"
		return action
	end

	
	if actionType == "despawn" then 
		local rtNpcGrpId = arg[2]
		action.Action = "code"
		local prefix = ""
		if rtNpcGrpId then
			prefix = r2:getNamespace() .. rtNpcGrpId.."."
		end
		action.Parameters = "()"..prefix.. actionType.."(0);"
		return action
	end


	if (actionType == "sit_down") or (actionType == "stand_up") then
		local rtGrpId = arg[2]
		action.Action = "code"

		local prefix = ""
		if rtGrpId then
			prefix = r2:getNamespace() .. rtGrpId.."."
		end

		local sitting = 0
		if actionType=="sit_down" then
			sitting = 1
		end
		if sitting == 1 then
			action.Parameters =
				"()"..prefix.."sitDown();\n" ..
				"()"..prefix.."setTimer(40,1);\n" -- wait 4 second
				.. prefix.."isSitting = ".. tostring( sitting) .. ";"
		else
			action.Parameters =
				"()"..prefix.."standUp();\n" ..
				"()"..prefix.."setTimer(40,1);\n" -- wait 4 second
				.. prefix.."isSitting = ".. tostring( sitting) .. ";"		

		end
	end

--multi actions
	if actionType == "multi_actions" then
		local actions = arg[2]
		if actions ~= nil then
			local max = table.getn(actions)
			for i=1, max do
				assert(actions[i])
				table.insert(action.Children, actions[i])
			end
		end
		return action
	end

--say action
	if actionType == "say"
	then
		action.Parameters = "say: "..arg[2]
		return action
	end

	if actionType == "switch_actions"
	then
		action.Parameters = arg[2]
		return action
	end

--npc_say action
	if actionType == "npc_say"
	then
		action.Parameters = ""

		local str = arg[2]

		if str == nil then str = "\n" end
		if (string.find(str, "\n") == nil) then
			str = str .. "\n"
		end

		if (table.getn(arg)==3)
		then
			assert(arg[3])
			assert( tostring(arg[3]) )
			action.Parameters = tostring(arg[3]).."\n"
		end
		action.Parameters = action.Parameters..str
		return action
	end
--emot action
	if actionType == "emot"
	then
		local max = table.getn(arg)
		debugInfo(colorTag(255,0,0,255).."action emot")
		local parameters =""
		for i=2, max do
			parameters = parameters .. arg[i]
			parameters = parameters .. "\n"
		end
		action.Parameters = parameters
		return action
	end

--if action
--arg2: expression
--arg3: action if expression is true
--arg4(optional): action if expression false
	if (actionType == "condition_if")or(actionType == "condition_if_else") or (actionType == "dynamic_if")
	then
		local max = table.getn(arg)
		if max == 4 then
		--	action.Action="dynamic_if_else"
			action.Action="dynamic_if"
			table.insert(action.Children, arg[3])
			table.insert(action.Children, arg[4])
		elseif max ==3 	then
			action.Action="dynamic_if"
			table.insert(action.Children, arg[3])
		elseif max ==2 then
			action.Action="dynamic_if"
		else
			return nil
		end

		action.Parameters = arg[2]
		return action
	end

-- waraning
	if actionType == "code"
	then
		action.Parameters = arg[2]
		return action
	end

--random action
	if actionType == "random_select"
	then
		local max = table.getn(arg)
		for i=2, max do
		table.insert(action.Children, arg[i])
		end
		return action
	end

--set timer action
	if string.find(actionType, "set_timer_t") ~= nil
	then
		local max = table.getn(arg)
		parameters=""
		for i=2, max do
			parameters = parameters .. arg[i]
		end
		action.Parameters = parameters
		return action
	end

--modify variable action
	if actionType == "modify_variable"  or actionType == "begin_state"
	then
		action.Parameters = arg[2]
		return action
	end

	if actionType == "punctual_state"
	then
		action.Parameters = arg[2]
		return action
	end

	if (actionType == "stand_up")or(actionType == "sit_down")or(actionType == "punctual_state_end")
	then
		return action
	end
	printWarning("Unhandled action " .. actionType)
	return nil
end


--first param : event type
--second param: StatesByName
--third param : GroupsByName
--then, parameters
Translator.createEvent = function(...)
	local event = r2.newComponent("RtNpcEventHandler")
	local eventType = arg[1]
	event.Event = eventType
	event.StatesByName = arg[2]
	event.GroupsByName = arg[3]
	assert(arg[1])
	assert(arg[2])
	assert(arg[3])

	if eventType == nil then
		debugInfo("Error invalid eventType")
		assert(nil)
		return nil
	end

	if string.find(eventType, "timer_t%d_triggered") ~=nil or string.find(eventType, "user_event_%d")
	then
		return event
	end

	if (eventType == "end_of_state") or (eventType == "start_of_state")
		or (eventType == "group_under_attack") or (eventType == "destination_reached_all")
		or (eventType == "bot_killed") or (eventType == "group_eliminated") or (eventType == "group_under_attack")
		or (eventType == "destination_reached") or (eventType == "variable_changed")
		or (eventType == "group_despawned") or (eventType == "group_spawned") or (eventType == "bot_target_killed")
		or (eventType == "player_target_npc")

		or string.find(eventType, "variable_v%d_changed") ~= nil
	then
		return event
	end

	if (eventType == "on_player_arrived") then
		event.Event = "player_arrived_trigger_zone"
		event.IsTriggerZone = 1
		return event
	end

	if (eventType == "on_player_left")  then
		event.Event = "player_left_trigger_zone"
		event.IsTriggerZone = 1
		return event
	end


	printWarning("Error invalid event ".. eventType)

	return nil
end

-- it adds a new simple activity in zone within the given context
Translator.createSimpleActivityInZone = function(context, zoneName, groupsByName, mode, static, aiScriptDebug)
	local action
	local event
	local code

	-- init when the group arrives in the zone
	code = '()setMode("' .. mode .. '");\n'
	if static then
	code = code .. '()stopMoving();\n';
	end
	if aiScriptDebug then
		code = 'log("destination_reached_all: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("destination_reached_all", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- restore things when the group quits the zone
	code = '()setMode("Normal");'
	if aiScriptDebug then
		code = 'log("end_of_state: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("end_of_state", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)
end

-- it adds a new activity in zone within the given context
Translator.createActivityInZone = function(context, zoneName, groupsByName, mode, timerId, wanderTime, activityTime, aiScriptDebug)
	assert(wanderTime > 0)
	assert(activityTime > 0)

	local action
	local event
	local code
	local timerEventName = "timer_t" .. timerId .. "_triggered"

	-- init start of state
	code = 'nextState = 0;\n'
	if aiScriptDebug then
		code = 'log("start_of_state: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("start_of_state", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- init when the group arrives in the zone
	code =
	'if (nextState == 0)\n' ..
	'{\n'
	if aiScriptDebug then
		code = code ..
	'	log("destination_reached_all: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n'
	end
	code = code ..
	'	nextState = 1;\n' ..
	'	()setTimer(1, ' .. timerId .. ');\n' ..
	'}\n'
	event = Translator.createEvent("destination_reached_all", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- activity states
	code =
	'if (nextState == 1) {\n' ..
	'	nextState = 2;\n' ..
	'	()stopMoving();\n' ..
	'	()setMode("' .. mode .. '");\n' ..
	'	()setTimer(' .. activityTime .. ', ' .. timerId .. ');\n' ..
	'} else if (nextState == 2) {\n' ..
	'	nextState = 1;\n' ..
	'	()setMode("Normal");\n' ..
	'	()wander();\n' ..
	'	()setTimer(' .. wanderTime .. ', ' .. timerId .. ');\n' ..
	'} else {\n' ..
	'	log("unknown state=", nextState, ", zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' ..
	'	()break();\n' ..
	'}\n'
	if aiScriptDebug then
		code = 'log("' .. timerEventName .. ': state=", nextState, ", zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent(timerEventName, zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- restore things when the group quits the zone
	code =
	'()timerDisable(' .. timerId .. ');\n' ..
	'()setMode("Normal");\n'
	if aiScriptDebug then
		code = 'log("end_of_state: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("end_of_state", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)
end

-- it adds a new hunt activity in zone within the given context
Translator.createHuntActivityInZone = function(context, zoneName, groupsByName, timerId, wanderTime, alertTime, eatTime, aiScriptDebug)
	assert(wanderTime > 0)
	assert(alertTime > 0)
	assert(eatTime > 0)

	local action
	local event
	local code
	local timerEventName = "timer_t" .. timerId .. "_triggered"

	-- init start of state
	code = 'nextState = 0;\n'
	if aiScriptDebug then
		code = 'log("start_of_state: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("start_of_state", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- init when the group arrives in the zone
	code =
	'if (nextState == 0)\n' ..
	'{\n'
	if aiScriptDebug then
		code = code ..
	'	log("destination_reached_all: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n'
	end
	code = code ..
	'	nextState = 1;\n' ..
---	'	()setFactionProp("ennemyFaction", "' .. Translator.PredatorEnemyFaction .. '");\n' ..
	'	()setTimer(1, ' .. timerId .. ');\n' ..
	'}\n'
	event = Translator.createEvent("destination_reached_all", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- activity states
	code =
	'if (nextState == 1) {\n' ..
	'	nextState = 2;\n' ..
	'	()stopMoving();\n' ..
	'	()setMode("Alert");\n' ..
	'	()setTimer(' .. alertTime .. ', ' .. timerId .. ');\n' ..
	'} else if (nextState == 2) {\n' ..
	'	nextState = 1;\n' ..
	'	()setMode("Normal");\n' ..
	'	()wander();\n' ..
	'	()setTimer(' .. wanderTime .. ', ' .. timerId .. ');\n' ..
	'} else if (nextState == 3) {\n' ..
	'	nextState = 1;\n' ..
--	'	()setFactionProp("ennemyFaction", "'.. Translator.PredatorEnemyFaction .. '");\n' ..
	'	()setMode("Normal");\n' ..
	'	()wander();\n' ..
	'	()setTimer(' .. wanderTime .. ', ' .. timerId .. ');\n' ..
	'} else {\n' ..
	'	log("unknown state=", nextState, ", zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' ..
	'	()break();\n' ..
	'}\n'
	if aiScriptDebug then
		code = 'log("' .. timerEventName .. ': state=", nextState, ", zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent(timerEventName, zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- eat the corpse when the target is killed
	code =
	'nextState = 3;\n' ..
--.	'()setFactionProp("ennemyFaction", "");\n' ..
	'()stopMoving();\n' ..
	'()setMode("Eat");\n' ..
	'()setTimer(' .. eatTime .. ', ' .. timerId .. ');\n'
	if aiScriptDebug then
		code = 'log("bot_target_killed: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("bot_target_killed", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)

	-- restore things when the group quits the zone
	code =
	'()timerDisable(' .. timerId .. ');\n' ..
	'()setMode("Normal");\n'
-- ..	'()setFactionProp("ennemyFaction", "Player");\n'
	if aiScriptDebug then
		code = 'log("end_of_state: zone=' .. zoneName .. ', group=' .. groupsByName .. '");\n' .. code
	end
	event = Translator.createEvent("end_of_state", zoneName, groupsByName)
	table.insert(context.RtAct.Events, event)
	action = Translator.createAction("code", code)
	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)
end

-- set GroupParameters of a rt Group by readin a hl Np
-- eg set BotAttackable, aggro_range ..
Translator.setGroupParameters = function(hlNpc, rtNpcGrp)
	assert(hlNpc and type(hlNpc) == "userdata")
	rtNpcGrp.GrpParameters = "ring\n" .. rtNpcGrp.GrpParameters
	rtNpcGrp.AiProfilParams = ""

	local botAttackable = hlNpc.BotAttackable
	if botAttackable == 1
	then
		rtNpcGrp.GrpParameters = rtNpcGrp.GrpParameters.."bot_attackable".. "\n"
	end

	local playerAttackable = hlNpc.PlayerAttackable
	if playerAttackable == 1
	then
		rtNpcGrp.GrpParameters = rtNpcGrp.GrpParameters.."player_attackable".. "\n"
	end

	local aggroRange = hlNpc.Aggro
	if aggroRange ~= nil and aggroRange >= 0
	then
		if (aggroRange > 120) then aggroRange = 120 end
		rtNpcGrp.GrpParameters = rtNpcGrp.GrpParameters.."aggro range: "..aggroRange.."\n"

	end

	-- if hlNpc.UseFame and hlNpc.UseFame == 1 and hlNpc.Fame then
	-- 	rtNpcGrp.AiProfilParams = rtNpcGrp.AiProfilParams 
	-- 	.. "faction:" .. hlNpc.Fame .. "\n"
	--      ..  "fame_for_guard_attack:-300000\n"
	-- end


	local speed = hlNpc.Speed
	if speed ~= nil and type(speed) == "number" and speed == 1 then
		rtNpcGrp.AiProfilParams = rtNpcGrp.AiProfilParams .. "running\n"
	end

	local autoSpawn = hlNpc.AutoSpawn
	if autoSpawn ~= nil and autoSpawn == 0 then
		rtNpcGrp.AutoSpawn = 0
	end

	local noRespawn = hlNpc.NoRespawn
	if noRespawn ~= nil and noRespawn == 1 then
		rtNpcGrp.GrpParameters = rtNpcGrp.GrpParameters.. "respawn time:-1\n"
	end

	local isFauna = hlNpc.IsFauna
	if isFauna ~= nil and isFauna == 1 then
		rtNpcGrp.GrpParameters = rtNpcGrp.GrpParameters.. "denied_astar_flags:WaterAndNoGo\n"
	end

end


-- TODO doc



---- EventHandlers

-- Components
--- Condition
Translator.getComponentGenericEvent =function(rtNpcGrp, id)
	assert(rtNpcGrp)
	assert(id)
	
	local prefix = ""
	
	if rtNpcGrp.Id and rtNpcGrp.Id ~= "" then
		prefix = r2:getNamespace() .. rtNpcGrp.Id.."."
	end

	local eventHandler =  Translator.createEvent("user_event_0", "",  rtNpcGrp.Id)

	local condition = prefix.. "e == " .. tostring(id)

	local firstCondition = Translator.createAction("dynamic_if", condition)

	return eventHandler, firstCondition, firstCondition
end

Translator.getComponentUserEvent =function(rtNpcGrp, id)
	assert(rtNpcGrp)
	assert(rtNpcGrp.Id)
	assert(id)

	local eventHandler =  Translator.createEvent("user_event_".. id, "",  rtNpcGrp.Id)

--	local condition = "1 == 1"

--	local firstCondition = Translator.createAction("dynamic_if", condition)

	return eventHandler, nil, nil
--, firstCondition, firstCondition
end



-- NPC
-- Selecter

Translator.getNpcLogicCondition = function(entity, context, condition )
	assert( condition.Class == "ConditionStep")
	local rtNpcGrp = Translator.getRtGroup(context, condition.Entity)
	assert(rtNpcGrp)
	local funs ={}
	funs["is in activity sequence"] =Translator.getNpcLogicConditionIsInActivitySequence
	funs["is in activity step"] = Translator.getNpcLogicConditionIsInActivityStep
	funs["is in chat sequence"] = Translator.getNpcLogicConditionIsInChatSequence
	funs["is in chat step"] = Translator.getNpcLogicConditionIsInChatStep
	funs["is dead"] = Translator.getNpcLogicConditionIsDead
	funs["is alive"] = Translator.getNpcLogicConditionIsAlive


	local fun = funs[ condition.Condition.Type ]
	if fun then
		return fun(entity, context, condition, rtNpcGrp)
	end

	local firstAction, lastAction = nil,nil

	return firstAction, lastAction
end


Translator.getNpcLogicAction = function(entity, context, action)
	assert( action.Class == "ActionStep")
	local rtNpcGrp = Translator.getRtGroup(context, action.Entity)
	assert(rtNpcGrp)
	local funs ={}
	funs["Deactivate"] =Translator.getNpcLogicActionDeactivate
	funs["Activate"] =Translator.getNpcLogicActionActivate
	funs["Kill"] = Translator.getNpcLogicActionKill
	funs["begin activity sequence"] = Translator.getNpcLogicActionBeginActivitySequence
	funs["begin chat sequence"] = Translator.getNpcLogicActionBeginChatSequence
	funs["Stand Up"] = Translator.getNpcLogicActionStandUp
	funs["Sit Down"] = Translator.getNpcLogicActionSitDown
	funs["Fight with player"] = Translator.getNpcLogicActionFightPlayer
	funs["Fight with Npcs"] = Translator.getNpcLogicActionFightNpcs
	funs["Dont fight with player"] = Translator.getNpcLogicActionDontFightPlayer
	funs["Dont fight with Npcs"] = Translator.getNpcLogicActionDontFightNpcs
	funs["Run"] = Translator.getNpcLogicActionRun
	funs["Dont run"] = Translator.getNpcLogicActionDontRun
	funs["emits user event"] = Translator.getNpcLogicActionEmitsUserEvent


	local fun = funs[ action.Action.Type ]
	if fun then
		return fun(entity, context, action, rtNpcGrp)
	end


	local firstAction, lastAction = nil,nil

	return firstAction, lastAction
end




Translator.getNpcLogicEvent = function(entity, context, event)
	assert( event.Class == "LogicEntityAction")
	local rtNpcGrp = Translator.getRtGroup(context, entity.InstanceId)
	assert(rtNpcGrp)

	local funs ={}
	funs["activation"] = Translator.getNpcLogicEventActivation
	funs["desactivation"] = Translator.getNpcLogicEventDesactivation
	funs["death"] = Translator.getNpcLogicEventDeath
	
	funs["end of activity step"] = Translator.getNpcLogicEventEndOfActivityStep
	funs["begin of activity step"] = Translator.getNpcLogicEventBeginOfActivityStep
	
	funs["end of activity sequence"] = Translator.getNpcLogicEventEndOfActivitySequence
	funs["begin of activity sequence"] = Translator.getNpcLogicEventBeginOfActivitySequence

	funs["end of chat step"] = Translator.getNpcLogicEventEndOfChatStep
	funs["end of chat sequence"] = Translator.getNpcLogicEventEndOfChatSequence
	funs["user event emitted"] = Translator.getNpcLogicEventUserEventEmitted



	-- There is also group specific functions

	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	local value = event.Event.Type
	local fun = funs[value ]
	if fun then
		return fun(entity, context, event, rtNpcGrp)
	end

	if value == "member death" then
		local eventHandler =  Translator.createEvent("bot_killed", "",  rtNpcGrp.Id)
		return eventHandler, nil, nil
	elseif value == "group death" then
		local eventHandler =  Translator.createEvent("group_eliminated", "",  rtNpcGrp.Id)
		return eventHandler, nil, nil
	elseif value == "targeted by player" then
		local eventHandler =  Translator.createEvent("player_target_npc", "",  rtNpcGrp.Id)
		return eventHandler, nil, nil
	end

	return  eventHandler, firsCondition, lastCondition
end
--- Event

Translator.getGenericLogicEventDesactivation = function (rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	eventHandler = Translator.createEvent("group_despawned", "",  rtNpcGrp.Id)
	return eventHandler, firsCondition, lastCondition
end



Translator.getNpcLogicEventDesactivation = function (hlComponent, context, event, rtNpcGrp)
	return Translator.getGenericLogicEventDesactivation(rtNpcGrp)
end

Translator.getNpcLogicEventUserEventEmitted = function (hlComponent, context, event, rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	value = event.Event.ValueString
	if value then value = tonumber(value) end
	if not value then return end
	return Translator.getComponentGenericEvent(rtNpcGrp, value)
end


Translator.getNpcLogicEventActivation = function (hlComponent, context, event, rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	eventHandler = Translator.createEvent("group_spawned", "",  rtNpcGrp.Id)
	return eventHandler, firsCondition, lastCondition
end


Translator.getGenericLogicEventDeath= function(rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	eventHandler = Translator.createEvent("bot_killed", "",  rtNpcGrp.Id)
	return eventHandler, firsCondition, lastCondition
end


Translator.getNpcLogicEventDeath= function(hlComponent, context, event, rtNpcGrp)
	return Translator.getGenericLogicEventDeath(rtNpcGrp)
end


Translator.getGenericLogicEventEndOfActivitySequence = function(hlComponent, value, rtNpcGrp)
	local sequenceInstanceId = tostring(value)
	local sequence  = r2:getInstanceFromId(sequenceInstanceId)
	assert(sequence)

	local n = table.getn(sequence.Components)

	if n > 0 then

		local firstStep = sequence.Components[ n-1 ].InstanceId
		eventHandler = Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)
		local value = tostring(firstStep)
		local tab=Logic.findActivityStepIdByInstanceId(sequence.Parent.Parent.Parent, value)
		local id, id2 = tab[1], tab[2]

		if (id == -1 or id2 == -1) then
			printWarning("error in translation: the event '"..event.Name .. "' in component '" .. hlComponent.Name ..
				"': the selected activity step can not be found");
			return nil, nil, nil

		end

		local prefix = ""
		local condition1 = prefix .. "oldActivityStepVar2 == ".. tostring(id + 1) -- in theory it must be (id-1) +1
		local condition2 = prefix .. "currentActivitySequenceVar == ".. tostring(id2-1)
		local firstCondition =	Translator.createAction("dynamic_if", condition1)
		local lastCondition = Translator.createAction("dynamic_if", condition2)
		table.insert(firstCondition.Children, lastCondition)

		return eventHandler, firstCondition, lastCondition

	else

		local eventHandler, firsCondition, lastCondition = nil, nil, nil
		eventHandler = Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)

		local prefix = ""
		local tab=Logic.findActivitySequenceIdByInstanceId(sequence.Parent.Parent.Parent, value)
		local id  = tab[1]
		local condition = prefix .. "currentActivitySequenceVar == ".. tostring(id-1)
		local firstCondition =	Translator.createAction("dynamic_if", condition)
		return eventHandler, firstCondition, firstCondition
	end

end


Translator.getGenericLogicEventStartOfActivitySequence = function(value, rtNpcGrp)
	local sequenceInstanceId = tostring(value)
	local sequence  = r2:getInstanceFromId(sequenceInstanceId)
	assert(sequence)

	local n = table.getn(sequence.Components) 

	if n > 0 then
		local firstStep = sequence.Components[0].InstanceId

		local eventHandler, firsCondition, lastCondition = nil, nil, nil

		eventHandler = Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)
		local value = tostring(firstStep)
		local tab=Logic.findActivityStepIdByInstanceId(sequence.Parent.Parent.Parent, value)
		local id, id2 = tab[1], tab[2]

		if (id == -1 or id2 == -1) then
			printWarning("error in translation: the event '"..event.Name .. "' in component '" .. hlComponent.Name ..
				"': the selected activity step can not be found");
			return nil, nil, nil

		end

		local prefix = ""
		local condition1 = prefix .. "oldActivityStepVar2 == ".. tostring(id ) -- in theory it must be (id-1) +1
		local condition2 = prefix .. "currentActivitySequenceVar == ".. tostring(id2-1)
		local firstCondition =	Translator.createAction("dynamic_if", condition1)
		local lastCondition = Translator.createAction("dynamic_if", condition2)
		table.insert(firstCondition.Children, lastCondition)
		return eventHandler, firstCondition, lastCondition
	else

		local eventHandler, firsCondition, lastCondition = nil, nil, nil
		eventHandler = Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)

		local prefix = ""
		local tab=Logic.findActivitySequenceIdByInstanceId(sequence.Parent.Parent.Parent, value)
		local id  = tab[1]
		local condition = prefix .. "currentActivitySequenceVar == ".. tostring(id-1)
		local firstCondition =	Translator.createAction("dynamic_if", condition)
		return eventHandler, firstCondition, firstCondition
	end
end

Translator.getNpcLogicEventBeginOfActivitySequence = function(hlComponent, context, event, rtNpcGrp)
	local value = tostring(event.Event.Value)
	return Translator.getGenericLogicEventStartOfActivitySequence(value, rtNpcGrp)
end


Translator.getNpcLogicEventEndOfActivitySequence = function(hlComponent, context, event, rtNpcGrp)
	local value = tostring(event.Event.Value)
	return Translator.getGenericLogicEventEndOfActivitySequence(hlComponent, value, rtNpcGrp)
end



Translator.getNpcLogicEventEndOfActivityStepImpl = function(value, rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil

	eventHandler = Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)

	local step = r2:getInstanceFromId(value)
	assert(step)
	local sequence = step.Parent.Parent
	assert(sequence)
	local hlComponent = sequence.Parent.Parent.Parent
	assert(hlComponent)

	local tab=Logic.findActivityStepIdByInstanceId(hlComponent, value)
	local id, id2 = tab[1], tab[2]

	if (id == -1 or id2 == -1) then
		printWarning("error in translation: the event '"..event.Name .. "' in component '" .. hlComponent.Name ..
			"': the selected activity step can not be found");
		return nil, nil, nil

	end

	local prefix = ""
	local condition1 = prefix .. "oldActivityStepVar2 == ".. tostring(id+1) -- in theory it must be (id-1) +1
	local condition2 = prefix .. "currentActivitySequenceVar == ".. tostring(id2-1)
	local firstCondition =	Translator.createAction("dynamic_if", condition1)
	local lastCondition = Translator.createAction("dynamic_if", condition2)
	table.insert(firstCondition.Children, lastCondition)
	return eventHandler, firstCondition, lastCondition
end

Translator.getNpcLogicEventEndOfActivityStep = function(hlComponent, context, event, rtNpcGrp)
	local value = tostring(event.Event.Value)
	return Translator.getNpcLogicEventEndOfActivityStepImpl( value, rtNpcGrp)
end


Translator.getNpcLogicEventBeginOfActivityStepImpl = function(value, rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	
	assert(rtNpcGrp)
	local step = r2:getInstanceFromId(value)
	assert(step)
	local sequence = step.Parent.Parent
	assert(sequence)
	local hlComponent = sequence.Parent.Parent.Parent
	assert(hlComponent)
	
	eventHandler = Translator.createEvent("timer_t0_triggered", "",  rtNpcGrp.Id)
	
	local tab=Logic.findActivityStepIdByInstanceId(hlComponent, value)
	local id, id2 = tab[1], tab[2]

	if (id == -1 or id2 == -1) then
		printWarning("error in translation: the event '"..event.Name .. "' in component '" .. hlComponent.Name ..
			"': the selected activity step can not be found");
		return nil, nil, nil

	end

	local prefix = ""
	local condition1 = prefix .. "oldActivityStepVar2 == ".. tostring(id ) -- in theory it must be (id-1) +1
	local condition2 = prefix .. "currentActivitySequenceVar == ".. tostring(id2-1)
	local firstCondition =	Translator.createAction("dynamic_if", condition1)
	local lastCondition = Translator.createAction("dynamic_if", condition2)
	table.insert(firstCondition.Children, lastCondition)
	return eventHandler, firstCondition, lastCondition
end


Translator.getNpcLogicEventBeginOfActivityStep = function(hlComponent, context, event, rtNpcGrp)
	local value = tostring(event.Event.Value)
	return Translator.getNpcLogicEventBeginOfActivityStepImpl( value, rtNpcGrp)
end




Translator.getNpcLogicEventEndOfChatStepImpl = function(hlComponent, context, event, rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil

	eventHandler = Translator.createEvent("timer_t1_triggered", "",  rtNpcGrp.Id)
	local value = tostring(event.Event.Value)

	local tab=Logic.findChatStepIdByInstanceId(hlComponent, value)
	local id, id2 = tab[1], tab[2]



	if (id ==-1 or id2 == -1) then
		printWarning("error in translation: the event '"..event.Name .. "' in component '" .. hlComponent.Name ..
			"': the selected chat step can not be found");
		return nil, nil, nil

	end
	local prefix = ""
	local condition1 = prefix.."oldChatStepVar == ".. tostring(id-1)
	local condition2 = prefix.."v0 == ".. tostring(id2-1)
	local firstCondition =	Translator.createAction("dynamic_if", condition1)
	local lastCondition = Translator.createAction("dynamic_if", condition2)
	table.insert(firstCondition.Children, lastCondition)
	return eventHandler, firstCondition, lastCondition
end


Translator.getNpcLogicEventEndOfChatStep = function(hlComponent, context, event, rtNpcGrp)
	local value = tostring(event.Event.Value)
	return Translator.getNpcLogicEventEndOfChatStepImpl(hlComponent, value, rtNpcGrp)
end
--- Conditions


Translator.getGenericLogicConditionIsInActivitySequence = function(entity, conditionValue, rtNpcGrp)
	local prefix = ""
	if rtNpcGrp then prefix =  r2.getNamespace() ..rtNpcGrp.Id.."." end

	local theValue = conditionValue
	local tab=Logic.findActivitySequenceIdByInstanceId(entity, theValue)
	local id, id2 = tab[1], tab[2]

	assert(id ~= -1)

	local condition1 = prefix.."oldActivitySequenceVar == ".. tostring(id-1)
	local firstCondition =	Translator.createAction("dynamic_if", condition1)
	local lastCondition = firstCondition
	return firstCondition, lastCondition

end

Translator.getNpcLogicConditionIsInActivitySequence = function(entity, context, condition, rtNpcGrp)
	local theValue = condition.Condition.Value
	return Translator.getGenericLogicConditionIsInActivitySequence(entity, theValue, rtNpcGrp)
end

Translator.getGenericLogicConditionIsInActivityStep = function(entity, conditionValue, rtNpcGrp)
	assert(entity and type(entity) == "userdata")
	assert(conditionValue and type(conditionValue) == "string")
	assert(rtNpcGrp and type(rtNpcGrp) == "table")

	local prefix = ""
	if rtNpcGrp then prefix =  r2.getNamespace() ..rtNpcGrp.Id.."." end


	local theValue = conditionValue
	local tab=Logic.findActivityStepIdByInstanceId(entity, theValue)
	local id, id2 = tab[1], tab[2]

	assert(id ~= -1 and id2 ~= -2);

	local condition1 = prefix.."oldActivityStepVar2 == ".. tostring(id)
	local condition2 = prefix.."currentActivitySequenceVar == ".. tostring(id2-1)
	local firstCondition =	Translator.createAction("dynamic_if", condition1)
	local lastCondition = Translator.createAction("dynamic_if", condition2)
	table.insert(firstCondition.Children, lastCondition)
	return firstCondition, lastCondition
end

Translator.getNpcLogicConditionIsInActivityStep = function(entity, context, condition, rtNpcGrp)
	assert(entity and type(entity) == "userdata")
	assert(context and type(context) == "table")
	assert(condition and type(condition) == "userdata" and condition.Class == "ConditionStep")
	assert(rtNpcGrp and type(rtNpcGrp) == "table")

	local conditionValue = condition.Condition.Value
	return Translator.getGenericLogicConditionIsInActivityStep(entity, conditionValue, rtNpcGrp)
end

Translator.getNpcLogicConditionIsDeadOrAlive = function(entity, context, condition, rtNpcGrp, isAlive)
	assert(entity and type(entity) == "userdata")
	assert(context and type(context) == "table")
	assert(condition and type(condition) == "userdata" and condition.Class == "ConditionStep")
	assert(rtNpcGrp and type(rtNpcGrp) == "table")
	
	local prefix = ""
	if rtNpcGrp then prefix =  r2.getNamespace() ..rtNpcGrp.Id.."." end

	local lastCondition = Translator.createAction("dynamic_if", prefix.."alive == "..tonumber(isAlive) )
	local firstCondition =	Translator.createAction("multi_actions", 
		{
			Translator.createAction("code","("..prefix.."alive)"..prefix.."isAlived();" ),
			lastCondition
		})
	return firstCondition, lastCondition

end

Translator.getNpcLogicConditionIsDead = function(entity, context, condition, rtNpcGrp)
	return Translator.getNpcLogicConditionIsDeadOrAlive(entity, context, condition, rtNpcGrp, 0)
end

Translator.getNpcLogicConditionIsAlive = function(entity, context, condition, rtNpcGrp)
	return Translator.getNpcLogicConditionIsDeadOrAlive(entity, context, condition, rtNpcGrp, 1)
end


-- Action

Translator.getNpcLogicActionDeactivate = function(entity, context, action, rtNpcGrp)

	if (not entity or not rtNpcGrp) then
		 return nil
	end

	local prefix = ""
	if rtNpcGrp then prefix =  r2.getNamespace() ..rtNpcGrp.Id.."." end

	local retAction = Translator.createAction("code", "()"..prefix.."despawn(0);", "")
	assert(retAction)
	retAction.Name = "desactivate"
	return retAction, retAction
end




Translator.getNpcLogicActionActivate = function(entity, context, action, rtNpcGrp)

	if (not entity or not rtNpcGrp) then
		 return nil
	end

	local prefix = ""
	if rtNpcGrp then prefix =  r2.getNamespace() ..rtNpcGrp.Id.."." end
--()setAutoSpawn(1);
	local retAction = Translator.createAction("code", "()"..prefix.."spawn();", "")
	assert(retAction)
	retAction.Name = "activate"
	return retAction, retAction
end


Translator.getNpcLogicActionKill = function(entity, context, action, rtNpcGrp)
	local prefix = ""
	if rtNpcGrp then prefix =  r2.getNamespace() ..rtNpcGrp.Id.."." end

	local retAction = Translator.createAction("code", "()"..prefix.."setHPScale(0);")
	assert(retAction)
	retAction.Name = "kill"
	return retAction, retAction
end

Translator.getGenericLogicActionBeginActivitySequence = function(sequenceInstanceId, rtNpcGrp)

	local activityStates = Logic.ActivitiesStates[sequenceInstanceId]
	assert(activityStates)
 	local activityStatesId = activityStates[sequenceInstanceId][1].Id

 	local retAction = Translator.createAction("code", "()"..r2:getNamespace() .. rtNpcGrp.Id .. "." .. "postNextState(\""..r2:getNamespace()..activityStatesId.."\");", rtNpcGrp.Id)
	return retAction, retAction

end
Translator.getNpcLogicActionBeginActivitySequence = function(entity, context, action, rtNpcGrp)
	local sequenceInstanceId = action.Action.Value
	return	 Translator.getGenericLogicActionBeginActivitySequence(sequenceInstanceId, rtNpcGrp)
end


Translator.getNpcLogicActionSitDown = function(entity, context, action, rtNpcGrp)
	local retAction = Translator.createAction("sit_down",  rtNpcGrp.Id)
	assert(retAction)
	retAction.Name = "sitDown"
	return retAction, retAction
end


Translator.getNpcLogicActionStandUp = function(entity, context, action, rtNpcGrp)
	local retAction =Translator.createAction("stand_up",  rtNpcGrp.Id)
	retAction.Name = "standUp"
	return retAction, retAction
end


Translator.getNpcLogicActionFightPlayer = function(entity, context, action, rtNpcGrp)
	local leader = entity
	if entity:isKindOf("NpcGrpFeature") then
		if table.getn(entity.Components) >= 0 then
			leader = entity.Components[0]
		else
			leader = nil
		end
	end
	assert(leader)
	local category = leader.SubCategory
	local aggro = leader.Aggro
	if not category then 
		category = leader.Category
	end 

	local action1 = Translator.createAction("set_player_attackable", rtNpcGrp.Id, 1)
	local action2 = r2.Translator.createAction("faction_init", rtNpcGrp.Id, category, aggro, leader.BotAttackable, 1)
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	retAction.Name = "Fight with player"
	return retAction ,retAction	
end

Translator.getNpcLogicActionDontFightPlayer = function(entity, context, action, rtNpcGrp)
	local leader = entity
	if entity:isKindOf("NpcGrpFeature") then
		if table.getn(entity.Components) >= 0 then
			leader = entity.Components[0]
		else
			leader = nil
		end
	end
	assert(leader)
	local category = leader.SubCategory
	local aggro = leader.Aggro
	if not category then 
		category = leader.Category
	end 

	local action1 = Translator.createAction("set_player_attackable", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("faction_init", rtNpcGrp.Id, category, aggro, leader.BotAttackable, 0)
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	retAction.Name = "Dont fight with player"
	return retAction ,retAction	
end


Translator.getNpcLogicActionFightNpcs = function(entity, context, action, rtNpcGrp)
	local leader = entity
	if entity:isKindOf("NpcGrpFeature") then
		if table.getn(entity.Components) >= 0 then
			leader = entity.Components[0]
		else
			leader = nil
		end
	end
	assert(leader)
	local category = leader.SubCategory
	local aggro = leader.Aggro
	if not category then 
		category = leader.Category
	end 

	local action1 = Translator.createAction("set_bot_attackable", rtNpcGrp.Id, 1)
	local action2 = Translator.createAction("faction_init", rtNpcGrp.Id, category, aggro, 1, leader.PlayerAttackable)
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	retAction.Name = "Fight with Npcs"
	--inspect(action2)
	return retAction ,retAction	
end

Translator.getNpcLogicActionDontFightNpcs = function(entity, context, action, rtNpcGrp)
	local leader = entity
	if entity:isKindOf("NpcGrpFeature") then
		if table.getn(entity.Components) >= 0 then
			leader = entity.Components[0]
		else
			leader = nil
		end
	end
	assert(leader)
	local category = leader.SubCategory
	local aggro = leader.Aggro
	if not category then 
		category = leader.Category
	end 

	local action1 = Translator.createAction("set_bot_attackable", rtNpcGrp.Id, 0)
	local action2 = r2.Translator.createAction("faction_init", rtNpcGrp.Id, category, aggro, 0, leader.PlayerAttackable)
	
	local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
	retAction.Name = "Dont fight with Npcs"
	return retAction ,retAction	
end


Translator.getNpcLogicActionRun = function(entity, context, action, rtNpcGrp)
	local action1 = Translator.createAction("set_running_speed", rtNpcGrp.Id)
	action1.Name = "Run"
	return action1, action1
end


Translator.getNpcLogicActionDontRun = function(entity, context, action, rtNpcGrp)
	local action1 = Translator.createAction("set_walking_speed", rtNpcGrp.Id)
	action1.Name = "Dont run"
	return action1, action1
end

Translator.getNpcLogicActionEmitsUserEvent = function(entity, context, action, rtNpcGrp)
	if not action.Action.ValueString then return end
	local value = tostring(action.Action.ValueString)

	local action1 = Translator.createAction("generic_event_trigger", rtNpcGrp.Id, value)
	action1.Name = "Trigger User Event"
	return action1, action1
end
-- TODO to up
-- Register an RtNpcGrp to a specific instanceId
Translator.registerManager = function(context, comp)
	local rtNpcGrp = r2.newComponent("RtNpcGrp")
	table.insert(context.RtAct.NpcGrps, rtNpcGrp)
	context.RtGroups[tostring(comp.InstanceId)] = rtNpcGrp
	rtNpcGrp.Name = rtNpcGrp.Id
end


function Translator.createAiGroup(entity,  context)
	local rtNpcGrp = Translator.getRtGroup(context, entity.InstanceId)
	rtNpcGrp.GrpParameters = "ring\n".. rtNpcGrp.GrpParameters

	rtNpcGrp.AutoSpawn = 0

	local aiState = r2.newComponent("RtAiState")

	aiState.AiActivity = "normal"
	table.insert(context.RtAct.AiStates, aiState)
	table.insert(aiState.Children, rtNpcGrp.Id)
	context.GroupStatesName[rtNpcGrp.Id] = aiState.Id

	return rtNpcGrp, aiState
end

function Translator.translateAiGroup(entity, context)
	local rtNpcGrp = Translator.getRtGroup(context, entity.InstanceId)
	if  entity.Behavior.Actions then
		Translator.translateEventHandlers( context, entity, entity.Behavior.Actions, rtNpcGrp)
	end
end



function Translator.translateAiGroupEvent(eventName, entity, context, rtAction)
	assert(rtAction)
	--inspect(context)
	local rtNpcGrp = Translator.getRtGroup(context, entity.InstanceId)
	local states = Translator.getRtStatesNames(context, entity.InstanceId)
--	local rtAct = context.RtAct

	local act = entity:getParentAct()
	local rtAct2 = context.ActIdToRtAct[act.InstanceId]
	local entityAct = context.Act

	local entityIndex = context.Scenario:getActIndex(act.InstanceId)
	local componentIndex =  context.Scenario:getActIndex(context.Act.InstanceId)

	-- entity on act2, component on act1 (they can not interact)
	if entityIndex ~= componentIndex and entityIndex ~= 0 and componentIndex ~= 0 then
		return
	end

	local rtAct = context.RtAct
	if rtAct2 ~= rtAct then
		local baseAct = context.Scenario:getBaseAct()
		local index = context.Scenario:getActIndex(context.Act.InstanceId)
		if index == -1 then
			printWarning("Invalid Scenario")
		end
		-- Permanent content is known by everyone
		if index ~= 0 then
			local rtNpcGrpBase = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		
			local action = Translator.createAction("test_act", rtNpcGrpBase.Id , index)
			table.insert(action.Children, rtAction)
			rtAction = action
		end
	end
	
	local rtEvent = Translator.createEvent(eventName, states,  rtNpcGrp.Id)
	table.insert(rtEvent.ActionsId, rtAction.Id)
	table.insert(rtAct2.Events, rtEvent)
	table.insert(rtAct2.Actions, rtAction)
end

function Translator.translateAiGroupInitialState(entity, context, rtAction)
	return Translator.translateAiGroupEvent("start_of_state", entity, context, rtAction)
end

function Translator.translateAiGroupPlayerTargetNpc(entity, context, rtAction)
	return Translator.translateAiGroupEvent("player_target_npc", entity, context, rtAction) 
end

--Group
Translator.getNpcGroupLogicGroupDeath= function(this, context, eventrtNpcGrp, rtNpcGrp)
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	eventHandler = Translator.createEvent("group_eliminated", "",  rtNpcGrp.Id)
	return eventHandler, firsCondition, lastCondition
end


function Translator.translateDialog(entity, context)
	local rtNpcGrp = Translator.getRtGroup(context, entity.InstanceId)

	local beforeWait = 0

	if (table.getn(entity.Components) > 0) then
		local i = 0
		while i < table.getn(entity.Components) and not entity.Components[i]:isKindOf("ChatStep") do
			i = i + 1 
		end
		if i < table.getn(entity.Components) and entity.Components[i]:isKindOf("ChatStep") then
			beforeWait = entity.Components[i].Time
		end
	end

	local rtEventStart = Translator.createEvent("timer_t1_triggered", "", rtNpcGrp.Id) -- Start Of Chat Step



	local rtDialogOk = Translator.createAction("switch_actions",  Logic.chatStepVar)
	local rtAction = nil
	local rtAction2 = nil

	rtAction = Translator.createAction("chat_step_first", rtNpcGrp.Id, beforeWait)
	table.insert(rtDialogOk.Children, rtAction)
	local chatStepIndex = 0
	local componentIndex, component = next(entity.Components)

	local endParam = { Grps={}, Whos={}, Emotes={}, Indexs={}, Says={}, WhoNoEntitys={}}

	while componentIndex do
		
		if component and component:isKindOf("ChatStep") then
			chatStepIndex = chatStepIndex + 1
			local param = {}


			local who = r2:getInstanceFromId(component.Actions[0].Who)
			local facing = r2:getInstanceFromId(component.Actions[0].Facing)
	


			local whoGrp = nil
			local whoName = nil
			local facingGrp = nil
			local facingName = nil

			if who then
				whoGrp = Translator.getRtGroup(context, who.InstanceId)
				whoGrp = whoGrp.Id
				whoName = who.Name
			end

			if facing then
				facingGrp = Translator.getRtGroup(context, facing.InstanceId)
				facingGrp = facingGrp.Id
				facingName = facing.Name
			end

			param.WhoGrp = whoGrp
			param.Who = whoName
			param.FacingGrp = facingGrp
			param.Facing = facingName
			if not param.Facing then param.Facing = "" end
			param.Says = r2.Features.TextManager.getRtId(context, component.Actions[0].Says)
			if not param.Says then param.Says = "" end
			param.Emote = component.Actions[0].Emote
			if not param.Emote then param.Emote = "" end
			param.Index = chatStepIndex

			param.WhoNoEntity = component.Actions[0].WhoNoEntity
			if not param.WhoNoEntity then param.WhoNoEntity = "" end
		
			param.Break = 0
			if component.BreakAtEnd then
				param.Break = component.BreakAtEnd
			end

			componentIndex, component = next(entity.Components, componentIndex)

			if component then
				param.Time = component.Time
			else
				param.Time = 3
			end

			rtAction = Translator.createAction("chat_step", rtNpcGrp.Id, param)
			table.insert(rtDialogOk.Children, rtAction)

			table.insert(endParam.Indexs, param.Index)
			table.insert(endParam.Grps, param.WhoGrp)
			table.insert(endParam.Whos, param.Who)
			table.insert(endParam.Emotes, param.Emote)
			table.insert(endParam.Says, param.Says)
			table.insert(endParam.WhoNoEntitys, param.WhoNoEntity)			
		

		else -- !if isKindOf("ChatStep")
			componentIndex, component = next(entity.Components, componentIndex)
		end
	end

	rtAction = Translator.createAction("chat_step_last", rtNpcGrp.Id, chatStepIndex+1)
	table.insert(rtDialogOk.Children, rtAction)



	local rtDialogStart = Translator.createAction("dynamic_if", "start == 1" , rtDialogOk )



	table.insert(rtEventStart.ActionsId, rtDialogStart.Id)
	table.insert(context.RtAct.Events, rtEventStart)
	table.insert(context.RtAct.Actions, rtDialogStart)


	local baseAct = context.Scenario:getBaseAct()
	local rtNpcGrpBase = r2.Translator.getRtGroup(context, baseAct.InstanceId)


	local rtActionEnd = Translator.createAction("chat_step_end", rtNpcGrp.Id, endParam, rtNpcGrpBase.Id)
	if (rtActionEnd) then
		local rtEvenEnd = Translator.createEvent("timer_t0_triggered", "", rtNpcGrp.Id) -- Endo Of Chat Step
		table.insert(rtEvenEnd.ActionsId, rtActionEnd.Id)
		table.insert(context.RtAct.Events, rtEvenEnd)
		table.insert(context.RtAct.Actions, rtActionEnd)
	end



	do
		local rtInitialState= Translator.createAction("dialog_init",  rtNpcGrp.Id , entity.Repeating, entity.AutoStart )
		Translator.translateAiGroupInitialState(entity, context, rtInitialState)
	end

	if (entity.AutoStart == 1) then
		local rtInitialState = r2.Translator.createAction("dynamic_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				Translator.createAction("dialog_starts",  rtNpcGrp.Id )
			)
		Translator.translateAiGroupInitialState(entity, context, rtInitialState)
	end


	Translator.translateAiGroup(entity, context)
end


Translator.getNpcGroupLogicGroupAMemberDeath = Translator.getNpcLogicEventDeath

-- user Event 0
-- start = 1


--
-- TODO: démêler les events (tmp: on utilise les events 7 et 8 pour respectivement activate et deactivate)
--
function Translator.getDialogLogicAction(entity, context, action)
	assert( action.Class == "ActionStep")

	local firstAction, lastAction = nil, nil

	assert( action.Class == "ActionStep")
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	if action.Action.Type == 'activate' then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 1)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 7)
		local action3 = r2.Translator.createAction("dynamic_if", r2:getNamespace()..rtNpcGrp.Id..".AutoStart == 1", 
				Translator.createAction("dialog_starts", rtNpcGrp.Id)
			)
		local actionActivate = r2.Translator.createAction("multi_actions", {action1, action2, action3})

		local retAction = r2.Translator.createAction("dynamic_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 0", actionActivate)
		return retAction, retAction
	elseif action.Action.Type == 'deactivate' then
		--local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 0)
		local action1 = r2.Translator.createAction("dialog_deactivate", rtNpcGrp.Id)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 8)
		--local action3 = r2.Translator.createAction("dialog_stops", rtNpcGrp.Id)
		local retAction = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1, 
			r2.Translator.createAction("multi_actions", {action1, action2}))
		return retAction, retAction
	elseif action.Action.Type == 'starts dialog' then
		local retAction = r2.Translator.createAction("dynamic_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				Translator.createAction("dialog_starts", rtNpcGrp.Id)
			)
		return retAction, retAction
	elseif action.Action.Type == 'continues dialog' then
		local retAction = r2.Translator.createAction("dynamic_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				Translator.createAction("dialog_continues", rtNpcGrp.Id)
			)
		return retAction, retAction
	elseif action.Action.Type == 'starts chat' then		
		local index = Translator.getChatPositionFromDialog(component, action.Action.Value)
		if index == -1 then return nil end
		local retAction = r2.Translator.createAction("dynamic_if", r2:getNamespace()..rtNpcGrp.Id..".Active == 1", 
				Translator.createAction("chat_starts", rtNpcGrp.Id, index)
			)
		return retAction, retAction
	elseif action.Action.Type == 'stops dialog' then
		--add condition if Active == 1 ?
		local retAction = Translator.createAction("dialog_stops", rtNpcGrp.Id)
		return retAction, retAction

	end

	printWarning('Action not implemented yet :'.. action.Action.Type)
	assert(nil)
	return nil, nil
end




function Translator.getChatPositionFromDialog(dialog, chatInstanceId)
	assert(dialog)
	assert(dialog.Class == 'ChatSequence' or dialog.Class == "ProximityDialog")
	assert(chatInstanceId ~= nil and chatInstanceId ~= "")
	local index = -1
	local componentIndex, component = next(dialog.Components)
	while componentIndex do
		if component.Class == 'ChatStep' then
			index = index + 1	
			if tostring(component.InstanceId) == tostring(chatInstanceId) then
				return index
			end
		end
		componentIndex, component = next(dialog.Components, componentIndex)
	end
	return -1

end

--
-- TODO: démêler les events (tmp: on utilise les events 7 et 8 pour respectivement activate et deactivate)
--
function Translator.getDialogLogicEvent(entity, context, event)
	assert(entity)
	assert( event.Class == "LogicEntityAction")


	local component =  entity -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local states = Translator.getRtStatesNames(context, entity.InstanceId)



	local eventType = tostring(event.Event.Type)
	
	if eventType == 'activation' then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 7)
	
	elseif eventType == 'deactivation' then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 8)
	
	elseif eventType == 'end of chat' then
		local action =  Translator.createEvent("timer_t2_triggered", states,  rtNpcGrp.Id)
		local index = Translator.getChatPositionFromDialog(component, event.Event.Value)
		if index ==  -1  then return nil end
		local firstCondition = Translator.createAction("if_value_equal", rtNpcGrp.Id, "v1", index + 2, firstCondition)
		return action, firstCondition, firstCondition

	elseif eventType == 'end of dialog' then
		return Translator.getComponentUserEvent(rtNpcGrp, 2)
	
	elseif eventType == 'start of chat' then
		local action =  Translator.createEvent("user_event_4", states,  rtNpcGrp.Id)
		local index = Translator.getChatPositionFromDialog(component, event.Event.Value)
		if index == -1  then return nil end
		local firstCondition = Translator.createAction("if_value_equal", rtNpcGrp.Id, "v1", index+2, firstCondition)
		return action, firstCondition, firstCondition
	
	elseif eventType == 'start of dialog' then
		return Translator.getComponentUserEvent(rtNpcGrp, 1)
	end
	
	printWarning('Event not implemented yet :'.. event.Event.Type)
	assert(nil)
end


Translator.getDialogLogicCondition = function(entity, context, condition )
	assert( condition.Class == "ConditionStep")
	local rtNpcGrp = Translator.getRtGroup(context, condition.Entity)
	assert(rtNpcGrp)
	local funs ={}

	if condition.Condition.Type == "is in dialog" then
		local firstCondition = Translator.createAction("if_value_equal", rtNpcGrp.Id, "start", 1)
		return firstCondition, firstCondition
	end

	if condition.Condition.Type == "is not in dialog" then
		local firstCondition = Translator.createAction("if_value_equal", rtNpcGrp.Id, "start", 0)
		return firstCondition, firstCondition
	end


	if condition.Condition.Type == "is in chat" then
		local index = Translator.getChatPositionFromDialog(entity, condition.Condition.Value)
		assert(index ~= -1)
		local lastCondition = Translator.createAction("if_value_equal", rtNpcGrp.Id, "v1", index+2)
		local firstCondition = Translator.createAction("if_value_equal", rtNpcGrp.Id, "start", 1, lastCondition)
		return firstCondition, lastCondition
	end

	printWarning('Condition not implemented yet :'.. condition.Condition.Type)
	return nil, nil
end


function Translator.translateDefaultFeature(entity, context, translateActivity)
	local components = entity.Components
	--luaObject(context.Feature)
	local key,comp = next(components,nil)
	while(key ~= nil)
	do
		-- Npc case (npc alone not object)
		if (comp.isKindOf and comp:isKindOf("Npc")) then
			local hlNpc = comp
			context.Feature = hlNpc

			-- create and set rtNpc
			
			local rtNpc = r2.Translator.translateNpc(hlNpc, context)	
			table.insert(context.RtAct.Npcs, rtNpc)

			-- create or get rtGrp
			-- set rtGrp.GroupParameter  by reading hlNpc (Aggro, Player attackable..)
			local rtNpcGrp = r2.Translator.getRtGroup(context,hlNpc.InstanceId)
			r2.Translator.setGroupParameters (hlNpc, rtNpcGrp)
			table.insert(rtNpcGrp.Children, rtNpc.Id)
				
			-- set activity
			-- when translating a usercomponentholder (container which has aiActivities), we must translate the AiActivities
			if translateActivity and translateActivity == true
			then
				local aiActivity = r2.Translator.getAiActivity(hlNpc)
				r2.Translator.translateActivities(context, hlNpc,  hlNpc:getBehavior(), rtNpcGrp, aiActivity)
			end

			-- set eventHandlers
			r2.Translator.translateEventHandlers(context, hlNpc, hlNpc:getBehavior().Actions, rtNpcGrp)
		end
		key,comp = next(components,key)
	end
end

function Translator.pretranslateDefaultFeature(this, context)
	local components = this.Components

	local key, comp = next(components, nil)
	while (key ~= nil) do
		if (comp.isKindOf and comp:isKindOf("Npc")) then
			local rtNpcGrp = r2.Translator.getRtGroup(context,comp.InstanceId)		
		end
		key, comp = next(components, key)										
	end
end

function Translator.pretranslateDefaultFeature2(this, context)
	local components = this.Components

	local key, comp = next(components, nil)
	while (key ~= nil) do
		if (comp.isKindOf and comp:isKindOf("Npc")) then
			local rtNpcGrp = r2.Translator.getRtGroup(context,comp.InstanceId)
			-- set activity
			local hlNpc = comp
			context.Feature = hlNpc
			local aiActivity = r2.Translator.getAiActivity(hlNpc)
			r2.Translator.translateActivities(context, hlNpc,  hlNpc:getBehavior(), rtNpcGrp, aiActivity)

		end
		key, comp = next(components, key)										
	end
end




function Translator.getDefaultFeatureActivitiesIds(this)

	local activitiesIds = {}

	local function getActivitiesIdsFrom(entity)
		local components = entity.Components
		local key,comp = next(components,nil)
		while(key ~= nil)
		do
			-- Npc case (npc alone not object)
			if (comp.isKindOf and comp:isKindOf("Npc")) then
				local behavior = comp:getBehavior()
				local k, v = next(behavior.Activities, nil)
				while k do
					table.insert(activitiesIds, v.InstanceId)
					k, v = next(behavior.Activities, k)
				end
			else
				if comp.Components then
					getActivitiesIdsFrom(comp)
				end
			end
 			key,comp = next(components, key)
		end
	end

	getActivitiesIdsFrom(this)

	return activitiesIds

end


--function Translator.getTopParentTreeNode(instance)
--	local tmpInstance = instance
--	if tmpInstance.ParentInstance.Class ~= "LogicEntity" and tmpInstance.ParentInstance.Class ~= "DefaultFeature"
--		and tmpInstance.ParentInstance.Class ~= "Act" then
--		return tmpInstance:getFeatureParentTreeNode()--tmpInstance:getParentTreeNode()
--	else
--		return tmpInstance:getFeatureParentTreeNode()
--	end
--end

function Translator.getDebugBase(base)
	if dataDevMode then
		return "palette.entities.botobjects.milestone"
	else
		return base
	end
end

function Translator.getDebugCreature(creature)
	if dataDevMode then
		return "object_milestone.creature"
	else
		return creature
	end
end


-- feature generic activate & deactivate 
function Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
	if (action.Action.Type == "activate") then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 1)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 4)
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		assert(retAction)
		return retAction, retAction
	elseif (action.Action.Type == "deactivate") then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 0)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 5)
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		assert(retAction)
		return retAction, retAction
	end
	return nil, nil
end

function Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)

	local eventType = event.Event.Type
	if eventType == "activation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 4)
	elseif eventType == "deactivation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 5)
	end
	return nil, nil, nil
end

function Translator.getFeatureActivationCondition(condition, rtNpcGrp)
	if condition.Condition.Type == "is active" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 1);
		return action1, action1
	elseif condition.Condition.Type == "is inactive" then
		local action1 = r2.Translator.createAction("if_value_equal", rtNpcGrp.Id, "Active", 0);
		return action1, action1
	end
	return nil, nil
end


function Translator.translateFeatureActivation(instance, context)
	local rtNpcGrp = r2.Translator.getRtGroup(context, instance.InstanceId)
	assert(rtNpcGrp)
	if instance.Active and instance.Active == 1 then
		local action1 = r2.Translator.createAction("set_value",  rtNpcGrp.Id, "Active", 1)
		local action2 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 4)
		local retAction = r2.Translator.createAction("multi_actions", {action1, action2})
		r2.Translator.translateAiGroupEvent("start_of_state" , instance, context, retAction)
	else
		local retAction = r2.Translator.createAction("set_value",  rtNpcGrp.Id, "Active", 0)
		r2.Translator.translateAiGroupEvent("start_of_state" , instance, context, retAction)	
	end
end


function Translator.addActivationToTranslations(logicTranslations)
	if logicTranslations.ApplicableActions == nil then logicTranslations.ApplicableActions = {} end
	--logicTranslations.ApplicableActions.activate = {menu=i18n.get("uiR2EdActivate"):toUtf8(), text="activates"}
	--logicTranslations.ApplicableActions.deactivate = {menu=i18n.get("uiR2EdDesactivate"):toUtf8(), text="deactivates"}
	--logicTranslations.ApplicableActions.trigger = {menu=i18n.get("uiR2EdTrigger"):toUtf8(), text="triggers"}

	if logicTranslations.Events == nil then logicTranslations.Events = {} end
	--logicTranslations.Events.activation	= {menu=i18n.get("uiR2EdActivation"):toUtf8(), text=r2:lowerTranslate("uiR2EdActivation")}
	--logicTranslations.Events.deactivation = {menu=i18n.get("uiR2EdDesactivation"):toUtf8(), text=r2:lowerTranslate("uiR2EdDesactivation")}
	--logicTranslations.Events.trigger = {menu=i18n.get("uiR2EdTrigger"):toUtf8(), text=r2:lowerTranslate("uiR2EdTrigger")}

	if logicTranslations.Conditions == nil then logicTranslations.Conditions = {} end
	--logicTranslations.Conditions["is active"] = {menu=i18n.get("uiR2EdIsActive"):toUtf8(), text=r2:lowerTranslate("uiR2EdIsActive")}
	--logicTranslations.Conditions["is inactive"]	= {menu=i18n.get("uiR2EdIsInactive"):toUtf8(), text=r2:lowerTranslate("uiR2EdIsInactive")}
	return logicTranslations
end


function Translator.CreateUserComponent(featureName)
	if not featureName or featureName == "" then
		debugInfo("Translator: calling createUserComponent on nil or empty featureName")
		return
	end

	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of '"..featureName.."' at pos (%d, %d, %d)", x, y, z))
		local component = r2.Features[featureName].createUserComponent( x, y)
		component.Texts = nil
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end

	local function posCancel()
		debugInfo("Cancel choice '"..featureName.."' position")
	end

	r2:choosePos("object_component_user_event.creature", posOk, posCancel, "")	
end

function Translator.CheckPickedEntity(component, prop)
	local k, v = next(prop, nil)
	while k do
		local property = v
	
		if property.Type and property.Type == "RefId" then			
			local this = r2:getInstanceFromId(component.InstanceId)			

			local targetId = this[property.Name]

			if targetId ~= r2.RefId("") then
				debugInfo("targetId:" ..targetId)
				local filterFunc = "return " ..property.PickFunction .."('"..targetId.."')"
				debugInfo("filterfunc: " ..filterFunc)			
				local ok = loadstring(filterFunc)()
				if ok == false then
					debugInfo("name: " ..property.Name)
					r2.requestSetNode(this.InstanceId, property.Name, r2.RefId(""))	
				end

			end
		end
		k, v = next(prop, k)
	end
	
end



function Translator.checkActForPicking(componentId, entityId)
	local component = r2:getInstanceFromId(componentId)
	assert(component)
	--debugInfo("CheckActForPicking: EntityId =" ..entityId)
	--inspect(component)
	local entity = r2:getInstanceFromId(entityId)
	assert(entity)

	return entity:getParentAct().InstanceId == r2.Scenario.Acts[0].InstanceId 
	or entity:getParentAct().InstanceId == component:getParentAct().InstanceId
end


function Translator.getEventFromType(instance, context, eventType)
	if not context or not instance or not instance.getLogicEvent then return nil end
	local fakeEvent = {}
	fakeEvent.Class = "LogicEntityAction"
	fakeEvent.Event = {}
	fakeEvent.Event.Type = eventType
	return instance:getLogicEvent(context, fakeEvent)
end

----------------------------------------
--- TASK MODULE ------------------------
----------------------------------------

Translator.Tasks = {}

--Start of state (init) = event 7
function Translator.Tasks.startOfStateLogic(component, context, rtGrp)
	do
		local action = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 7)
		r2.Translator.translateAiGroupEvent("start_of_state" , component, context, action)
	end
	
	do
		local repeatable = component.Repeatable
		if not repeatable then repeatable = 0 end

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", component.Active)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) 
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v3", 0) -- Success
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,  } )
		r2.Translator.translateAiGroupEvent("user_event_7", component, context, rtAction)
	end
end

--Activation = event 4
function Translator.Tasks.activationLogic(component, context, rtGrp)
	do	
		local repeatable = component.Repeatable
		if not repeatable then repeatable = 0 end

		local rtAction1 = r2.Translator.createAction("set_value",  rtGrp.Id, "Active", 1)
		local rtAction2 = r2.Translator.createAction("set_value",  rtGrp.Id, "v1", repeatable)
		local rtAction3 = r2.Translator.createAction("set_value",  rtGrp.Id, "v2", 0) -- Success
		local rtAction = r2.Translator.createAction("multi_actions", { rtAction1, rtAction2, rtAction3,  } )
		r2.Translator.translateAiGroupEvent("user_event_4", component, context, rtAction)
	end
end

--Deactivation = event 5
function Translator.Tasks.deactivationLogic(component, context, rtGrp)
	do
		local rtAction = r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				})
		r2.Translator.translateAiGroupEvent("user_event_5", component, context, rtAction)
	end
end

--When using talkTo, giveItem or requestItem actions, event 3 is emitted when the player took the missin (contextual validation)
function Translator.Tasks.setStatusLogic(component, context, rtGrp)
	do
		local action = r2.Translator.createAction("set_value", rtGrp.Id, "v2", 1 )
		r2.Translator.translateAiGroupEvent("user_event_3", component, context, action)
	end
end

--Success = event 9
--No broadcast means the broadcast action is done elsewhere in the translate (not on success)
function Translator.Tasks.successNoBroadcastLogic(component, context, rtGrp)
	do	
		--if repeatable	
		local action1 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				})
			); 
		--if not repeatable
		local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0, 
			r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
				r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2 ),
				r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5),
				})
			); 
	
		local actions = r2.Translator.createAction("multi_actions", {action1, action2})	

		local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2, 
			r2.Translator.createAction("condition_if", r2:getNamespace()..rtGrp.Id..".Active == 1", actions))
		r2.Translator.translateAiGroupEvent("user_event_9", component, context, action)
	end
end

--Success with broadcast : for components using event 8 as an intermediate step.
function Translator.Tasks.successBroadcastLogic(component, context, rtGrp)
	local validationNeeded = component.ValidationNeeded

	local action2 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 1,  --if Repeatable
		r2.Translator.createAction("multi_actions", {
			r2.Translator.createAction("set_value", rtGrp.Id, "Active", 1 ),
			r2.Translator.createAction("set_value", rtGrp.Id, "v2", 0 ),
			r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
			})
		); 
	local action3 = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v1", 0,  -- if not Repeatable
		r2.Translator.createAction("multi_actions", {
			r2.Translator.createAction("set_value", rtGrp.Id, "Active", 0 ) ,
			r2.Translator.createAction("set_value", rtGrp.Id, "v2", 2 ),
			r2.Translator.createAction("set_value", rtGrp.Id, "v3", 1 ),
			r2.Translator.createAction("user_event_trigger", rtGrp.Id, 5)
			})
		); 
	
	local actions = {}
	if validationNeeded == 1 then
		actions = {action2, action3}
	else
		local baseAct = r2.Scenario:getBaseAct()
		local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
		local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, component:textAdapter(component.BroadcastText) )
	
		actions = {action2, action3, actionBroadcast}
	end
	local multiActions = r2.Translator.createAction("multi_actions", actions)
			
	local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2, 
		r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1, multiActions))
	r2.Translator.translateAiGroupEvent("user_event_9", component, context, action)
end

--When validation is needed, emit success event when targeting mission giver
function Translator.Tasks.validationByMissionGiver(component, giver, context, rtGrp)
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
	assert(rtGiverGrp)
	do	
		local actionEvent = r2.Translator.createAction("user_event_trigger", rtGrp.Id, 9)
		
		local action1 = r2.Translator.createAction("npc_say", component:textAdapter(component.MissionSucceedText),   rtGiverGrp.Id ..":"..giver.Name)

		local action = r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 2, 
			r2.Translator.createAction("condition_if", r2:getNamespace()..rtGrp.Id..".Active == 1", 
			r2.Translator.createAction("multi_actions", {actionEvent, action1})))
		
		r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, action)
	end
end

-- when receiving event 8, just broadcast a msg indicating that the mission is successful but that the player has 
-- to go back to the giver to complete it.
function Translator.Tasks.broadcastOnEvent8(component, context)
	local baseAct = r2.Scenario:getBaseAct()
	local baseActRtGrp = r2.Translator.getRtGroup(context, baseAct.InstanceId)
	local actionBroadcast = r2.Translator.createAction("broadcast_msg",baseActRtGrp.Id, component:textAdapter(component.BroadcastText) )
	
	r2.Translator.translateAiGroupEvent("user_event_8", component, context, actionBroadcast)
end

function Translator.Tasks.giverLogic(component, giver, context, rtGrp)
	local rtGiverGrp = r2.Translator.getRtGroup(context, giver.InstanceId)
	assert(rtGiverGrp)

	local actionWaitValidation = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
			 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 1,  -- giver has been spoken to
				r2.Translator.createAction("multi_actions", {
				r2.Translator.createAction("npc_say", component:textAdapter(component.WaitValidationText),   rtGiverGrp.Id ..":"..giver.Name),
				r2.Translator.createAction("user_event_trigger", rtGrp.Id, 2)
		})))
		
	--say mission text + contextual text (talk to)
	local multiActions = r2.Translator.createAction("multi_actions", {
			r2.Translator.createAction("npc_say", component:textAdapter(component.MissionText),   rtGiverGrp.Id ..":"..giver.Name), 
			r2.Translator.createAction("talk_to", rtGrp.Id,  component:textAdapter(component.ContextualText)),
			})
	
	local actionTalkTo = r2.Translator.createAction("if_value_equal", rtGrp.Id, "Active", 1,  -- Active
		 r2.Translator.createAction("if_value_equal", rtGrp.Id, "v2", 0, multiActions))

	local rtAction = r2.Translator.createAction("multi_actions", {actionWaitValidation, actionTalkTo})

	r2.Translator.translateAiGroupEvent("player_target_npc", giver, context, rtAction)
end






























