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
	local classScenarioVersion = 4

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
		updateVersion = function(this, scenarioValue, currentValue)
			local patchValue = scenarioValue
			if patchValue < 1 then
				-- Patch only for old save (not the 0.0.3)
				if VersionName ~= "0.0.3" then
					-- TODO use identifier instead
					local oldValue = this.Description.LocationId
					local newValue = oldValue

					if 0 == oldValue  then 
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
						
					r2.requestSetNode(this.Description.InstanceId, "LocationId", newValue)
				end	
				patchValue = 1
			end

			-- plot item feature
			if patchValue < 2 then
				r2.requestSetNode(this.InstanceId, "PlotItems", {})
				r2.requestSetNode(this.InstanceId, "UserComponents", {})				
				patchValue = 2
			end

			-- update of enum for scenario type
			if patchValue < 3 then
				if this.Type == nil or this.Type == "" then
					r2.requestSetNode(this.InstanceId, "Type", "so_story_telling")
				elseif this.Type == "Roleplay" then					
					r2.requestSetNode(this.InstanceId, "Type", "so_story_telling")
				elseif this.Type == "Combat" then
					r2.requestSetNode(this.InstanceId, "Type", "so_hack_slash")
				end
				patchValue = 3
			end

			-- patch for level
			if patchValue < 4 then
				if this.Description.LevelId == nil or this.Description.LevelId < 0 or this.Description.LevelId > 5 then					
					r2.requestSetNode(this.Description.InstanceId, "LevelId", 0)
				end					
				patchValue = 4
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
		local uc_name = ucstring()
		uc_name:fromUtf8(r2.Scenario:getName())
		subMenu:addLine(uc_name, "lua", calledFunction.."('".. r2.Scenario.InstanceId .."')", r2.Scenario.InstanceId)
	end

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function classScenario:getLogicTranslations()
		local logicTranslations = {
			["ApplicableActions"] = {
					--["Start Scenario Timing"]	= { menu=i18n.get( "uiR2AA0ScenarioStartTiming" ):toUtf8(), 
					--						text=i18n.get( "uiR2AA1ScenarioStartTiming" ):toUtf8()},
					--["Stop Scenario Timing"]	= { menu=i18n.get( "uiR2AA0ScenarioStopTiming" ):toUtf8(), 
					--						text=i18n.get( "uiR2AA1ScenarioStopTiming" ):toUtf8()},
					--["add scenario points"]	= { menu=i18n.get( "uiR2AA0ScenarioAddPoints" ):toUtf8(), 
					--						text=i18n.get( "uiR2AA1ScenarioAddPoints" ):toUtf8()},
					--["succeed scenario"]	= { menu=i18n.get( "uiR2AA0ScenarioSucceed" ):toUtf8(), 
					--						text=i18n.get( "uiR2AA1ScenarioSucceed" ):toUtf8()},
					--["fail scenario"]	= { menu=i18n.get( "uiR2AA0ScenarioFail" ):toUtf8(), 
					--						text=i18n.get( "uiR2AA1ScenarioFail" ):toUtf8()},
			},
			["Events"] = {	
				--["On Scenario Started"]	= { menu=i18n.get( "uiR2Event0ScenarioStart" ):toUtf8(), 
				--							text=i18n.get( "uiR2Event1ScenarioStart" ):toUtf8()},
				--["on scenario succeeded"]	= { menu=i18n.get( "uiR2Event0ScenarioSucceed" ):toUtf8(), 
				--							text=i18n.get( "uiR2Event1ScenarioSucceed" ):toUtf8()},
				--["on scenario failed"]	= { menu=i18n.get( "uiR2Event0ScenarioFailed" ):toUtf8(), 
				--							text=i18n.get( "uiR2Event1ScenarioFailed" ):toUtf8()},
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
					subMenu:addLine(ucstring(lineStr), "", "", tostring(i))

					subMenu:addSubMenu(lineNb)
					local subMenu2= subMenu:getSubMenu(lineNb)
					for s=0, 19 do
						lineStr = tostring(i+s) 
						local func = "r2.events:setEventValue('','" .. categoryEvent .."','".. lineStr.."')"
						subMenu2:addLine(ucstring(lineStr), "lua", func, lineStr)
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


	local classActVersion = 6

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

		updateVersion = function(this, scenarioValue, currentValue )
			local patchValue = scenarioValue
			if patchValue < 1 then
				r2.requestSetNode(this.InstanceId, "ManualWeather", 0)				
				r2.requestSetNode(this.InstanceId, "WeatherValue", 0)				
				patchValue = 1
			end

			if patchValue < 2 then
				if not this.Behavior then
					local behavior = r2.newComponent("LogicEntityBehavior")				
					r2.requestInsertNode(this.InstanceId, "", -1, "Behavior", behavior)	
					r2.requestSetNode(this.InstanceId, "InheritPos", 1)
					-- TODO Add position (0,0,0)
				end
				patchValue = 2

			end
			if patchValue < 3 then
				if not this.Name then
					r2.requestSetNode(this.InstanceId, "Name", this.Title)				
				end
				patchValue = 3
			end
			if patchValue < 4 then
				if not this.ExportList then
				--	r2.requestInsertNode(this.InstanceId, "", -1, "ExportList", {})
				end
				patchValue = 4
			end
			-- version 5 : Remove the "Cost" field -> hold locally now
			if patchValue < 5 then
				if this.Cost then
					r2.requestEraseNode(this.InstanceId, "Cost", -1)
				end
				if this.StaticCost then
					r2.requestEraseNode(this.InstanceId, "StaticCost", -1)
				end
				patchValue =  5
			end
			
			if patchValue < 6 then
				if this.ExportList then
					r2.requestEraseNode(this.InstanceId, "ExportList", -1)
				end
				patchValue = 6
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

			local firstPart = i18n.get("uiR2EDDefaultActTitle"):toUtf8().. actNb 
			local firstPartSpace = i18n.get("uiR2EDDefaultActTitle"):toUtf8().. " " .. actNb 
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
			local result = ucstring()
			result:fromUtf8(this:getName())
			return result
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
				local uc_name = ucstring()
				uc_name:fromUtf8(act.Name)
				subMenu:addLine(uc_name, "lua", calledFunction.."('".. act.InstanceId .."')", act.InstanceId)
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
				["Start Act"]			= { menu=i18n.get( "uiR2AA0ActStart"	):toUtf8(), 
											text=i18n.get( "uiR2AA1ActStart"	):toUtf8()},
			},
			["Events"] = {	
				["On Act Started"]		= { menu=i18n.get( "uiR2Event0ActStart"	):toUtf8(), 
											text=i18n.get( "uiR2Event1ActStart"	):toUtf8()},
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
	
	local classLocationVersion = 1
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

		updateVersion = function(this, scenarioValue, currentValue )
			local patchValue = scenarioValue
			if patchValue < 1 then
				local updateMap = { ["summer"] = "Summer", ["winter"] = "Winter", ["fall"] = "Autumn", ["spring"] = "Spring",
				 ["Summer"] = "Summer", ["Winter"] = "Winter", ["Autumn"] = "Autumn", ["Spring"] = "Spring"}
				if (this.ManualSeason == 1 ) then
					if updateMap[ this.Season ]  then
						r2.requestSetNode(this.InstanceId, "Season", updateMap[this.Season])
					else
						debugInfo("Wrong conversion function")
						assert(nil);
					end
				end
				patchValue = 1
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
--			["Start Act"]	= i18n.get("uiR2EdStartAct"):toUtf8(), 
--		},
--		["Events"] = {	
--			["On Act Started"]		= i18n.get("uiR2EdOnActStarted"):toUtf8(),
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
	
		
