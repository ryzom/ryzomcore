local registerFeature = function ()
	local feature={}

	feature.Name="DefaultFeature"

	feature.Description="The default feature"

	feature.Components=
	{
		ActiveLogicEntity = {
			Name="ActiveLogicEntity",		
			BaseClass="LogicEntity",
			DisplayerVisual = "R2::CDisplayerVisualEntity",
			DisplayerVisualParams = { InheritDisplayMode = true },
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "defaultUIDisplayer",
			DisplayerProperties = "R2::CDisplayerLua",
			DisplayerPropertiesParams = "activeLogicEntityPropertySheetDisplayer",
			Menu="ui:interface:r2ed_entity_menu",

			Prop=
			{										
				{Name="Behavior", Type="Behavior"},
				{Name="ActivitiesId",Type="Table" },
			},
			isNextSelectable = function(this)
				return true
			end,
			---------------------------------------------------------------------------------------------------------
			-- get list of command for display in the mini toolbar
			getAvailableMiniCommands = function(this, result)
				-- OBSOLETE
				--local result = this:delegate():getAvailableMiniCommands(this)
				r2.Classes.LogicEntity.getAvailableMiniCommands(this, result)			
			end,
			--------------------------------------------------------------------------------------------
			-- Test if this entity is a bot object
			isBotObject = function(this)								
				return false
			end,
			--------------------------------------------------------------------------------------------
			-- Test if thisentity is a plant
			isPlant = function(this)							
				return false
			end,
			--------------------------------------------------------------------------------------------
			-- is it a named entity ?
			isNamed = function(this)
				if this.IsNamed and this.IsNamed == 1 then return true end
				return false
			end,
			--------------------------------------------------------------------------------------------
			addPrimitiveActivities = function (this, dest, activityWnd)
				if activityWnd then
					table.insert(dest, "Wander")
					table.insert(dest, "Follow Route")
					table.insert(dest, "Patrol")
					table.insert(dest, "Repeat Road")
					table.insert(dest, "Stand Still")
				else
					table.insert(dest, this:buildActivityCommand(this.onPickWanderZone, "wander_zone", "uimR2EDMenuPickZone", "r2_toolbar_wander_zone.tga", true))
					table.insert(dest, this:buildActivityCommand(this.onPickFollowRoute, "follow_route", "uimR2EDMenuFollowRoute", "r2_toolbar_follow_road.tga", false))
					table.insert(dest, this:buildActivityCommand(this.onPickPatrolRoute, "patrol_route", "uimR2EDMenuPatrolRoute", "r2_toolbar_patrol_road.tga", false))
					table.insert(dest, this:buildActivityCommand(this.onPickRepeatRoute, "repeat_route", "uimR2EDMenuRepeatRoute", "r2_toolbar_repeat_road.tga", false))
					table.insert(dest, this:buildActivityCommand(this.onStandStill, "stand_still", "uimR2EDMenuStandInPlace", "r2_toolbar_stand_still.tga", false))
				end
			end,	

			-- from 'BaseClass'
			getAvailableCommands = function(this, dest, activityWnd)
				
				--local result = this:delegate():getAvailableCommands(this)
				if not activityWnd then
					r2.Classes.LogicEntity.getAvailableCommands(this, dest)
				end
				local category = this:getCategory()

				if category == "Herbivore" then
					if this:isNamed() then

						if activityWnd then
							table.insert(dest, "Guard Zone")
							table.insert(dest, "Follow Route")
							table.insert(dest, "Patrol")
							table.insert(dest, "Repeat Road")
							table.insert(dest, "Stand Still")	
						else
							table.insert(dest, this:buildActivityCommand(this.onPickGuardZone, "guard_zone", "uimR2EDMenuPickGuardZone", "r2ed_toolbar_guard_zone.tga", true))
							table.insert(dest, this:buildActivityCommand(this.onPickFollowRoute, "follow_route", "uimR2EDMenuFollowRoute", "r2_toolbar_follow_road.tga", false))
							table.insert(dest, this:buildActivityCommand(this.onPickPatrolRoute, "patrol_route", "uimR2EDMenuPatrolRoute", "r2_toolbar_patrol_road.tga", false))
							table.insert(dest, this:buildActivityCommand(this.onPickRepeatRoute, "repeat_route", "uimR2EDMenuRepeatRoute", "r2_toolbar_repeat_road.tga", false))
							table.insert(dest, this:buildActivityCommand(this.onStandStill, "stand_still", "uimR2EDMenuStandInPlace", "r2_toolbar_stand_still.tga", false))	
						end
					else

						if activityWnd then
							table.insert(dest, "Rest In Zone")
							table.insert(dest, "Feed In Zone")
						else
							table.insert(dest, this:buildActivityCommand(this.onPickRestZone, "rest_zone", "uimR2EDMenuPickRestZone", "r2ed_toolbar_rest_zone.tga", true))
							table.insert(dest, this:buildActivityCommand(this.onPickFeedZone, "feed_zone", "uimR2EDMenuPickFeedZone", "r2ed_toolbar_feed_zone.tga", false))
						end
					end
				elseif category == "Carnivore" then
					if this:isNamed() then

						if activityWnd then
							table.insert(dest, "Guard Zone")
							table.insert(dest, "Follow Route")
							table.insert(dest, "Patrol")
							table.insert(dest, "Repeat Road")
							table.insert(dest, "Stand Still")	
						else
							table.insert(dest, this:buildActivityCommand(this.onPickGuardZone, "guard_zone", "uimR2EDMenuPickGuardZone", "r2ed_toolbar_guard_zone.tga", true))
							table.insert(dest, this:buildActivityCommand(this.onPickFollowRoute, "follow_route", "uimR2EDMenuFollowRoute", "r2_toolbar_follow_road.tga", false))
							table.insert(dest, this:buildActivityCommand(this.onPickPatrolRoute, "patrol_route", "uimR2EDMenuPatrolRoute", "r2_toolbar_patrol_road.tga", false))
							table.insert(dest, this:buildActivityCommand(this.onPickRepeatRoute, "repeat_route", "uimR2EDMenuRepeatRoute", "r2_toolbar_repeat_road.tga", false))
							table.insert(dest, this:buildActivityCommand(this.onStandStill, "stand_still", "uimR2EDMenuStandInPlace", "r2_toolbar_stand_still.tga", false))	
						end
					else

						if activityWnd then
							table.insert(dest, "Rest In Zone")
							table.insert(dest, "Hunt In Zone")
						else
							table.insert(dest, this:buildActivityCommand(this.onPickRestZone, "rest_zone", "uimR2EDMenuPickRestZone", "r2ed_toolbar_rest_zone.tga", true))
							table.insert(dest, this:buildActivityCommand(this.onPickHuntZone, "hunt_zone", "uimR2EDMenuPickHuntZone", "r2ed_toolbar_hunt_zone.tga", false))
						end
					end
					--table.insert(dest, this:buildActivityCommand(this.onPickGuardZone, "guard_zone", "uimR2EDMenuPickGuardZone", "r2ed_toolbar_guard_zone.tga", true))
				elseif category == "WorkerKitin" then

					if activityWnd then
						table.insert(dest, "Feed In Zone")
						table.insert(dest, "Follow Route")
						table.insert(dest, "Patrol")
						table.insert(dest, "Repeat Road")
						table.insert(dest, "Stand Still")
						--table.insert(dest, "Guard Zone")
					else
						table.insert(dest, this:buildActivityCommand(this.onPickFeedZone, "feed_zone", "uimR2EDMenuPickWorkZone", "r2ed_toolbar_work_zone.tga", true))
						table.insert(dest, this:buildActivityCommand(this.onPickFollowRoute, "follow_route", "uimR2EDMenuFollowRoute", "r2_toolbar_follow_road.tga", false))
						table.insert(dest, this:buildActivityCommand(this.onPickPatrolRoute, "patrol_route", "uimR2EDMenuPatrolRoute", "r2_toolbar_patrol_road.tga", false))
						table.insert(dest, this:buildActivityCommand(this.onPickRepeatRoute, "repeat_route", "uimR2EDMenuRepeatRoute", "r2_toolbar_repeat_road.tga", false))
						table.insert(dest, this:buildActivityCommand(this.onStandStill, "stand_still", "uimR2EDMenuStandInPlace", "r2_toolbar_stand_still.tga", false))
						--table.insert(dest, this:buildActivityCommand(this.onPickGuardZone, "guard_zone", "uimR2EDMenuPickGuardZone", "r2ed_toolbar_guard_zone.tga", true))
					end
				elseif category == "SoldierKitin" then
					if activityWnd then
						table.insert(dest, "Guard Zone")
						table.insert(dest, "Follow Route")
						table.insert(dest, "Patrol")
						table.insert(dest, "Repeat Road")
						table.insert(dest, "Stand Still")					
					else
						table.insert(dest, this:buildActivityCommand(this.onPickGuardZone, "guard_zone", "uimR2EDMenuPickGuardZone", "r2ed_toolbar_guard_zone.tga", true))
						table.insert(dest, this:buildActivityCommand(this.onPickFollowRoute, "follow_route", "uimR2EDMenuFollowRoute", "r2_toolbar_follow_road.tga", false))
						table.insert(dest, this:buildActivityCommand(this.onPickPatrolRoute, "patrol_route", "uimR2EDMenuPatrolRoute", "r2_toolbar_patrol_road.tga", false))
						table.insert(dest, this:buildActivityCommand(this.onPickRepeatRoute, "repeat_route", "uimR2EDMenuRepeatRoute", "r2_toolbar_repeat_road.tga", false))
						table.insert(dest, this:buildActivityCommand(this.onStandStill, "stand_still", "uimR2EDMenuStandInPlace", "r2_toolbar_stand_still.tga", false))					
					end
				elseif not this:isBotObject() and not this:isPlant() then
					-- activity (only if not a plant)
					this:addPrimitiveActivities(dest, activityWnd)
				end					
			end,

			-- for activities UI
			getAvailableActivities = function(this, dest)
				r2.Classes.ActiveLogicEntity.getAvailableCommands(this, dest, true)
			end,
			--------------------------------------------------------------------------------------------
			-- Called when the menu is displayed
			onSetupMenu = function(this)

				--this:delegate():onSetupMenu()
				r2.Classes.LogicEntity.onSetupMenu(this)
				local class = r2:getClass(this)
				local isBO = this:isBotObject()
				local isPlant = this:isPlant()
				getUI(class.Menu .. ":activities").active = not  isBO and not isPlant
				
				if not  isBO and not isPlant  then
					getUI(class.Menu .. ":activities").uc_hardtext = i18n.get("uimR2EDNewActivity")
				end
			end,
			--------------------------------------------------------------------------------------------
			-- function to change activity
			onStandStill = function(this)
				r2:setNPCStandInPlace(this)				
			end,
			onPickWanderZone = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickZoneForNPC|PickFunc=r2:affectZoneToSelectedNPC|PickPosFunc=r2:createZoneAndAffectZoneToNPC|WantMouseUp=true|IgnoreInstances=Npc,Road")
				r2.ContextualCommands:highlightCommandButton("wander_zone")
			end,
			onPickRestZone = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickZoneForNPC|PickFunc=r2:affectRestZoneToSelectedNPC|PickPosFunc=r2:createRestZoneAndAffectZoneToNPC|WantMouseUp=true|IgnoreInstances=Npc,Road")
				r2.ContextualCommands:highlightCommandButton("rest_zone")
			end,
			onPickFeedZone = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickZoneForNPC|PickFunc=r2:affectFeedZoneToSelectedNPC|PickPosFunc=r2:createFeedZoneAndAffectZoneToNPC|WantMouseUp=true|IgnoreInstances=Npc,Road")
				r2.ContextualCommands:highlightCommandButton("feed_zone")	
			end,
			onPickHuntZone = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickZoneForNPC|PickFunc=r2:affectHuntZoneToSelectedNPC|PickPosFunc=r2:createHuntZoneAndAffectZoneToNPC|WantMouseUp=true|IgnoreInstances=Npc,Road")
				r2.ContextualCommands:highlightCommandButton("hunt_zone")	
			end,
			onPickGuardZone = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickZoneForNPC|PickFunc=r2:affectGuardZoneToSelectedNPC|PickPosFunc=r2:createGuardZoneAndAffectZoneToNPC|WantMouseUp=true|IgnoreInstances=Npc,Road")
				r2.ContextualCommands:highlightCommandButton("guard_zone")	
			end,
			onPickFollowRoute = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickRoadForNPC|PickFunc=r2:setBehaviorFollowRouteToNPC|PickPosFunc=r2:createRouteAndSetBehaviorFollowRouteToNPC|WantMouseUp=true|IgnoreInstances=Npc, Region ")
				r2.ContextualCommands:highlightCommandButton("follow_route")	
			end,
			onPickPatrolRoute = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickRoadForNPC|PickFunc=r2:setBehaviorPatrolRouteToNPC|PickPosFunc=r2:createRouteAndSetBehaviorPatrolRouteToNPC|WantMouseUp=true|IgnoreInstances=Npc,Region")
				r2.ContextualCommands:highlightCommandButton("patrol_route")
			end,
			onPickRepeatRoute = function(this)
				runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2:testCanPickRoadForNPC|PickFunc=r2:setBehaviorRepeatRoadToNPC|PickPosFunc=r2:createRoadAndSetBehaviorRepeatRoadToNPC|WantMouseUp=true|IgnoreInstances=Npc,Region")
				r2.ContextualCommands:highlightCommandButton("repeat_route")
			end,
			---------------------------------------------------------------------------------------------------------
			-- From base class
			isSequencable = function(this)
				return not this:isPlant() and not this:isBotObject()
			end,

			---------------------------------------------------------------------------------------------------------
			-- From base class
			getActivityVerbLookupName = function (this, activityName)
				if this.Category == "WorkerKitin" and activityName == "Feed In Zone" then
					return "Work In Zone"
				end
				return activityName
			end,
			initEventValuesMenu = function(this, menu, categoryEvent)

				-- activity sequences
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

					elseif r2.events.eventTypeWithValue[eventType]~=nil then
						menu:addSubMenu(ev)
						local subMenu = menu:getSubMenu(ev)

						for s=0, this:getBehavior().Activities.Size-1 do
							local sequence = this:getBehavior().Activities[s]
							local func = ""
							if r2.events.eventTypeWithValue[eventType]=="ActivitySequence" then
								func = "r2.events:setEventValue('".. sequence.InstanceId .."','" .. categoryEvent .."')"
							end

							local uc_name = ucstring()
							uc_name:fromUtf8(sequence:getName())
							subMenu:addLine(uc_name, "lua", func, sequence.InstanceId)
						end

						if this:getBehavior().Activities.Size==0 then
							subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
						
						-- activity steps
						elseif r2.events.eventTypeWithValue[eventType]=="ActivityStep" then

							for s=0,subMenu:getNumLine()-1 do
								local sequenceId = tostring(subMenu:getLineId(s))
								local sequence = r2:getInstanceFromId(sequenceId)
								assert(sequence)

								subMenu:addSubMenu(s)
								local activitiesMenu = subMenu:getSubMenu(s)
								
								for a=0, sequence.Components.Size-1 do
									local activity = sequence.Components[a]
									local uc_name = ucstring()
									uc_name:fromUtf8(activity:getShortName())
									activitiesMenu:addLine(uc_name, "lua", 
										"r2.events:setEventValue('".. activity.InstanceId .."','" .. categoryEvent .."')", activity.InstanceId)
								end

								-- no activity in the sequence
								if sequence.Components.Size==0 then
									activitiesMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
								end
							end
						end
					end
				end
			end,
		},

		Behavior = {
			Name="Behavior",
			BaseClass="LogicEntityBehavior",
			Prop=
			{				
				{Name="Type", Type="String",DefaultValue=""},--TEMP
				{Name="ZoneId", Type="String"},--TEMP
				{Name="Activities",Type="Table"},
				{Name="ChatSequences",Type="Table"}

			},
		},
		------------------------------------------------------------------------------------------------------
		Npc = {
			PropertySheetHeader = r2.DisplayNpcHeader(),
			Name="Npc",	
			InEventUI = true,	
			BaseClass="ActiveLogicEntity",
			DisplayerVisual = "R2::CDisplayerVisualEntity",
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "defaultUIDisplayer",
			DisplayerProperties = "R2::CDisplayerLua",
			DisplayerPropertiesParams = "npcPropertySheetDisplayer",
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
									-- "emits user event",
								},
			Events =			{
									"activation", 
									"desactivation", "death", 	
									"end of activity step", "end of activity sequence",
									"begin of activity step", "begin of activity sequence",
									-- "user event emitted",
									
									 "targeted by player"
								},
			Conditions =		{
									--"is dead", "is alive", "is active", "is inactive", 
									"is dead", "is alive", 
									"is in activity sequence",
									"is in activity step", 
								},
			TextContexts =		{
									"NPC is dead", "NPC is alive"
								},
			TextParameters =	{
								},
			LiveParameters =	{
									"is active", "current activity sequence and activity step", 
									"current chat sequence and chat step"
								},			
			-----------------------------------------------------------------------------------------------
			Prop=
			{								
				{Name="Base", Type="String", WidgetStyle="StaticText", Category="Advanced", Visible=false},
				{Name="Name", Type="String", DefaultInBase=1, MaxNumChar="32"},
				{Name="Angle", Type="Number", 
					WidgetStyle="Slider", Min="0", Max="360",
					--------------------
					convertToWidgetValue = 
					function(value)						
						local result = math.fmod(math.floor(180 * value / math.pi), 360)
						if result < 0 then result = 360 + result end
						return result
					end,
					--------------------
					convertFromWidgetValue =
					function(value) 
						return math.pi * math.min(359, value) / 180
					end,
				},
				{	Name="PlayerAttackable", Type="Number", WidgetStyle="Boolean", Default="0", DefaultInBase=1,
					Visible=function(this) return this:isGroupedAndLeader() or not this:isGrouped() and not this:isBotObject() and this:canUpdatePlayerAttackable() end
				},
				{
					Name="BotAttackable", Type="Number", WidgetStyle="Boolean", Default="0", DefaultInBase=1,
					Visible=function(this) return this:isGroupedAndLeader() or not this:isGrouped() and not this:isBotObject() and this:canUpdateBotAttackable() end
				},
--				{
--					Name="UseFame", Type="Number", WidgetStyle="Boolean", Default="0", DefaultInBase=1,
--					Visible=function(this) return  this.SubCategory and ( this.SubCategory == "Kami" or this.SubCategory == "Karavan") end
--				},

				{
					Name="AutoSpawn", Type="Number", WidgetStyle="Boolean", Default="0", DefaultInBase=1,
					--Visible=function(this) return not this:isBotObject() end
					Visible = function(this) return this:isGroupedAndLeader() or not this:isGrouped() and not this:isBotObject() end
				},
				{
					Name="NoRespawn", Type="Number", WidgetStyle="Boolean", Default="0", DefaultInBase=1,
					Visible=function(this) return this:isGroupedAndLeader() or not this:isGrouped() and not this:isBotObject() end
				},
				{Name="Aggro", Type="Number",  Min="0", Max="120", DefaultValue="30", DefaultInBase=1, 
					Visible=function(this) return this:isGroupedAndLeader() or not this:isGrouped() and not this:isBotObject() end
				},
				{Name="TypeNPC", Type="Number", WidgetStyle="EnumDropDown", SecondRequestFunc=r2.updateType,
					Enum= {}, Visible=true, DefaultValue="-1",
				},	

				--
				--
				--{Name="TestRefId", Type="RefId", Category="uiR2EDRollout_Test"},
			},

			


			isGroupedAndLeader = function(this)
				if this:isGrouped() and this:isLeader() then return true end
				return false
			end,

			TreeIcon= function(this)

				if this:isKindOf("NpcCreature") or this:isKindOf("NpcPlant") then 
					return "r2ed_icon_creatures.tga"
				elseif not this:isBotObject() then
					return "r2ed_icon_npc.tga"
				end

				return ""
			end,

			PermanentTreeIcon= function(this)
				if this:isKindOf("NpcCreature") or this:isKindOf("NpcPlant") then 
					return "r2ed_icon_permanent_creatures.tga"
				elseif not this:isBotObject() then
					return "r2ed_permanent_node.tga"
				end

				return ""
			end,

			---------------------------------------------------------------------------------------------------------
			-- get select bar type
			SelectBarType = function(this)
				if not this:isBotObject() then
					return i18n.get("uiR2EDScene"):toUtf8()
				else
					return i18n.get("uiR2EDbotObjects"):toUtf8()
				end
			end,

			---------------------------------------------------------------------------------------------------------
			-- from base class
			getContextualTreeIcon = function(this)
				if this:getParentAct():isBaseAct() then
					return this:getPermanentTreeIcon()
				end
				return ""
			end,

			getSelectBarIcon = function(this)
				if  this:isBotObject() then 
					return "r2ed_icon_botobject.tga"
				else
					return r2.Classes.BaseClass.getContextualTreeIcon(this)
				end
			end,

			----------------------------------------------
			updatePermanentStatutIcon = function(this)
				--this.DisplayerVisual:updatePermanentStatutIcon(this:getContextualTreeIcon())
				this.DisplayerVisual:updatePermanentStatutIcon(this:getPermanentStatutIcon())
			end,

			--------------------------------------------------------------------------------------------
			onPostCreate= function(this)
				if this.BoxSelection == 1 and this.DisplayerVisual ~= nil then -- read in palette
					this.DisplayerVisual.SelectionDisplayMode = 1
				end
				if this:isBotObject() then
					this.DisplayerVisual.DisplayMode = select(r2.BotObjectsFrozen, 2, 0)				
				end
			end,	
			onActChanged = function(this)
				if this:isBotObject() then
					this.DisplayerVisual.DisplayMode = select(r2.BotObjectsFrozen, 2, 0)				
				end
			end,
			--------------------------------------------------------------------------------------------
			-- from WorldObject
			isDisplayModeToggleSupported = function(this, displayMode)
				if not this:isBotObject() then
					return displayMode == 3
				end					
				return false
			end,
			--------------------------------------------------------------------------------------------
			-- Test if this entity is a bot object
			isBotObject = function(this)							
				return r2:isBotObject(this.SheetClient)					   
			end,
			

			canUpdatePlayerAttackable = function(this)
				if this.CanUpdatePlayerAttackable == 0 then return false end
				if this.CanUpdatePlayerAttackable == 1 then return true end
				return this:isBotObject() == false
			end,

			canUpdateBotAttackable = function(this)
				return this:isBotObject() == false				   
			end,

			--------------------------------------------------------------------------------------------
			-- Test if this entity is a plant
			isPlant = function(this)						
				return string.match(this.SheetClient, "cp[%w_]*%.creature")
			end,
			--------------------------------------------------------------------------------------------
			-- check if that npc is the leader of its group
			isLeader = function(this)
				if not this:isGrouped() then
					return false
				end				
				return this.IndexInParent == 0
			end,			
			--------------------------------------------------------------------------------------------
			-- return the group of this npc if it has one, else return nil
			getParentGroup = function(this)
				if this.ParentInstance:isKindOf("NpcGrpFeature")
				then
					return this.ParentInstance
				else
					return nil
				end
			end,
			--------------------------------------------------------------------------------------------
			-- change the mouse to choose a new group to group with
			onChooseGroup = function(this)				
				if this:isGrouped() then return end
				runAH(nil, "r2ed_picker_lua", "TestFunc=r2:testCanGroupSelectedInstance|PickFunc=r2:groupSelectedInstance")
				r2.ContextualCommands:highlightCommandButton("group")
			end,
			--------------------------------------------------------------------------------------------
			-- if this npc was part of a group, ungroup it
			onUngroup = function(this)
				r2:ungroup(this)
			end,
			--------------------------------------------------------------------------------------------
			-- If this npc is part of a group, make it the leader of its group
			onSetAsLeader = function(this)
				if this:isLeader() then return end
				r2:setAsGroupLeader(this)
			end,
			--------------------------------------------------------------------------------------------
			-- from 'BaseClass'
			getAvailableCommands = function(this, dest)
				--local result = this:delegate():getAvailableCommands(this)
				r2.Classes.ActiveLogicEntity.getAvailableCommands(this, dest)

				if not this:isBotObject() and not this:isPlant() then
					if not this:isGrouped() then
						table.insert(dest, this:buildCommand(this.onChooseGroup, "group", "uimR2EDMenuGroup", "r2_toolbar_group.tga", true))
					else
						table.insert(dest, this:buildCommand(this.onUngroup, "ungroup", "uimR2EDMenuUngroup", "r2_toolbar_ungroup.tga", true))
						if not this:isLeader() then							
							table.insert(dest, this:buildCommand(this.onSetAsLeader, "set_as_leader", "uimR2EDMenuSetAsGroupLeader", "r2_toolbar_set_as_leader.tga", false))
						end
					end
					--debugInfo(this.SheetClient)
				end	

				this:getAvailableDisplayModeCommands(dest)							
			end,
			--------------------------------------------------------------------------------------------
			-- from 'BaseClass'
			getParentTreeNode = function(this)
				if not this:isInDefaultFeature() then 
					return r2.Classes.ActiveLogicEntity.getParentTreeNode(this)
				end
				if this:isBotObject() then
					local container = getUI("ui:interface:r2ed_scenario")
					--return {container:find("content_tree_list"):getRootNode():getNodeFromId("scenery_objects")}
					return {container:find("content_tree_list"):getRootNode()}
				elseif ( this:isKindOf("NpcCreature") or this:isKindOf("NpcPlant") ) then
					return this:getParentAct():getContentTreeNodes("creatures")
				else	
					return this:getParentAct():getContentTreeNodes("people")
				end			
			end,
			--------------------------------------------------------------------------------------------
			-- special handler for deletion : this method is called when the user click on 'delete' in the
			-- context menu and should perform the actual deletion
			onDelete = function(this)
				if this.User.DeleteInProgress == true then return end
				this.User.DeleteInProgress = true
				this:setDeleteActionName()
				-- if deleted object is not in the default group, and was the last of its group, then
				-- its parent group should be removed
				if not this:isInDefaultFeature() then
					if this.Parent.Size <= 2 then						
						local parentTable = this.Parent
						local parentGroup = this.ParentInstance
						local defaultFeature = this:getParentAct():getDefaultFeature()
						for i = parentTable.Size - 1, 0, -1 do							
							if parentTable[i].InstanceId ~= this.InstanceId then
								parentTable[i]:requestMakePosRelativeTo(defaultFeature)
								r2:setSelectedInstanceId(parentTable[i].InstanceId)
								r2.requestMoveNode(parentTable[i].InstanceId, "", -1, defaultFeature.InstanceId, "Components", -1)
								break
							end
						end						
						r2.requestEraseNode(parentGroup.InstanceId, "", -1)
						r2.requestEndAction()						
						return
					end
				end		
				this:selectNext()
				r2.requestEraseNode(this.InstanceId, "", -1)
				r2.requestEndAction()				
			end,
			--------------------------------------------------------------------------------------------
			-- return the behavior object, depending on wether this npc is grouped or not
			getBehavior = function(this)					
				if this:isGrouped() and this.ParentInstance:isKindOf("NpcGrpFeature") then
					return this.ParentInstance.Components[0].Behavior
				else		
					return this.Behavior					
				end
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			hasScenarioCost = function(this)
				return true
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			isCopyable = function(this)
				return true
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			paste = function(src, newPlace, srcInstanceId)	

				local Q, leftQ, leftStaticQ = r2:getLeftQuota()
				local quota = leftQ
				if r2:isBotObject(r2.getPropertyValue(src, "SheetClient")) then quota = leftStaticQ end
				if quota <= 0 then
					r2:makeRoomMsg()
					return
				end
				local options = 
				{
					CopyEvents = 0,
					CopyActivities = 0,
					-- CopyChatSequences = 0
					DuplicateGroup = -1
				}

				-- if component is the leader and original group is still present, then give the option to duplicate the whole group
				local srcInstance 
				if srcInstanceId then
					srcInstance = r2:getInstanceFromId(srcInstanceId)
				end
				local groupCopy = nil
				if srcInstance and srcInstance:isLeader() then
					groupCopy = srcInstance.ParentInstance:copy()
               groupCopy = r2.Classes[groupCopy.Class].newCopy(groupCopy)
					options.DuplicateGroup = 0 -- offer option to do the copy
				end
				if srcInstance and srcInstance.isBotObject then
					if srcInstance:isBotObject() then
						if not r2:checkStaticQuota() then return end
					else
						if not r2:checkAiQuota() then return end
					end

				end
				--
				local function paramsOk(options)
					if options.DuplicateGroup == 1 then
						r2.Classes[groupCopy.Class].paste(groupCopy, src.Position, nil, options)
						return
					end
					if options.CopyActivities == 0 then
						src.ActivitiesId = {}
						src.Behavior.Activities = {}
					end
					if options.CopyEvents == 0 then
						src.Behavior.Actions = {}
					end
					--if options.CopyChatSequences == 0 then
					-- src.Behavior.ChatSequences = {}
					-- end
					if newPlace then
					   src.Position.x, src.Position.y, src.Position.z = r2:getPastePosition()
					end		
					r2:setCookie(src.InstanceId, "Select", true)
					if r2:isBotObject(r2.getPropertyValue(src, "SheetClient")) then -- not already an object, so can't call a method yet ...
						-- add to permanent content
						r2.requestInsertNode(r2.Scenario:getBaseAct():getDefaultFeature().InstanceId , "Components",-1,"", src)
					else
						-- insert in current act
						r2.requestInsertNode(r2:getCurrentAct():getDefaultFeature().InstanceId , "Components", -1,"", src)
					end
				end
				local function paramsCancel()
					debugInfo('paste was cancelled')
				end
				if table.getn(src.Behavior.Activities) == 0 then
					options.CopyActivities = -1
				end
				if table.getn(src.Behavior.Actions) == 0 then
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
				
				local target            
				if r2:isBotObject(r2.getPropertyValue(src, "SheetClient")) then -- not already an object, so can't call a method yet ...
					-- insert in current act
					target = r2.Scenario:getBaseAct():getDefaultFeature()
					if not r2:checkStaticQuota() then return end
				else
					-- insert in current act
					target = r2:getCurrentAct():getDefaultFeature()
					if not r2:checkAiQuota() then return end
				end
				-- create the 'Ghosts' entry locally if it doesn't already exists
				if target.Ghosts == nil then
					r2.requestInsertGhostNode(target.InstanceId, "", -1, "Ghosts", {})
				end					
				--				
				r2.requestInsertGhostNode(target.InstanceId, "Ghosts",-1,"", src)				
				-- insertion should have been done right now
				return r2:getInstanceFromId(src.InstanceId)
			end,

			getAiCost = function(this)
				if this.User.GhostDuplicate then return 0 end
				assert(this)
				if this.IsBotObject == 0 then
					return 1				
				end
				return 0
			end,

			getStaticObjectCost = function(this)
				if this.User.GhostDuplicate then return 0 end
				assert(this)
				if this.IsBotObject == 1 then
					return 1
				end
				return 0
			end,

			-- from 'ActiveLogicEntity'
			getApplicableActions = function(this)				
				local actions = r2.Classes[this.Class].ApplicableActions

				if not this:canUpdateBotAttackable() then
					local actionsTemp = {}
					for k, v in pairs(actions) do
						if v~="Fight with Npcs" and v~="Dont fight with Npcs" then --and v~="Sit Down" and v~="Stand Up" then
							table.insert(actionsTemp, v)
						end
					end
					actions = actionsTemp
				end

				if not this:canUpdatePlayerAttackable() then
					local actionsTemp = {}
					for k, v in pairs(actions) do
						if v~="Fight with player" and v~="Dont fight with player"  then -- and v~="Sit Down" and v~="Stand Up" then
							table.insert(actionsTemp, v)
						end
					end
					actions = actionsTemp	
				end

				return actions
			end,

		},
		------------------------------------------------------------------------------------------------------
		-- a 'custom' npc : this is a npc that is customizable
		NpcCustom = {
			Name="NpcCustom",		
			BaseClass="Npc",	
			DisplayerProperties = "R2::CDisplayerLua",
			DisplayerPropertiesParams = "npcCustomPropertySheetDisplayer",		
			Prop=
			{				
				-- Look (all widgets have Visible=false because they are edited in the npc editor, not in the property sheet)

				{ Name="GabaritHeight", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="GabaritTorsoWidth", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="GabaritArmsWidth", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="GabaritLegsWidth", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="GabaritBreastSize", Type="Number", Visible=false, DefaultInBase=1 },

				{ Name="HairType", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="HairColor", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="Tattoo", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="EyesColor", Type="Number", Visible=false, DefaultInBase=1 },

				{ Name="MorphTarget1", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget2", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget3", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget4", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget5", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget6", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget7", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="MorphTarget8", Type="Number", Visible=false, DefaultInBase=1 },

				--{ Name="Sex", Type="Number", Visible=false, DefaultInBase=1 },

				{ Name="JacketModel", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="TrouserModel", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="FeetModel", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="HandsModel", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="ArmModel", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="WeaponRightHand", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="WeaponLeftHand", Type="Number", Visible=false, DefaultInBase=1 },

				{ Name="JacketColor", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="ArmColor", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="HandsColor", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="TrouserColor", Type="Number", Visible=false, DefaultInBase=1 },
				{ Name="FeetColor", Type="Number", Visible=false, DefaultInBase=1 },

				{ Name="LinkColor", Type="Number", Visible=false, DefaultInBase=0 },

				--{ Name="Notes", Type="String", Visible=false, DefaultInBase=1 },
				{ Name="Function", Type="String", Visible=false, DefaultInBase=1 },
				--{ Name="Level", Type="String", Visible=false, DefaultInBase=1 },
				{ Name="Profile", Type="String", Visible=false, DefaultInBase=1 },
				{Name="Speed", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_NpcCustom",
					Enum= { "uiR2EDWalk", "uiR2EDRun"},
					Visible=true
				},
				{Name="Level", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_NpcCustom",
					Enum= { "uiR2EDLowLevel", "uiR2EDAverageLevel", "uiR2EDHighLevel", "uiR2EDVeryHighLevel"}, SecondRequestFunc=r2.updateLevel,
					Visible=function(this) return this:isGroupedAndLeader() or not this:isGrouped() and not this:isBotObject() end
				},
			},
			-- from "BaseClass"
			getAvailableCommands = function(this, dest)
				--local result = this:delegate():getAvailableCommands()
				r2.Classes.Npc.getAvailableCommands(this, dest)				
			end,
			-- Additionnal property sheet header to access npc customisation
			PropertySheetHeader = 
			[[
				<ctrl style="text_button_16" id="customize" active="true" posref="TL TL" onclick_l="lua" x="0" y="0" params_l="r2:getSelectedInstance():customizeLook()" hardtext="uiR2EDCustomizeLook"/>
			]],
			-- Pop the npc editor
			customizeLook = function(this)				
				-- if the npc edition window is not shown, display it
				local npcEditionWnd = getUI("ui:interface:r2ed_npc")
				if not npcEditionWnd.active then
					 npcEditionWnd.active = true
					 npcEditionWnd:updateCoords()
					 npcEditionWnd:center()
					 -- update the npc window content
					 this.DisplayerProperties:updateAll(this)
				else					
					setTopWindow(npcEditionWnd)
					npcEditionWnd:blink(1)					
				end
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			hasScenarioCost = function(this)
				return true
			end,
			-----------------------------------------------------------------------------------------------
			-- special paste with renaming
			paste = function(src, newPlace, srcInstanceId)		
				local base = r2.getPaletteElement(src.Base)            
				local sex = r2.getPropertyValue(base, "Sex")
				
				r2.Classes.Npc.paste(src, newPlace, srcInstanceId)
			end,
		},

		------------------------------------------------------------------------------------------------------
		-- NPC CREATURE
		NpcCreature = {
			Name="NpcCreature",	
			InEventUI = true,		
			BaseClass="Npc",	
			DisplayerProperties = "R2::CDisplayerLua",
			DisplayerPropertiesParams = "npcPropertySheetDisplayer",		
			Prop=
			{	
				{Name="Speed", Type="Number", WidgetStyle="EnumDropDown", Category="uiR2EDRollout_Default",
					Enum= { "uiR2EDWalk", "uiR2EDRun"},
					Visible=function(this) return  not this:isKindOf("NpcPlant") end,
				},
			},

			getApplicableActions = function(this)				
				local actions = r2.Classes[this.Class].ApplicableActions

				local actionsTemp = {}
				for k, v in pairs(actions) do
					if v~="Sit Down" and v~="Stand Up" then
						table.insert(actionsTemp, v)
					end
				end	

				return actionsTemp
			end,
		},

		------------------------------------------------------------------------------------------------------
		-- NPC PLANT
		NpcPlant = {
			Name="NpcPlant",
			InEventUI = true,				
			BaseClass="NpcCreature",	
			DisplayerProperties = "R2::CDisplayerLua",
			DisplayerPropertiesParams = "npcPropertySheetDisplayer",		
			Prop=
			{	
				
			},

			ApplicableActions = {
									"Activate", 
									"Deactivate", "Kill",
								},
			Events =			{
									"activation", 
									"desactivation", "death", 	
								},
			Conditions =		{},
		},
		

		
		-- base class for primitives, include copy-paste code
		{
			BaseClass="WorldObject",
			Name="BasePrimitive",		
			Prop=
			{
				{Name="Name", Type="String", MaxNumChar="32"}
			},
			--------------------------------------------------------------------------------------------
			onPostCreate = function(this)
				this.DisplayerVisual.DisplayMode = r2:getPrimDisplayMode()
				this.DisplayerVisual.ContextualVisibilityActive = r2.PrimDisplayContextualVisibility
			end,
			onActChanged = function(this)
				this.DisplayerVisual.ContextualVisibilityActive = r2.PrimDisplayContextualVisibility
			end,			
			--------------------------------------------------------------------------------------------
			onPostCreate = function(this)
				this.DisplayerVisual.DisplayMode = r2:getPrimDisplayMode()
				this.DisplayerVisual.ContextualVisibilityActive = r2.PrimDisplayContextualVisibility
			end,
			onActChanged = function(this)
				this.DisplayerVisual.ContextualVisibilityActive = r2.PrimDisplayContextualVisibility
			end,			
			--------------------------------------------------------------------------------------------
			-- from WorldObject
			canChangeDisplayMode = function(this)
				return true
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			isCopyable = function(this)
				return true
			end,
			isNextSelectable = function(this)
				return true
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			getAvailableCommands = function(this, dest)	
				r2.Classes.WorldObject.getAvailableCommands(this, dest) -- fill by ancestor
				table.insert(dest, this:buildCommand(this.onNewVertexTool, "new_vertex", "uimR2EDAddNewVertices", "r2ed_tool_new_vertex.tga"))				
			end,
			-----------------------------------------------------------------------------------------------
			onNewVertexTool = function(this)
				r2:setCurrentTool('R2::CToolNewVertex')
				r2.ContextualCommands:highlightCurrentCommandButton("new_vertex")
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			paste = function(src, newPlace, srcInstanceId)
				--if r2:getLeftQuota() <= 0 then
				--	r2:makeRoomMsg()
				--	return
				--end
				if newPlace then
					--if r2.Classes[src.Class].isCopyInsideCurrIsland(src) then
					--	src.Position.x = src.Position.x + 4 * (math.random() - 0.5)
					--	src.Position.y = src.Position.y + 4 * (math.random() - 0.5)
					--	src.Position.z = src.Position.z + 4 * (math.random() - 0.5)
					--else
						local mx, my, mz = r2.Classes[src.Class].getCopyCenter(src)
						-- express in world
						mx = mx + src.Position.x
						my = my + src.Position.y
						mz = mz + src.Position.z
						-- get player pos in world
						local px, py, pz = getPlayerPos()						
												
						-- add delta to have primitive center in world over player pos
						src.Position.x = src.Position.x + px - mx + 4 * (math.random() - 0.5)
						src.Position.y = src.Position.y + py - my + 4 * (math.random() - 0.5)
						src.Position.z = src.Position.z + pz - mz + 4 * (math.random() - 0.5)
					--end
				end						
				r2.requestInsertNode(r2.Scenario:getBaseAct():getDefaultFeature().InstanceId, "Components", -1, "", src)
				r2:setCookie(src.InstanceId, "Select", true)
			end,
			-----------------------------------------------------------------------------------------------
			-- Function (not method) : test if the passed primitive copy (obtained
			-- with BasePrimitive:copy is inside the current island
			-- The default behavior is to test each vertices inside the 'Points' array
			-- are inside the current island rectangle. If a derivers is not defined
			-- as a set of points, it should provide the good test here
			isCopyInsideCurrIsland = function(src)
				for k, v in pairs(src.Points) do
					if not r2:isInIslandRect(v.Position.x, v.Position.y) then return false end
				end
				return true
			end,
			-----------------------------------------------------------------------------------------------
			-- Function (not method) : return the center a a copîed primitive obtained with
			-- BasePrimitive:copy
			getCopyCenter = function(src)
				local x = 0
				local y = 0
				local z = 0
				local count = 0
				for k, v in pairs(src.Points) do
					x = x + v.Position.x
					y = y + v.Position.y
					z = z + v.Position.z
					count = count + 1
				end
				if count ~= 0 then
					x = x / count
					y = y / count
					z = z / count
				end
				return x, y, z
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			pasteGhost = function(src)	     
				--if r2:getLeftQuota() <= 0 then	
				--	r2:makeRoomMsg()
				--	return
				--end				
				target = r2.Scenario:getBaseAct():getDefaultFeature()				
				if target.Ghosts == nil then
					r2.requestInsertGhostNode(target.InstanceId, "", -1, "Ghosts", {})
				end									
				r2.requestInsertNode(target.InstanceId, "Ghosts", -1, "", src)            
				return r2:getInstanceFromId(src.InstanceId)
			end,			
		},
		------------------------------------------------------------------------------------------------------
		{
			BaseClass="BasePrimitive",
			Name="Region",			
			Menu="ui:interface:r2ed_base_menu",
			TreeIcon= function(this)			
				if this:isInDefaultFeature() then return "" else return "r2ed_icon_region.tga" end
			end,
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "defaultUIDisplayer",
			DisplayerVisual = "R2::CDisplayerVisualGroup",
			DisplayerVisualParams = 
			{
				Look = r2.PrimRender.RegionLook,
				InvalidLook = r2.PrimRender.RegionInvalidLook,
				--
				ArrayName = "Points"
			},				
			Prop=
			{				
--				{Name="Base", Type="String", WidgetStyle="StaticText", Category="Advanced"},					
				{Name="Points", Type="Table"},				
			},
			--------------------------------------------------------------------------------------------
			getSelectBarIcon = function(this)				
				return "r2ed_icon_region.tga"				
			end,
			getAvailableCommands = function(this, dest)	
				r2.Classes.BasePrimitive.getAvailableCommands(this, dest) -- fill by ancestor				
				this:getAvailableDisplayModeCommands(dest)
			end,

			---------------------------------------------------------------------------------------------------------
			-- get select bar type
			SelectBarType = function(this)
				return i18n.get("uiR2EDbotObjects"):toUtf8()
			end,
			--------------------------------------------------------------------------------------------
			-- from 'BaseClass'
			getParentTreeNode = function(this)				
--				if not this:isInDefaultFeature() then 
--					return r2.Classes.WorldObject.getParentTreeNode(this)
--				end
--				local tree = getUI("ui:interface:r2ed_scenario")							
--				return tree:find("content_tree_list"):getRootNode():getNodeFromId("places")

				return {}
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			--getUsedQuota = function(this)
			--	return 1
			--end		
		},
		------------------------------------------------------------------------------------------------------
		{
			BaseClass="BasePrimitive",
			Name="Road",
			--DisplayerVisual = "R2::CDisplayerVisualRoad",
			DisplayerVisual = "R2::CDisplayerVisualGroup",
			TreeIcon= function(this)			
				if this:isInDefaultFeature() then return "" else return "r2ed_icon_road.tga" end
			end,
			DisplayerVisualParams = 
			{
				Look = r2.PrimRender.RoadLook,
				InvalidLook = r2.PrimRender.RoadLookInvalid,
				--
				ArrayName = "Points"
			},			
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "defaultUIDisplayer",
			Menu="ui:interface:r2ed_base_menu",
			Prop=
			{				
--				{Name="Base", Type="String", WidgetStyle="StaticText", Category="Advanced"},				
				{Name="Points", Type="Table"}				
			},
			--------------------------------------------------------------------------------------------
			getSelectBarIcon = function(this)				
				return "r2ed_icon_road.tga"
			end,

			---------------------------------------------------------------------------------------------------------
			-- get select bar type
			SelectBarType = function(this)
				return i18n.get("uiR2EDbotObjects"):toUtf8()
			end,
			--------------------------------------------------------------------------------------------
			-- from 'BaseClass'
			getParentTreeNode = function(this)				
--				if not this:isInDefaultFeature() then 
--					return r2.Classes.WorldObject.getParentTreeNode(this)
--				end
--				local tree = getUI("ui:interface:r2ed_scenario")	
--				return tree:find("content_tree_list"):getRootNode():getNodeFromId("routes")	

				return {}
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			getAvailableCommands = function(this, dest)	
				r2.Classes.BasePrimitive.getAvailableCommands(this, dest) -- fill by ancestor
				table.insert(dest, this:buildCommand(this.onExtendRoad, "extend_road", "uiR2EDExtendRoad", "r2ed_tool_extend_prim.tga"))				
				this:getAvailableDisplayModeCommands(dest)			
			end,
			-----------------------------------------------------------------------------------------------
			onExtendRoad = function(this)
				r2:setCurrentTool('R2::CToolDrawPrim', 
								  { 
									Look = r2.PrimRender.RoadCreateLook,
									InvalidLook = r2.PrimRender.RoadCreateInvalidLook,
									Type="Road",
									ExtendedPrimitiveId = this.InstanceId
								  }
								 )
				r2.ContextualCommands:highlightCurrentCommandButton("extend_road")				
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			--getUsedQuota = function(this)
			--	return 1
			--end			
		},
		------------------------------------------------------------------------------------------------------
		{
			BaseClass="WorldObject",
			Name="WayPoint",
			BuildPropertySheet = false,
			DisplayerVisual = "R2::CDisplayerVisualShape",
			DisplayerVisualParams = { ShapeName = r2.PrimRender.RoadCreateLook.VertexShapeName, 
			                          Scale = r2.PrimRender.RoadCreateLook.VertexShapeScale,
									  InheritDisplayMode = true
									},
			DisplayerUI = "",			
			Menu="ui:interface:r2ed_base_menu",
			Prop=
			{
				-- NOTE : position inherit from 'WorldObject'
			},
			onDelete = function(this)
				if this.User.DeleteInProgress == true then return end
				this.User.DeleteInProgress = true
				this:setDeleteActionName()
				-- if I'm the last in the array, then delete my parent				
				if this.Parent.Size <= 2 then
					this.ParentInstance:selectNext()
					r2.requestEraseNode(this.ParentInstance.InstanceId, "", -1)
				else
					-- just delete my self					
					this:selectNext()
					r2.requestEraseNode(this.InstanceId, "", -1)
				end
				r2.requestEndAction()				
			end,
			setDeleteActionName = function(this)
				r2.requestNewAction(i18n.get("uiR2EDDeletingWayPoint"))
			end,
			isNextSelectable = function(this)
				return true
			end
		},
		------------------------------------------------------------------------------------------------------
		{
			BaseClass="WorldObject",
			Name="RegionVertex",
			BuildPropertySheet = false,
			DisplayerVisual = "R2::CDisplayerVisualShape",
			DisplayerVisualParams = { ShapeName = r2.PrimRender.RegionCreateLook.VertexShapeName,
			                          Scale = r2.PrimRender.RegionCreateLook.VertexShapeScale,
									  InheritDisplayMode = true
									 },
			DisplayerUI = "",
			Menu="ui:interface:r2ed_base_menu",
			Prop=
			{				
				-- NOTE : position inherit from 'WorldObject'
			},
			setDeleteActionName = function(this)
				r2.requestNewAction(i18n.get("uiR2EDDeletingRegionVertex"))
			end,
			onDelete = function(this)
				if this.User.DeleteInProgress == true then return end
				this.User.DeleteInProgress = true
				this:setDeleteActionName()
				-- if I'm the last in the array, then delete my parent				
				if this.Parent.Size <= 3 then
					this.ParentInstance:selectNext()
					r2.requestEraseNode(this.ParentInstance.InstanceId, "", -1)
				else
					-- just delete my self					
               this:selectNext()
					r2.requestEraseNode(this.InstanceId, "", -1)
				end
				r2.requestEndAction()				
			end,
			isNextSelectable = function(this)
				return true
			end
		},
		------------------------------------------------------------------------------------------------------
		{
			BaseClass="BasePrimitive",
			Name="Place",
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "defaultUIDisplayer",
			Menu="ui:interface:r2ed_base_menu",
			Prop=
			{				
				{Name="Radius", Type="Number"}
			},
			-- from BasePrimitive
			isCopyInsideCurrIsland = function(src)
				return r2:isInIslandRect(src.Position.x, src.Position.y)
			end,
			-- from BasePrimitive
			getCopyCenter = function(src)				
				return src.Position.x, src.Position.y, src.Position.z
			end,
			--
			getAvailableCommands = function(this, dest)	
				r2.Classes.BasePrimitive.getAvailableCommands(this, dest) -- fill by ancestor				
				this:getAvailableDisplayModeCommands(dest)
			end,
		},
		------------------------------------------------------------------------------------------------------
		{
			BaseClass="BaseClass",
			Name="Position",
			DisplayerUI = "",
			Prop=
			{				
				{Name="x", Type="Number"},
				{Name="y", Type="Number"},
				{Name="z", Type="Number"}
			},
			-- test if this position is equal to another position (not necessarily an instance,
			-- may be any table with the { x = ..., y = ..., z = ... } format
			equals = function(this, other)				
				return this.x == other.x and this.y == other.y and this.z == other.z
			end,
			-- return string version of position			
			toString = function(this)
				return "(" .. tostring(this.x) .. ", " .. tostring(this.y) .. ", " .. tostring(this.z) .. ")"
			end
		},
		UserComponentHolder = 
		{
			BaseClass="LogicEntity",
			Name="UserComponentHolder",

			Menu="ui:interface:r2ed_feature_menu",
			DisplayerUI = "R2::CDisplayerLua",
			DisplayerUIParams = "defaultUIDisplayer",
			DisplayerVisual = "R2::CDisplayerVisualEntity",


			Parameters = {},

			ApplicableActions = {},

			Events = {},

			Conditions = {},

			TextContexts = {},

			TextParameters = {},

			LiveParameters = {},

			Prop =
			{
				{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
				{Name="Name", Type="String"},
				{Name="Description", Type="String"},
				{Name="Components", Type="Table", Visible=false},
			},

			getParentTreeNode = function(this)
				return this:getFeatureParentTreeNode()
			end,

			appendInstancesByType = function(this, destTable, kind)
				assert(type(kind) == "string")
				r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
				for k, component in specPairs(this.Components) do
					component:appendInstancesByType(destTable, kind)
				end
			end,

			getSelectBarSons = function(this)
				return Components
			end,

			canHaveSelectBarSons = function(this)
				return false;
			end,

			onPostCreate = function(this)
			end,

			pretranslate = function(this, context)
				r2.Translator.createAiGroup(this, context)
				r2.Translator.pretranslateDefaultFeature(this, context)
			end,

			translate = function(this, context)
				r2.Translator.translateDefaultFeature(this, context, true)
			end,

			getActivitiesIds = r2.Translator.getDefaultFeatureActivitiesIds,
			-- TODO use sexy images	
			getAvailableCommands = function(this, dest)
				r2.Classes.LogicEntity.getAvailableCommands(this, dest)
				table.insert(dest, this:buildActivityCommand(this.onPickAddEntity, "add_entity", "uimR2EDMenuPickAddEntity", "r2_toolbar_customize_look.tga", true))
				table.insert(dest, this:buildActivityCommand(this.onPickRemoveEntity, "remove_entity", "uimR2EDMenuPickRemoveEntity", "r2_toolbar_delete.tga", false))
				table.insert(dest, this:buildActivityCommand(this.onPickExport, "export", "uimR2EDMenuPickExport", "r2_toolbar_properties.tga", false))
			end,

		},
	
	}
	 -- !feature.Components
		------------------------------------------------------------------------------------------------------		
	classLogicEntityBehaviorVersion = 1

	feature.Components.LogicEntityBehavior = {
		Name="LogicEntityBehavior",
		BaseClass="BaseClass",
		Version = classLogicEntityBehaviorVersion,
		Prop=
		{				
			{Name="Actions", Type="Table"}, 
--				{Name="Reactions", Type="Table"}

		},
		updateVersion = function(this, scenarioValue, currentValue)
			local patchValue = scenarioValue
			if patchValue < 1 then
				-- Patch only for old save (not the 0.0.3)
				if this.Reactions  ~= nil then
					-- TODO use identifier instead				
					r2.requestEraseNode(this.InstanceId, "Reactions", -1)
				end	
				patchValue = 1
			end

			if patchValue == currentValue then return true end
			return false
		end
	}


		------------------------------------------------------------------------------------------------------	
	local userComponentHolder = feature.Components.UserComponentHolder
		
	userComponentHolder.getLogicAction = function(entity, context, action)
		local firstAction, lastAction = nil,nil
		return firstAction, lastAction
	end


	userComponentHolder.getLogicCondition = function(this, context, condition)
		return nil,nil
	end


	userComponentHolder.getLogicEvent = function(this, context, event)	
		local eventHandler, firstCondition, lastCondition = nil, nil, nil
		return eventHandler, firstCondition, lastCondition
	end

	function userComponentHolder:registerMenu(logicEntityMenu)
		local name = i18n.get("uiR2EdUserComponent")
		logicEntityMenu:addLine(ucstring(name), "lua", "", "uiR2EdUserComponent")
	end


	function userComponentHolder:getLogicTranslations()
		local logicTranslations = {
			["ApplicableActions"] = {},
			["Events"] = {},
		}
		return logicTranslations
	end
	function userComponentHolder.onPickExport(this)
		local refX = this.Position.x
		local refY = this.Position.y
		local refZ = this.Position.z
		
		local components = this.Components
		r2_core.UserComponentManager.CurrentDesc = this.Description
		local exportList = {}
		local k, v = next(components, nil)
		while k do
			if v.InstanceId then
				table.insert(exportList, v.InstanceId)
			end
			k, v = next(components, k)
		end
		r2_core.UserComponentManager:export(exportList, refX, refY, refZ)
	end

	function userComponentHolder.create()	
		if not r2:checkAiQuota() then return end
	
		local function paramsCancel()
			debugInfo("Cancel form for 'User Component' creation")
		end
		local function posOk(x, y, z)
			debugInfo(string.format("Validate creation of 'User Component' at pos (%d, %d, %d)", x, y, z))
			if r2.mustDisplayInfo("UserComponent") == 1 then 
				r2.displayFeatureHelp("UserComponent")
			end
			local component = r2.newComponent("UserComponentHolder")
			component.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
			component.Name = r2:genInstanceName(i18n.get("uiR2EdUserComponent")):toUtf8()			
			
			component.Position.x = x
			component.Position.y = y
			component.Position.z = r2:snapZToGround(x, y)

			r2:setCookie(component.InstanceId, "DisplayProp", 1)
			r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
			
		end
		local function posCancel()
			debugInfo("Cancel choice 'User Component' position")
		end	
		local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
		r2:choosePos(creature, posOk, posCancel, "createUserComponent")
	end

	function userComponentHolder:registerMenu(logicEntityMenu)
		local name = i18n.get("uiR2EdUserComponent")
		logicEntityMenu:addLine(ucstring(name), "lua", "", "uiR2EdUserComponent")
	end

	function userComponentHolder.onPickAddEntity(this)
		r2_core.CurrentHolderId = this.InstanceId
		runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2_core:testIsExportable|PickFunc=r2_core:addEntityToExporter|PickPosFunc=r2_core:doNothing|WantMouseUp=true")
		r2.ContextualCommands:highlightCommandButton("add_entity")	
	end
	
	function userComponentHolder.onPickRemoveEntity(this)
		r2_core.CurrentHolderId = this.InstanceId
		runAH(nil, "r2ed_picker_lua", "CursCanPickPos=curs_create.tga|CursCannotPickPos=curs_stop.tga|TestFunc=r2_core:testCanPickUserComponentElement|PickFunc=r2_core:removeUserComponentElement|PickPosFunc=r2_core:doNothing|WantMouseUp=true")
		r2.ContextualCommands:highlightCommandButton("remove_entity")	
	end
	
	function userComponentHolder.onPostCreate(this)
		if this.User.DisplayProp and this.User.DisplayProp == 1 then
			r2:setSelectedInstanceId(this.InstanceId)				
			r2:showProperties(this)		
			this.User.DisplayProp = nil
		end
	end

	function userComponentHolder.onPickExport(this)
		local refX = this.Position.x
		local refY = this.Position.y
		local refZ = this.Position.z
		
		local components = this.Components
		r2_core.UserComponentManager.CurrentDesc = this.Description
		local exportList = {}
		local k, v = next(components, nil)
		while k do
			if v.InstanceId then
				table.insert(exportList, v.InstanceId)
			end
			k, v = next(components, k)
		end
		r2_core.UserComponentManager:export(exportList, refX, refY, refZ)
	end

	function userComponentHolder.create()	
		if not r2:checkAiQuota() then return end
	
		local function paramsCancel()
			debugInfo("Cancel form for 'User Component' creation")
		end
		local function posOk(x, y, z)
			debugInfo(string.format("Validate creation of 'User Component' at pos (%d, %d, %d)", x, y, z))
			if r2.mustDisplayInfo("UserComponent") == 1 then 
				r2.displayFeatureHelp("UserComponent")
			end
			local component = r2.newComponent("UserComponentHolder")
			component.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
			component.Name = r2:genInstanceName(i18n.get("uiR2EdUserComponent")):toUtf8()			
			
			component.Position.x = x
			component.Position.y = y
			component.Position.z = r2:snapZToGround(x, y)

			r2:setCookie(component.InstanceId, "DisplayProp", 1)
			r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
			
		end
		local function posCancel()
			debugInfo("Cancel choice 'User Component' position")
		end	
		local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
		r2:choosePos(creature, posOk, posCancel, "createUserComponent")
	end

	feature.Components.DefaultFeature = {
			BaseClass="BaseClass",
			Name="DefaultFeature",
			--TreeIcon="r2ed_icon_default_feature.tga",
			DisplayerUI = "",			
			Menu="ui:interface:r2ed_base_menu",			
			Prop=
			{				
				{Name="Components", Type="Table"},				
			},
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
			-- from base class			
			appendInstancesByType = function(this, destTable, kind)
				assert(type(kind) == "string")
				--this:delegate():appendInstancesByType(destTable, kind)
				r2.Classes.BaseClass.appendInstancesByType(this, destTable, kind)
				for k, component in specPairs(this.Components) do
					component:appendInstancesByType(destTable, kind)
				end
			end,
			-----------------------------------------------------------------------------------------------
			-- from base class
			hasScenarioCost = function(this)
				return true
			end,
        		 -----------------------------------------------------------------------------------------------
			-- from base class
			displayInSelectBar = function(this)
				return false
			end,
			
			pretranslate = r2.Translator.pretranslateDefaultFeature,
			pretranslate2 = r2.Translator.pretranslateDefaultFeature2,
			
			translate = r2.Translator.translateDefaultFeature,

			getAiCost = function(this)
				return r2.getAiCost(this) - 1
			end,

			getStaticObjectCost = function(this)
				return r2.getStaticObjectCost(this)
			end
					
		}

	function feature.Components.DefaultFeature.getActivitiesIds(this)
		return r2.Translator.getDefaultFeatureActivitiesIds(this)
	end

	--- Npc
	local componentNpc = feature.Components.Npc

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function componentNpc.initLogicEntitiesMenu(this, logicEntityMenu, testApplicableAction)				
		local name = i18n.get("uiR2EDnpc")		
		local existPeople = nil

		local enumerator = r2:enumInstances("Npc")		
		while 1 do
			local inst = enumerator:next()
			if not inst then break end					
			local condExistPeople = not (inst:isKindOf("NpcCreature") or inst:isBotObject() or inst:isPlant())
			if not r2.events.memberManagement then  --TEMP
				condExistPeople = not (inst:isKindOf("NpcCreature") or inst:isBotObject() or inst:isPlant() or inst:isGrouped())  --TEMP
			end		--TEMP
			if condExistPeople then
				existPeople = inst
				break
			end
		end

		if existPeople and (not r2.events.memberManagement or testApplicableAction==nil or (testApplicableAction==true and r2.events:hasApplicableActions(existPeople)))  then
			logicEntityMenu:addLine(name, "lua", "", "Npc")
		end
	end

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	function componentNpc.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)		
		local empty = true
		local enumerator = r2:enumInstances("Npc")		
		while 1 do
			local entity = enumerator:next()
			if not entity then break end			
			local addLine = not (entity:isKindOf("NpcCreature") or entity:isBotObject() or entity:isPlant())
			if not r2.events.memberManagement then  --TEMP
				addLine = not (entity:isKindOf("NpcCreature") or entity:isBotObject() or entity:isPlant() or entity:isGrouped()) --TEMP
			end  --TEMP
			if addLine then
				local uc_name = ucstring()
				uc_name:fromUtf8(entity.Name)
				subMenu:addLine(uc_name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
				empty = false
			end
		end

		if empty==true then
			subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
		end
	end

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function componentNpc:getLogicTranslations()

		local logicTranslations = { 
			["ApplicableActions"] = {
				["Activate"]		= {menu=i18n.get("uiR2EdActivates"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdActivates")},
				["Deactivate"]		= {menu=i18n.get("uiR2EdDeactivates"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdDeactivates")},
				["Sit Down"]		= {menu=i18n.get("uiR2EdSitDown"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdSitsDown")},
				["Stand Up"]		= {menu=i18n.get("uiR2EdStandUp"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdStandsUp")},
				["Kill"]			= {menu=i18n.get("uiR2EdKill"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdKills")},
				["begin activity sequence"]		= {menu=i18n.get("uiR2EdBeginActivitySequ"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdSequenceStarts")},
				["Fight with player"]		= {menu=i18n.get("uiR2EdFightWithPlayer"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdFightWithPlayer")},
				["Fight with Npcs"]		= {menu=i18n.get("uiR2EdFightWithNpcs"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdFightWithNpcs")},
				["Dont fight with player"]		= {menu=i18n.get("uiR2EdDontFightWithPlayer"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdDontFightWithPlayer")},
				["Dont fight with Npcs"]		= {menu=i18n.get("uiR2EdDontFightWithNpcs"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdDontFightWithNpcs")},
				["Run"]		= {menu=i18n.get("uiR2EdRun"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdRun")},
				["Dont run"]		= {menu=i18n.get("uiR2EdDontRun"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdDontRun")},
			},

			["Events"] = {	
				["activation"]					= {menu=i18n.get("uiR2EdActivation"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdActivation")},
				["desactivation"]				= {menu=i18n.get("uiR2EdDeactivation"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdDeactivation")},
				["death"]						= {menu=i18n.get("uiR2EdDeath"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdDeath")},
				["end of activity step"]		= {menu=i18n.get("uiR2EdEndActivityStep"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdEndActivityStep")},
				["end of activity sequence"]	= {menu=i18n.get("uiR2EdEndActivitySequ"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdEndActivitySequ")},
				["begin of activity step"]		= {menu=i18n.get("uiR2EdBeginActivityStep"):toUtf8(), 
												text=r2:lowerTranslate("uiR2EdBeginActivityStep")},
				["begin of activity sequence"]	= {menu=i18n.get("uiR2EdBeginOfActivitySequ"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdBeginOfActivitySequ")},
				["targeted by player"]				= {menu=i18n.get("uiR2EdTargetedByplayer"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdTargetedByplayer")}

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

	componentNpc.getLogicCondition = r2.Translator.getNpcLogicCondition
	componentNpc.getLogicAction = r2.Translator.getNpcLogicAction
	componentNpc.getLogicEvent = r2.Translator.getNpcLogicEvent

	-- obsolete
	feature.getCost = function (featureInstance)
		local cost = 0
		local components = featureInstance.Components

		local key,comp = next(components,nil)
		while(key ~= nil)
		do			
	
			if (comp.Class == "Npc" or comp.Class == "NpcCustom")
			then
				cost = cost +1
			end
			key,comp = next(components,key)
		end
		return cost
	end

	-- obsolete		
	feature.Translator = function (context)	

		local components = context.Feature.Components
		--luaObject(context.Feature)
		local key,comp = next(components,nil)
		while(key ~= nil)
		do		
			-- Npc case (npc alone not object)	
			if (comp.isKindOf and comp:isKindOf( "Npc"))
			then
				local hlNpc = comp
				
				context.Feature = hlNpc

				-- create and set rtNpc
				local rtNpc = r2.Translator.translateNpc(hlNpc, context)			
				table.insert(context.RtAct.Npcs, rtNpc)

				-- create or get rtGrp
				-- set rtGrp.GroupParameter  by reading hlNpc (Aggro, Player attackable..)
				local rtNpcGrp = r2.Translator.getRtGroup(context,hlNpc.InstanceId)
				r2.Translator.setGroupParameters(hlNpc, rtNpcGrp)
				--table.insert(context.RtAct.NpcGrps, rtNpcGrp)
				table.insert(rtNpcGrp.Children, rtNpc.Id)
							
				-- set activity 
				local aiActivity = r2.Translator.getAiActivity(hlNpc)
				r2.Translator.translateActivities(context, hlNpc,  hlNpc:getBehavior(), rtNpcGrp, aiActivity)

				-- set eventHandlers
				r2.Translator.translateEventHandlers(context, hlNpc, hlNpc:getBehavior().Actions, rtNpcGrp)	
			end
				
			key,comp = next(components,key)
		end
	end

	-- NpcCustom
	local componentNpcCustom = feature.Components.NpcCustom 

	----------------------------------------------------------------------------
	-- add no line to the event menu
	function componentNpcCustom.initLogicEntitiesMenu(this, logicEntityMenu)
	end

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	function componentNpcCustom.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
	end

	----------------------------------------------------------------------------
	-- add a line to the event menu
	--function componentNpcCustom:getLogicTranslations()
	--	return {
	--end

	function componentNpcCustom.newCopy(this)
		local result = r2.Classes.BaseClass.newCopy(this)

		local sex
		if isR2PlayerMale(result.SheetClient) then
			sex = r2.male
		else 
			sex = r2.female
		end
		local race = getR2PlayerRace(result.SheetClient)

		result.Name = r2:randomNPCName2(race, sex)
		
		return result
	end

	---------------------------------------------------------------------------------------------------------
	-- Show the property window for this instance
	function componentNpcCustom.onShowProperties(this)
		local npcUI = getUI("ui:interface:r2ed_npc")
		assert(npcUI)

		if npcUI.active then
			r2:updateName()
		end

		r2.Classes.BaseClass.onShowProperties(this)
	end


	-- NpcCreature
	local componentNpcCreature = feature.Components.NpcCreature 
	
	----------------------------------------------------------------------------
	-- add no line to the event menu
	function componentNpcCreature.initLogicEntitiesMenu(this, logicEntityMenu, testApplicableAction)

		local name = i18n.get("uiR2EDCreatures")		
		local existCreature = nil
		local enumerator = r2:enumInstances("NpcCreature")		
		while 1 do
			local inst = enumerator:next()
			if not inst then break end
			local condExistCreature = not inst:isKindOf("NpcPlant")
			if not r2.events.memberManagement then		--TEMP
				condExistCreature = not (inst:isKindOf("NpcPlant") or inst:isGrouped()) --TEMP
			end											--TEMP
			if condExistCreature then
				existCreature = inst
				break
			end
		end

		if existCreature and (not r2.events.memberManagement or testApplicableAction==nil or (testApplicableAction==true and r2.events:hasApplicableActions(existCreature))) then
			logicEntityMenu:addLine(name, "lua", "", "NpcCreature")
		end
	end

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	function componentNpcCreature.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)		
		local empty = true
		local enumerator = r2:enumInstances("NpcCreature")
		while 1 do
			local entity = enumerator:next()
			if not entity then break end					
			local addLine = not entity:isKindOf("NpcPlant")
			if not r2.events.memberManagement then									
				addLine = not (entity:isKindOf("NpcPlant") or entity:isGrouped())	
			end																		
			if addLine then
				local uc_name = ucstring()
				uc_name:fromUtf8(entity.Name)
				subMenu:addLine(uc_name, "lua", calledFunction.."('".. entity.InstanceId .."')", entity.InstanceId)
				empty = false
			end
		end

		if empty==true then
			subMenu:addLine(i18n.get("uiR2EdNoSelelection"), "", "", "")
		end
	end


	-- NpcPlant
	local componentNpcPlant = feature.Components.NpcPlant 

	----------------------------------------------------------------------------
	-- add no line to the event menu
	function componentNpcPlant.initLogicEntitiesMenu(this, logicEntityMenu)
		r2.Classes.LogicEntity.initLogicEntitiesMenu(this, logicEntityMenu)
	end

	----------------------------------------------------------------------------
	-- add a line to the event sub menu
	function componentNpcPlant.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
		r2.Classes.LogicEntity.initLogicEntitiesInstancesMenu(this, subMenu, calledFunction)
	end

	----------------------------------------------------------------------------
	-- add a line to the event menu
	function componentNpcPlant:getLogicTranslations() 
		local logicTranslations = {
			["ApplicableActions"] = {
				["Activate"]		= {menu=i18n.get("uiR2EdActivates"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdActivates")},
				["Deactivate"]		= {menu=i18n.get("uiR2EdDeactivates"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdDeactivates")},
				["Kill"]			= {menu=i18n.get("uiR2EdKill"):toUtf8(), 
										text=r2:lowerTranslate("uiR2EdKills")},
			},

			["Events"] = {	
				["activation"]					= {menu=i18n.get("uiR2EdActivation"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdActivation")},
				["desactivation"]				= {menu=i18n.get("uiR2EdDeactivation"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdDeactivation")},
				["death"]						= {menu=i18n.get("uiR2EdDeath"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdDeath")},
				["targeted by player"]						= {menu=i18n.get("uiR2EdTargetedByplayer"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdTargetedByplayer")}
			},
			["Conditions"] = {	
				["is active"]					= {menu=i18n.get("uiR2EdIsActive"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdIsActive")},
				["is dead"]						= {menu=i18n.get("uiR2EdIsDead"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdIsDead")},
				["is alive"]					= {menu=i18n.get("uiR2EdIsAlive"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdIsAlive")},
				["is inactive"]					= {menu=i18n.get("uiR2EdIsInactive"):toUtf8(), 
													text=r2:lowerTranslate("uiR2EdIsInactive")},
			} 
		}
		return logicTranslations
	end
	




	return feature
end

r2.Features["DefaultFeature"] = registerFeature()






--------------------------------------------------------------------------------------------------
-------------------------- ACTIVE LOGIC ENTITY DisplayerProperties -----------------------------------------
--------------------------------------------------------------------------------------------------

local activeLogicEntityPropertySheetDisplayerTable = clone(r2:propertySheetDisplayer())

------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onPostCreate(instance)	
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onErase(instance)
	r2:logicEntityPropertySheetDisplayer():onErase(instance)
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onPreHrcMove(instance)		
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onPostHrcMove(instance)	
end
------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onFocus(instance, hasFocus)		
end

------------------------------------------------
r2.activitiesAndChatsUIUpdate = false
function activeLogicEntityPropertySheetDisplayerTable:onSelect(instance, isSelected)

	r2:logicEntityPropertySheetDisplayer():onSelect(instance, isSelected)
	
	if not isSelected or (instance.Ghost == true) then
	
		r2.activities:closeEditor()
		r2.miniActivities:closeEditor()
	else
		if instance:isKindOf("Npc") then
			local helpButton = getUI("ui:interface:r2ed_property_sheet_Npc:header_opened:help")
			assert(helpButton)
			if instance:isBotObject() then
				debugInfo("1")
				helpButton.active = false
				helpButton.parent.parent.title_delta_max_w = r2.DefaultPropertySheetTitleClampSize
			else
				debugInfo("2")
				helpButton.active = true
				helpButton:updateCoords()
				helpButton.parent.parent.title_delta_max_w = r2.DefaultPropertySheetTitleClampSize - helpButton.w_real - 4
			end
		end
		r2.activities.isInitialized = false
		r2.miniActivities:openEditor()
	end
end

------------------------------------------------
function activeLogicEntityPropertySheetDisplayerTable:onAttrModified(instance, attributeName)

	r2:logicEntityPropertySheetDisplayer():onAttrModified(instance, attributeName)

	if attributeName == "Name" then

		if r2.events.filteredLogicEntityId == instance.InstanceId then
			r2.events:updateSequenceUI()
		end

		if not r2.activitiesAndChatsUIUpdate or instance ~= r2:getSelectedInstance() then
			return
		end

		local activitiesUI = getUI(r2.activities.uiId)
		assert(activitiesUI)
		local dialogsUI = getUI(r2.dialogs.uiId)
		assert(dialogsUI)

		activitiesUI.uc_title = i18n.get("uiR2EDActivitySequenceEditor"):toUtf8() .. instance[attributeName]
		dialogsUI.uc_title = i18n.get("uiR2EDChatSequenceEditor"):toUtf8() .. instance[attributeName]	
	end
end	

------------------------------------------------
function r2:activeLogicEntityPropertySheetDisplayer()	
	return activeLogicEntityPropertySheetDisplayerTable -- returned shared displayer to avoid wasting memory
end
















