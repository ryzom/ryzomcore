
r2.Features.BanditCampFeature = {}

local feature = r2.Features.BanditCampFeature 

feature.Name = "BanditCampFeature"

feature.BanditCount = 0

feature.Description = "A bandits camp feature"

feature.Components = {}

local BanditZoneRadius = 5
local BanditZoneNumCorners = 6

local BanditCampVersion = 2
feature.Components.BanditCamp =
	{
		BaseClass = "LogicEntity",			
		Name = "BanditCamp",
		InEventUI = true,
		Menu = "ui:interface:r2ed_feature_menu",
		Version = BanditCampVersion,	
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",
		-----------------------------------------------------------------------------------------------	
		Parameters = 		{
							},
		ApplicableActions = {
								"activate", 
								"deactivate", "Wander", "Sit Down",
								"Kill"
							},
		Events = 			{
								"activation", 
								"deactivation", 
								"member death", "group death", 	
								"head to wander zone", "arrive at wander zone", "head to camp", 
								"arrive at camp"
							},
		Conditions = 		{
								"is active", "is inactive",--"is dead", 
								"is wandering", "is sitting", 
								"is heading to wander zone", "is heading to camp"
							},
		TextContexts = 		{
							},
		TextParameters = 	{
								"max bandits number", "still alive bandits number"
							},
		LiveParameters = 	{
								"is active", "alive bandits number", 
								"max bandits number", state = {"sitting", "heading to wander", "wandering or heading home"}
							},
		-----------------------------------------------------------------------------------------------	
		--Category = "uiR2EDRollout_BanditsCamp",
		Prop =
		{
			{Name = "InstanceId", Type = "String", WidgetStyle = "StaticText", Visible = false},
			{Name = "Components", Type = "Table"},
			{Name = "Name", Type="String", MaxNumChar="32"},
			{Name = "BanditsCount", Type = "String",  WidgetStyle = "StaticText"},
			{Name = "Race", Type = "String", WidgetStyle = "StaticText"},
			{Name = "BanditsLevel", Type = "String", WidgetStyle = "StaticText"},
			--{Name = "Cycle", Type = "Number", Min = "10", Max = "999999", DefaultValue = "30"},
			{Name = "SitDuration", Type = "Number", Min = "10", Max = "999999", DefaultValue = "20"},
			{Name = "WanderDuration", Type = "Number", Min = "10", Max = "999999", DefaultValue = "20"},
			--{Name = "Behavior", Type = "Table"},
			{Name = "Cost", Type = "Number", Visible = false },
			{Name = "Ghosts", Type = "Table", Visible = false },
			{Name = "Active", Type = "Number", WidgetStyle="Boolean", DefaultValue="1" },
			{Name = "NoRespawn", Type ="Number", WidgetStyle = "Boolean", DefaultValue="0"},
		
		},
		-----------------------------------------------------------------------------------------------		
		-- from base class
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class			
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
			r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
			for k, component in specPairs(this.Components) do
				component:appendInstancesByType(destTable, kind)
			end
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class
		getSelectBarSons = function(this)
			return Components
		end,
		---------------------------------------------------------------------------------------------------------
		-- from base class		
		canHaveSelectBarSons = function(this)
			return false;
		end,
		
		onPostCreate = function(this)
			--this:createGhostComponents()
			if this.User.DisplayProp and this.User.DisplayProp == 1 then
				r2:setSelectedInstanceId(this.InstanceId)				
				r2:showProperties(this)		
				this.User.DisplayProp = nil
			end
		end,
				
		translate = function(this, context)
			r2.Translator.translateAiGroup(this, context)
			r2.Translator.translateFeatureActivation(this, context)
		end,

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
		end,

		updateVersion = function(this, scenarioValue, currentValue )
			local patchValue = scenarioValue
			if patchValue < 1 then
				r2.requestEraseNode(this.InstanceId, "Cost", -1)				
				patchValue = 1
			end
			if patchValue < 2 then
				local invalidEvents = {}
				invalidEvents["desactivation"] = "deactivation"
				r2.updateLogicEvents(this, invalidEvents)
				r2.requestSetNode(this.InstanceId, "BanditsLevel", "22")
				patchValue = 2
			end
			
			if patchValue == currentValue then return true end
			return false
		end,	

	}
