
--
--

Logic = {}

Logic.Activities = {"Follow Route", "Patrol", "Repeat Road", "Deploy", "Wander", "Rest In Zone", "Feed In Zone", "Hunt In Zone", "Guard Zone", "Stand Still", "Stand On Start Point", "Go To Start Point"}
Logic.TimeLimits = {"No Limit", "Until", "Few Sec", "Few Min", "Chat", "Season", "Day", "Month"}
Logic.Seasons = {"Spring", "Summer", "Autumn", "Winter"}
Logic.Days = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"}
Logic.Months = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"}
Logic.ScriptActions ={"Sit Down", "Stand Up", "Go To Step", "Set Activity", "Stop Actions", "Start Mission"}

Logic.EventModes ={"Once Only", "Repeating"}
Logic.EventTypes ={"Time", "Place", "Entity or Group", "State Machine", "Counter"}
Logic.ConditionTypes ={
							{},
							{"Any player in zone", "All players in zone", "No players in zone", "Some players in zone"},
							{"At Destination", "Enters State", "Leaves State", "Activity Is", "Die", "Is Attacked"},
							{},
							{"v0_changed", "v3_changed", "Equals", "Lesser", "Greater", ""},
						}

Logic.ChatTypes = {"None", "Repeating", "Non Repeating", "Continue"}
Logic.InteractionTypes = {"Give", "Say"}

Logic.Name = "Activity"
Logic.Description = "A feature for describing activities sequences"




Logic.Components = {}




	-- ACTION -------------------
Logic.Components.LogicEntityAction = 
	{
		BaseClass ="BaseClass",
		Name = "LogicEntityAction",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "logicEntityActionPropertySheetDisplayer",
		Prop =
		{
			{Name ="Name", Type ="String"}, 
			{Name ="Event", Type ="EventType"}, 
			{Name ="Actions", Type ="Table"}, -- "ActionStep" list
			{Name ="Conditions", Type ="Table"}, -- "ConditionStep" list
		},

		getName = function(this)

			local logicEntity = this.Parent.Parent.Parent
			assert(logicEntity)

			if logicEntity:isGrouped() then
				logicEntity = logicEntity.ParentInstance
			end

			local coloredName = (logicEntity.InstanceId == r2.events.filteredLogicEntityId)

			local eventType = r2.events.keyWordsColor .. i18n.get("uiR2EdEventTxtPreEvent"):toUtf8() .. " "

			if coloredName then		
				eventType = eventType .. r2.events.filterWordColor 
			else
				eventType = eventType .. r2.events.entityWordColor 
			end
			local name = "No name"
			if logicEntity.getName then
				name = logicEntity:getName()
			elseif logicEntity.Name then
				name = logicEntity.Name
			end
			eventType = eventType .. name .. " " 
			eventType = eventType .. r2.events.communWordsColor 

			eventType = eventType ..r2.getLogicAttribute(logicEntity.Class, "Events", this.Event.Type).text
			
			if this.Event.Value~="" then
				local instance = r2:getInstanceFromId(this.Event.Value)
				assert(instance)

				eventType = eventType .. " " .. instance:getShortName()
			end

			
			if this.Event.ValueString and this.Event.ValueString ~= "" then
				if string.gfind(eventType, "%%1")() then
					eventType = string.gsub(eventType, "%%1", "'"..tostring(this.Event.ValueString).."'")
				else
					eventType = eventType .. " '" .. tostring(this.Event.ValueString).. "'"
				end
			end

			eventType = eventType .. ", "

			
			-- conditions
			local conditionsList = ""
			for i=0, this.Conditions.Size-1 do
				local conditionInst = this.Conditions[i]
				
				if conditionInst.Entity~="" and conditionInst.Condition.Type~="" then

					conditionsList = conditionsList .. "\n" .. r2.events.keyWordsColor 
						..string.upper(i18n.get("uiR2EdEventTxtPreCondition"):toUtf8()).." " 
						.. r2.events.communWordsColor 

					local conditionLogicEntity = r2:getInstanceFromId(tostring(conditionInst.Entity))
					assert(conditionLogicEntity)

					coloredName = (conditionLogicEntity.InstanceId == r2.events.filteredLogicEntityId)
					if coloredName then 
						conditionsList = conditionsList .. r2.events.filterWordColor 
					else
						conditionsList = conditionsList .. r2.events.entityWordColor
					end
					conditionsList = conditionsList .. conditionLogicEntity.Name .. " " 
					conditionsList = conditionsList .. r2.events.communWordsColor 
			
					conditionsList = conditionsList ..r2.getLogicAttribute(conditionLogicEntity.Class, "Conditions", conditionInst.Condition.Type).text

					if conditionInst.Condition.Value~="" then
						local instance = r2:getInstanceFromId(conditionInst.Condition.Value)
						assert(instance)
						conditionsList = conditionsList .. " '" .. instance:getShortName() .."'"
					end

					conditionsList = conditionsList .. ", "
				end
			end

			-- actions
			local actionsList = ""
			for i=0, this.Actions.Size-1 do
				local actionInst = this.Actions[i]
				
				if actionInst.Entity~="" and actionInst.Action.Type~="" then
					
					actionsList = actionsList .. "\n" .. r2.events.keyWordsColor 
						..string.upper(i18n.get("uiR2EdEventTxtPreActions"):toUtf8()).." "
						.. r2.events.communWordsColor 

					local actionLogicEntity = r2:getInstanceFromId(tostring(actionInst.Entity))
					assert(actionLogicEntity)
					
					coloredName = (actionLogicEntity.InstanceId == r2.events.filteredLogicEntityId)
					if coloredName then 
						actionsList = actionsList .. r2.events.filterWordColor 
					else
						actionsList = actionsList .. r2.events.entityWordColor 
					end
					local name = "No name"
					if actionLogicEntity.getName then
						name = actionLogicEntity:getName()
					elseif actionLogicEntity.Name then
						name = actionLogicEntity.Name
					end
					actionsList = actionsList .. name .. " " 
					actionsList = actionsList .. r2.events.communWordsColor 
					
					actionsList = actionsList ..r2.getLogicAttribute(actionLogicEntity.Class, "ApplicableActions", actionInst.Action.Type).text
					if actionInst.Action.Value~="" then
						local instance = r2:getInstanceFromId(actionInst.Action.Value)
						assert(instance)
						actionsList = actionsList .. " '" .. instance:getShortName() .. "'"
					end

					if actionInst.Action.ValueString and actionInst.Action.ValueString ~= "" then

						if string.gfind(actionsList, "%%1")() then
							actionsList = string.gsub(actionsList, "%%1", "'"..tostring(actionInst.Action.ValueString).."'")
						else
							actionsList = actionsList .. " '" .. tostring(actionInst.Action.ValueString).. "'"
						end
					end

					if i~=this.Actions.Size-1 then
						actionsList = actionsList .. ","
					else
						actionsList = actionsList .. "."
					end
				end
			end

			if actionsList=="" then
				actionsList = "\n..."
			end
		
			return eventType .. conditionsList .. actionsList --.. "\n"
		end,

		getLogicEntityParent = function(this)

			local logicEntity = this.Parent.Parent.Parent
			if logicEntity:isGrouped() then logicEntity = logicEntity.Parent.Parent end

			return logicEntity
		end,

		-- The act of the event
		getLogicAct = function(this)
			
			local currentAct = r2:getCurrentAct()
			local baseAct =  r2.Scenario:getBaseAct()
			local isBaseAct = false
			local presentActs = {}
			local isCurrentAct = false
			local counter = 0
			

			local entity = this:getLogicEntityParent()
			if entity and entity.getParentAct then
				local act = entity:getParentAct()
				if act then
					if act==currentAct then 
						isCurrentAct=true 
					elseif act==baseAct then
						isBaseAct = true
					else
						presentActs[act.InstanceId] = act.InstanceId
						counter = counter+1
					end
				end
			end


			local function verifyAct ( container)
				
				local key, value = next(container, nil)
				while key do
					if value.Action and value.Action.Type == "Start Act" then
					else
						local entityId = value.Entity
						local entity= r2:getInstanceFromId(entityId)
						if entity and entity.getParentAct then
							local act = entity:getParentAct()
							if act then
								if act==currentAct then 
									isCurrentAct=true 
								elseif act==baseAct then
									isBaseAct = true
								else
									presentActs[act.InstanceId] = act.InstanceId
									counter = counter+1
								end
							end
						end
					end
					key, value = next(container, key)
				end
			end

			verifyAct(this.Actions)
			verifyAct(this.Conditions)

			if counter>=2 or (counter==1 and isCurrentAct) then
				return nil
			elseif counter==1 and not isCurrentAct and isBaseAct then
				local act = nil
				for k, actId in pairs(presentActs) do
					act = r2:getInstanceFromId(actId)
					break
				end
				--inspect(act)
				return act
			else
				--inspect(currentAct)
				return currentAct
			end
		end,
		



		----------
		-- called upon translation
		getLogicActForTranslate = function(this)
			local currentAct = this:getLogicEntityParent():getParentAct()
			local baseAct =  r2.Scenario:getBaseAct()

			local entity = this.Event.Entity
			if entity and entity.getParentAct then
				local act = entity:getParentAct()
				if currentAct == nil then
					currentAct = act
				elseif act ~= currentAct then
					if currentAct == baseAct then
						currentAct = act
					elseif act == baseAct then
						-- do nothing
					else 
						return nil
					end
				end
			end


			
			local function verifyAct ( container)
				local key, value = next(container, nil)
				while key do
					if value.Action and value.Action.Type == "Start Act" then
					else
						local entityId = value.Entity
						local entity= r2:getInstanceFromId(entityId)
						if entity and entity.getParentAct then
							local act = entity:getParentAct()
							if act then
								if currentAct == nil then
									currentAct = act
								elseif act ~= currentAct then
									if currentAct == baseAct then
										currentAct = act
									elseif act == baseAct then
										-- do nothing
									else 																				return false
									end
								end
							end
						end
					end
					key, value = next(container, key)
				end
				return true
			end


			if not verifyAct(this.Actions) then return false end
			if not verifyAct(this.Conditions) then return false end
			return currentAct		
		end,
		--
		getLogicActInstanceId = function(this)
			--local ok, act = pcall(Logic.Components.LogicEntityAction.getLogicAct, this)
			local ok, act = pcall(Logic.Components.LogicEntityAction.getLogicActForTranslate, this)
			if not ok or not act then
				return r2.RefId("")
			end
			
			return act.InstanceId
		end,
	}

