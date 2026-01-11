
-- for client : register a component into C using the 'registerGenerator' function and add 
-- it into the list of classes


-- copy class properties & methods
local reservedMembers = { Prop = true, 
                          BaseClass = true,						  
						  NameToProp = true, 
						  -- native properties
						  Parent = true, 
						  ParentInstance= true, 
						  IndexInParent = true,
						  User = true, 
						  Size = true, 
						  DisplayerUI = true, 
						  DisplayerVisual = true,
						  DisplayerProperties = true, 
						  Selectable = true, 
						  SelectableFromRoot = true
						}

-- check is a string is a valid identifier (e.g begin with an alphabetic letter followed by alpha numeric letters)
local function isValidIdentifier(id)
	return string.match(id, "[%a_][%w_]*")	
end

-- register a new class (features, components ...)
r2.registerComponent = function(generator, package)
	-- check that all identifiers are correct
	local badProps = {}
	if generator.Prop == nil then
		generator.Prop = {}
	end
	-- serach invalid identifiers
	for k, prop in pairs(generator.Prop) do

		if (type(prop) ~= "table") then 
			debugWarning("Invalid property found in ".. generator.Name)
		end
		
		if not isValidIdentifier( prop.Name ) then		
			debugWarning("Invalid property name found : '" .. tostring(prop.Name) ..
			          "' in class " .. tostring(generator.Name) .. ". Property is ignored")
			table.insert(badProps, k)
		end
		if reservedMembers[prop.Name] then
			debugWarning("Invalid property name found : '" .. tostring(prop.Name) ..
			          "' in class " .. tostring(generator.Name) .. ". This is the name of a native poperty, and can't be redefined")
			table.insert(badProps, k)
		end
	end
	-- check that no identifier is duplicated	
	local propName = {}
	for k, prop in pairs(generator.Prop) do
		if propName[prop.Name] ~= nil then
			debugWarning("Duplicated property found when registering class " .. tostring(generator.Name) .. ", property = " .. tostring(prop.Name))
			debugWarning("Aborting class registration.")
			return
		end
		propName[prop.Name] = true			
	end
	-- remove bad properties from the class
	for k, prop in pairs(badProps) do
		generator.Prop[prop] = nil
	end
	
	local name = nil

	if package then
		name = generator.Name
		--name = package:getUniqId() .. "|" .. generator.Name TODO
	else
		name = generator.Name;
	end
	
	if r2.Classes[name] ~= nil then
		debugWarning("Component registered twice : " .. generator.Name)
		return
	end
	if type(name) ~= "string" then
		debugWarning("Can't register class, 'Name' field not found or bad type")
		assert(false)
	end
	r2.Classes[name] = generator
end

-- private : perform subclassing by copying properties / methods of the base class into the class passed as a parameter
-- only methods / props that are present in the base class and not in the derived class are copied
-- subclass is done recursively so after calling 'subclass' on a class definition it will be complete
function r2.Subclass(classDef)
	assert(classDef)
	assert(type(classDef) == "table")
	if classDef.BaseClass ~= "" and classDef.BaseClass ~= nil then		
		local baseClass = r2.Classes[classDef.BaseClass]
		if baseClass == nil then
			debugInfo("Cant found base class " .. strify(classDef.BaseClass) .. " of class " .. strify(classDef.Name))
			return
		end
		--debugInfo("sub classing " .. tostring(classDef.Name) .. " from " .. tostring(baseClass.Name))
		-- make sure that base class is complete, too
		r2.Subclass(baseClass)		
		for key, value in pairs(baseClass) do
			if classDef[key] == nil then					
				-- if property or method not defined in derived class then copy from the base class
				-- if this is a method, then just replace with a function that will delegate
				-- the call to the parent class
				--if type(value) == "function" and key ~= "delegate" then
				--	assert(type(key) == "string")
				--	local localKey = key -- apparently, closure are not supported with locals from the enclosing 'for' loop
				--	local function delegator(this, ...)							
						--debugInfo("Calling parent version in parent class '" .. tostring(baseClass.Name) .. "' for " .. tostring(localKey))							
						-- TODO nico : here if would be much faster to call the 
						-- first parent class where function is defined instead
						-- of chaining the calls to 'delegate'
						-- There are 1 thing to rememeber however :
						-- * The this pointer could be a delegated one, 
						--   so when doing the call directly this should be with the non-delgated this (to have a polymorphic call)
						
				--		local delegated = this:delegate()									
				--		return delegated[localKey](delegated, unpack(arg))
				--	end
				--	classDef[key] = delegator
				--else						
					classDef[key] = value
				--end
			end			
		end
		-- copy instances properties
		assert(classDef.NameToProp)
		assert(baseClass.NameToProp)
		for key, prop in pairs(baseClass.Prop) do
			-- if property not declared in derived class then add it
			if classDef.NameToProp[prop.Name] == nil then
				-- its okay to make a reference here because classes definitions are read-only
				classDef.NameToProp[prop.Name] = prop 
				table.insert(classDef.Prop, prop)
			end
		end
	end
	-- else no-op