local component = feature.Components.BanditCamp 


function component.onPostCreate(this)

	if this.User.DisplayProp and this.User.DisplayProp == 1 then
		r2:setSelectedInstanceId(this.InstanceId)				
		r2:showProperties(this)		
		this.User.DisplayProp = nil
	end

	this:setLocalCost(_BanditCount)
	local comp = this
	if comp._Seed then math.randomseed(comp._Seed) end
	do
		local x = comp.Position.x
		local y = comp.Position.y
		local n = comp._BanditsCount 
		local pas = (2 * math.pi)/n
		local r = (n/(2*math.pi))+2							
		for i = 1, n do
			local npc = r2:randomNPC( component.getBanditPaletteId(comp), component.Races[comp._Race + 1])
			npc.Position.x = (r-1) * math.cos((i-1)*pas) -- make position relative to the feature positio
 			npc.Position.y = (r-1) * math.sin((i-1)*pas) 
 			npc.Position.z = 0
 			npc.Angle = (i-1)*pas + math.pi
 
 			r2.requestInsertGhostNode(this.InstanceId, "Ghosts", -1, "", npc) 
 			local comp = r2:getInstanceFromId(npc.InstanceId)
 			comp.Selectable = false	
 		end
	end
end


function component.onPostHrcMove(this)

	for i=0, this.Ghosts.Size-1 do
		local entity = this.Ghosts[i]
		entity.DisplayerVisual:updateName()
		entity:updatePermanentStatutIcon()
	end	
	r2.events:updateElementsUI()
end

function component:getLogicTranslations()

	local logicTranslations = {
		["ApplicableActions"] = {
			["activate"]				= { menu=i18n.get( "uiR2AA0Spawn"					):toUtf8(), 
											text=i18n.get( "uiR2AA1Spawn"					):toUtf8()},
			["deactivate"]				= { menu=i18n.get( "uiR2AA0Despawn"					):toUtf8(), 
											text=i18n.get( "uiR2AA1Despawn"					):toUtf8()},
			["Wander"]					= { menu=i18n.get( "uiR2AA0BanditWander"			):toUtf8(), 
											text=i18n.get( "uiR2AA1BanditWander"			):toUtf8()},
			["Sit Down"]				= { menu=i18n.get( "uiR2AA0BanditSitDown"			):toUtf8(), 
											text=i18n.get( "uiR2AA1BanditSitDown"			):toUtf8()},
			["Kill"]					= { menu=i18n.get( "uiR2AA0BanditKill"				):toUtf8(), 
											text=i18n.get( "uiR2AA1BanditKill"				):toUtf8()},
		},
		["Events"] = {	
			["activation"]				= { menu=i18n.get( "uiR2Event0Spawn"				):toUtf8(), 
											text=i18n.get( "uiR2Event1Spawn"				):toUtf8()},
			["deactivation"]			= { menu=i18n.get( "uiR2Event0Despawn"				):toUtf8(), 
											text=i18n.get( "uiR2Event1Despawn"				):toUtf8()},
			["member death"]			= { menu=i18n.get( "uiR2Event0MemberDeath"			):toUtf8(), 
											text=i18n.get( "uiR2Event1MemberDeath"			):toUtf8()},
			["group death"]				= { menu=i18n.get( "uiR2Event0GroupDeath"			):toUtf8(), 
											text=i18n.get( "uiR2Event1GroupDeath"			):toUtf8()},
			["head to wander zone"]		= { menu=i18n.get( "uiR2Event0BanditHeadWander"		):toUtf8(), 
											text=i18n.get( "uiR2Event1BanditHeadWander"		):toUtf8()},
			["arrive at wander zone"]	= { menu=i18n.get( "uiR2Event0BanditArriveWander"	):toUtf8(), 
											text=i18n.get( "uiR2Event1BanditArriveWander"	):toUtf8()},
			["head to camp"]			= { menu=i18n.get( "uiR2Event0BanditHeadCamp"		):toUtf8(), 
											text=i18n.get( "uiR2Event1BanditHeadCamp"		):toUtf8()},
			["arrive at camp"]			= { menu=i18n.get( "uiR2Event0BanditArriveCamp"		):toUtf8(), 
											text=i18n.get( "uiR2Event1BanditArriveCamp"		):toUtf8()},
			
		},
		["Conditions"] = {	
			["is active"]				= { menu=i18n.get( "uiR2Test0Spawned"				):toUtf8(), 
											text=i18n.get( "uiR2Test1Spawned"				):toUtf8()},
			["is inactive"]				= { menu=i18n.get( "uiR2Test0Despawned"				):toUtf8(), 
											text=i18n.get( "uiR2Test1Despawned"				):toUtf8()},
			["is dead"]					= { menu=i18n.get( "uiR2Test0BanditDead"			):toUtf8(), 
											text=i18n.get( "uiR2Test1BanditDead"			):toUtf8()},
			["is wandering"]			= { menu=i18n.get( "uiR2Test0BanditPatrolZone"		):toUtf8(), 
											text=i18n.get( "uiR2Test1BanditPatrolZone"		):toUtf8()},
			["is sitting"]				= { menu=i18n.get( "uiR2Test0BanditAtCamp"			):toUtf8(), 
											text=i18n.get( "uiR2Test1BanditAtCamp"			):toUtf8()},
			["is heading to wander zone"]= {menu=i18n.get( "uiR2Test0BanditHeadPatrolZone"	):toUtf8(), 
											text=i18n.get( "uiR2Test1BanditHeadPatrolZone"	):toUtf8()},
			["is heading to camp"]		= { menu=i18n.get( "uiR2Test0BanditHeadCamp"		):toUtf8(), 
											text=i18n.get( "uiR2Test1BanditHeadCamp"		):toUtf8()},
		}
	}
	return logicTranslations