Logic.Components.EventType = 
	{
		BaseClass ="BaseClass",
		Name = "EventType",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "eventTypePropertySheetDisplayer",
		Prop =
		{
			{Name ="Type", Type ="String"}, 
			{Name ="Value", Type ="RefId"}, -- if type is end of activity/chat step/sequence, instance id
			{Name ="ValueString", Type ="String", DefaultValue="", DefaultInBase=1}, -- if type is emit user event, sub timer...

		}
	}

local classActionStepVersion = 2
Logic.Components.ActionStep = {
		BaseClass ="BaseClass",
		Name = "ActionStep",
		Version = classActionStepVersion,
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "actionStepPropertySheetDisplayer",
		Prop =
		{
			{Name ="Entity", Type ="RefId"}, -- entity target id
			{Name ="Action", Type ="ActionType"} -- sequence id
		},

		updateVersion = function(this, scenarioValue, currentValue )
			
			local patchValue = scenarioValue
			if patchValue < 1 then
				local invalidActions = {}
				invalidActions["desactivate"] = "deactivate"
				invalidActions["Desactivate"] = "deactivate"
				r2.updateLogicActions(this, invalidActions, "BanditCamp")
				patchValue = 1
			end

			if patchValue < 2 then

				local action = this.Action
				if action.Type == "Add 10 Seconds" then
					r2.requestSetNode(action.InstanceId, "Type", "add seconds")
					r2.requestInsertNode(action.InstanceId, "", -1, "ValueString", "10")
				end

				if action.Type == "Sub 10 seconds" then
					r2.requestSetNode(action.InstanceId, "Type", "sub seconds")
					r2.requestInsertNode(action.InstanceId, "", -1, "ValueString", "10")
				end
				if action.Type == "Add 1 minute" then
					r2.requestSetNode(action.InstanceId, "Type", "add seconds")
					r2.requestInsertNode(action.InstanceId, "", -1, "ValueString", "60")
				end

				if action.Type == "Sub 1 minute" then
					r2.requestSetNode(action.InstanceId, "Type", "sub seconds")
					r2.requestInsertNode(action.InstanceId, "", -1, "ValueString", "60")
				end

				patchValue = 2
			end
			
			if patchValue == currentValue then return true end
			return false
		end,	
	}


Logic.Components.ActionType = 
	{
		BaseClass ="BaseClass",
		Name = "ActionType",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "actionTypePropertySheetDisplayer",
		Prop =
		{
			{Name ="Type", Type ="String"}, 
			{Name ="Value", Type ="RefId"}, -- if type is begin of activity/chat sequence, instance id
			{Name ="ValueString", Type ="String", DefaultValue="", DefaultInBase=1}, -- if type is emit user event, sub timer...

		
		}
	}

Logic.Components.ConditionStep =
	{
		BaseClass ="BaseClass",
		Name = "ConditionStep",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "conditionStepPropertySheetDisplayer",
		Prop =
		{
			{Name ="Entity", Type ="RefId"}, -- entity id
			{Name ="Condition", Type ="ConditionType"}
			
		}
	}

Logic.Components.ConditionType = 
	{
		BaseClass ="BaseClass",
		Name = "ConditionType",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "conditionTypePropertySheetDisplayer",
		Prop =
		{
			{Name ="Type", Type ="String"}, 
			{Name ="Value", Type ="RefId"}, -- if type is being in activity/chat step/sequence, instance id

		
		}
	}

	-- REACTION -----------------

Logic.Components.LogicEntityReaction = 
	{
		BaseClass ="BaseClass",
		Name = "LogicEntityReaction",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "logicEntityReactionPropertySheetDisplayer",
		Prop =
		{
			{Name ="Name", Type ="String"}, 
			{Name ="LogicEntityAction", Type ="String"}, -- id of associated LogicEntityAction
			{Name ="ActionStep", Type ="String"},		  -- id of associated ActionStep
		}
	}

Logic.Components.ChatAction=
	{
		BaseClass ="BaseClass",
		Name ="ChatAction",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "chatActionPropertySheetDisplayer",
		Prop =
		{
			{Name ="Who", Type ="RefId"},
			{Name ="WhoNoEntity", Type ="String"},
			{Name ="Says", Type ="String"},
			{Name ="Emote", Type ="String"},
			{Name ="Facing", Type ="RefId"},
--			{Name ="FacingNoEntity", Type ="String"},
		}
	}

Logic.Components.ChatStep = {
		BaseClass ="BaseClass",
		Name = "ChatStep",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "chatStepPropertySheetDisplayer",
		Prop =
		{
			{Name ="Actions", Type ="Table"},
			{Name ="Time", Type ="Number" },
			{Name ="Name", Type ="String" }
		},

		getName = function(this)

			-- after time
			local minNb, secNb = r2.dialogs:calculMinSec(this.Time)
			local time = ""
			if minNb ~= 0 then
				time = tostring(minNb)..i18n.get("uiR2EdShortMinutes"):toUtf8()
			end
			time = time.." " ..tostring(secNb)..i18n.get("uiR2EdShortSeconds"):toUtf8()
			local afterTime = "(" ..i18n.get("uiR2EdAfter"):toUtf8().." ".. time..") "

			-- says
			local whoToWho = ""
			local action = this.Actions[0]
			local who = action.Who
			if who=="" then who=action.WhoNoEntity end
			if who and who ~= "" then
	
				if r2:getInstanceFromId(who) then
					who=r2:getInstanceFromId(who).Name	
				else
					who = r2.dialogs.whoToWhoTranslation[who]
				end

				whoToWho = who .. " " 

				local facing = action.Facing
				if facing~="" then
					whoToWho = whoToWho ..i18n.get("uiR2EdSaysTo"):toUtf8().. " " .. r2:getInstanceFromId(facing).Name .. " "	
				end

				local emote = action.Emote
				if emote~="" and r2.dialogs.fromEmoteIdToName[emote] ~= nil then
					whoToWho = whoToWho .."(" .. r2.dialogs.fromEmoteIdToName[emote] .. ") :"
				end
			end

			return afterTime..whoToWho

		end,

		getShortName = function(this)

			-- says
			local saysWhat = ""
			local action = this.Actions[0]
			local who = action.Who
			if who ~= "" or WhoNoEntity~="" then
				who = r2:getInstanceFromId(who)
				if who then
					saysWhat = who.Name
				elseif action.WhoNoEntity=="_System" then
					saysWhat = i18n.get("uiR2EdSystem"):toUtf8()
				elseif action.WhoNoEntity=="_DM" then
					saysWhat = i18n.get("uiR2EdDonjonMaster"):toUtf8()
				end
			
				saysWhat = saysWhat .. " " ..i18n.get("uiR2EdSays"):toUtf8().. " " 

				local says = action.Says
				if says ~= "" then
					local inst=r2:getInstanceFromId(says)
					if inst then
						says = inst.Text 
						local uc_says = ucstring()
						uc_says:fromUtf8(says)
						uc_says = uc_says:substr(0, 4)
						says = uc_says:toUtf8()					
					end

				end

				saysWhat = saysWhat .. says .. "..."
			end

			return saysWhat

		end,

		--------------------------------------------------------------------------------------------
		-- from 'BaseClass'
		getParentTreeNode = function(this)				
		end,
	}