end

function r2.getLoadedFeaturesStatic()
	local loadedFeatures = 
	{	
		--Mob Spawners
		{"r2_features_boss_spawner.lua",	"BossSpawnerFeature",	"uiR2EdMobSpawnersCategory"},
		{"r2_features_timed_spawn.lua",		"TimedSpawner",			"uiR2EdMobSpawnersCategory"},
		{"r2_features_scenery_object_remover.lua", "SceneryObjectRemoverFeature",	"uiR2EdMobSpawnersCategory"},
		
		--Chests
		{"r2_features_easter_egg.lua",		"EasterEggFeature",		"uiR2EdChestsCategory"},
		{"r2_features_random_chest.lua",	"RandomChest",			"uiR2EdChestsCategory"},
		{"r2_features_get_item_from_scenery.lua", "GetItemFromSceneryObject",	"uiR2EdChestsCategory"},


		--Tasks
		{"r2_features_give_item.lua",		"GiveItemFeature",		"uiR2EdTaskStepCategory"},
		{"r2_features_talk_to.lua",			"TalkToFeature",		"uiR2EdTaskStepCategory"},
		{"r2_features_request_item.lua",	"RequestItemFeature",	"uiR2EdTaskStepCategory"},
		{"r2_features_visit_zone.lua",		"VisitZone",			"uiR2EdTasksCategory"},
		{"r2_features_target_mob.lua",		"TargetMob",			"uiR2EdTasksCategory"},
		{"r2_features_kill_npc.lua",		"KillNpc",				"uiR2EdTasksCategory"},
		{"r2_features_hunt_task.lua",		"HuntTask",				"uiR2EdTasksCategory"},
		{"r2_features_delivery_task.lua",	"DeliveryTask",			"uiR2EdTasksCategory"},
		{"r2_features_get_item_from_scenery_task.lua", "GetItemFromSceneryObjectTaskStep",	"uiR2EdTaskStepCategory"},
		{"r2_features_scenery_object_interaction_task.lua",	"SceneryObjectInteractionTaskStep",	"uiR2EdTaskStepCategory"},


		--Triggers
		{"r2_features_timer.lua",			"TimerFeature",			"uiR2EdTriggersCategory"},
		{"r2_features_zone_triggers.lua",	"ZoneTrigger",			"uiR2EdTriggersCategory"},
		{"r2_features_user_trigger.lua",	"UserTriggerFeature",	"uiR2EdTriggersCategory"},
		{"r2_features_man_hunt.lua",		"ManHuntFeature",		"uiR2EdTriggersCategory"},
		{"r2_features_scenery_object_interaction.lua", "SceneryObjectInteractionFeature",	"uiR2EdTriggersCategory"},
		{"r2_features_proximity_dialog.lua", "ChatSequence",		"uiR2EdTriggersCategory"},
		--{"r2_features_reward_provider.lua", "RewardProvider",	"uiR2EdTriggersCategory"},

		--MacroComponents
		{"r2_features_ambush.lua",			"Ambush",				"uiR2EdMacroComponentsCategory"},
		{"r2_features_loot_spawner.lua",	"LootSpawnerFeature",	"uiR2EdMacroComponentsCategory"},
		{"r2_features_hidden_chest.lua",	"HiddenChest",			"uiR2EdMacroComponentsCategory"},
		{"r2_features_proximity_dialog.lua", "ProximityDialog",		"uiR2EdMacroComponentsCategory"},
		{"r2_features_bandits_camp.lua",	"BanditCampFeature",	"uiR2EdMacroComponentsCategory"},
		{"r2_features_fauna.lua",			"FaunaFeature",			"uiR2EdMacroComponentsCategory"},		
		
	}
	return loadedFeatures
end

function r2.doFileProtected(filename)
	local ok, msg = pcall(r2.doFile, filename)
	if not ok then
		debugInfo("Error while loading component '"..filename.."' err: "..msg)
	end
end


r2.loadFeatures = function()
	
	r2.doFileProtected("r2_features_default.lua")
	
	r2.doFileProtected("r2_features_npc_groups.lua")
	r2.doFileProtected("r2_features_counter.lua")
	r2.doFileProtected("r2_features_reward_provider.lua")
	
	--Loading features
	r2.doFileProtected("r2_features_loaded.lua")

	local loadedFeatures = r2.getLoadedFeaturesStatic()
	local k, v = next(loadedFeatures, nil)
	while k do
		if v and v[1] then
			r2.doFileProtected(v[1])
		end
		k, v = next(loadedFeatures, k)
	end
	
	if config.R2EDLoadDynamicFeatures == 1 then
		local loadedFeatures = r2.getLoadedFeaturesDynamic()
		local k, v = next(loadedFeatures, nil)
		while k do
			if v and v[1] then
				r2.doFileProtected(v[1])
			end
			k, v = next(loadedFeatures, k)
		end
	end

	r2.doFileProtected("r2_texts.lua")
	r2.doFileProtected("r2_logic.lua")
	r2.doFileProtected("r2_logic_entities.lua")
	r2.doFileProtected("r2_event_handler_system.lua")
	r2.doFileProtected("r2_unit_test.lua")
	
	r2.doFileProtected("r2_core_user_component_manager.lua")
	--r2_core.UserComponentManager:init()
		
	--debugInfo("REGISTERING FEATURES")
	r2.UserComponentsManager:updateUserComponents()

	local featureId, feature = next(r2.Features, nil)
	while (featureId ~= nil)
	do 
		--debugInfo("Registering feature " .. feature.Name)
		local componentId, component = next(feature.Components, nil)
		while (component ~= nil)
		do
			--debugInfo("    Registering feature component " .. component.Name)
			r2.registerComponent(component)
			componentId, component = next(feature.Components, componentId)
		end
		
		featureId, feature = next(r2.Features, featureId);
	end