end

component.getLogicAction = function(entity, context, action)
	
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local banditEntity = r2:getInstanceFromId(component.User._BanditGroup)
	BOMB_IF(banditEntity, "In feature '".. component.Name .. "' associated bandit camp not found.")
	local banditRtNpcGrp = r2.Translator.getRtGroup( context, banditEntity.InstanceId)
	BOMB_IF(banditEntity, "In feature '" .. component.Name .. "' associated bandit camp rtGroup not found.")

	if action.Action.Type == "deactivate" then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 0)
		local action2 = r2.Translator.getNpcLogicActionDeactivate(banditEntity, context, action, banditRtNpcGrp)
		local action3 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 5)
		local multiaction = r2.Translator.createAction("multi_actions", {action1, action2, action3})
		return multiaction, multiaction
	elseif action.Action.Type == "activate" then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 1)
		local action2 = r2.Translator.getNpcLogicActionActivate(banditEntity, context, action, banditRtNpcGrp)
		local action3 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 4)
		local multiaction = r2.Translator.createAction("multi_actions", {action1, action2, action3})
		return multiaction, multiaction
	elseif action.Action.Type == "Kill" then
		return r2.Translator.getNpcLogicActionKill(banditEntity, context, action, banditRtNpcGrp)
	elseif action.Action.Type == "Sit Down" then
		return r2.Translator.getGenericLogicActionBeginActivitySequence(entity.User._SequenceHeadToFire, banditRtNpcGrp)
	elseif action.Action.Type == "Wander" then
		return r2.Translator.getGenericLogicActionBeginActivitySequence(entity.User._SequenceHeadToWander, banditRtNpcGrp)
	end
	
	local firstAction, lastAction = nil,nil

	return firstAction, lastAction
end



component.getLogicCondition = function(this, context, condition)

	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local banditEntity = r2:getInstanceFromId(component.User._BanditGroup)
	BOMB_IF(banditEntity, "In feature '".. component.Name .. "' associated bandit camp not found.")
	local banditRtNpcGrp = r2.Translator.getRtGroup( context, banditEntity.InstanceId)
	BOMB_IF(banditEntity, "In feature '" .. component.Name .. "' associated bandit camp rtGroup not found.")

	local conditionType = condition.Condition.Type
	if conditionType == "is wandering" then
		return r2.Translator.getGenericLogicConditionIsInActivitySequence(banditEntity, component.User._SequenceWander, banditRtNpcGrp)
	elseif conditionType == "is sitting" then
		return r2.Translator.getGenericLogicConditionIsInActivitySequence(banditEntity, component.User._SequenceStayNearFire, banditRtNpcGrp)
	elseif conditionType == "is heading to wander zone" then
		return r2.Translator.getGenericLogicConditionIsInActivitySequence(banditEntity, component.User._SequenceHeadToWander, banditRtNpcGrp)
	elseif conditionType == "is heading to camp" then
		return r2.Translator.getGenericLogicConditionIsInActivitySequence(banditEntity, component.User._SequenceHeadToFire, banditRtNpcGrp)
	end
	return r2.Translator.getFeatureActivationCondition(condition, rtNpcGrp)
	