local classChatSequenceVersion = 1
Logic.Components.ChatSequence = {
		PropertySheetHeader = r2.getDisplayButtonHeader("r2.dialogs:openEditor()", "uiR2EdEditDialogButton"),
		BaseClass = "LogicEntity",
		Name = "ChatSequence",
		InEventUI = true,
		Menu="ui:interface:r2ed_feature_menu",
		Version = classChatSequenceVersion,

		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "chatSequencePropertySheetDisplayer",

		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},
		ApplicableActions = {
								"activate", "deactivate", "starts dialog", "stops dialog", "starts chat"  , "continues dialog"
							},
		Events =			{
								"start of dialog", "end of dialog" ,
								"start of chat", "end of chat",
								"activation", "deactivation"
							},
		Conditions =		{
								"is not in dialog", "is in dialog", "is in chat"
							},
		TextContexts =		{},
		TextParameters =	{},
		LiveParameters =	{},

		
		Prop =
		{	
			{Name = "Type", Type="String",DefaultValue = "None", Visible=false},
			{Name ="Repeating", Type="Number" ,DefaultValue = "0", Visible=false},
			{Name="Components",Type="Table"},
			{Name="SubComponents", Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"},
			{Name="AutoStart", Type="Number", WidgetStyle="Boolean", DefaultValue="1", Visible=function(this) return this:mustDisplayAutostart() end},
			{Name="Active", Type="Number", WidgetStyle="Boolean", DefaultValue = "1"},
		},
	
		
		mustDisplayAutostart = function(this)
			if this.Class == "ProximityDialog" then
				return false
			end
			return true
		end,

		-- it's a feature
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,

		onPostCreate = function(this)
			--this:createGhostComponents()
			if this.User.DisplayProp and this.User.DisplayProp == 1 then
				r2:setSelectedInstanceId(this.InstanceId)				
				r2:showProperties(this)		
				this.User.DisplayProp = nil
			end

		end,

		create = function()
			
			--if r2:getLeftQuota() <= 0 then -- ??	
			--	r2:makeRoomMsg()
			--	return
			--end
	
			local function posOk(x, y, z)
				if r2.mustDisplayInfo("ChatSequence") == 1 then 
					r2.displayFeatureHelp("ChatSequence")
				end
				debugInfo(string.format("Validate creation of 'Dialog' at pos (%d, %d, %d)", x, y, z))
				r2.dialogs:newSequenceInst(x, y, z)
			end
			local function posCancel()
				debugInfo("Cancel choice 'Dialog' position")
			end	
			
			local creature = r2.Translator.getDebugCreature("object_component_dialog.creature")
			r2:choosePos(creature, posOk, posCancel, "createDialog")
		end,

		---- get Name ------
		getName = function(this)

			local name = this.Name
			if name == "" then

				local index = r2.logicComponents:searchElementIndex(this)
				if index >= 0 then
					name = i18n.get("uiR2EDDialog"):toUtf8() .. index
				end
			end
			
			return name 
		end,

		---- get Name ------
		getShortName = function(this)

			return this:getName() 
		end,

		---------------------------------------------------------------------------------------------------------

		editDialogs = function(this)
			r2.dialogs:openEditor()
		end,

		getAvailableCommands = function(this, dest)
			r2.Classes.LogicEntity.getAvailableCommands(this, dest)
			table.insert(dest, this:buildCommand(this.editDialogs, "edit_dialogs", "uiR2EDEditDialogs", "r2ed_edit_dialog.tga", true))				
			this:getAvailableDisplayModeCommands(dest)
		end,

		initEventValuesMenu = function(this, menu, categoryEvent)
			for ev=0,menu:getNumLine()-1 do

				local eventType = tostring(menu:getLineId(ev))

				if r2.events.eventTypeWithValue[eventType] == "Number" then
					menu:addSubMenu(ev)
					local subMenu = menu:getSubMenu(ev)
					local func = ""
					for i=0, 9 do
						local uc_name = ucstring()
						uc_name:fromUtf8( tostring(i) )
						func = "r2.events:setEventValue('','" .. categoryEvent .."','".. tostring(i).."')"
						subMenu:addLine(uc_name, "lua", func, tostring(i))
					end

				elseif r2.events.eventTypeWithValue[eventType]=="ChatStep" then

					menu:addSubMenu(ev)
					local subMenu = menu:getSubMenu(ev)

					for c=0, this.Components.Size-1 do
						local chat = this.Components[c]
						local uc_name = ucstring()
						uc_name:fromUtf8(chat:getShortName())
						subMenu:addLine(uc_name, "lua", 
							"r2.events:setEventValue('".. chat.InstanceId .."','" .. categoryEvent .. "')", chat.InstanceId)
					end

					if this.Components.Size==0 then
						subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
					end
				end
			end
		end,
		
		getLogicAction = function(this, context, action)
			return r2.Translator.getDialogLogicAction(this, context, action)
		end,
		getLogicEvent = function(this, context, event)
			return r2.Translator.getDialogLogicEvent(this, context, event)
		end,
		getLogicCondition = function(this, context, condition)
			return r2.Translator.getDialogLogicCondition(this, context, condition)
		end,

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
		end,

		translate = function(this, context)
			r2.Translator.translateFeatureActivation(this, context)
			r2.Translator.translateDialog( this, context)
		end,

		updateVersion = function(this, scenarioValue, currentValue )
			local patchValue = scenarioValue
			if patchValue < 1 then
				local subComponents = {}
				r2.requestInsertNode(this.InstanceId, "", -1, "SubComponents", subComponents)
				r2.requestInsertNode(this.InstanceId, "", -1, "AutoStart", this.Active)
				r2.requestSetNode(this.InstanceId, "Active", 1)
				r2.requestSetNode(this.InstanceId, "Base", r2.Translator.getDebugBase("palette.entities.botobjects.dialog"))
				patchValue = 1
			end
			
			if patchValue == currentValue then return true end
			return false
		end,

		initLogicEntitiesInstancesMenu = function(this, subMenu, calledFunction)
			
			local entitiesTable = r2.Scenario:getAllInstancesByType(this.Name)
			for key, entity in pairs(entitiesTable) do
				local uc_name = ucstring()
				uc_name:fromUtf8(entity.Name)
				subMenu:addLine(uc_name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
			end

			if table.getn(entitiesTable)==0 then
				subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
			end
		end
		
	}


local ActivityStepVersion = 1
Logic.Components.ActivityStep = {
		BaseClass ="BaseClass",
		Name ="ActivityStep",
		Version = ActivityStepVersion,
		BuildPropertySheet = true,
		Menu ="ui:interface:r2ed_feature_menu",
		
		BuildPropertySheet = false,
		DisplayerVisualParams = 
		{
			Look = r2.PrimRender.GroupLook,		
			ArrayName = "Components"
		},
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "activityStepPropertySheetDisplayer",	
		
		Prop =
		{
			{Name="Activity", Type="String"},
			{Name="ActivityZoneId",Type="RefId",Category="ActivityStep"},
			{Name="TimeLimit",Type="String",Category="ActivityStep"},
			{Name="TimeLimitValue", Type="String",Category="ActivityStep",Category="ActivityStep"},
			{Name="RoadCountLimit", Type="String",Category="ActivityStep",Category="ActivityStep", DefaultInBase = 1, DefaultValue = "0"},
			{Name="Chat",Type="RefId"},
			{Name = "Type", Type="String",DefaultValue = "None"},  --TEMP TEMP TEMP
			{Name="EventsIds",Type="Table"},
			{Name="Name", Type="String", MaxNumChar="32"}
		},
		-- get sequencable parent for this activity step
		getSequencableParent = function(this)
			local currParent = this.Parent
			while currParent do
				if currParent.InstanceId and currParent:isSequencable() then
					return currParent
				end
				currParent = currParent.Parent
			end
			return nil
		end,
		getVerbLookupString = function(this)
			local seqParent = this:getSequencableParent()			
			if seqParent then
				return seqParent:getActivityVerbLookupName(this.Activity)
			else
				return this.Activity
			end			
		end,
		-- do lookup from the verb lookup string used to know the display name, and button texture
		doLookupFromVerb = function(this, lookupTable)			
			local lookupString = this:getVerbLookupString()			
			-- activity type
			local value = lookupTable[lookupString]
			if  not value then
				debugInfo(colorTag(255,0,0).."The activity '".. lookupString .."' is not properly registred")				
				return nil				
			end
			return value
		end,
		-- get texture for the button displayed in the mini activity view
		getMiniButtonTexture = function(this)						
			local result = this:doLookupFromVerb(r2.miniActivities.activityTextures)
			if result then return result end
			return "brick_default.tga"
		end,
		-- get the verb for this activity (rest, feed, patrol etc...)
		getVerb = function(this)									
			local result = this:doLookupFromVerb(r2.activities.activityTypeTranslation)
			if result then result = result.trans end
			if result then return result end
			return ucstring("Activity not registered : " .. lookupString)
		end,
		-- element name
		getName = function(this)			
			local activityType = this:getVerb() .. " "
			if this.ActivityZoneId~= "" then
				local place = r2:getInstanceFromId(tostring(this.ActivityZoneId))
				if place~=nil then
					activityType = activityType.. place.Name .." "
				end	
			end

			-- activity time
			local activityTime = ""
			if this.TimeLimit == "Few Sec" then
			
				local hourNb, minNb, secNb = r2.logicComponents:calculHourMinSec(tonumber(this.TimeLimitValue))

				local timeLimitText = i18n.get("uiR2EdFor"):toUtf8() .. " "
				if hourNb ~= 0 then timeLimitText = timeLimitText .. hourNb .. i18n.get("uiR2EdShortHours"):toUtf8() .. " " end
				if minNb ~= 0 then timeLimitText = timeLimitText .. minNb .. i18n.get("uiR2EdShortMinutes"):toUtf8() .. " " end
				timeLimitText = timeLimitText .. secNb .. i18n.get("uiR2EdShortSeconds"):toUtf8() 
				
				activityTime = timeLimitText

			elseif r2.activities.timeLimitsTranslation[this.TimeLimit] ~= nil then
				activityTime = string.lower(r2.activities.timeLimitsTranslation[this.TimeLimit])
			end

			return activityType..activityTime
		end,

		-- element name
		getShortName = function(this)

			-- activity type			
			local activityType = this:getVerb()
			if this.ActivityZoneId~= "" then
				local place = r2:getInstanceFromId(tostring(this.ActivityZoneId))
				if place~=nil then
					activityType = activityType.. " " .. place.Name 
				end	
			end

			return activityType
		end,

		updateVersion = function(this, scenarioValue, currentValue )
			local patchValue = scenarioValue

			if patchValue < 1 then
				local activity = this.Activity
				if (activity and activity == "Inactive") then
					r2.requestSetNode(this.InstanceId, "Activity", "Stand Still")
				end
				local name = this.Name
				
				patchValue = 1
			end
			if patchValue == currentValue then return true end
			return false
		end
	}



Logic.Components.ActivitySequence = {
		BaseClass="BaseClass",
		Name="ActivitySequence",
		BuildPropertySheet = true,
		Menu ="ui:interface:r2ed_feature_menu",

		--DisplayerUI = "R2::CDisplayerLua",
		--DisplayerUIParams = "defaultUIDisplayer",
		BuildPropertySheet = false,
		DisplayerVisual = "R2::CDisplayerVisualActivitySequence",
		DisplayerProperties = "R2::CDisplayerLua",
		DisplayerPropertiesParams = "activitySequencePropertySheetDisplayer",	
		
		Prop =
		{
			--{Name ="InstanceId", Type ="String"},
			{Name ="Repeating", Type ="Number" , DefaultValue = "1"},
			{Name ="Components", Type ="Table"},
			{Name ="Name", Type ="String"}
		},

		---- get Name ------
		getName = function(this)

			local name = this.Name
			if name == "" then

				local index = r2.logicComponents:searchElementIndex(this)

				if index >= 0 then
					name = i18n.get("uiR2EdSeq"):toUtf8() .. index
				end
			end
			
			return name 
		end,

		---- get Name ------
		getShortName = function(this)

			return this:getName() 
		end,

		-- get ActiveLogicEntity parent
		getActiveLogicEntityParent = function(this)

			-- ActiveLogicEntity.Behavior.Activities
			local activeLogicEntity = this.Parent.Parent.Parent

			-- NpcGrpFeature.Components
			--if activeLogicEntity:isGrouped() then activeLogicEntity = activeLogicEntity.Parent.Parent end

			return activeLogicEntity
		end,
	}



--this function must be called when we delete a ChatAction,
--to unregister the text
onDeleteChatAction = function(chatAction)
	if chatAction.User.DeleteInProgress == true then return end
	chatAction.User.DeleteInProgress = true
	if chatAction.Says ~= nil and chatAction.Says ~= ""
	then

		debugInfo("onDeleteChatAction")
		r2.unregisterTextFromId(chatAction.Says)
	end
end

--counts the states created, to ensure states names uniqueness

--timer used to time the sequences steps
Logic.activityStepTimer = "t0"
Logic.activityStepTimerId = 0
--timer used to time the chat steps
Logic.chatTimer = "t1"
Logic.chatTimerId = 1
--timer used inside activities
Logic.activityInternalTimer = "t2"
Logic.activityInternalTimerId = 2

--variable used to know which action of the current chat sequence
--must be played
Logic.chatStepVar = "v1"
--variable used to know which chat sequence is currently played
Logic.chatSequenceVar = "v0"
--variable used to know which activity sequence is currently played
Logic.activityStepVar = "v2"
--variable used to know which interrupt action must be executed
Logic.triggerEventVar = "v3"

Logic.EndOfActivitySequenceEvent = 9
Logic.EndOfChatSequenceEvent = 8

--switch_action which hold all chat sequences
Logic.DialogAction = nil
--switch action holding chat sequences transitions
Logic.DialogAction2 = nil

--switch_action which hold all sequence steps
Logic.SequenceAction = nil
Logic.ActionStepCounter =0
Logic.ChatSequenceCounter = 0
--used to know the names of all the states generated
--to translate this activity
Logic.StatesByName = ""

--timer_triggered event, triggered 
--each time we must enter a new chat step
Logic.DialogEvent = nil

Logic.DialogEvent2 = nil

--timer_triggered event, triggered
--each time we must enter a new sequence step
Logic.SequenceEvent = nil

--hold the states used in the current activity.
--all the states are created at the start of the translation,
--before translating the events and actions.
Logic.States = {}

--hold the states used in all the activities
--of the group
Logic.ActivitiesStates ={}

--the activity which is currently being translated.
Logic.CurrentActivity = nil

----------------------------------------------------
--to be called once for each sequence of each group
--complete the AiActivity field of the first state of this sequence
--and create the actions to go to the first step
Logic.initGroupActivitiesTranslation = function(context, hlComponent, sequence, first, activityIndex, aiActivity, rtGrp)
	assert(context and hlComponent and sequence and activityIndex and aiActivity and rtGrp)
	--get the states of this activity
	
	local activityStates = Logic.ActivitiesStates[ tostring(sequence.InstanceId) ]	
	assert(sequence.Class == "ActivitySequence")
	if not activityStates then
		
		if hlComponent then
			printWarning("Error while translating '" .. hlComponent.Name .. "' activities in sequence " .. sequence.Name)			
			-- pretranslate sequence.InstanceId errors instanceId not added in currentAct.ActivitiesId ?
			return nil
		end		
	end
	--get the initial state of this activity
	local aiState = activityStates[sequence.InstanceId][1]
	Logic.StatesByName = ""
	for k, v in pairs(activityStates)
	do
		if v.Class ~= "RtAiState"
		then
			local  k1, v1 = next( v, nil)
			while k1 do
				Logic.StatesByName = r2.Utils.concat(Logic.StatesByName, v1.Name)
				k1, v1 = next(v, k1)
			end
		else
			Logic.StatesByName = r2.Utils.concat(Logic.StatesByName, v.Name)
		end
	end
	aiState.AiActivity = aiActivity
	assert(aiState.AiActivity)
	local event = r2.Translator.createEvent("start_of_state", aiState.Name, rtGrp.Name)
	-- local event = r2.Translator.createEvent("group_spawned", aiState.Name, rtGrp.Name)
	table.insert(context.RtAct.Events, event)
	if first == true
	then
		table.insert(aiState.Children, rtGrp.Id)
	end

	local speed = ""

	if hlComponent.Speed and hlComponent.Speed == "run" then speed = "()addProfileParameter(\"running\");\n" end
	
	local code = "currentActivitySequenceVar = " .. tostring(activityIndex -1 ) ..";\n"			
		.. "oldActivitySequenceVar = currentActivitySequenceVar;\n"
		.. "RoadCountLimit=0;\n"
		.. "oldActivityStepVar = -1;\n" 
		.. "oldActivityStepVar2 = 0;\n" 
		.. Logic.activityStepVar .." = 0;\n" 
		.. speed
		.."alive = 0;\n"
		.."(alive)isAlived();\n"
		.. "if (alive == 1)\n{\n"
		.. "\t()setTimer(30, " .. Logic.activityStepTimerId .. ");\n"
		.. "\tinit = 1;\n"
		.. "}\n"


	-- Fauna activities


	local action = r2.Translator.createAction("code", code)
	action.Name = "startActivitySequence"

	table.insert(context.RtAct.Actions, action)
	table.insert(event.ActionsId, action.Id)



	do
		local event = r2.Translator.createEvent("group_spawned", aiState.Name, rtGrp.Name)
		local action = r2.Translator.createAction("code", "if (init != 1)\n{\n\t()setTimer(30, " .. Logic.activityStepTimerId .. ");\n}\n")
		table.insert(context.RtAct.Events, event)
		table.insert(context.RtAct.Actions, action)
		table.insert(event.ActionsId, action.Id)
	end


	local leader = hlComponent

	if hlComponent:isKindOf("NpcGrpFeature") then
		if hlComponent.Components.Size ~= 0 then
			leader = hlComponent.Components[0]
		else
			leader = nil
		end
	end
end

--insert an activity step at the specified position
Logic.insertActivityStep = function(sequence, step, pos)
	local n = table.getn(sequence.Components)
	if n < pos
	then
		table.insert(sequence.Components, step)
	else
		table.insert(sequence.Components, pos, step)
	end
end

--insert a chat step at the specified position
Logic.insertChatStep = function(chatSequence, chatStep, pos)
	local n = table.getn(chatSequence.Components)
	if n < pos
	then
		table.insert(chatSequence.Components, chatStep)
	else
		table.insert(chatSequence.Components, pos)
	end
end


Logic.createInteraction = function(action, parameter)
	local index = Logic.find(Logic.InteractionTypes, action)
	local interaction
	
	if index == nil or parameter == nil
	then
--			debugInfo(colorTag(255,0,0).."unknown interaction : " .. action .." !")
		return nil
	end

	interaction = r2.newComponent("Interaction")
	interaction.Type = action
	interaction.Parameter = parameter
	return interaction
end

Logic.createActivityStep = function(activity, timeLimit, zoneId, time)
	local activityStep = nil
	
	if activity == nil or timeLimit == nil
	then
		return nil
	end

--checking activity type and time limit type
	if Logic.find(Logic.Activities, activity)~= nil and Logic.find(Logic.TimeLimits, timeLimit)~= nil
	then
		--checking activity parameter
		if ( (activity ~= "Stand Still") and (activity ~= "Inactive") and (zoneId == nil) )
		then
			--this activity need a zone, and no zone id is given
--				debugInfo(colorTag(255,0,0).."activity <" .. activity .."> need a zone id!")
			return nil
		end
		
	--checking time parameter
		--if time parameter needed, check its presence
		if timeLimit~="No Limit" and timeLimit~="Chat" and time == nil
		then
			--this activity needs a time limit and no limit is given
--				debugInfo(colorTag(255,0,0).."Time limit <" .. timeLimit .."> need a time limit parameter!")
			return nil
		end

		--check season validity
		if (timeLimit == "Season") and (Logic.find(Logic.Seasons, time) == nil)
		then
--				debugInfo(colorTag(255,0,0).."Unknown season: " .. time)
			return nil
		end
		
		--check day validity
		if (timeLimit == "Day") and (Logic.find(Logic.Days, time) == nil)
		then
			debugInfo(colorTag(255,0,0).."Uknown week day: " .. time)
			return nil
		end
		
		if (timeLimit == "Month") and (Logic.find(Logic.Months, time)== nil)
		then
--				debugInfo(colorTag(255,0,0,0).."Uknown month: " .. time)
			return nil
		end

		activityStep = r2.newComponent("ActivityStep")
		activityStep.Activity = activity
		activityStep.TimeLimit = timeLimit
		activityStep.ActivityZoneId = zoneId
		activityStep.TimeLimitValue = time
	else
--			debugInfo(colorTag(255,0,0).."Unknown activity !")
	end
	return activityStep
end

Logic.createState = function()
	local aiState = r2.newComponent("RtAiState")
	aiState.Name = aiState.Id
	Logic.StatesByName = Logic.StatesByName .."\n" .. aiState.Name
	return aiState
end

--create the AiStates necessay to translate this step
Logic.createStepStates = function(step, context)
	local states = {}
	local state = Logic.createState()
	table.insert(states, state)
	table.insert(context.RtAct.AiStates, state)
	if step.Activity == "Patrol"
	then
		local state = Logic.createState()
		table.insert(states, state)
		table.insert(context.RtAct.AiStates, state)
	end
	return states
end

--assign a zone or a road to an AiState
--if invert is true, invert the road
Logic.assignZone = function(rtAiState, zone, invert)
	assert( zone and type(zone) == "userdata")
	local points = zone.Points
	assert( points ~= nil)

	local size = points.Size + 1
	rtAiState.Pts = {}
	local i =0
	local j

	local k, v = next(points, nil)
	while k ~= nil
	do
		assert(v ~= nil)
		i = i +1
		if invert == true
		then
			j = size - i
		else
			j = i
		end
		rtAiState.Pts[j]={}
		rtAiState.Pts[j].x = r2.getWorldPos(v).x
		rtAiState.Pts[j].y = r2.getWorldPos(v).y
		rtAiState.Pts[j].z = r2.getWorldPos(v).z
		k, v = next(points, k)
	end
end

--create all the states used in this activity Sequence
Logic.createActivityStates = function(context, sequence)
	assert( sequence ~= nil)

	local activityStates ={}
	--activity's initial stat, aiStopStatee
	
	-- init state
	local aiStartState = r2.newComponent("RtAiState")
	aiStartState.Name = aiStartState.Id
	Logic.StatesByName = aiStartState.Name
	aiStartState.AiMovement = "idle" -- "stand_on_start_point"
	table.insert(context.RtAct.AiStates, aiStartState)
	

	-- end state
	local aiStopState = r2.newComponent("RtAiState")
	aiStopState.Name = aiStopState.Id
	--Logic.StatesByName = aiStopState.Name
	aiStopState.AiMovement = "idle" -- "stand_on_start_point"
	table.insert(context.RtAct.AiStates, aiStopState)

	activityStates[sequence.InstanceId]= { aiStartState, aiStopState}


	if (sequence.Components == nil) then assert(nil) end
	--creation of the states of the step
	local k, v = next(sequence.Components, nil)
	while k do
		local states = Logic.createStepStates(v, context)
		activityStates[v.InstanceId]= states
		 k, v = next(sequence.Components, k)
	end
	Logic.ActivitiesStates[sequence.InstanceId] = activityStates
end

Logic.translateActivitySequence = function(context, hlComponent, activitySequence, sequenceId, rtNpcGrp)
	assert(rtNpcGrp ~= nil)
	if context.Real == false
	then
		return
	end
	Logic.ActionStepCounter = 0
	Logic.ChatSequenceCounter = 0
	--Logic.States = {}
	Logic.CurrentActivity = activitySequence
	local ok = false

	local nbSteps = activitySequence.Components.Size
	local aiState
	local action
	local event
	local sequence = activitySequence


	if sequence.Components.Size ==0
	then	
		return
	end


	Logic.initSequence(context, rtNpcGrp)
	if  Logic.ActivitiesStates[sequence.InstanceId] then
		Logic.States = Logic.ActivitiesStates[sequence.InstanceId]
	else
		Logic.States = ""
	end
	
	

	local i =1
	local k, activityStep = next(sequence.Components, nil)
	while k do
		--translate step
		Logic.translateActivityStep(context,  hlComponent, activitySequence, activityStep, i, rtNpcGrp)
		i = i + 1
		k, activityStep = next(sequence.Components, k)

	end

	--create actions to repeat the sequence
	if sequence.Repeating == 1
	then

		local code =
			"//sequence will repeat"..
			"oldChatSequenceVar = " .. Logic.chatSequenceVar .. ";\n" ..
			"RoadCountLimit=0;\n" ..
			"oldActivityStepVar = " .. Logic.activityStepVar .. ";\n" ..
			"oldActivityStepVar2 = " .. Logic.activityStepVar .. " + 1" .. ";\n" ..
			"v2=0;\n" ..
			"()setTimer(1, " ..Logic.activityStepTimerId .. ");\n"
--			.. "()setEvent(" .. Logic.EndOfActivitySequenceEvent ..");\n"
		
		local action = r2.Translator.createAction("code", code)
		action.Name = "repeat_sequence"
		table.insert(Logic.SequenceAction.Children, action)
		
	else
	
		local backupState =
			"//sequence will not repeat"..
			"oldChatSequenceVar = " .. Logic.chatSequenceVar .. ";\n" ..
			"RoadCountLimit=0;\n"..
			"oldActivityStepVar = " .. Logic.activityStepVar .. ";\n" ..
			"oldActivityStepVar2 = " .. Logic.activityStepVar ..  " + 1" .. ";\n"
		
		local stateName = Logic.ActivitiesStates[sequence.InstanceId][sequence.InstanceId][2].Id

		local setNextActivityState = "()postNextState(\"" .. r2.getNamespace() .. stateName .."\");\n"
		
		local action = r2.Translator.createAction("code", backupState 
			-- .."()setEvent(" .. Logic.EndOfActivitySequenceEvent ..");\n"
			..setNextActivityState)
		
		
		action.Name = "end_of_activity_sequence"
		table.insert(Logic.SequenceAction.Children, action)
	end

	Logic.SequenceEvent.StatesByName = Logic.StatesByName
	Logic.DialogEvent = nil
	Logic.SequenceEvent = nil
end



Logic._obsolete_TranslateScriptAction = function(context, step, nextTime)
	local actionType = Logic.getAction(step)
	assert(actionType~= nil)
--		debugInfo(colorTag(0,255,0).. actionType)
	
	if actionType == "stop_actions"
	then
		local action = r2.Translator.createAction("set_timer_" ..Logic.chatTimer, "0")
		return action
	end
	
	if actionType == "start_mission"
	then
		local counterMission = context.Components[step.Parameter]
		local action = r2.Translator.createAction("trigger_event_1", context.RtCounters[counterMission.InstanceId].RtGroup.Name)
--			debugInfo("action created!!")
		return action
	end

	if 0==1 and actionType == "go_to_step"
	then
		--TODO
		local param = tonumber(step.Parameter)
		debugInfo("param = " .. param)
		if (param < 1) or (Logic.CurrentActivity.Components[param]== nil)
		then
--				debugInfo(colorTag(255,0,0).."Impossible to go to step " .. param .." !")
			return nil
		end
		debugInfo(context.Feature.Class)
		return Logic.selectActivityStep(param)
	end

	if actionType == "set_activity"
	then
		local group = nil
		local param
		local rtGroup
		if step.Who ~= "" and step.Who ~= nil
		then
			group = step.Who
			rtGroup = r2.Translator.getRtGroup(context, group)
		end
		
		param = tonumber(step.Parameter)

--			debugInfo("param = " .. param)

		local action
		if group ~= nil
		then
			action = r2.Translator.createAction("begin_state", Logic.getActivityInitialStateName(group, param, context))
			assert(action ~= nil)
			local index = Logic.addTriggeredAction(context, context.RtGroups[group].Name, action)
			action = r2.Translator.createAction("modify_variable", rtGroup.Name ..":" ..Logic.triggerEventVar .." = " .. index)
			action.Name ="set_activity"
		else
			action = r2.Translator.createAction("begin_state", Logic.getActivityInitialStateName(context.Group.Name, param, context))
			assert(action ~= nil)
		end
		return action
	end

	local action = r2.Translator.createAction(actionType)
	assert(action ~= nil)
	return action
end


	

Logic.translateChatAction = function(context, chatSequenceNumber, chatStepNumber, chatSequence, chatStep, chatAction, rtMAction)
	local getRtId = r2.Features["TextManager"].getRtId
	if chatAction.Class == "ChatAction"
	then	
		local who =  r2.Utils.getNpcParam( tostring( chatAction.Who), context)
		-- create say action
		if not who then

			local whoInstance = r2:getInstanceFromId( tostring(chatAction.Who) )
			local chatSequenceInstance = r2:getInstanceFromId(chatSequence.InstanceId)
			
			local whoName = ""
			if whoInstance then  whoName = "(" .. whoInstance.Name ..")" end

			if false then
				printWarning( "Error in ChatStep '" .. chatStep:getName()
				..	"' of the ChatSequence '" .. chatSequence:getName() .. "'"
				.. 	" the field Who '" ..  chatAction.Who .."' is invalid " )
			end

			return nil
				
		end

		if  chatAction.Says ~= "" and chatAction.Says ~= nil
		then
			local param = getRtId(context, tostring(chatAction.Says))
			if param then
				action = r2.Translator.createAction("npc_say", param,   who); 

			--	action = r2.newComponent("RtNpcEventHandlerAction")
			--	action.Action = "npc_say"
			
			--	action.Parameters = who .."\n" .. param
				table.insert(rtMAction.Children, action)
			else
				debugInfo(colorTag(255,255,0).."WRN: Text " .. chatAction.Says .. " error")
			end
	
		end

			-- create emot action
		if tostring(chatAction.Emote) ~= "" and chatAction.Emote ~= nil
		then
			action = r2.newComponent("RtNpcEventHandlerAction")
			action.Action = "emot"
			action.Parameters = tostring(chatAction.Emote).."\n" .. who
			table.insert(rtMAction.Children, action)
		end

		-- create facing action
		if tostring(chatAction.Facing) ~= "" and chatAction.Facing ~= nil
		then
			local facing = r2.Utils.getNpcParam(tostring(chatAction.Facing), context)
			if facing then
				action = r2.newComponent("RtNpcEventHandlerAction")
				action.Action = "facing"
				action.Parameters = facing .."\n" .. who
				table.insert(rtMAction.Children, action)
			else
				printWarning("Can not find npc")
			end
		end
	else
		printWarning( chatAction.Class .. " not implemented yet")
	end
end

Logic.translateChatStep = function(context, chatSequenceNumber, chatSequence, chatStepNumber, chatStep, rtActionSwitchChatStep)
	
	assert(chatStep)
	assert(chatStep.Class == "ChatStep")
	
	if  chatStep.Actions.Size > 0  then

		local rtMAction = r2.Translator.createAction("multi_actions")		
		table.insert(rtActionSwitchChatStep.Children, rtMAction)
		
		local k, chatAction = next(chatStep.Actions, nil)
		while k do
			Logic.translateChatAction(context, chatSequenceNumber, chatStepNumber, chatSequence, chatStep, chatAction, rtMAction)

			local oldChatAction = chatAction
			-- next wait
			do
				
				if (chatStepNumber < chatSequence.Components.Size and chatSequence.Components[chatStepNumber ]) then
				
					local action = r2.Translator.createAction("code", 
						"oldChatStepVar = " .. tostring(chatStepNumber)..";\n"  ..
						Logic.chatStepVar .." = " .. tostring(chatStepNumber+1)..";\n" ..
						"()setTimer(" .. tostring(1+chatSequence.Components[chatStepNumber].Time*10) .. ", " .. Logic.chatTimerId .. ");\n");
					action.Name = "next"
					table.insert(rtMAction.Children, action)
				
				else
					local action = r2.Translator.createAction("code", 
						"oldChatStepVar = " .. tostring(chatStepNumber)..";\n"  ..
						Logic.chatStepVar .." = " .. tostring(chatStepNumber+1)..";\n" ..
						"()setEvent(" ..Logic.EndOfChatSequenceEvent ..");\n" ..
						"if (repeatSequence == 1)\n{\n  " .. Logic.chatSequenceVar .."=" ..Logic.chatSequenceVar ..";\n" ..
						"}\n")

					action.Name = "end_of_chat_sequence"
					table.insert(rtMAction.Children, action)

				end
	
	

			end

			
			k, chatAction = next(chatStep.Actions, k)

		end

		
	else
		local action = r2.Translator.createAction("null_action")
		action.Name = "empty_chat_step_" .. chatStepNumber
		table.insert(rtActionSwitchChatStep.Children, action)
	end

end

Logic.translateChatSequence = function (context, chatSequenceNumber, chatSequence, rtSwitchChatSequence)
	assert(chatSequence)
	assert(chatSequence.Class == "ChatSequence")
	if  chatSequence.Components.Size  > 0  then
		
		local rtActionSwitchChatStep = r2.Translator.createAction("switch_actions", Logic.chatStepVar)
		rtActionSwitchChatStep.Name = "switch_chat_step" 
		table.insert(rtSwitchChatSequence.Children, rtActionSwitchChatStep)
		
		local chatStepNumber = 0
	
		-- initial wait
		do
			local action = r2.Translator.createAction("code", "oldChatStepVar = 0;\n" 
				.. Logic.chatStepVar .." = 1;\n()setTimer(" .. tostring( tonumber(chatSequence.Components[0].Time)*10 + 1) .. ", " .. Logic.chatTimerId ..");\n");
			action.Name = "initial_wait"
			table.insert(rtActionSwitchChatStep.Children, action)
		end

	
		local k, chatStep = next(chatSequence.Components, nil)
		while k do
			chatStepNumber = chatStepNumber + 1
			Logic.translateChatStep(context, chatSequenceNumber, chatSequence, chatStepNumber, chatStep, rtActionSwitchChatStep)
			k, chatStep = next(chatSequence.Components, k)
		end
	else
		local action = r2.Translator.createAction("null_action")
		action.Name = "empty_chat_sequence_" .. chatSequenceNumber
		table.insert(rtSwitchChatSequence.Children, action)
	end
end

Logic.translateChatSequences = function (context, hlComponent, behavior, rtNpcGrp)
	assert(behavior.ChatSequences)
	if behavior.ChatSequences.Size > 0 then
		assert(rtNpcGrp)

		do
			local	event = r2.Translator.createEvent("variable_" ..Logic.chatSequenceVar .."_changed", "", rtNpcGrp.Name)
			event.Name = "activity_sequence_changed"
			table.insert(context.RtAct.Events, event)
			
			local rtInitChatStep = r2.Translator.createAction("code", "oldChatStepVar = -1;\n" .. Logic.chatStepVar .." = 0;\n()setTimer(1, " ..Logic.chatTimerId .. ")\;\n")
			rtInitChatStep.Name = "init_chat_step"
			table.insert(context.RtAct.Actions, rtInitChatStep)
			table.insert(event.ActionsId, rtInitChatStep.Id)
		end
		

		local	event = r2.Translator.createEvent("timer_" ..Logic.chatTimer .."_triggered", "", rtNpcGrp.Name)
		event.Name = "dialog_event"
		table.insert(context.RtAct.Events, event)
			
		local rtSwitchChatSequence = r2.Translator.createAction("switch_actions", Logic.chatSequenceVar)
		rtSwitchChatSequence.Name = "switch_chat_sequence"
		table.insert(context.RtAct.Actions, rtSwitchChatSequence)
		table.insert(event.ActionsId, rtSwitchChatSequence.Id)
		
		local chatSequenceNumber = 0
		local k, chatSequence = next(behavior.ChatSequences, nil)
		while k do
			context.Feature = nil
			chatSequenceNumber = chatSequenceNumber + 1
			Logic.translateChatSequence(context, chatSequenceNumber, chatSequence, rtSwitchChatSequence)	
			k, chatSequence = next(behavior.ChatSequences, k)
		end
	end
end



function Logic.isWanderActivity(activity)
	return activity == "Wander" or activity == "Go To Zone" or activity == "Rest In Zone" or activity == "Feed In Zone"
		or activity == "Hunt In Zone" or activity == "Guard Zone"
end


-------------------------------------------------
--create a transition to go to step number stepNb
--set a timer to go to next step
--select the step
--goto the step's state
Logic.createTransition = function(context,  hlComponent, activitySequence, activityStep, activityStepIndex, stepAction)
	local findChatSequenceIdByInstanceId = function (chat)

		local parent = chat.ParentInstance.ChatSequences
		assert(parent)

		
		local nbSequence = 0
		local k, sequence = next(parent, nil)
		while k do
			if tostring(sequence.InstanceId) == tostring(chat.InstanceId) then return nbSequence end			
 			nbSequence = nbSequence + 1
			k, sequence = next(parent, k)
		end
		return -1
	end

	assert(activityStep)

	local chat = nil


	if activityStep.Chat ~= "" then
		chat = r2:getInstanceFromId( tostring(activityStep.Chat) )
	end

	local code = ""


	local backupState = ""
	local setChatSequence = ""
	local setNextActivity = ""
	local setNextActivityTimer = ""
	local setNextActivityState = ""

	
	backupState =
		"oldChatSequenceVar = " .. Logic.chatSequenceVar .. ";\n" ..
		"RoadCountLimit=0;\n"..
		"oldActivityStepVar = " .. Logic.activityStepVar .. ";\n" ..
		"oldActivityStepVar2 = " .. Logic.activityStepVar.. " + 1;\n"
	
		
	if chat~= nil then		
		local id = findChatSequenceIdByInstanceId( chat )
		assert(id ~= -1 )
		setChatSequence =  Logic.chatSequenceVar .. " = " .. tostring( id ) .. ";\n"		
		if (activityStep.Type == "Repeating") then 
			setChatSequence = setChatSequence .. "repeatSequence = 1;\n" 
		else
			setChatSequence = setChatSequence .. "repeatSequence = 0;\n" 
		end
	end
	
	-- next activity
	setNextActivity =  Logic.activityStepVar .. " = " .. activityStepIndex ..";\n"	

	-- next activity timer
	local param = Logic.getTimeLimit(activityStep)
	if param ~= nil  then
		if Logic.isWanderActivity(activityStep.Activity) then
			-- The time limite is use by the activity
		else
			setNextActivityTimer  = "()setTimer(" .. param ..", " ..Logic.activityStepTimerId .. ");\n"
		end
	end

	local states = Logic.States[activityStep.InstanceId]
	local stateName = states[1].Name

	-- next activity state
	
	setNextActivityState = "()postNextState(\"" .. r2.getNamespace() .. stateName .."\");\n"
	local action = r2.Translator.createAction("code", backupState .. setChatSequence .. setNextActivity .. setNextActivityTimer .. setNextActivityState)


	action.Name = "process_activity_step" .. activityStepIndex
	
	if stepAction then
		local tmpAction = r2.Translator.createAction("multi_actions")
		table.insert(tmpAction.Children, action)
		table.insert(tmpAction.Children, stepAction)		
		action = tmpAction
	end
	
	action.Name = "process_activity_step" .. activityStepIndex
	return action
	
end


------------------------------------------
--crée une action qui permet de 
--sélectionner une séquence de chat
Logic.selectDialog = function(dialogNb, rtNpcGrp)
	local action
	local prefix = ""
	if (rtNpcGrp ~= nil) then
		prefix = r2.getNamespace() .. rtNpcGrp .. "." 
	end
		
	action = r2.Translator.createAction("code",
		prefix .."oldChatSequenceVar =" .. prefix..Logic.chatSequenceVar ..";\n" ..
		prefix .. Logic.chatSequenceVar .." = " ..(dialogNb-1)..";\n")
	action.Name = "select_dialog"
	return action
end

--create the actions and events necessary to choose
--the dialog while running sequence
Logic.initDialog = function(context)
	local event = r2.Translator.createEvent("variable_" ..Logic.chatSequenceVar .."_changed", "all_sequence_states", context.Group.Name)
	event.Name = "change_dialog_event"
	Logic.DialogEvent2 = event
	local action
	local mAction = r2.Translator.createAction("multi_actions")
	action = r2.Translator.createAction("code", "oldChatStepVar = " .. Logic.chatStepVar ..";\n" .. Logic.chatStepVar .." = 0;\n")
	action.Name = "reset_dialog"
	table.insert(mAction.Children, action)
	action = r2.Translator.createAction("switch_actions", Logic.chatSequenceVar)
	action.Name = "init_dialog_timer"
	table.insert(mAction.Children, action)
	Logic.DialogAction2 = action
	table.insert(context.RtAct.Events, event)
	table.insert(context.RtAct.Actions, mAction)
	table.insert(event.ActionsId, mAction.Id)


	event = r2.Translator.createEvent("timer_" ..Logic.chatTimer .."_triggered", "all_sequence_states", context.Group.Name)
	event.Name = "dialog_event"
	Logic.DialogEvent = event
	table.insert(context.RtAct.Events, event)

	Logic.DialogAction = r2.Translator.createAction("switch_actions", Logic.chatSequenceVar)
	table.insert(context.RtAct.Actions, Logic.DialogAction)
	table.insert(event.ActionsId, Logic.DialogAction.Id)
end

Logic.initSequence = function(context, rtNpcGrp)
	assert(context)
	assert(rtNpcGrp)

	event = r2.Translator.createEvent("timer_" ..Logic.activityStepTimer .."_triggered", "all_sequence_states", rtNpcGrp.Name)
	event.Name = "sequence_event"
	Logic.SequenceEvent = event
	table.insert(context.RtAct.Events, event)

	Logic.SequenceAction = r2.Translator.createAction("switch_actions", Logic.activityStepVar)
	table.insert(context.RtAct.Actions, Logic.SequenceAction)
	table.insert(event.ActionsId, Logic.SequenceAction.Id)
end

-- ust to implement translateEvents used to implement event created for implementing 
Logic.translateEvent = function( context,  hlComponent, activitySequence, activityStep,  activityStepIndex, event, rtNpcGrp, statesByName)
	local chat = nil
-- context, groups, states, actionToAdd)
	local eventType, eventParam = Logic.getEvent(event)
	local eventHandler = nil

	if context.Events[event.InstanceId] == nil
	then
		eventHandler = r2.Translator.createEvent(eventType, states, groups)
		assert(eventHandler ~= nil)
			
		local stepAction = Logic.translateChatStep(context, event.Action, nil)
			
		if actionToAdd ~= nil
		then
			table.insert(stepAction.Children, actionToAdd)
		end

		assert(stepAction ~= nil)

		table.insert(context.RtAct.Actions, stepAction)
		table.insert(eventHandler.ActionsId, stepAction.Id)
		context.Events[event.InstanceId] = eventHandler
	else
		eventHandler = context.Events[event.InstanceId]
			
		if string.find(eventHandler.StatesByName, states) == nil
		then
			eventHandler.StatesByName = eventHandler.StatesByName .. "\n" .. states
		end

		if string.find(eventHandler.GroupsByName, groups) == nil
		then
			eventHandler.GroupsByName = eventHandler.GroupsByName .. "\n" .. groups
		end
				
	end

	return eventHandler