end

-- Function to init default scenario stuffs, with the given client ID
-- tmp : returns ids for the scenario, the first act, and the default group
r2.initBaseScenario = function() 

	local function ici(index) 
		-- debugInfo(colorTag(255, 255, 0) .. "ICI " .. tostring(index))
	end
   -- create scenario   
   ici(1)
	local scenario= r2.newComponent("Scenario")
	if (scenario == nil) then
		debugWarning("Failed to create Scenario");
		return
	end
   ici(2)
	--debugInfo("Scenario created with id " .. scenario.InstanceId)	
	scenario.title = "TestMap"
	scenario.shortDescription = "TestMap"
	scenario.optimalNumberOfPlayer = 1	
	-- create first act & default feature group
	
	do 
		local act =r2.newComponent("Act")
		act.States = {}
		if (act == nil) then
			debugWarning("Failed to create first 'Act'");
			return
		end	
	
		local features = act.Features
		local tmpDefault = r2.newComponent("DefaultFeature")	
		if (tmpDefault == nil) then
			debugWarning("Failed to create default feature");
			return
		end
		table.insert(features, tmpDefault)
		table.insert(scenario.Acts, act)

	end

	-- By default create act I and have it selected

	do 
		local act =r2.newComponent("Act")
		-- force to select the act 1 at display
		r2.ActUIDisplayer.LastSelfCreatedActInstanceId = act.InstanceId
		act.States = {}
		if (act == nil) then
			debugWarning("Failed to create secondary 'Act'");
			return
		end	
		

		act.Name = i18n.get("uiR2EDAct1"):toUtf8()	
		act.Title = i18n.get("uiR2EDAct1"):toUtf8() -- obsolete	
	
		local features = act.Features
		local tmpDefault = r2.newComponent("DefaultFeature")	
		if (tmpDefault == nil) then
			debugWarning("Failed to create default feature");
			return
		end
		table.insert(features, tmpDefault)
		table.insert(scenario.Acts, act)
	end
	r2.requestCreateScenario(scenario)	
	
end

-- called by the frame work to reset the current scenario
-- function r2.resetScenario()
--	
--	do 
--
--		r2.requestEraseNode(r2.ScenarioInstanceId, "Acts" )
--				
--
--		local acts= {}
--
--		do
--			local act =r2.newComponent("Act")
--			local features = act.Features
--			local tmpDefault = r2.newComponent("DefaultFeature")
--			table.insert(features, tmpDefault)
--			table.insert(acts, act)
--		end
--		do
--			local act =r2.newComponent("Act")
--			local features = act.Features
--			local tmpDefault = r2.newComponent("DefaultFeature")
--			r2.ActUIDisplayer.LastSelfCreatedActInstanceId = act.InstanceId
--			act.Title = i18n.get("uiR2EDAct1"):toUtf8()		
--			table.insert(features, tmpDefault)
--			table.insert(acts, act)
--			-- table.insert(scenario.Acts, act)
--		end
--
--				
--		r2.requestInsertNode(r2.ScenarioInstanceId, "", -1, "Acts", acts)
--		r2.requestReconnection()
--	end	
--end


-- called when a gm/ai has do a scheduleStartAct (Animation or test time)
function r2.onScheduleStartAct(errorId, actId, nbSeconds)
	if (r2.Mode == "DM" or r2.Mode == "AnimationModeDm") then
		if errorId == 0 then
			local ucStringMsg = ucstring()	
					 
			local str = "Act " .. actId 
			if nbSeconds ~= 0 then
				str = str .. " will start in " .. nbSeconds .. " seconds"
			end
			ucStringMsg:fromUtf8(str)		
			displaySystemInfo(ucStringMsg, "BC")
		elseif errorId == 1 then
			messageBox("Act ".. actId .." can not be started because another act is already starting.")
		elseif errorId == 2 then
			messageBox("Act ".. actId .." can not be started because this act does not exist.")		
		end
	end
end

function r2.onDisconnected()
	local str = "You have been disconnected by the server."
	local ucStringMsg = ucstring()	
	messageBox(str)		
	ucStringMsg:fromUtf8(str)				
	displaySystemInfo(ucStringMsg, "BC")
end