end

component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 

	local component = this -- r2:getInstanceFromId(event.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	local banditEntity = r2:getInstanceFromId(component.User._BanditGroup)
	BOMB_IF(banditEntity, "In feature '".. component.Name .. "' associated bandit camp not found.")
	local banditRtNpcGrp = r2.Translator.getRtGroup( context, banditEntity.InstanceId)
	BOMB_IF(banditEntity, "In feature '" .. component.Name .. "' associated bandit camp rtGroup not found.")

	local eventType = event.Event.Type
	if eventType == "activation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 4)
	elseif eventType == "deactivation" then
		return r2.Translator.getComponentUserEvent(rtNpcGrp, 5)
	elseif eventType == "member death" or eventType == "group death" then
		return r2.Translator.getNpcLogicEvent(banditEntity, context, event)
	elseif eventType == "head to wander zone" then
		return r2.Translator.getNpcLogicEventBeginOfActivityStepImpl(this.User._StepHeadToWander, banditRtNpcGrp)
	elseif eventType == "arrive at wander zone" then
		return r2.Translator.getNpcLogicEventEndOfActivityStepImpl(this.User._StepHeadToWander, banditRtNpcGrp)
	elseif eventType == "head to camp" then
		return r2.Translator.getNpcLogicEventBeginOfActivityStepImpl(this.User._StepHeadToFire, banditRtNpcGrp)
	elseif eventType == "arrive at camp" then
		return r2.Translator.getNpcLogicEventEndOfActivityStepImpl(this.User._StepHeadToFire, banditRtNpcGrp)
	end
	
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	return eventHandler, firsCondition, lastCondition
end

component.Races = { [1] = "Fyros", [2] = "Matis", [3] = "Tryker", [4] = "Zorai"} 

component.RacesId = { [1] = "f", [2] = "m", [3] = "t", [4] = "z"}

component.LevelsId = { [1] = "20", [2] = "70", [3] = "120", [4] = "170", [5] = "220"}
	
component.RolesId = { 
	[1] = "melee_dd", [2] = "melee_tank", [3] = "light_melee", [4] = "mage_damage_dealer",
	[5] = "mage_aoe", [6] = "mage_atysian_curser", [7] = "mage_celestial_curser"
}

--component.createComponent = function(x, y, banditsCount, race, banditsLevel, cycle, ZoneSize)
component.createComponent = function(x, y, banditsCount, race, banditsLevel, ZoneSize)
	
	r2.requestNewAction(i18n.get("uiR2EDCreateBanditCampAction"))
	local comp = r2.newComponent("BanditCamp")
	assert(comp)

	-- base is the camp fire, to display the bandit camp as a camp fire
	comp.Base = "palette.entities.botobjects.campfire"
	
	--feature.BanditCount = feature.BanditCount + 1
	--comp.Name = "BanditCampFeature[".. feature.BanditCount .."]"
	comp.Name = r2:genInstanceName(i18n.get("uiR2EDNameBanditCampFeature")):toUtf8()			
	
	local fire = comp.Fire
	
	 --do
	--	fire.Base = "palette.entities.botobjects.campfire"
	--	fire.Name = "Camp Fire"
	--	local tmpPosition = fire.Position
	--	tmpPosition.x = x
	--	tmpPosition.y = y
	--	tmpPosition.z = r2:snapZToGround(x, y)
	--end
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)		

	comp._BanditsCount = banditsCount
	comp._Race = race
	comp._BanditsLevel = banditsLevel	
	comp.BanditsCount = tostring(banditsCount)
	comp.Race = component.Races[race+1]
	comp.BanditsLevel = component.LevelsId[banditsLevel + 1]
	--comp.Cycle = cycle
	comp.WanderDuration = 20
	comp.SitDuration = 20
	comp.ZoneSize = tostring(ZoneSize)

	local zone = r2.newComponent("Region")		
	r2.Utils.createNonDeleteableRegion(zone, 0, 0, BanditZoneRadius, BanditZoneNumCorners) -- the region doesn't inherit the feature position, so must give its pos
	zone.Deletable = 0
	zone.Position.x = comp.Position.x
	zone.Position.y = comp.Position.y
	zone.Position.z = comp.Position.z
	zone.InheritPos = 0 -- don't inherit position of parents
	zone.Name = r2:genInstanceName(i18n.get("uiR2EDNameWanderRegion")):toUtf8()			
	comp._Zone = zone.InstanceId
	table.insert(comp.Components, zone)
	comp._Seed = os.time() 
	