end

-- translateEvents
Logic.translateEvents = function(context,  hlComponent, activitySequence, activityStep,  activityStepIndex, rtNpcGrp, statesByName)
	
	local activityStep

	local last = nil

	if activityStepIndex == activitySequence.Components.Size then 
		last = true
	end


	local  k, eventId  = next(step.EventsIds, nil)
	do
		local eventHandler = r2:getInstanceFromId(eventId)
		assert(eventHandler ~= nil)
		local action = nil
		
		if  eventHandler.Condition.Type == "At Destination"	then
			if step.Activity == "Follow Route" and last ~= true then
				action = Logic.selectActivityStep(activityStepIndex + 1)
			end
		end

		--the event returned here is a RtNpcEventHandler
		event = translateEvent(context, groups, states, action)
		assert(event ~= nil)
		
		table.insert(context.RtAct.Events, event)
		k, eventId  = next(step.EventsIds, k)
	end
end

--translate an activity step
Logic.translateActivityStep = function(context,  hlComponent, activitySequence, activityStep,  activityStepIndex, rtNpcGrp)

	--translate this activity activityStep's events
	assert(rtNpcGrp)
	local aiState
	local aiState2
	local statesByName
	local groupsByName = rtNpcGrp.Name
	local zone = nil
	local chat = nil
	local aiScriptDebug = false

	local activityStep = activityStep

	if activityStep.Chat ~= "" then
		chat = r2:getInstanceFromId(activityStep.Chat)
		if not chat then
			printErr("translation error: in the activitySequence " .. activitySequence.Name .. " in component " .. hlComponent.Name .. ": " .. " the chat sequence associated to this activity sequence can not be found")
			return nil
		end
	end
		
	local states = Logic.States[activityStep.InstanceId]
	--states creation
	aiState = states[1] -- 1 is first ( a lua object not a instance )

	--setting state AiMovement
	aiState.AiMovement = Logic.getAiMovement(activityStep.Activity)
	assert(aiState.AiMovement ~= "")
	--setting state zone or road


	local positionalActivity= {["Patrol"]		= "road", 
							["Wander"]			= "zone",
							["Repeat Road"]		= "road",		
							["Follow Route"]	= "road",		
							["Rest In Zone"]	= "zone",
							["Feed In Zone"]	= "zone",	
							["Hunt In Zone"]	= "zone",	
							["Guard Zone"]		= "zone",		
							["Go To Zone"]		= "zone",	
							 }
	if positionalActivity[activityStep.Activity] then
		local zone = r2:getInstanceFromId(activityStep.ActivityZoneId)
		if zone == nil then
			aiState = states[1]
			aiState.AiMovement = "idle"
			return aiState
		else
			if positionalActivity[activityStep.Activity] == "zone" then
				if table.getn(zone.Points) < 3 then
					aiState = states[1]
					aiState.AiMovement = "idle"
					return aiState
				end
			else
				if table.getn(zone.Points) < 2 then
					aiState = states[1]
					aiState.AiMovement = "idle"
					return aiState
				end
			end	
		end
	end

	


	if activityStep.Activity == "Patrol" then
		local zone = r2:getInstanceFromId(activityStep.ActivityZoneId)
		if (zone == nil)
		then
			printErr("translation error: in the activitySequence " .. activitySequence.Name .. " in component " .. hlComponent.Name .. ": " .. " the zone associated to the action can not be found")
			return nil
		end

		--un etat aller, un etat retour 
		aiState2 = states[2]
		aiState2.AiMovement = "follow_route"
		Logic.assignZone(aiState, zone, false)
		Logic.assignZone(aiState2, zone, true)

		statesByName = aiState.Name .."\n" .. aiState2.Name
	elseif activityStep.Activity == "Repeat Road" or  activityStep.Activity == "Follow Route" or  activityStep.Activity == "Wander" 
		or activityStep.Activity == "Rest In Zone" or activityStep.Activity == "Feed In Zone" or activityStep.Activity ==  "Hunt In Zone"
		or activityStep.Activity == "Guard Zone" or activityStep.Activity == "Go To Zone" then
		local zone = r2:getInstanceFromId(activityStep.ActivityZoneId)
		if (zone == nil) then
			printErr("translation error: in the activitySequence " .. activitySequence.Name .. " in component " .. hlComponent.Name .. ": " .. " the zone associated to the action can not be found")
			return nil
		end
		Logic.assignZone(aiState, zone, false)
		statesByName = aiState.Name
	else
		statesByName = aiState.Name
	end


	if activityStep.Activity == "Repeat Road" then
		local action
		local event
		--when at end of the road, ...
		event = r2.Translator.createEvent("destination_reached_all", statesByName, groupsByName)
		table.insert(context.RtAct.Events, event)
		--return to the start of the road
		local repeatCount = activityStep.RoadCountLimit
		if not repeatCount then repeatCount = 2 end

		action = r2.Translator.createAction("next_road", groupsByName, aiState.Name, activityStepIndex ,  repeatCount)
		table.insert(context.RtAct.Actions, action)
		table.insert(event.ActionsId, action.Id)

	elseif activityStep.Activity == "Patrol" then
		local action
		local event
		local repeatCount = activityStep.RoadCountLimit
		if not repeatCount then repeatCount = 2 end
		event = r2.Translator.createEvent("destination_reached_all", aiState.Name, groupsByName)
		table.insert(context.RtAct.Events, event)
		action = r2.Translator.createAction("next_road", groupsByName, aiState2.Name, activityStepIndex,  repeatCount)
		action.Name = "return"
		table.insert(context.RtAct.Actions, action)
		table.insert(event.ActionsId, action.Id)

		event = r2.Translator.createEvent("destination_reached_all", aiState2.Name, groupsByName)
		table.insert(context.RtAct.Events, event)
		action = r2.Translator.createAction("next_road", groupsByName, aiState.Name, activityStepIndex ,  repeatCount)
		action.Name = "go"
		table.insert(context.RtAct.Actions, action)
		table.insert(event.ActionsId, action.Id)

	elseif activityStep.Activity == "Follow Route" or activityStep.Activity == "Go To Start Point" or activityStep.Activity == "Go To Zone" then
		local action
		local event
		event = r2.Translator.createEvent("destination_reached_all", aiState.Name, groupsByName)
		table.insert(context.RtAct.Events, event)
		action =  Logic.selectActivityStep(activityStepIndex + 1)
		table.insert(context.RtAct.Actions, action)
		table.insert(event.ActionsId, action.Id)
	elseif activityStep.Activity == "Rest In Zone" then
		local wanderTime = 100
		local restTime = 300
		r2.Translator.createActivityInZone(context, aiState.Name, groupsByName, "Rest", Logic.activityInternalTimerId, wanderTime, restTime, aiScriptDebug)
	elseif activityStep.Activity == "Feed In Zone" then
		local wanderTime = 50
		local feedTime = 150
		r2.Translator.createActivityInZone(context, aiState.Name, groupsByName, "Eat", Logic.activityInternalTimerId, wanderTime, feedTime, aiScriptDebug)
	elseif activityStep.Activity == "Hunt In Zone" then
		local wanderTime = 100
		local alertTime = 25
		local eatTime = 80
		r2.Translator.createHuntActivityInZone(context, aiState.Name, groupsByName, Logic.activityInternalTimerId, wanderTime, alertTime, eatTime, aiScriptDebug)
	elseif activityStep.Activity == "Guard Zone" then
		r2.Translator.createSimpleActivityInZone(context, aiState.Name, groupsByName, "Alert", false, aiScriptDebug)
	end


	if Logic.isWanderActivity(activityStep.Activity) and activityStep.TimeLimit == "Few Sec" then
		local event = r2.Translator.createEvent("destination_reached_all", aiState.Name, groupsByName)
		table.insert(context.RtAct.Events, event)
		local number = tonumber(activityStep.TimeLimitValue)
		if number == nil then
			number = 0
		end
		local action =  r2.Translator.createAction("wander_destination_reached",  groupsByName, aiState.Name, activityStepIndex, number)
		table.insert(context.RtAct.Actions, action)
		table.insert(event.ActionsId, action.Id)
	end

	local stepAction = nil	
	if activityStep.Activity == "Stand Up" then
		stepAction = r2.Translator.createAction("stand_up", rtNpcGrp.Id )		
	elseif activityStep.Activity == "Sit Down" then
		stepAction = r2.Translator.createAction("sit_down", rtNpcGrp.Id )		
	end
	

	-- translate event create by activitySequence (eg patrol)
	if activityStep.EventsIds.Size ~= 0
	then
		Logic.translateEvents(context,  hlComponent, activitySequence, activityStep,  activityStepIndex, rtNpcGrp, statesByName)
	end


	
	--go to next activityStep
	local action = Logic.createTransition(context,  hlComponent, activitySequence, activityStep, activityStepIndex, stepAction)
	table.insert(Logic.SequenceAction.Children, action)


	-- if ActivityStep.TimeLimit == "Chat" creates a eventhandler to go to next state
	
	if activityStep.TimeLimit == "Chat" then
		local action
		local condition
		local event

		local number = Logic.findChatSequenceIdByInstanceId(hlComponent, chat.InstanceId)	
		if (number == nil or number == -1) then
			printWarning("Error in translation:  in component '" .. hlComponent.Name .. " the chat sequence selected as time limit of Activity Step " .. activityStepIndex .. " does not exist.")

		end
	
		

		event = r2.Translator.createEvent("user_event_8", aiState.Name,  rtNpcGrp.Id)
		table.insert(context.RtAct.Events, event)

	
		
		condition =  r2.Translator.createAction("condition_if", "v0  == "..tostring(number-1))
		table.insert(context.RtAct.Actions, condition)

		
		action =  Logic.selectActivityStep(activityStepIndex + 1)
		table.insert(condition.Children, action)
		table.insert(event.ActionsId, condition.Id)


	end

	assert(State ~= "")
	return aiState