function r2.onKicked(timeBeforeDisconnection, kicked)
	if kicked then
		local str = "You have been kicked. You must come back to mainland or leave this session otherwise you will be disconnected in "
			.. tostring(timeBeforeDisconnection) .. " secondes."
		local ucStringMsg = ucstring()	
		messageBox(str)		
		ucStringMsg:fromUtf8(str)				
		displaySystemInfo(ucStringMsg, "BC")
	else
		local str = "You have been unkicked."		
		local ucStringMsg = ucstring()	
		messageBox(str)		
		ucStringMsg:fromUtf8(str)				
		displaySystemInfo(ucStringMsg, "BC")
	end

end

-- called in start mode of a dm
function r2.onRuntimeActUpdated(runtimeAct)
	-- use runtimeAct or r2.getRunTimeActs()
	r2.AnimGlobals.Acts = runtimeAct
	-- update the ui
	r2.ui.AnimBar:update()
end

function r2.onTalkingAsListUpdated()
	r2.ui.AnimBar:updateDMControlledEntitiesWindow()
end



function r2.onIncarnatingListUpdated()
	r2.ui.AnimBar:updateDMControlledEntitiesWindow()	
end


function r2.onScenarioHeaderUpdated(scenario)
	local ui=getUI('ui:interface:r2ed_scenario_control')
	if ui.active == true then
		ui.active = false 
		ui.active = true
	end

	-- inspect(scenario)
	-- or use r2.getScenarioHeader();
end

function r2.onSystemMessageReceived(msgType, msgWho, msg)

	local ucStringMsg = ucstring()
	ucStringMsg:fromUtf8(msg)
	if string.len(msg) > 2 and string.sub(msg, 1, 2) == "ui" then
		ucStringMsg = i18n.get(msg)	
		msg = ucStringMsg:toString()
	end
	if msgType == "BC" or msgType == "BC_ML"  then
		printMsgML(msg)
	elseif  msgType == "SYS" or msgType == "DM" then

		local str = ""
		if msgType == "DM" then
			str = "(AM ONLY)"..str
			if (r2.Mode ~= "DM" and r2.Mode ~= "AnimationModeDm") then return end

		end
		if string.len(msgWho) ~= 0 then 
			str = str .. msgWho .. ": "
		end

		str = str.. msg
		printMsgML(msg)
	elseif msgType == "ERR" then
		printMsgML(msg)
		messageBox(msg)
	end
end




-- TMP : place holder function to know the current act
if not r2.getCurrentActIndex then
	debugInfo("Creating place holder for r2.getCurrentActIndex")
	function r2.getCurrentActIndex()
		return 1
	end
end

function r2.onUserTriggerDescriptionUpdated(userTrigger)
	-- use userTrigger or r2.getUserTriggers()
	r2.AnimGlobals.UserTriggers = userTrigger
	r2.ui.AnimBar:update()
end

function r2.onCurrentActIndexUpdated( actIndex)
	-- actIndex==r2.getCurrentActIndex())
end

-- called when a session has begin but no scenario has been created
function r2.onEmptyScenarioUpdated()
	if r2.Mode == "AnimationModeLoading" then
		UnitTest.testLoadAnimationScenarioUi()
		
	elseif r2.Mode == "AnimationModeWaitingForLoading" then
		UnitTest.testWaitAnimationScenarioLoadingUi()
	else
		--UnitTest.testCreateScenarioUi()
		r2.acts:openScenarioActEditor(true, true)
	end
end

-- called by the framework when the scenario has been updated
function r2.onScenarioUpdated(scenario, startingActIndex)

	--luaObject(scenario)
	--breakPoint()
	
	if (scenario == nil) then
		r2.onEmptyScenarioUpdated()
		return
	else
		hide('ui:interface:r2ed_form_CreateNewAdventureStep2')
	end
	
	r2.Scenario = r2:getInstanceFromId(scenario.InstanceId)
	r2.ScenarioInstanceId = scenario.InstanceId

	-- add permanent nodes to act node
	r2:defaultUIDisplayer():addPermanentNodes()
	
	if r2.Version.updateVersion() then
		r2.setScenarioUpToDate(true)
	else
		r2.setScenarioUpToDate(false)
	end

	local currentAct = nil

	assert(startingActIndex);
	assert( type(startingActIndex) == "number");

	if  startingActIndex < table.getn(scenario.Acts) then
		r2.DefaultActInstanceId = scenario.Acts[startingActIndex].InstanceId
		r2.ActUIDisplayer.LastSelfCreatedActInstanceId = scenario.Acts[startingActIndex].InstanceId
		if scenario.Acts[startingActIndex].Features.Size > 0 then
			r2.DefaultFeatureInstanceId = scenario.Acts[startingActIndex].Features[0].InstanceId
		end
		currentAct=scenario.Acts[startingActIndex]

		r2.ScenarioWindow:setAct(currentAct)
	else
		r2.DefaultActInstanceId = scenario.Acts[0].InstanceId
		r2.ActUIDisplayer.LastSelfCreatedActInstanceId = scenario.Acts[0].InstanceId
		if scenario.Acts[0].Features.Size > 0 then
			r2.DefaultFeatureInstanceId = scenario.Acts[0].Features[0].InstanceId
		end
		currentAct=scenario.Acts[0]	
	end

	
	if scenario ~= nil and currentAct ~= nil then
		r2.Scenario.User.SelectedActInstanceId  = tostring(currentAct.InstanceId) 
		r2.Scenario.User.SelectedLocationInstanceId = tostring(currentAct.LocationId)
	end

	r2.ScenarioWindow:updateScenarioProperties()
	-- usefull to know if the scenario is updating
	ld.lock = 0

	if not r2.RingAccess.LoadAnimation and not r2.getIsAnimationSession()  then
		local ok, level, err = r2.RingAccess.verifyScenario()
		r2.updateScenarioAck(ok, level, err.What)
		return
	end

	r2.acts.deleteOldScenario = false

	if r2.getUseVerboseRingAccess() then
		r2.RingAccess.dumpRingAccess()
	end