--	r2.registerText("Pas faché que cette journée se termine !")
--	r2.registerText("On rentre a la maison!")
--	r2.registerText("Asseyez vous mes frères!")
--	r2.registerText("Allons detrousser les honnetes gens!")
--	r2.registerText("Allez, au boulot!")
--	r2.registerText("A l'attaque!!")

	return comp
end
-- Global function are bad
function updateBanditCampEnum()	
	local currentForm = r2.CurrentForm
	if (currentForm == nil) then		
		return
	end
		
	local formRace = currentForm:find("Race")	
	if not formRace then 
		return 
	end
	local formLevel = currentForm:find("BanditsLevel")
	if not formLevel then 
		return 
	end
	local previousLevel = formLevel.selection
	formLevel:resetTexts()
	local k, v = next(component.LevelsId, nil)
	while k do
		local race = component.RacesId[formRace.selection + 1]
		local level = v
		local elementName = "palette.entities.npcs.bandits."..race.."_melee_dd_"..tostring(level)
		local ok = false
		if r2.isInPalette(elementName) then

			local paletteElt = r2.getPaletteElement(elementName)			
			if paletteElt and paletteElt.RingAccess and r2.RingAccess.testAccess(paletteElt.RingAccess) then
				formLevel:addText(ucstring(tostring(level)))
				ok = true
			--	local levelEnum = currentForm:find(creature.."Level")

			end
		end
		if k == previousLevel and not ok then
			formLevel.selection = 0
		end
		k, v = next(component.LevelsId, k)
	end


end

	local function initValue()

		local toRet = {}

		local k, v = next(component.LevelsId, nil)
		while k do
			local race = component.RacesId[0+ 1]
			local level = v
			local elementName = "palette.entities.npcs.bandits."..race.."_melee_dd_"..tostring(level)

			if r2.isInPalette(elementName) then
				local paletteElt = r2.getPaletteElement(elementName)			
				if paletteElt and paletteElt.RingAccess and r2.RingAccess.testAccess(paletteElt.RingAccess) then
					table.insert(toRet, level)

				end
			end
			k, v = next(component.LevelsId, k)
		end
		return toRet
	end


function feature.registerForms()
	r2.Forms.BanditCamp_Form =
	{
		Caption = "uiR2EDBanditCamp",
		Prop =
		{
			-- following field are tmp for property sheet building testing
			{Name = "BanditsCount", Type = "Number", Category = "uiR2EDRollout_BanditsCamp", Min = "1", Max = "12", Default = "6"},
			
			{Name = "Race", Type = "Number", WidgetStyle = "EnumDropDown", Category = "uiR2EDRollout_BanditsCamp",
			 Enum = component.Races,
			 onChange = updateBanditCampEnum
			},
			
			{Name = "BanditsLevel", Type = "Number", WidgetStyle = "EnumDropDown", Category = "uiR2EDRollout_BanditsCamp",
			 Enum = initValue(),
			 onChange = updateBanditCampEnum
			},

			--{Name = "Cycle", Type = "Number", Category = "uiR2EDRollout_BanditsCamp", Min = "0", Max = "999999", Default = "30"},
		}
	}
end








-- read values from a BanditsComp and get Sheet Id
component.getBanditPaletteId = function(comp)
--	local tmpId = "palette.entities.npcs.cuthroats.cuthroat_b_melee_a_z_f"
--	return tmpId
	local id = "palette.entities.npcs.bandits." .. 
		component.RacesId[comp._Race + 1] .."_" ..
		component.RolesId[math.random(0,5) + 1] .."_" ..
		component.LevelsId[comp._BanditsLevel + 1]
	
	return id