end

---------------------------------------------------------------------------------------
--helpers functions--
---------------------------------------------------------------------------------------


Logic.getAiMovement = function(activity)
	assert(activity and type(activity) == "string")
	if (activity == "Follow Route") or (activity == "Patrol") or (activity == "Repeat Road") then
		return "follow_route"
	elseif activity == "Deploy" then
		return "stand_on_vertices"
	elseif activity == "Wander" or activity == "Go To Zone" or activity == "Rest In Zone" or activity == "Feed In Zone"
		or activity == "Hunt In Zone" or activity == "Guard Zone" then
		return "wander"
	elseif activity == "Stand Still" or activity == "Inactive" or activity == "Stand Up" or activity == "Sit Down" then
		return "idle"
	elseif activity == "Stand On Start Point" or activity == "Go To Start Point" then
		return "stand_on_start_point"
	end
	
	printWarning("Undef activity '"..activity.."'")
	assert(nil)
	return ""
end

Logic.getAction = function(action)
local action_type = action.Action

	if action_type == "Sit Down"
	then
		return "sit_down"
	end
	if action_type == "Stand Up"
	then
		return "stand_up"
	end
	if action_type == "Go To Step"
	then
		if action.Parameter == nil or action.Parameter == ""
		then
			return nil
		end
		return "go_to_step"
	end
	if action_type == "Set Activity"
	then
		if action.Parameter == nil or action.Parameter == ""
		then
			return nil
		end
		return "set_activity"
	end

	if action_type == "Stop Actions"
	then
		return "stop_actions"
	end

	if action_type == "Start Mission"
	then	
		return "start_mission"
	end

	return nil