end

function r2.verifyScenario()
	local ok, level, err = r2.RingAccess.verifyScenario()
	local msg=""
	if not ok then
		printMsg(err.What)
		msg = err.What
	end	
	return ok, msg
end

function r2.printMsg(msg)
	r2.printMsg(msg)
end

-- assign default menu for each classes 
function r2.initDefaultMenuSetup()
	forEach(r2.Classes, 
        function(k, v) 
		  if v.Menu ~= nil and v.onSetupMenu == nil then 
			v.onSetupMenu = r2.defaultMenuSetup
		  end
		end
	   )	 
end

-- assign default menu for each classes 
function r2.initDefaultPropertyDisplayer()	
	for k, class in pairs(r2.Classes) do
		if class.BuildPropertySheet == true then
			if class.DisplayerProperties == nil then
				class.DisplayerProperties = "R2::CDisplayerLua"
				class.DisplayerPropertiesParams = "propertySheetDisplayer"
			end
		end
	end
end

-- setup the classes
function r2.setupClasses()
	-- first build a table that gives access to a property from its name
	for k, class in pairs(r2.Classes) do
		class.NameToProp = {}
		for k, prop in pairs(class.Prop) do
			if prop.Name == nil then
				debugInfo("Found a property in class " .. tostring(class.Name) .. " with no field 'Name'")
			end
			class.NameToProp[prop.Name] = prop
		end
	end
	-- perform subclassing
	for k, class in pairs(r2.Classes) do				
		r2.Subclass(class)
	end
	-- register into C
	for k, class in pairs(r2.Classes) do
		r2.registerGenerator(class)
	end

	
end

-- returns a table which map each instanceId of the scenario component's to each component
r2.createComponentsMap = function (scenario)
	function createComponentsMapImpl	(t, components)
		if ( type(t) == "table")
		then
			if (t.InstanceId ~= nil)
			then
				components[t.InstanceId] = t
			end
			for key, value in pairs(t) do
				createComponentsMapImpl(value, components)
			end
		end
	end
	local components = {}
	createComponentsMapImpl(scenario, components)
	return components
end




r2.updateActCost = function(act)
	assert(act)
	local cost = 0
	local staticCost = 0
	local features = act.Features
	assert(features ~= nil )
	local featureId, feature = next(features, nil)
	while (featureId ~= nil)
	do
		-- feature:getCost() is obsolete
		if feature.User.GhostDuplicate ~= true then
			if feature and feature.getAiCost then
				local added = feature:getAiCost() 			
				if added then
					cost = cost + added 
				end
			end
			if feature and feature.getStaticObjectCost then
				local added = feature:getStaticObjectCost()
				if added then
					staticCost = staticCost + added
				end
			end
		end
		featureId, feature = next(features, featureId)
	end
	
	-- NB nico : removed cost from the real object and put is in the 'User' table (interfere with undo redo, because considered
	-- as an action)

	act:setLocalCost(cost)
	--if (act.Cost  ~= cost) then
	--	r2.requestSetLocalNode(act.InstanceId, "Cost", cost)
	--	r2.requestCommitLocalNode(act.InstanceId, "Cost")		
	--end

	act:setLocalStaticCost(staticCost)
	--if (act.StaticCost  ~= staticCost) then
	--	r2.requestSetLocalNode(act.InstanceId, "StaticCost", staticCost)
	--	r2.requestCommitLocalNode(act.InstanceId, "StaticCost")		
	--end

end




r2.registerText = function(text)

	--TODO : when several texts are registered "at the same time", the local scenario
	--has not the time to receive the changes, and a new entry is created.

	local checkText = r2.Features["TextManager"].checkText
	local textMgr = getTextMgr()
	if(textMgr==nil)
	then
		debugInfo("text mgr nil!!")
	end
	local result = checkText(textMgr,text)
	if result.Count ~= 0
	then

		--the entry already exist, just increment the counter
		r2.requestSetNode(result.InstanceId,"Count",result.Count+1)
		--temporaire
		--result.Count = result.Count + 1
		--/temporaire	
		debugInfo("Entry already exist")
	else
		--the entry don't exist, insert it 
		result.Count=1
		-- debugInfo("New entry created")
		r2.requestInsertNode(r2.Scenario.Texts.InstanceId,"Texts",-1,"",result)

		--temporaire
		--table.insert(r2.TextMgr.Texts,result)
		--temporaire
	end
	return result