end

component.init = function()
	math.randomseed( os.time() ) -- To remove
end

component.createGhostComponents = function(comp, act)
	local bandits
	local npcGroup = r2.newComponent("NpcGrpFeature")
	assert(npcGroup)


	local leader = nil
	local sequences = {}
	local stepSitDown = nil
	local stepStandUp = nil


	do

		local fire = r2.newComponent("Npc")
		fire.Name = "Fire"	
		fire.Base = "palette.entities.botobjects.campfire"
		fire.Position.x = comp.Position.x
		fire.Position.y = comp.Position.y
		fire.Position.z = 0
		fire.Angle = 0
		r2.requestInsertGhostNode(act.Features[0].InstanceId, "Components", -1, "", fire)
	end

	if comp._Seed then math.randomseed(comp._Seed) end
	do
		local x = comp.Position.x
		local y = comp.Position.y
		local n = comp._BanditsCount
		
		local pas = (2 * math.pi)/n
		local r = (n/(2*math.pi))+2						
		for i = 1, n do
			local npc = r2:randomNPC( component.getBanditPaletteId(comp), component.Races[comp._Race + 1])
			npc.Position.x = (r-1) * math.cos((i-1)*pas) -- make position relative to the feature positio
 			npc.Position.y = (r-1) * math.sin((i-1)*pas) 
 			npc.Position.z = 0
 			npc.Angle = (i-1)*pas + math.pi
  		
			if comp.Active == 1 then npc.AutoSpawn = 1 else npc.AutoSpawn = 0 end
			if i == 1 then leader = npc end
			
			if comp.NoRespawn then