end

Logic.getEvent = function(Event)
	local i = Logic.find(Logic.EventTypes, Event.Condition.SourceType)
	--Time event type
	if i == 0
	then
	end
	
	--place event type
	if i == 1
	then
	end

	--entity event type
	if i == 2
	then
		if Event.Condition.Type == "At Destination"
		then
			return "destination_reached_all"
		end
		
		if Event.Condition.Type == "Is Attacked"
		then
			return "group_under_attack"
		end

		if Event.Condition.Type == "Enters State"
		then
			return "start_of_state"
		end

		if Event.Condition.Type == "Leaves State"
		then
			return "end_of_state"
		end
		if Event.Condition.Type == "Activity Is"
		then				
		end
	end

	--State Machine event type
	if i == 3
	then
	end

	--Counter event type
	if i == 4
	then
		local counterName = Event.Parameter
		local param
		
		if counterName == nil
		then
			
		end

		if string.find(Event.Condition.Type, "v%d_changed") ~= nil
		then
			return "variable_" ..Event.Condition.Type
		end

		if Event.Condition.Type == "Equals"
		then
			param = " = "
		else
			if Event.Condition.Type == "Lesser"
			then
				param = " < "
			else
				if Event.Condition.Type == "Greater"
				then
					param = " > "
				else
					param = nil
				end
			end
		end
		if param == nil
		then
			return nil
		end
		param = param ..Event.Condition.ConditionParameter

		return "variable_changed", param
	end