end

getTextMgr = function()
	--return r2.TextMgr
	return r2.Scenario.Texts
end

r2.unregisterText = function(text)
	
	local removeText = r2.Features["TextManager"].removeText
	removeText(r2.Scenario.Texts,text)
end

r2.unregisterTextFromId = function(id)

	local text = r2.getText(id)
	if text ~= nil
	then
		r2.unregisterText(text)
	end
end

r2.getText = function(id)
	local textMgr = getTextMgr()
	return r2.Features["TextManager"].getText(textMgr, id)
end

r2.split = function(str, sep)
		assert( type(str) == "string")
		local ret = {}
		local start=0
		if sep == nil then sep = "\n" end
		local fin=string.find(str, sep)

		while fin ~= nil do
			local tmp = string.sub(str,start,fin-1)
			if string.len(tmp)~=0
			then
				table.insert(ret,tmp)
			end
			start = fin+1
			fin = string.find(str,sep,start)
		end
		
		if start<string.len(str)
		then
			local tmp =string.sub(str,start)
			if string.len(tmp)~=0
			then
				table.insert(ret,tmp)
			end
		end
		return ret
	end

r2.dumpAI = function(rtAct, rtGrp)
	if 1 == 0
	then
		do
			local event = Actions.createEvent("timer_t0_triggered", "",  rtGrp.Id)
			
			local action = Actions.createAction("code","print(\"timer_t0_triggered\");\n"
					.. "print(\"--------"..  rtGrp.Id .. "---\");\n"	
					.. "print(\"oldActivitySequenceVar:\",oldActivitySequenceVar);\n"			
					.. "print(\"currentActivitySequenceVar:\",currentActivitySequenceVar);\n"		
					.. "print(\"oldActivityStepVar:\", oldActivityStepVar);\n"		
					.. "print(\"v2:\",v2);\n"		
					.. "print(\"oldChatStepVar:\", oldChatStepVar);\n"		
					.. "print(\"v1:\",v1);\n"		
					.. "print(\"oldChatSequenceVar:\", oldChatSequenceVar);\n"	
					.. "print(\"v0:\",v0);\n"	
					.. "print(\"----------------\");\n"	
					
					);

			table.insert(rtAct.Actions, action)
			table.insert(rtAct.Events, event)
			table.insert(event.ActionsId, action.Id)
		end

		do
			local event = Actions.createEvent("timer_t1_triggered", "",  rtGrp.Id)
			
			local action = Actions.createAction("code", "print(\"timer_t1_triggered\");\n"	
					.. "print(\"--------"..  rtGrp.Id .. "---\");\n"	
					.. "print(\"oldActivitySequenceVar:\",oldActivitySequenceVar);\n"
					.. "print(\"currentActivitySequenceVar:\",currentActivitySequenceVar);\n"
					.. "print(\"oldActivityStepVar:\",oldActivityStepVar);\n"		
					.. "print(\"v2:\",v2);\n"		
					.. "print(\"oldChatStepVar:\",oldChatStepVar);\n"
					.. "print(\"v1:\",v1);\n"		
					.. "print(\"oldChatSequenceVar:\",oldChatSequenceVar);\n"				
					.. "print(\"v0:\",v0);\n"
					.. "print(\"----------------\");\n"	
					);

			table.insert(rtAct.Actions, action)
			table.insert(rtAct.Events, event)
			table.insert(event.ActionsId, action.Id)
		end

		do
			local event = Actions.createEvent("variable_v0_changed", "",  rtGrp.Id)
			
			local action = Actions.createAction("code", "print(\"variable_v0_changed\");\n"
					.. "print(\"--------"..  rtGrp.Id .. "---\");\n"				
					.. "print(\"oldActivitySequenceVar:\",oldActivitySequenceVar);\n"			
					.. "print(\"currentActivitySequenceVar:\",currentActivitySequenceVar);\n"
					.. "print(\"oldActivityStepVar:\",oldActivityStepVar);\n"		
					.. "print(\"v2:\",v2);\n"		
					.. "print(\"oldChatStepVar:\",oldChatStepVar);\n"
					.. "print(\"v1:\",v1);\n"		
					.. "print(\"oldChatSequenceVar:\",oldChatSequenceVar);\n"				
					.. "print(\"v0:\",v0);\n"
					.. "print(\"----------------\");\n"	
					);

			table.insert(rtAct.Actions, action)
			table.insert(rtAct.Events, event)
			table.insert(event.ActionsId, action.Id)
		end

		do
			local event = Actions.createEvent("variable_v1_changed", "",  rtGrp.Id)
			
			local action = Actions.createAction("code", "print(\"variable_v1_changed\");\n"
					.. "print(\"--------"..  rtGrp.Id .. "---\");\n"	
					.. "print(\"oldActivitySequenceVar:\",oldActivitySequenceVar);\n"			
					.. "print(\"currentActivitySequenceVar:\",currentActivitySequenceVar);\n"			
					.. "print(\"oldActivityStepVar:\",oldActivityStepVar);\n"		
					.. "print(\"v2:\",v2);\n"		
					.. "print(\"oldChatStepVar:\",oldChatStepVar);\n"
					.. "print(\"v1:\",v1);\n"		
					.. "print(\"oldChatSequenceVar:\",oldChatSequenceVar);\n"				
					.. "print(\"v0:\",v0);\n"
					.. "print(\"----------------\");\n"	
					);

			table.insert(rtAct.Actions, action)
			table.insert(rtAct.Events, event)
			table.insert(event.ActionsId, action.Id)
		end

		
		do
			local event = Actions.createEvent("variable_v2_changed", "",  rtGrp.Id)
			
			local action = Actions.createAction("code", "print(\"variable_v2_changed\");\n"
					.. "print(\"--------"..  rtGrp.Id .. "---\");\n"	
					.. "print(\"oldActivitySequenceVar:\",oldActivitySequenceVar);\n"			
					.. "print(\"currentActivitySequenceVar:\",currentActivitySequenceVar);\n"				
					.. "print(\"oldActivityStepVar:\",oldActivityStepVar);\n"		
					.. "print(\"v2:\",v2);\n"		
					.. "print(\"oldChatStepVar:\",oldChatStepVar);\n"
					.. "print(\"v1:\",v1);\n"		
					.. "print(\"oldChatSequenceVar:\",oldChatSequenceVar);\n"				
					.. "print(\"v0:\",v0);\n"
					.. "print(\"----------------\");\n"	
					);

			table.insert(rtAct.Actions, action)
			table.insert(rtAct.Events, event)
			table.insert(event.ActionsId, action.Id)
		end
	end

