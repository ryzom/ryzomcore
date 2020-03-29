
local registerFeature = function ()
	local feature={}

	feature.Name="NpcGrpFeature"

	feature.Description="The default feature"

	local NpcGrpFeatureVersion = 1
	feature.Components=
	{		
		NpcGrpFeature = {	
			BaseClass="ActiveLogicEntity",
			Name="NpcGrpFeature",
			Version = NpcGrpFeatureVersion,
			InEventUI = true,			
			--Menu="ui:interface:r2ed_base_menu",
			--Menu="ui:interface:r2ed_entity_menu",
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "groupUIDisplayer",
			DisplayerVisual = "R2::CDisplayerVisualGroup",
			DisplayerProperties = "R2::CDisplayerLua",
			DisplayerPropertiesParams = "npcGroupPropertySheetDisplayer",
			DisplayerVisualParams = 	
			{
				Look = r2.PrimRender.GroupLook,				
				ArrayName = "Components"
			},
			-----------------------------------------------------------------------------------------------	
			Parameters =		{
								},
			ApplicableActions = {
									"Activate", 
									"Deactivate", "Kill", "begin activity sequence",
									"Sit Down", "Stand Up",
									"Fight with player", "Fight with Npcs",
									"Dont fight with player", "Dont fight with Npcs",
									"Run", "Dont run",
									--"begin chat sequence"
								},
			Events =			{
									"activation", 
									"desactivation", "member death", "group death", 	
									"end of activity step", "end of activity sequence",
									"begin of activity step", "begin of activity sequence",
									"targeted by player", 
									
									--"end of chat step", "end of chat sequence"
								},
			Conditions =		{
									
									--"is active", "is inactive", 
									"is in activity sequence",
									"is in activity step", --"is in chat sequence", "is in chat step"
									"is dead", "is alive", 
								},
			TextContexts =		{
									"a member is dead", "a member is dead", "a member is alive", 
									"group is dead", "group is alive"
								},
			TextParameters =	{
									"members number"
								},
			LiveParameters =	{
									"is active", "current activity sequence and activity step", 
									"current chat sequence and chat step"
								},
			-----------------------------------------------------------------------------------------------		
			Prop =
			{				
				{Name="Name", Type="String", MaxNumChar="32"},
				{Name="Components", Type="Table"},				
			},

			TreeIcon= function(this)

				if this.Components.Size>0 and (this.Components[0]:isKindOf("NpcCreature") or this.Components[0]:isKindOf("NpcPlant")) then 
					return "r2ed_icon_group_creatures.tga"
				else
					return "r2ed_icon_group.tga"
				end

				return ""
			end,

			PermanentTreeIcon= function(this)
				if this.Components.Size>0 and (this.Components[0]:isKindOf("NpcCreature") or this.Components[0]:isKindOf("NpcPlant")) then 
					return "r2ed_icon_permanent_group_creatures.tga"
				else
					return "r2ed_icon_permanent_group.tga"
				end

				return ""
			end,

			---------------------------------------------------------------------------------------------------------
			-- get select bar type
			SelectBarType = function(this)
				return i18n.get("uiR2EDScene"):toUtf8()
			end,

			updateVersion = function(this, scenarioValue, currentValue )
				local patchValue = scenarioValue			
				-- version 1 : Remove the "Cost" field -> hold locally now
				if patchValue < 1 then
					r2.requestEraseNode(this.InstanceId, "Cost", -1)				
					patchValue = 1
				end

				if patchValue == currentValue then return true end
				return false
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			isCopyable = function(this)
				return true
			end,
			--------------------------------------------------------------------------------------------
			-- from WorldObject
			canChangeDisplayMode = function(this)
				return true
			end,
			-- from WorldObject
			isDisplayModeToggleSupported = function(this, displayMode)
				return this.Components[0]:isDisplayModeToggleSupported(displayMode)			
			end,
			getAvailableCommands = function(this, dest)				
				r2.Classes.ActiveLogicEntity.getAvailableCommands(this, dest)
				this:getAvailableDisplayModeCommands(dest)							
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			-- additionnal parameter 'srcOptions' gives the options inherited 
			paste = function(src, newPlace, srcInstanceId, srcOptions)
				local options
				if not srcOptions then					
					options = 
					{
						CopyEvents = 0,
						CopyActivities = 0,
						-- CopyChatSequences = 0
						DuplicateGroup = -1 -- option available when duplicating leader only
					}
				end
				local function paramsOk(options)
               if not r2:checkAiQuota(table.getn(src.Components)) then return end
					if options.CopyActivities == 0 then
						src.ActivitiesId = {}
						src.Components[1].Behavior.Activities = {}
					end
					if options.CopyEvents == 0 then
						src.Components[1].Behavior.Actions = {}
					end
					--if options.CopyChatSequences == 0 then
					-- src.Behavior.ChatSequences = {}
					-- end               
					if newPlace then                  
						  -- compute min position and use as group ref pos
						  local mx = 0
						  local my = 0
						  local mz = 0
						  -- compute center
						  for k, v in pairs(src.Components) do
							 v.Position.x = v.Position.x + src.Position.x
							 v.Position.y = v.Position.y + src.Position.y
							 v.Position.z = v.Position.z + src.Position.z
							 mx = mx + v.Position.x
							 my = my + v.Position.y
							 mz = mz + v.Position.z
						  end
						  mx = mx / table.getn(src.Components)
						  my = my / table.getn(src.Components)
						  mz = mz / table.getn(src.Components)
						  -- make relative to center
						  for k, v in pairs(src.Components) do
							 v.Position.x = v.Position.x - mx
							 v.Position.y = v.Position.y - my
							 v.Position.z = v.Position.z - mz
						  end
						  -- compute new center
						  if type(newPlace) == "table" then
							src.Position.x, src.Position.y, src.Position.z = newPlace.x, newPlace.y, newPlace.z
						  else
							src.Position.x, src.Position.y, src.Position.z = r2:getPastePosition()
						  end
					end		
					r2:setCookie(src.InstanceId, "Select", true)				
					-- insert in current act
					r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1,"", src)				
					
				end
				if srcOptions then
					-- if options were given, do not ask the user
					paramsOk(srcOptions)
					return
				end
				local function paramsCancel()
					debugInfo('paste was cancelled')
				end				
				if table.getn(src.Components[1].Behavior.Activities) == 0 then
					options.CopyActivities = -1
				end
				if table.getn(src.Components[1].Behavior.Actions) == 0 then
					options.CopyEvents = -1
				end
				--if table.getn(src.Behavior.ChatSequences) == 0 then
				-- options.CopyChatSequences = -1
				-- end
				if options.CopyActivities >= 0 or
				   options.CopyEvents >= 0
				   --or options.CopyChatSequences >= 0
				then
					r2:doForm("SpecialPaste", options, paramsOk, paramsCancel)
				else
					-- nothing specific to copy, do direct paste
					paramsOk(options)
				end
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			pasteGhost = function(src)	             				
				if not r2:checkAiQuota(table.getn(src.Components)) then return end
				local target = r2:getCurrentAct():getDefaultFeature()				
				-- create the 'Ghosts' entry locally if it doesn't already exists
				if target.Ghosts == nil then
					r2.requestInsertGhostNode(target.InstanceId, "", -1, "Ghosts", {})
				end					
				--				
				r2.requestInsertGhostNode(target.InstanceId, "Ghosts",-1,"", src)				
				-- insertion should have been done right now
				return r2:getInstanceFromId(src.InstanceId)
			end,
			---------------------------------------------------------------------------------------------------------
			-- create a new copy with renaming
			newCopy = function(this)
				local result = r2.Classes.BaseClass.newCopy(this)
				
				local counterNames = {}

				for k, v in pairs(result.Components) do
					local category = r2.getPropertyValue(v, "Category")
					local subCategory = r2.getPropertyValue(v, "SubCategory")
					if category == "Npc" then
						
						if subCategory=="Kami" or subCategory=="Karavan" then
							
							local baseName = r2.PaletteIdToTranslation[this.Components[k-1].Base]
							if counterNames[baseName]==nil then
								local uc_name = ucstring()
								uc_name:fromUtf8(baseName)
								local name = r2:genInstanceName(uc_name):toUtf8()
								counterName = string.gsub(name, tostring(uc_name), "")
								counterNames[baseName] = tonumber(counterName)
							else
								counterNames[baseName] = counterNames[baseName]+1
							end
							
							v.Name = baseName .. " " .. counterNames[baseName]
						else
							local sex
							local sheetClient = r2.getPropertyValue(v, "SheetClient")
							if isR2PlayerMale(sheetClient) then
								sex = r2.male
							else 
								sex = r2.female
							end						
							local race = getR2PlayerRace(sheetClient)
							v.Name = r2:randomNPCName2(race, sex)
						end
					end
				end
				return result
			end,
			---------------------------------------------------------------------------------------------------------
			-- From logic entity
			getCategory = function(this)
				return this.Components[0]:getCategory()
			end,
			---------------------------------------------------------------------------------------------------------
			-- From logic entity
			getSubCategory = function(this)
				return this.Components[0]:getSubCategory()				
			end,
			-----------------------------------------------------------------------------------------------		
			-----------------------------------------------------------------------------------------------		
			-- cut / paste
			accept = function(this, targetInstance)
				return r2:testCanGroup(targetInstance, this)
			end,
			-- 
			insert = function(this, instanceToInsert)
				assert(r2:testCanGroup(instanceToInsert, this))
				r2:group(instanceToInsert, this)
			end,
			---------------------------------------------------------------------------------------------------------
			getFirstSelectableSon = function(this)
				for k = 0, this.Components.Size - 1 do
					if this.Components[k].Selectable then
						return this.Components[k]
					end
				end
				return nil
			end,
			---------------------------------------------------------------------------------------------------------
			isNextSelectable = function(this)
				return true
			end,
			-----------------------------------------------------------------------------------------------		
			-- from base class
			getParentTreeNode = function(this)				
--				if not this.ParentInstance:isKindOf("Act") then									
--					return r2.Classes.BaseClass.getParentTreeNode(this)
--				end							
--				return this:getParentAct():getContentTreeNodes("people")

				if this.Components.Size>0 and (this.Components[0]:isKindOf("NpcCreature") or this.Components[0]:isKindOf("NpcPlant")) then
					return this:getParentAct():getContentTreeNodes("creatures")
				else	
					return this:getParentAct():getContentTreeNodes("people")
				end		
			end,
			---------------------------------------------------------------------------------------------------------
			-- from base class			
			appendInstancesByType = function(this, destTable, kind)
				assert(type(kind) == "string")
				--this:delegate():appendInstancesByType(destTable, kind)
				r2.Classes.BaseClass.appendInstancesByType(this, destTable, kind) 
				for k, component in specPairs(this.Components) do
					component:appendInstancesByType(destTable, kind)
				end
			end,
			---------------------------------------------------------------------------------------------------------
			-- from base class	
			hasScenarioCost = function(this)
				return true
			end,

			pretranslate = function (this, context)
				debugInfo("##pretranslate npcgrp##")
				local feature = this
				local components = this.Components
				-- create one RtNpcGrp by group

				local rtNpcGrp = r2.newComponent("RtNpcGrp")
				table.insert(context.RtAct.NpcGrps, rtNpcGrp)
				-- register the groupe
				context.RtGroups[feature.InstanceId] = rtNpcGrp
				context.RtGroups[feature.InstanceId].Name = rtNpcGrp.Id

				-- register all group components
				local key, comp = next(components, nil)
				while (key ~= nil) do
					context.RtGroups[comp.InstanceId] = rtNpcGrp
					context.RtGroups[comp.InstanceId].Name = rtNpcGrp.Id
					key, comp = next(components, key)
				end
			end,
			pretranslate2 = function(this, context)
				--
				context.Feature = this
				--
				local scenario = context.Scenario
				
				local components = context.Feature.Components
				local leader = components[0]
				local hlComponent = context.Feature

				
				assert(components.Size >= 1)
				
				local rtNpcGrp = r2.Translator.getRtGroup(context, context.Feature.InstanceId)
		
				-- translate actionHandlers
				local aiActivity = r2.Translator.getAiActivity(leader)
				r2.Translator.translateActivities(context, hlComponent,  hlComponent:getBehavior(), rtNpcGrp, aiActivity)

			end,
			translate = function (this, context)
				--
				context.Feature = this
				--
				local scenario = context.Scenario
				
				local components = context.Feature.Components
				local leader = components[0]
				local hlComponent = context.Feature

				
				assert(components.Size >= 1)
				
				local rtNpcGrp = r2.Translator.getRtGroup(context, context.Feature.InstanceId)
				--if there's no sequence for the group,
				--create a state with no movement, and put the group in it.
				local key, comp = next(components, nil)
				while key do
					if (comp.isKindOf and comp:isKindOf( "Npc") ) then
						context.Component = comp

						-- insert Npc
						local rtNpc = r2.Translator.translateNpc(comp, context)
						table.insert(context.RtAct.Npcs, rtNpc)
						table.insert(rtNpcGrp.Children, rtNpc.Id)

					end
					key, comp = next(components, key)
				end
				
				-- dump every action of the ai
				-- r2.dumpAi(rpcGrp)		
				r2.Translator.setGroupParameters (leader, rtNpcGrp)
				-- translate actionHandlers
			--	local aiActivity = r2.Translator.getAiActivity(leader)
			--	r2.Translator.translateActivities(context, hlComponent,  hlComponent:getBehavior(), rtNpcGrp, aiActivity)

				-- set eventHandlers
				r2.Translator.translateEventHandlers(context, hlComponent, hlComponent:getBehavior().Actions, rtNpcGrp)
				--> events = leader or npc				
			end
		}
	}

	-- same for group and for npc
	local component = feature.Components.NpcGrpFeature
	component.getLogicCondition = r2.Translator.getNpcLogicCondition
	component.getLogicAction = r2.Translator.getNpcLogicAction
	component.getLogicEvent = r2.Translator.getNpcLogicEvent

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function component:getLogicTranslations()

		local logicTranslations = {
			["ApplicableActions"] = {
				["Activate"]					= { menu=i18n.get( "uiR2AA0Spawn"					):toUtf8(), 
													text=i18n.get( "uiR2AA1Spawn"					):toUtf8()},
				["Deactivate"]					= { menu=i18n.get( "uiR2AA0Despawn"					):toUtf8(), 
													text=i18n.get( "uiR2AA1Despawn"					):toUtf8()},
				["Sit Down"]					= { menu=i18n.get( "uiR2AA0NpcSit"					):toUtf8(), 
													text=i18n.get( "uiR2AA1NpcSit"					):toUtf8(),
													groupIndependant=true},
				["Stand Up"]					= { menu=i18n.get( "uiR2AA0NpcStand"				):toUtf8(), 
													text=i18n.get( "uiR2AA1NpcStand"				):toUtf8(),
													groupIndependant=true},
				["Kill"]						= { menu=i18n.get( "uiR2AA0Kill"					):toUtf8(), 
													text=i18n.get( "uiR2AA1Kill"					):toUtf8()},
				["begin activity sequence"]		= { menu=i18n.get( "uiR2AA0BeginSeq"				):toUtf8(), 
													text=i18n.get( "uiR2AA1BeginSeq"				):toUtf8()},
				["Fight with player"]			= { menu=i18n.get( "uiR2AA0FlagFightPlayersOn"		):toUtf8(), 
													text=i18n.get( "uiR2AA1FlagFightPlayersOn"		):toUtf8()},
				["Dont fight with player"]		= { menu=i18n.get( "uiR2AA0FlagFightPlayersOff"		):toUtf8(), 
													text=i18n.get( "uiR2AA1FlagFightPlayersOff"		):toUtf8()},
				["Fight with Npcs"]				= { menu=i18n.get( "uiR2AA0FlagFightNpcsOn"			):toUtf8(), 
													text=i18n.get( "uiR2AA1FlagFightNpcsOn"			):toUtf8()},
				["Dont fight with Npcs"]		= { menu=i18n.get( "uiR2AA0FlagFightNpcsOff"		):toUtf8(), 
													text=i18n.get( "uiR2AA1FlagFightNpcsOff"		):toUtf8()},
				["Run"]							= { menu=i18n.get( "uiR2AA0FlagRunOn"				):toUtf8(), 
													text=i18n.get( "uiR2AA1FlagRunOn"				):toUtf8()},
				["Dont run"]					= { menu=i18n.get( "uiR2AA0FlagRunOff"				):toUtf8(), 
													text=i18n.get( "uiR2AA1FlagRunOff"				):toUtf8()},
			},
			["Events"] = {	
				["activation"]					= { menu=i18n.get( "uiR2Event0Spawn"				):toUtf8(), 
													text=i18n.get( "uiR2Event1Spawn"				):toUtf8()},
				["desactivation"]				= { menu=i18n.get( "uiR2Event0Despawn"				):toUtf8(),  
													text=i18n.get( "uiR2Event1Despawn"				):toUtf8()},
				["member death"]				= { menu=i18n.get( "uiR2Event0MemberDeath"			):toUtf8(),  
													text=i18n.get( "uiR2Event1MemberDeath"			):toUtf8()},
				["group death"]					= { menu=i18n.get( "uiR2Event0GroupDeath"			):toUtf8(), 
													text=i18n.get( "uiR2Event1GroupDeath"			):toUtf8()},
				["end of activity step"]		= { menu=i18n.get( "uiR2Event0EndActivityStep"		):toUtf8(), 
													text=i18n.get( "uiR2Event1EndActivityStep"		):toUtf8()},
				["end of activity sequence"]	= { menu=i18n.get( "uiR2Event0EndActivitySeq"		):toUtf8(), 
													text=i18n.get( "uiR2Event1EndActivitySeq"		):toUtf8()},
				["begin of activity step"]		= { menu=i18n.get( "uiR2Event0BeginActivityStep"	):toUtf8(), 
													text=i18n.get( "uiR2Event1BeginActivityStep"	):toUtf8()},
				["begin of activity sequence"]	= { menu=i18n.get( "uiR2Event0BeginOfActivitySeq"	):toUtf8(), 
													text=i18n.get( "uiR2Event1BeginOfActivitySeq"	):toUtf8()},
				["targeted by player"]			= { menu=i18n.get( "uiR2Event0TargetedByPlayer"		):toUtf8(), 
													text=i18n.get( "uiR2Event1TargetedByPlayer"		):toUtf8()},
			},
			["Conditions"] = {	
				["is active"]					= { menu=i18n.get( "uiR2Test0Spawned"				):toUtf8(), 
													text=i18n.get( "uiR2Test1Spawned"				):toUtf8()},
				["is inactive"]					= { menu=i18n.get( "uiR2Test0Despawned"				):toUtf8(), 
													text=i18n.get( "uiR2Test1Despawned"				):toUtf8()},
				["is dead"]						= { menu=i18n.get( "uiR2Test0Dead"					):toUtf8(), 
													text=i18n.get( "uiR2Test1Dead"					):toUtf8()},
				["is alive"]					= { menu=i18n.get( "uiR2Test0Alive"					):toUtf8(), 
													text=i18n.get( "uiR2Test1Alive"					):toUtf8()},
				["is in activity sequence"]		= { menu=i18n.get( "uiR2Test0Seq"					):toUtf8(), 
													text=i18n.get( "uiR2Test1Seq"					):toUtf8()},
				["is in activity step"]			= { menu=i18n.get( "uiR2Test0Step"					):toUtf8(), 
													text=i18n.get( "uiR2Test1Step"					):toUtf8()},
			}
		}
		return logicTranslations
	end



	function component.getActivitiesIds(this)
		local activitiesIds = {}

		local behavior = this:getBehavior()
		local k, v = next(behavior.Activities, nil)
		while k do
			table.insert(activitiesIds, v.InstanceId)
			k, v = next(behavior.Activities, k)
		end
				
		return activitiesIds
	end

	function component.getAiCost(this)
		if this.User.GhostDuplicate then return 0 end
		return r2.getAiCost(this) - 1
	end


	-- obsolete
	feature.getCost = function (featureInstance)
		local cost = 0
		local components = featureInstance.Components
		local key, comp = next(components, nil)
		while(key ~= nil)
		do			
	
			if (comp.Class == "Npc" or comp.Class == "NpcCustom")
			then
				cost = cost +1
			end
			key, comp = next(components, key)
		end
		return cost
		
	end
	return feature
end


r2.Features["NpcGrpFeature"] = registerFeature()



--------------------------------------------------------------------------------------------------
-------------------------- NPC GROUP DisplayerProperties -----------------------------------------
--------------------------------------------------------------------------------------------------

local npcGroupPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onErase(instance)
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onPostHrcMove(instance)

	for i=0, instance.Components.Size-1 do
		local entity = instance.Components[i]
		entity.DisplayerVisual:updateName()
		entity:updatePermanentStatutIcon()
	end	
	r2.events:updateElementsUI()	
end
------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onSelect(instance, isSelected)	
	r2:activeLogicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
end

------------------------------------------------
function npcGroupPropertySheetDisplayerTable:onAttrModified(instance, attributeName)
	r2:activeLogicEntityPropertySheetDisplayer():onAttrModified(instance, attributeName)
end	

------------------------------------------------
function r2:npcGroupPropertySheetDisplayer()	
	return npcGroupPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end