end

Logic.checkEvent = function(eventMode, sourceType, sourceId, conditionType, parameter)
	local eventTypeInd = Logic.find(Logic.EventTypes, sourceType)
	local conditionTypeInd
	if eventTypeInd == nil
	then
		return false
	end

	--{"Time", "Place", "Entity or Group", "State Machine", "Counter"}

	conditionTypeInd = Logic.find(Logic.ConditionTypes[eventTypeInd], conditionType)
	if eventTypeInd == nil
	then
		return false
	end

	--Time event type
	if eventTypeInd == 0
	then
	end

	--Place event type
	if eventTypeInd == 1
	then
	end

	--Entity or Group event type
	if eventTypeInd == 2
	then
		if parameter == nil or parameter == ""
		then
			if conditionType == "At Destination" or conditionType == "Is Attacked"
			then
				return true
			else
				return false
			end
		else
			if conditionType == "Enters State" or conditionType == "Leaves State"
			then
				return true
			else
				return false
			end
		end
	end

	--State Machine event type
	if eventTypeInd == 3
	then
	end

	--Counter event type
	if eventTypeInd == 4
	then
		if string.find(conditionType, "v%d_changed") ~= nil
		then
			return true
		end
		if conditionType == "Lesser" or conditionType == "Greater" or conditionType == "Equals"
		then
			return true
		end
	end

	return false