--				debugInfo("setting no respawn for a bandit -- value : " ..comp.NoRespawn)
				npc.NoRespawn = comp.NoRespawn
			end
		
			table.insert(npcGroup.Components, npc)
		end
		npcGroup.Name = r2:genInstanceName(i18n.get("uiR2EDNameBanditCamp")):toUtf8()	
		bandits = npcGroup
		bandits.Position.x = x
		bandits.Position.y = y
		bandits.InheritPos = 0
	end

	-- Sequence1 standUp -> Wander
	do
		local sequence = r2.newComponent("ActivitySequence")


		table.insert(leader.Behavior.Activities, sequence)
		table.insert(bandits.ActivitiesId, sequence.InstanceId)
		sequences[1] = sequence

		sequence.Name = "Head to Wander"
		sequence.Repeating = 0

		-- Initial wait
		do

			local step = r2.newComponent("ActivityStep")
			table.insert(sequence.Components, step)
			step.Type = "None"
			step.Name = "Stand Up"

			step.Activity = "Stand Up"	
			step.ActivityZoneId = r2.RefId("")

			step.TimeLimit = "Few Sec"
			step.TimeLimitValue = "5"
		end 


		-- head to Wander
		do
			local step = r2.newComponent("ActivityStep")
			comp.User._StepHeadToWander = step.InstanceId
			table.insert(sequence.Components, step)
			step.Type = "None"


			step.Activity = "Go To Zone"	
			step.ActivityZoneId = r2.RefId(comp._Zone)

			step.TimeLimit = "No Limit"
			-- step.TimeLimitValue = ""
			-- step.Chat = r2.RefId("")
		end

		-- Go to zone until arrived
	end

	-- Sequence2  Wander
	do
		local sequence = r2.newComponent("ActivitySequence")


		table.insert(leader.Behavior.Activities, sequence)
		table.insert(bandits.ActivitiesId, sequence.InstanceId)
		sequences[2] = sequence

		sequence.Name = "Wander"
		sequence.Repeating = 0
		-- Stand up
		do
			local step = r2.newComponent("ActivityStep")
			table.insert(sequence.Components, step)
			step.Type = "None"
			step.Name = "Wander"

			step.Activity = "Wander"
			step.ActivityZoneId = r2.RefId(comp._Zone)

			--if comp.Cycle ~= 0 then
			if comp.WanderDuration ~= 0 then
				step.TimeLimit = "Few Sec"
				step.TimeLimitValue = tostring( math.ceil(comp.WanderDuration) )
			else
				step.TimeLimit = "No Limit"
			end
			-- step.TimeLimitValue = ""
			-- step.Chat = r2.RefId("")

		end
		-- Go to zone until arrived
	end


	-- Sequence3 wander - >sit
	do
		local sequence = r2.newComponent("ActivitySequence")
		sequence.Name = "Head to Fire"
		sequence.Repeating = 0
		table.insert(leader.Behavior.Activities, sequence)
		table.insert(bandits.ActivitiesId, sequence.InstanceId)
		
		sequences[3] = sequence



		-- to fire
		do
			local step = r2.newComponent("ActivityStep")
			comp.User._StepHeadToFire = step.InstanceId
			table.insert(sequence.Components, step)
			step.Type = "None"
			step.Name = "head to fire"

			step.Activity = "Go To Start Point"	
			step.ActivityZoneId = r2.RefId("")

			step.TimeLimit = "No Limit"
			stepSitDown = step
			-- step.TimeLimitValue = ""
			-- step.Chat = r2.RefId("")
		end

		do
			local step = r2.newComponent("ActivityStep")			
			table.insert(sequence.Components, step)
			step.Type = "None"
			step.Name = "Sit Down"

			step.Activity = "Sit Down"	
			step.ActivityZoneId = r2.RefId("")

			step.TimeLimit = "Few Sec"
			step.TimeLimitValue = "5"
		end

	end
		-- Sequence3 wander - >sit
	do
		local sequence = r2.newComponent("ActivitySequence")
		sequence.Name = "Head to Fire"
		sequence.Repeating = 0
		table.insert(leader.Behavior.Activities, sequence)
		table.insert(bandits.ActivitiesId, sequence.InstanceId)
		
		sequences[4] = sequence
	-- stand up : go to start point
		do
			local step = r2.newComponent("ActivityStep")

			table.insert(sequence.Components, step)
			step.Type = "None"
			step.Name = "Initial Wait"

			step.Activity = "Stand On Start Point"	
			step.ActivityZoneId = r2.RefId("")

			--if comp.Cycle ~= 0 then
			if comp.SitDuration ~= 0 then
				step.TimeLimit = "Few Sec"
				step.TimeLimitValue = tostring( math.ceil(comp.SitDuration) )
			else
				step.TimeLimit = "No Limit"
			end			
			-- step.Chat = r2.RefId("")
		end


	
	end

	-- activitySequence1 -> activitySequence2
	do
		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Name = "Heading to wander"
		table.insert(leader.Behavior.Actions, eventHandler)

		local action = r2.newComponent("ActionStep")
		table.insert(eventHandler.Actions, action)
		action.Entity = r2.RefId(leader.InstanceId)
		action.Action.Type = "begin activity sequence"
		action.Action.Value = r2.RefId(sequences[2].InstanceId)


		eventHandler.Event.Type = "end of activity sequence"
		eventHandler.Event.Value = r2.RefId(sequences[1].InstanceId)
	end

	-- activitySequence3 -> activitySequence4
	do
		local eventHandler = r2.newComponent("LogicEntityAction")
		eventHandler.Name = "Wander"
		table.insert(leader.Behavior.Actions, eventHandler)

		local action = r2.newComponent("ActionStep")
		table.insert(eventHandler.Actions, action)
		action.Entity = r2.RefId(leader.InstanceId)
		action.Action.Type = "begin activity sequence"
		action.Action.Value = r2.RefId(sequences[4].InstanceId)
		eventHandler.Event.Type = "end of activity sequence"
		eventHandler.Event.Value = r2.RefId(sequences[3].InstanceId)
	end

		--if comp.Cycle ~= 0 then
	if comp.WanderDuration ~= 0 then	
	-- activitySequence2 -> activitySequence3
		do
			local eventHandler = r2.newComponent("LogicEntityAction")
			eventHandler.Name = "Heading to Fire"
			table.insert(leader.Behavior.Actions, eventHandler)

			local action = r2.newComponent("ActionStep")
			table.insert(eventHandler.Actions, action)
			action.Entity = r2.RefId(leader.InstanceId)
			action.Action.Type = "begin activity sequence"
			action.Action.Value = r2.RefId(sequences[3].InstanceId)


			eventHandler.Event.Type = "end of activity sequence"
			eventHandler.Event.Value = r2.RefId(sequences[2].InstanceId)
		end
	end

		-- activitySequence4 -> activitySequence1
	if comp.SitDuration ~= 0 then
		do
			local eventHandler = r2.newComponent("LogicEntityAction")
			eventHandler.Name = "Stay around fire"
			table.insert(leader.Behavior.Actions, eventHandler)

			local action = r2.newComponent("ActionStep")
			table.insert(eventHandler.Actions, action)
			action.Entity = r2.RefId(leader.InstanceId)
			action.Action.Type = "begin activity sequence"
			action.Action.Value = r2.RefId(sequences[1].InstanceId)
			eventHandler.Event.Type = "end of activity sequence"
			eventHandler.Event.Value = r2.RefId(sequences[4].InstanceId)
		end
	end


	r2.requestInsertGhostNode(act.InstanceId, "Features", -1, "", bandits)