end


if r2.getInstanceFromId == nil
then
	r2.getInstanceFromId = function(instanceId)

		local function look(node,instanceId)
		if node.InstanceId ~=nil
		then
			-- debugInfo("looking in "..node.InstanceId)
		else
			-- debugInfo("no instance id!!")
		end
		--look if this node is the good one
			if node.InstanceId == instanceId
			then
				--then return it
				return node
			else
				--else, look in its children
				local children = node.Children
				if children == nil
				then
					return nil
				else
					local max = table.getn(children)
					for i=1,max do
						local tmp = look(children[i],instanceId)
						if tmp ~=nil
						then
							return tmp
						end
					end
				end
			end
		end
		local tmp = look(r2.Scenario,instanceId)
		if tmp~=nil
		then
			return tmp
		end
		tmp = look(r2.Scenario.Acts,instanceId)
		if tmp~=nil
		then
			return tmp
		end
		tmp = look(r2.Scenario.Texts,instanceId)
		if tmp~=nil
		then
			return tmp
		end
		return nil
	end
end




r2.getAiCost = function(instance)
	-- if the object is a ghost duplicate (when shift-dragging for exemple),
	-- don't take the cost into acount
	if instance.User.GhostDuplicate then return 0 end
	local cost = 1
	local components = instance.Components

	if components and table.getn(components) ~= 0 then
		local key, value = next(components, nil)
		while key do
			if value.getAiCost then
				cost = cost + value:getAiCost()
			end
			key, value = next(components, key)
		end
	end

	local ghosts = instance.Ghosts
	if ghosts and table.getn(ghosts) ~= 0 then
		local key, value = next(ghosts, nil)
		while key do
			if value.getAiCost then
				cost = cost + value:getAiCost()
			end
			key, value = next(ghosts, key)
		end
	end
	return cost
end

r2.getStaticObjectCost = function(instance)
	-- if the object is a ghost duplicate (when shift-dragging for exemple),
	-- don't take the cost into acount
	if instance.User.GhostDuplicate then return 0 end
	local cost = 0
	local components = instance.Components

	if components and table.getn(components) ~= 0 then
		local key, value = next(components, nil)
		while key do
			if value.getStaticObjectCost then
				cost = cost + value:getStaticObjectCost()
			end
			key, value = next(components, key)
		end
	end
	return cost
end	