end

Logic.fillCondition = function(condition, sourceType, sourceId, conditionType, conditionParameter)
	condition.SourceType = sourceType
	condition.SourceId = sourceId
	condition.Type = conditionType
	condition.ConditionParameter = conditionParameter
end

Logic.fillEvent = function(event, eventMode, sourceType, sourceId, conditionType, conditionParameter)
	if Logic.checkEvent(eventMode, sourceType, sourceId, conditionType, conditionParameter) == false
	then
		assert(0)
		return false
	end

	Logic.fillCondition(event.Condition, sourceType, sourceId, conditionType, conditionParameter)
	event.EventMode = eventMode
	return true
end
-----------------------------------------
--genere le parametre de l'action set_timer
Logic.getTimeLimit = function(step)
	local param = ""

	if step.TimeLimit == "Few Sec" then
		local limit = tonumber(step.TimeLimitValue)
		if limit == nil then
			limit = 0
		end
		limit = 1 + limit * 10
		param = tostring( limit )
		return param
	end
	return nil
end
	
Logic.getActivityInitialStateName = function(groupId, activityNb, context)
	local group = context.Components[groupId]
	local activity = group.Components[1].Behavior.Activities[activityNb]
	
	if activity == nil
	then
		return nil
	end

	local activityStates = Logic.ActivitiesStates[activity.InstanceId]
	return activityStates[activity.InstanceId][0].Name
end

--return the states used to represent this step activity
Logic.getActivityStepStates = function(activity, stepNb)
	local activityStates = Logic.ActivitiesStates[activity.InstanceId]
	local states = activityStates[activity.Components[stepNb].InstanceId]
	return states
end

Logic.find = function(array, elem)
	local i = 0
	for k, v in pairs(array)
	do
		if k ~="Keys"
		then
			if elem == v
			then
				return i
			end
			i = i + 1
		end
	end
	return nil
end



Logic.findChatSequenceIdByInstanceIdImpl = function(instance, instanceId)
	local behvior = instance:getBehavior()
	local k, sequence = next(behvior.ChatSequences, nil)
	local nbSequence = 0
	while k do
		nbSequence = nbSequence + 1
		if tostring(sequence.InstanceId) == instanceId then return nbSequence end
		 k, sequence = next(behvior.ChatSequences, k)
	end
	return -1
end

Logic.findChatSequenceIdByInstanceId = function(target, instanceId)
	local instance =target
	return Logic.findChatSequenceIdByInstanceIdImpl(instance, instanceId)

end

-- target -> hlNpc or hlNpcGrp
Logic.findChatStepIdByInstanceId = function(target, instanceId)
	assert( type(target) == "userdata")
	local nbSequence = 0
	local nbStep = 0

	local instance = target
	assert(instance ~= nil)
	local behavior = instance:getBehavior()
	assert(behavior ~= nil)
	local k2, sequence = next(behavior.ChatSequences, nil)
	while k2 do

		nbSequence = nbSequence + 1
		nbStep =  0
		local k, step = next(sequence.Components, nil)
		while k do
			nbStep = nbStep  +1
			if tostring(step.InstanceId) == instanceId then return {nbStep, nbSequence} end
			k, step = next(sequence.Components, k)
		end
		 k2, sequence = next(behavior.ChatSequences, k2)
	end
	assert(0)
	return {-1, -1}
end

-- target is a NpcGrp or a Npc or a CustomNpc
Logic.findActivitySequenceIdByInstanceId = function(target, instanceId)
	assert( type(target) == "userdata")
	
	local nbSequence = 0
	local behavior = nil

	behavior = target:getBehavior()

	if behavior == nil then
		debugInfo("Error: try to find activity Sequence on an unknown entity of type " .. target.Class )
		assert(nil)
	end

	local npc = behavior.ParentInstance
	local grp = behavior.ParentInstance.ParentInstance
	if not npc:isGrouped() then grp = nil end
	

	local k, sequence  = next(behavior.Activities, nil)
	while (k ~= nil) do

		nbSequence = nbSequence + 1
		if sequence.InstanceId == instanceId then
			if grp then return {nbSequence, grp} end
			if npc then return {nbSequence, npc} end
			assert(nil)
		end
		k, sequence  = next(behavior.Activities, k)
	end
	return {-1, nil}
end

Logic.findActivityStepIdByInstanceId = function(target, instanceId)
	assert( type(target) == "userdata")
	local nbSequence = 0
	local nbStep = 0
	local comp = target
	local behavior = comp:getBehavior()
	
	local k, sequence = next(behavior.Activities, nil)
	while k do

		if tostring(sequence.InstanceId) == instanceId then
			assert(nil) -- use findActivityStep instead of findactivitySequence
		end

		nbSequence = nbSequence + 1
		nbStep =  0
		local k2, step = next(sequence.Components, nil)
		while (k2) do
			nbStep = nbStep  +1
			if tostring(step.InstanceId) == instanceId then return {nbStep, nbSequence} end
			k2, step = next(sequence.Components, k2)
		end
		k, sequence = next(behavior.Activities, k)
	end
	return {-1, -1}
end





Logic.selectActivityStep = function(stepNb)
	local action 
	stepNb = stepNb - 1

	action  = r2.Translator.createAction("code", 
		Logic.activityStepVar .. " = " .. stepNb ..";\n"
		.. "()setTimer(1, " ..Logic.activityStepTimerId .. ");\n")
	action.Name = "select_activity_step " .. stepNb

	return action
end



----------------------------------------------------------------------------
-- add a line to the event menu

local component = Logic.Components.ChatSequence


----------------------------------------------------------------------------
-- add a line to the event menu
function component:getLogicTranslations()

	local logicTranslations = {
		["ApplicableActions"] = {
			["starts dialog"]			= { menu=i18n.get( "uiR2AA0ChatSeqStart"		):toUtf8(), 
											text=i18n.get( "uiR2AA1ChatSeqStart"		):toUtf8()},
			["stops dialog"]			= { menu=i18n.get( "uiR2AA0ChatSeqStop"			):toUtf8(), 
											text=i18n.get( "uiR2AA1ChatSeqStop"			):toUtf8()},
			["starts chat"]				= { menu=i18n.get( "uiR2AA0ChatStepStart"		):toUtf8(), 
											text=i18n.get( "uiR2AA1ChatStepStart"		):toUtf8()},
			["continues dialog"]				= { menu=i18n.get( "uiR2AA0ChatStepContinue"		):toUtf8(), 
											text=i18n.get( "uiR2AA1ChatStepContinue"		):toUtf8()},
		},
		["Events"] = {	
			["start of dialog"]			= { menu=i18n.get( "uiR2Event0ChatSeqStart"		):toUtf8(), 
											text=i18n.get( "uiR2Event1ChatSeqStart"		):toUtf8()},
			["end of dialog"]			= { menu=i18n.get( "uiR2Event0ChatSeqEnd"		):toUtf8(), 
											text=i18n.get( "uiR2Event1ChatSeqEnd"		):toUtf8()},
			["start of chat"]			= { menu=i18n.get( "uiR2Event0ChatStepStart"	):toUtf8(), 
											text=i18n.get( "uiR2Event1ChatStepStart"	):toUtf8()},
			["end of chat"]				= { menu=i18n.get( "uiR2Event0ChatStepEnd"		):toUtf8(), 
											text=i18n.get( "uiR2Event1ChatStepEnd"		):toUtf8()},
		},
		["Conditions"] = {	
			["is in dialog"]			= { menu=i18n.get( "uiR2Test0ChatSeq"			):toUtf8(), 
											text=i18n.get( "uiR2Test1ChatSeq"			):toUtf8()},
			["is not in dialog"]		= { menu=i18n.get( "uiR2Test0ChatNotSeq"		):toUtf8(), 
											text=i18n.get( "uiR2Test1ChatNotSeq"		):toUtf8()},
			["is in chat"]				= { menu=i18n.get( "uiR2Test0ChatStep"			):toUtf8(), 
											text=i18n.get( "uiR2Test1ChatStep"			):toUtf8()},
		}
	}
	return logicTranslations
end


r2.Features["ActivitySequence"] = Logic