--	r2.requestInsertGhostNode(this.InstanceId, "Ghost", -1, "", ghostNpc)
--	r2.requestInsertGhostNode(r2:getCurrentAct().InstanceId, "Features", -1, "", bandits)
--	
	local k, sequence = next(sequences, nil)
	while k do
		r2.requestInsertGhostNode(act.InstanceId, "ActivitiesIds", -1, "", sequence.InstanceId)
		k, sequence = next(sequences, k)
	end

	local k, bandit = next(npcGroup.Components, nil)
	while k do
		r2:getInstanceFromId(bandit.InstanceId).Selectable = true -- for debug
		k, bandit = next(npcGroup.Components, k)
	end

	comp.User._BanditGroup = npcGroup.InstanceId
	comp.User._SequenceHeadToWander = sequences[1].InstanceId
	comp.User._SequenceWander = sequences[2].InstanceId
	comp.User._SequenceHeadToFire = sequences[3].InstanceId
	comp.User._SequenceStayNearFire = sequences[4].InstanceId

end

component.create = function()	
	if not r2:checkAiQuota() then return end

	local function paramsOk(resultTable)
		
		local banditsCount = tonumber( resultTable["BanditsCount"] )
		if not r2:checkAiQuota(banditsCount) then return end

		local race = tonumber( resultTable["Race"] )
		local banditsLevel = tonumber( resultTable["BanditsLevel"] )
		local x = tonumber( resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		--local cycle = tonumber( resultTable["Cycle"] )
		--local sitDuration = tonumber( resultTable["SitDuration"] )
		--local wanderDuration= tonumber( resultTable["WanderDuration"] )
		local ZoneSize = tonumber(resultTable["ZoneSize"])
		if banditsCount == nil or race == nil or banditsLevel == nil 
		then
			debugInfo("Can't create Feature")
			return
		end

		--local component = feature.Components.BanditCamp.createComponent( x, y, banditsCount, race, banditsLevel, cycle,ZoneSize)
		local component = feature.Components.BanditCamp.createComponent( x, y, banditsCount, race, banditsLevel, ZoneSize)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)

--		feature.createContent(component)
		
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for bandit camp creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of bandit camp at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("BanditCamp") == 1 then 
			r2.displayFeatureHelp("BanditCamp")
		end
		r2:doForm("BanditCamp_Form", {X = x, Y = y}, paramsOk, paramsCancel)
		updateBanditCampEnum()
	end
	local function posCancel()
		debugInfo("Cancel choice of bandit camp position")
	end		
	local poly = {}
	local step = 2 * math.pi / BanditZoneNumCorners
	for k = 0, BanditZoneNumCorners - 1 do
		table.insert(poly, CVector2f(BanditZoneRadius * math.cos(k * step), BanditZoneRadius * math.sin(k * step)))
	end	
	r2:choosePos("object_campfire.creature", posOk, posCancel, "createFeatureBanditCamp", 
				 "curs_create.tga",
				 "curs_stop.tga",
	             { poly }, r2.PrimRender.ComponentRegionLook, r2.PrimRender.ComponentRegionInvalidLook)
end





function feature.hasScenarioCost(this)
	return true
end

component.init()
r2.Features["BanditCampFeature"] = feature