--r2.displayFeatureHelp = function(title, help)
r2.displayFeatureHelp = function(className)
	
	local title = "uiR2Ed"..className.."_HelpTitle"
	
	local test = i18n.get(title)
	if not test then
		debugInfo("Entry not found in translation file")
		assert(nil)
	end
	
	local help = "uiR2Ed"..className.."_HelpText"
	test = i18n.hasTranslation(help)
	if not test then
		debugInfo("Entry not found in translation file")
		assert(nil)
	end
	
	local checkBox = getUI("ui:interface:feature_help:content:custom_bbox_enabled")
	assert(checkBox)
	local chkBoxText = getUI("ui:interface:feature_help:content:text_custom")
	assert(chkBoxText)
	
	if className == "Npc" then
		debugInfo("ClassName = " ..className)
		checkBox.active = false
		chkBoxText.active = false
	else
		checkBox.active = true
		chkBoxText.active = true
		local pushed = false
		if r2.mustDisplayInfo(className) == 1 then pushed = true end
		checkBox.pushed = pushed
	end

	local uiInfo = getUI("ui:interface:feature_help")

	--local scrW, scrH = getWindowSize()
	--uiInfo.x = 0
	--uiInfo.y = scrH
	--uiInfo.h = scrH / 2
	--uiInfo.pop_min_h= scrH / 2
	--uiInfo.pop_max_h= scrH / 2
	uiInfo.active = true			
	uiInfo:invalidateCoords()
	uiInfo:updateCoords()
	uiInfo.title = title
	uiInfo.Env.uc_title = title	
	local uiText = getUI("ui:interface:feature_help:content:enclosing:help_text_enclosed:help_text")
	assert(uiText)
	uiText.uc_hardtext_format = ucstring(i18n.get(help))	
	uiInfo:invalidateCoords()
	uiInfo:updateCoords()	
	
	--local textH = uiText.h_real	
	--local localH = textH + 90
	--if localH > scrH then
	--	localH = scrH
	--end
	--uiInfo.h = localH
	--uiInfo.pop_min_h= localH
	--uiInfo.pop_max_h= localH
	
	uiInfo:invalidateCoords()
	uiInfo:updateCoords()	
	uiInfo:center()

	setTopWindow(uiInfo)
end


function r2.setFeatureDisplayHelp()
	
	local checkBox = getUI("ui:interface:feature_help:content:custom_bbox_enabled")
	assert(checkBox)
	local isChecked = checkBox.pushed
	debugInfo("checked: " ..tostring(isChecked))

	local ui = getUI("ui:interface:feature_help")
	local name = ui.Env.uc_title
	local len = string.len(name) - 10 - 6
	local className = string.sub(name, -10-len, 6+len) --removing uiR2Ed and _HelpTitle
	--formName = formName .."Form"
	
	assert(className)
	--debugInfo("Form name: " ..formName)

	if isChecked == false then 
			r2.setDisplayInfo(className, 1)
	else r2.setDisplayInfo(className, 0) end

end

function r2.getDisplayButtonHeader(func, buttonText)
	local header = 
	string.format(
		[[
				<ctrl style="text_button_16" id="help" posref="TL TL" color="255 255 255 255" col_over="255 255 255 255" col_pushed="255 255 255 255" 
					onclick_l="lua" params_l="%s" hardtext="%s"/>
		]], func, buttonText)
	
	return header
end

function r2.updateLogicEvents(this, invalidEvents)
	assert(invalidEvents)
	assert(this)
	
	local actions = this.Behavior.Actions
	assert(actions)
	

	local function updateLogicEvent(k, action)

		local event = action.Event
		assert(event)
		if invalidEvents[event.Type] then
			local instanceId = tostring(event.InstanceId)
			r2.requestSetNode(instanceId, "Type", tostring(invalidEvents[event.Type]))
		end
	end

	forEach(actions, updateLogicEvent)

end

function r2.updateLogicActions(this, invalidActions, entityClass)
	assert(invalidActions)
	assert(this)
	assert(entityClass)
	
	local instance = r2:getInstanceFromId(this.Entity)
	if not instance or not instance:isKindOf(entityClass) then
		return
	end

	local action = this.Action
	assert(action)
	
	if invalidActions[action.Type] then
		r2.requestSetNode(action.InstanceId, "Type", invalidActions[action.Type])
	end
end


function r2.onRingAccessUpdated(access)
	r2:buildPaletteUI()
	r2.acts:initActsEditor()
	updateBanditCampEnum()
	-- also available by r2.getRingAccess()
end

function r2:checkAiQuota(size)

	if not size then size = 1 end
	local leftQuota, leftAIQuota, leftStaticQuota = r2:getLeftQuota()

	if leftAIQuota < size then
		displaySystemInfo(i18n.get("uiR2EDMakeRoomAi"), "BC")

		return false
	end
	return true
end

function r2:checkStaticQuota(size)
	if not size then size = 1 end
	local leftQuota, leftAIQuota, leftStaticQuota = r2:getLeftQuota()
	if leftStaticQuota < size then
		displaySystemInfo(i18n.get("uiR2EDMakeRoomStaticObject"), "BC")	
		return false
	end
	return true
end

function r2.DisplayNpcHeader()
	local npc = r2:getSelectedInstance()
	if not npc then
		return ""
	end

	if npc:isGrouped() then
		local header = 
		[[
			<view type="text" id="t" multi_line="true" sizeref="w" w="-36" x="4" y="-2" posref="TL TL" global_color="true" fontsize="12" shadow="true" hardtext="uiR2EdNpcHeader"/>
		]]
		return header 
	else
		return ""
	end
end


function r2.mustDisplayProp(prop)
end
