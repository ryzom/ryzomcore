--
-- *****************
-- * FAUNA FEATURE *
-- *****************
--
-- The fauna feature contains 2 herds of creatures (herbivores and carnivores) that wander between 2 life zones (sleep zone and 
-- food zone).There are 2 differents kinds of components in this feature: fauna system and creature.
-- The fauna system component is some kind of manager for the feature. It creates the creatures and their life zones, and then
-- store them in its components table, so that their createChostComponents call and translation are automatically done by 
-- the translator.
-- The created life zones are affected to each of the creature components. But the properties panel of the creature components allows
-- the DM to choose other zones in the scenario through RefId picking.


r2.Features.FaunaFeature = {}

local feature = r2.Features.FaunaFeature

feature.Name="FaunaFeature"

feature.Description="Generates a pack of carnivores and a herd of herbivores that will wander between two life zones (sleep zone, food zone) each."

feature.Components = {}


-- *********************
-- * FEATURE FUNCTIONS *
-- *********************


--Form functions

-- Reinit makes sure the enums are reinitialized, ie contain all creatures (respectively herbivores or carnivores) from 
-- the desert ecosystem and with a level between 1 and 50.
local function reinit(form, creature)
	if form == nil then 
		debugInfo("Reinit impossible: nil form")
		return
	end

	local creatureEnum = form:find(creature.."Race")
	if creatureEnum == nil then
		debugInfo("Reinit impossible: can't find "..creature.."Race enum")
		return
	end
	creatureEnum:resetTexts()
	
	local creaturePalette = {}
	if creature == "Carnivore" then
		creaturePalette = r2.Palette.Entries.creature.creatures_predators.instances
	else 
		creaturePalette = r2.Palette.Entries.creature.creatures_passive.instances	
	end

	local k, v = next(creaturePalette, nil)
	while k do
		if r2.isInPalette(v.Id) then
			local paletteElt = r2.getPaletteElement(v.Id)
			if paletteElt and paletteElt.RingAccess and 
					r2.RingAccess.testAccess(paletteElt.RingAccess) then

				if paletteElt.Ecosystem == "Desert" and paletteElt.Level >= 1 and paletteElt.Level <= 50 then 
					creatureEnum:addText(ucstring(i18n.get(v.Translation)))
				end
			end
		end
		k, v = next(creaturePalette, k)
	end
end

-- Returns the chosen bases from the form before creating the components.
local function getBase(creature, form)
	if (form == nil) then
		debugInfo("getBase: form is nil")
		return
	end

	local creaturePalette = {}
	if creature == "Carnivore" then
		creaturePalette = r2.Palette.Entries.creature.creatures_predators.instances
	else 
		creaturePalette = r2.Palette.Entries.creature.creatures_passive.instances	
	end

	local creatureEnum = form:find(creature.."Race")
	local race = creatureEnum.selection_text

	local k, v = next(creaturePalette, nil)
	while k do
		local name = i18n.get(v.Translation):toUtf8()
		if name == race then
			return v.Id, name
		end
		k, v = next(creaturePalette, k)
	end
end

	-- When the selected ecosystem or level changes, the corresponding creature combobox is updated.
	local function updateEnum(creature)	
		local currentForm = r2.CurrentForm
		if (currentForm == nil) then
			debugInfo("UpdatePredators: r2.CurrentForm is nil")
			return
		end
			
		local creatureEnum = currentForm:find(creature.."Race")
		local ecoEnum = currentForm:find(creature.."Ecosystem")
		local currentEco = ecoEnum.selection_text
		if currentEco == "Lakes" then
			currentEco = "Lacustre"
		end

		local levelEnum = currentForm:find(creature.."Level")
		local levelRange = levelEnum.selection + 1		
		local levelMin
		local levelMax
		if levelRange == 0 then 
			levelMin = 1
			levelMax = 250
		else
			levelMin = (levelRange - 1) * 50 + 1
			levelMax = levelMin + 49
		end	

		creatureEnum:resetTexts()
		
		local creaturePalette = {}
		if creature == "Carnivore" then
			creaturePalette = r2.Palette.Entries.creature.creatures_predators.instances
		else 
			creaturePalette = r2.Palette.Entries.creature.creatures_passive.instances	
		end

		local k, v = next(creaturePalette, nil)
		while k do
			if r2.isInPalette(v.Id) then
				local paletteElt = r2.getPaletteElement(v.Id)
				if paletteElt and paletteElt.RingAccess and 
					r2.RingAccess.testAccess(paletteElt.RingAccess) then
					if paletteElt.Ecosystem == currentEco and paletteElt.Level >= levelMin and paletteElt.Level <= levelMax then 
						creatureEnum:addText(ucstring(i18n.get(v.Translation)))
					end
				end
			end
			k, v = next(creaturePalette, k)
		end
	end

	-- Calls update function for the carnivores combobox.
	local function updateCarnivores(form)
		updateEnum("Carnivore")
	end

	-- Calls update function for the herbivores combobox.
	local function updateHerbivores(form)
		updateEnum("Herbivore")
	end

	local function resetForm()

		local currentForm = r2:getForm("Fauna_Form")
		assert(currentForm)

		do
			local creature = "Carnivore"
			local ecoEnum = currentForm:find(creature.."Ecosystem")
			ecoEnum.selection_text= "Desert"
			local levelEnum = currentForm:find(creature.."Level")
			levelEnum.selection = 0
			updateCarnivores(form)
		end
		do
			local creature = "Herbivore"
			local ecoEnum = currentForm:find(creature.."Ecosystem")
			ecoEnum.selection_text= "Desert"
			local levelEnum = currentForm:find(creature.."Level")
			levelEnum.selection = 0
			updateHerbivores(form)
		end
	
	end
--
-- The creation form lets the DM choose the type of creatures and their number. 
-- Like the palette tree, the available creatures are filtered by their ecosystems and level.
-- Each life cycle duration is defined in the properties panel of the creature component.
-- The creation form will return the chosen creature base when ok button is pressed.
-- The form is reinitialized each time the ok or cancel button is pressed.
--
feature.registerForms = function()	

	-- Initializes the creature comboboxes. Default ecosystem is desert and default level range is 1-50.
	local function init(creature)
		local initEnum = {}
		
		local creaturePalette = {}
		if not r2.Palette.Entries.creature then
			return  -- special case for the 'light' palette
		end
		if creature == "Carnivore" then
			creaturePalette = r2.Palette.Entries.creature.creatures_predators.instances
		else 
			creaturePalette = r2.Palette.Entries.creature.creatures_passive.instances	
		end		
		local k, v = next(creaturePalette, nil)
		while k do
			if r2.isInPalette(v.Id) then
				local paletteElt = r2.getPaletteElement(v.Id)
				if paletteElt then
					if paletteElt.Ecosystem == "Desert" and paletteElt.Level >= 1 and paletteElt.Level <= 50 then 
						table.insert(initEnum, i18n.get(v.Translation))
					end
				end
			end
			k, v = next(creaturePalette, k)
		end

		return initEnum
	end

	r2.Forms.Fauna_Form =
	{
		Caption = "uiR2EdFauna",
		Prop =
		{
			{Name="CarnivoresCount", Type="Number", Category="uiR2EDRollout_Carnivores", Min="1", Max="12", Default="3", Translation="uiR2EDProp_Count"},
			{Name="CarnivoreRace", Type="Number", Category="uiR2EDRollout_Carnivores", WidgetStyle="EnumDropDown", Translation="uiR2EDProp_Race",
			Enum= init("Carnivore")
			},
			{Name="CarnivoreEcosystem", Type="Number", Category="uiR2EDRollout_Carnivores", WidgetStyle="EnumDropDown", Translation="uiR2EDProp_Ecosystem",
			Enum={"Desert", "Forest", "Jungle", "Lakes", "PrimeRoots", "Goo"},
			onChange = updateCarnivores
			},
			{Name="CarnivoreLevel", Type="Number", Category="uiR2EDRollout_Carnivores", WidgetStyle="EnumDropDown", Translation="uiR2EDProp_Level",
			Enum={"1-50", "51-100", "101-150", "151-200", "201-250"},
			onChange = updateCarnivores 
			},
			{Name="HerbivoresCount", Type="Number", Category="uiR2EDRollout_Herbivores", Min="1", Max="12", Default="7", Translation="uiR2EDProp_Count"}, 
			{Name="HerbivoreRace", Type="Number", Category="uiR2EDRollout_Herbivores", WidgetStyle="EnumDropDown", Translation="uiR2EDProp_Race",
			Enum= init("Herbivore")
			},
			{Name="HerbivoreEcosystem", Type="Number", Category="uiR2EDRollout_Herbivores", WidgetStyle="EnumDropDown", Translation="uiR2EDProp_Ecosystem",
			Enum={"Desert", "Forest", "Jungle", "Lakes", "PrimeRoots", "Goo"},
			onChange = updateHerbivores
			},
			{Name="HerbivoreLevel", Type="Number", Category="uiR2EDRollout_Herbivores", WidgetStyle="EnumDropDown", Translation="uiR2EDProp_Level",
			Enum={"1-50", "51-100", "101-150", "151-200", "201-250"},
			onChange = updateHerbivores
			},		
		}
	}

end

-- **************
-- * COMPONENTS *
-- **************

local FaunaRegionRadius = 5


	-----------------------------------------------------------------------------------------------	
	-- CREATURE COMPONENT

feature.Components.Creature =
{
	BaseClass="LogicEntity",			
	Name="Creature",
	Menu="ui:interface:r2ed_feature_menu",
	
	DisplayerUI = "R2::CDisplayerLua",
	DisplayerUIParams = "defaultUIDisplayer",
	DisplayerVisual = "R2::CDisplayerVisualEntity",
	-----------------------------------------------------------------------------------------------	
	Parameters = {},

	ApplicableActions = {},

	Events = {},

	Conditions = {},

	TextContexts =		{},

	TextParameters =	{},

	LiveParameters =	{},
	-----------------------------------------------------------------------------------------------	
	Prop = 
	{
		{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
		{Name="Components", Type="Table", Visible= false},
		{Name="Ghosts", Type = "Table", Visible = false },
		{Name="Name", Type="String", MaxNumChar="32"},
		{Name="CrittersCount", Type="String", WidgetStyle="StaticText", Translation="uiR2EDProp_CrittersCount"},
		{Name="RaceBase", Type="String", WidgetStyle="StaticText", Visible=false},
		{Name="RaceName", Type="String", WidgetStyle="StaticText", Translation="uiR2EDProp_Race"},
		{Name="SleepZone", Type="RefId", PickFuntion = "r2:canPickZone", SetRefIdFunction = "r2:setZoneRefIdTarget", Translation="uiR2EDProp_SleepZone"},
		{Name="FoodZone", Type="RefId", PickFuntion = "r2:canPickZone", SetRefIdFunction = "r2:setZoneRefIdTarget", Translation="uiR2EDProp_FoodZone"},
		{Name="FoodDuration", Type="Number", Category="uiR2EDRollout_LifeCyclesInSeconds", Min="1", Max="40000", Default="30", Translation="uiR2EDProp_FoodDuration"},
		{Name="SleepDuration", Type="Number", Category="uiR2EDRollout_LifeCyclesInSeconds", Min="1", Max="40000", Default="30", Translation="uiR2EDProp_SleepDuration"},
	},

	
	getAvailableCommands = function(this, dest)	
		r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
		this:getAvailableDisplayModeCommands(dest)
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
		this:createGhostComponents()
	end,

	translate = function(this, context)
		--local rtNpcGrp = r2.Translator.getRtGroup(context, context.Feature.InstanceId)
		--local aiState = r2.newComponent("RtAiState")
		--aiState.AiActivity = "normal"
		--table.insert(context.RtAct.AiStates, aiState)
		--table.insert(aiState.Children, rtNpcGrp.Id)
		--local rtNpcGrp = r2.Translator.getRtGroup(context, this.InstanceId)
		--r2.Translator.translateEventHandlers( context, this, this.Behavior.Actions, rtNpcGrp)
		r2.Translator.translateAiGroup(this, context)
		
	end,

	pretranslate = function(this, context)
		--local instance = r2:getInstanceFromId(context.Feature.InstanceId);
		--r2.Translator.registerManager(context, context.Feature)

		r2.Translator.createAiGroup(this, context)
	end
}

local component = feature.Components.Creature

component.getLogicAction = function(entity, context, action)
	return nil, nil
end

component.getLogicCondition = function(this, context, condition)
	return nil,nil
end

component.getLogicEvent = function(this, context, event)		
	return nil, nil, nil
end

function component:getLogicTranslations()

	local logicTranslations = {
		["ApplicableActions"] = {},
		["Events"] = {}
	}
	return nil--logicTranslations
end

component.createGhostComponents= function(this, act)
	local comp = this
	local leader = nil
	local herd = nil
	local isHerbivore

	if act then
		herd = r2.newComponent("NpcGrpFeature")
	end

	if comp._Seed then math.randomseed(comp._Seed) end

	do
		local x = comp.Position.x
		local y = comp.Position.y
		local n = comp._CrittersCount 
		local pas = (2 * math.pi)/n
		local r = (n/(2*math.pi))+2							
		for i = 1, n do
			local npc = r2.newComponent("Npc") 
			npc.Name = comp.RaceName				
			npc.Base = comp.RaceBase --"palette.entities.creatures.cbadc1"
			
			
			local rr = FaunaRegionRadius - 1--r + math.random() * 5
			npc.Position.x = (rr-1) * math.cos((i-1)*pas)
			npc.Position.y = (rr-1) * math.sin((i-1)*pas) 
			npc.Position.z = 0
			npc.Angle = 2 * math.pi * math.random(0, 100)/100.0
			
			if i == 1 then 
				local manager = r2:getInstanceFromId(comp.ManagerId)
				assert(manager)
				if manager.Active == 1 then npc.AutoSpawn = 1 else npc.AutoSpawn = 0 end
				leader = npc
				r2.requestSetGhostNode(comp.InstanceId, "_LeaderId",npc.InstanceId)
				isHerbivore = r2.getPropertyValue(npc, "IsHerbivore")
			end
			
			if act then -- TestMode --
				table.insert(herd.Components, npc)
			else 
				r2.requestInsertGhostNode(this.InstanceId, "Ghosts", -1, "", npc)
				r2:getInstanceFromId(npc.InstanceId).Selectable = false	
			end				
		end
	end

	-- 2 wander sequences corresponding to the activities in both life zones attached to the herd
	do
		local sequence = r2.newComponent("ActivitySequence")


		table.insert(leader.Behavior.Activities, sequence)

		sequence.Name = "Life Cycle"
		sequence.Repeating = 1

			-- Wander in sleep zone
			local step = r2.newComponent("ActivityStep")
			table.insert(sequence.Components, step)
			step.Type = "None"
			step.Name = "Rest In Zone"
			step.Activity = "Rest In Zone"
			step.ActivityZoneId = r2.RefId(comp.SleepZone)
			
			step.TimeLimit = "Few Sec"
			step.TimeLimitValue = tostring(comp.SleepDuration)

			-- Wander in food zone
			local step = r2.newComponent("ActivityStep")
			table.insert(sequence.Components, step)
			step.Type = "None"
			
			if isHerbivore == 1 then
				step.Name = "Feed In Zone"
				step.Activity = "Feed In Zone"
			else
				step.Name = "Hunt In Zone"
				step.Activity = "Hunt In Zone"
			end
			step.ActivityZoneId = r2.RefId(comp.FoodZone)

			step.TimeLimit = "Few Sec"
			step.TimeLimitValue = tostring(comp.FoodDuration)

	end 
	
	if act then 
		comp.User._Herd = herd.InstanceId
		--herd.Name = r2:genInstanceName(i18n.get("uiR2EdFaunaFeature")):toUtf8()
		herd.InheritPos = 0
		herd.Position.x = comp.Position.x
		herd.Position.y = comp.Position.y
		r2.requestInsertGhostNode(act.InstanceId, "Features", -1, "", herd)
	end
end


component.createComponent = function(x, y, nbcritters, raceBase, raceName, 
	sleepZoneId, foodZoneId)
	

	local comp = r2.newComponent("Creature")
	assert(comp)
		
	--comp.Base = "palette.entities.botobjects.user_event" 
	comp.Base = r2.Translator.getDebugBase("palette.entities.botobjects.user_event")
	
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdCreatureComponent")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)

	comp._CrittersCount= nbcritters
	comp.CrittersCount = tostring(nbcritters)
	comp.RaceBase = raceBase
	comp.RaceName = raceName

	comp.SleepDuration = 30
	comp.FoodDuration = 30

	comp._Seed = os.time() 

	comp.SleepZone = sleepZoneId
	comp.FoodZone = foodZoneId

	return comp
end

	-----------------------------------------------------------------------------------------------	
	-- FAUNA SYSTEM COMPONENT
	-- Fauna system containing 2 Creature components (herbivores & carnivores) and 3 life zones.
feature.Components.Fauna = 
{
	BaseClass="LogicEntity",			
	Name="Fauna",
	Menu="ui:interface:r2ed_feature_menu",
	InEventUI = true,
	
	DisplayerUI = "R2::CDisplayerLua",
	DisplayerUIParams = "defaultUIDisplayer",
	DisplayerVisual = "R2::CDisplayerVisualEntity",
	-----------------------------------------------------------------------------------------------	
	Parameters = {},

	ApplicableActions = {"activate", "deactivate"},

	Events = {"deactivation", "activation"},

	Conditions = {"is active", "is inactive"},

	TextContexts =		{},

	TextParameters =	{},

	LiveParameters =	{},
	-----------------------------------------------------------------------------------------------	
	Prop =
	{
		{Name="InstanceId", Type="String", WidgetStyle="StaticText", Visible = false},
		{Name="Components", Type="Table", Visible = false},
		{Name="Ghosts", Type = "Table", Visible = false },
		{Name="Name", Type="String", MaxNumChar="32"},
		{Name="Active", Type = "Number", WidgetStyle="Boolean", DefaultValue="1" },
		{Name="CarnivoresCount", Type="String", Category="uiR2EDRollout_Carnivores", WidgetStyle="StaticText", Translation="uiR2EDProp_Count"},
		{Name="CarnivoreBase", Type="String", Category="uiR2EDRollout_Carnivores", WidgetStyle="StaticText", Visible = false},
		{Name="CarnivoreRace", Type="String", Category="uiR2EDRollout_Carnivores", WidgetStyle="StaticText", Translation="uiR2EDProp_Race"},
		{Name="HerbivoresCount", Type="String", Category="uiR2EDRollout_Herbivores", WidgetStyle="StaticText",Translation="uiR2EDProp_Count"},
		{Name="HerbivoreBase", Type="String", Category="uiR2EDRollout_Herbivores", WidgetStyle="StaticText", Visible = false},
		{Name="HerbivoreRace", Type="String", Category="uiR2EDRollout_Herbivores", WidgetStyle="StaticText", Translation="uiR2EDProp_Race"},
	},

	getParentTreeNode = function(this)
		return this:getFeatureParentTreeNode()
	end,

	getAvailableCommands = function(this, dest)	
		r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
		this:getAvailableDisplayModeCommands(dest)
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
	end
}

component = feature.Components.Fauna


local FaunaRegionNumCorners = 6
--local FaunaRegionRadius = 5
local FaunaRegionOffsets = { { x = 10, y = 0 }, { x = -7, y = -7}, { x = 0, y = 10} }


component.create = function()	
	if not r2:checkAiQuota() then return end

	local function paramsOk(resultTable, form)
		

		local carnivoreBase, carnivoreName = getBase("Carnivore", form)
		local herbivoreBase, herbivoreName = getBase("Herbivore", form)
		
		local x = tonumber( resultTable["X"] )
		local y = tonumber( resultTable["Y"] )
		
		local carnCount = tonumber(resultTable["CarnivoresCount"])
		local herbCount = tonumber(resultTable["HerbivoresCount"])
		
		
		if not r2:checkAiQuota(carnCount + herbCount + 1) then return end


		if not x or not y or not carnivoreBase or not carnivoreName or not carnCount or not herbivoreBase 
			or not herbivoreName or not herbCount
		then			
			r2:doForm("Fauna_Form", resultTable , paramsOk, paramsCancel)
			resetForm()
	
			printMsg("FaunaSystem: Failed to create component either because your scenario is full or because you don't yet have access to creatures of the level and ecosystem that you have selected")

			return
		end
		r2.requestNewAction(i18n.get("uiR2EDNewFaunaFeatureAction"))
		local component = feature.Components.Fauna.createComponent(x, y, carnCount, carnivoreBase, carnivoreName,
		herbCount, herbivoreBase, herbivoreName)
		r2:setCookie(component.InstanceId, "DisplayProp", 1)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
		resetForm()
	end
	
	local function paramsCancel(data, form)
		resetForm()
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'FaunaFeature' at pos (%d, %d, %d)", x, y, z))
		if r2.mustDisplayInfo("Fauna") == 1 then 
			r2.displayFeatureHelp("Fauna")
		end
		r2:doForm("Fauna_Form", {X=x, Y=y}, paramsOk, paramsCancel)
		resetForm()
	end
	local function posCancel()
		debugInfo("Cancel choice 'FaunaFeature' position")
	end	
	--r2:choosePos("object_component_user_event.creature", posOk, posCancel, "createFeatureFauna")
	local creature = r2.Translator.getDebugCreature("object_component_user_event.creature")
	--r2:choosePos(creature, posOk, posCancel, "createFeatureFauna")

	local polys = {}
	for p = 1, table.getn(FaunaRegionOffsets) do
		local poly = {}
		local step = 2 * math.pi / FaunaRegionNumCorners
		for k = 0, FaunaRegionNumCorners - 1 do
			table.insert(poly, CVector2f(FaunaRegionOffsets[p].x + FaunaRegionRadius * math.cos(k * step),
			                             FaunaRegionOffsets[p].y + FaunaRegionRadius * math.sin(k * step)))
		end
		table.insert(polys, poly)
	end
	r2:choosePos(creature, posOk, posCancel, "createFeatureFauna", 
				 "curs_create.tga",
				 "curs_stop.tga",
	             polys, r2.PrimRender.ComponentRegionLook, r2.PrimRender.ComponentRegionInvalidLook)
end

function component.getAiCost(this)
	if this.User.GhostDuplicate then return 0 end
	return r2.getAiCost(this) - 2
end




--
-- create the fauna system component by creating and inserting zones and creature component into its own components table.
-- Generates a sleep zone and a food zone for the herbivores, and a sleep zone for the carnivores. The carnivore hunt zone is 
-- one of the herbivore zones (default is herbivore sleep zone).
--
component.createComponent = function(x, y, carnCount, carnivoreBase, carnivoresName,
		herbCount, herbivoreBase, herbivoresName)
	
	local comp = r2.newComponent("Fauna")
	assert(comp)
	
	--TODO: replace this milestone base by some default feature base
	comp.Base = "palette.entities.botobjects.user_event" 

	comp.Name = r2:genInstanceName(i18n.get("uiR2EdFaunaFeature")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	
	comp._CarnCount= carnCount
	comp.CarnivoresCount = tostring(carnCount)
	comp.CarnivoreBase = carnivoreBase
	comp.CarnivoreRace = tostring(carnivoresName)
	
	comp._HerbCount= herbCount
	comp.HerbivoresCount = tostring(herbCount)
	comp.HerbivoreBase = herbivoreBase
	comp.HerbivoresName = herbivoresName
	comp.HerbivoreRace = tostring(herbivoresName)

	comp._Seed = os.time() 

	-- Herbivore sleep zone
	local zoneSleep1 = r2.newComponent("Region")		
	r2.Utils.createRegion(zoneSleep1, 0, 0, FaunaRegionRadius, FaunaRegionNumCorners)
	zoneSleep1.Deletable = 1
	zoneSleep1.Position.x = comp.Position.x + FaunaRegionOffsets[1].x
	zoneSleep1.Position.y = comp.Position.y + FaunaRegionOffsets[1].y
	zoneSleep1.Position.z = comp.Position.z
	zoneSleep1.InheritPos = 0 
	zoneSleep1.Name = r2:genInstanceName(i18n.get("uiR2EDNameSleepRegion")):toUtf8()			
	table.insert(comp.Components, zoneSleep1)	
	
	-- Carnivore sleep zone
	local zoneSleep2 = r2.newComponent("Region")		
	r2.Utils.createRegion(zoneSleep2, 0, 0, FaunaRegionRadius, FaunaRegionNumCorners)
	zoneSleep2.Deletable = 1
	zoneSleep2.Position.x = comp.Position.x + FaunaRegionOffsets[2].x
	zoneSleep2.Position.y = comp.Position.y + FaunaRegionOffsets[2].y
	zoneSleep2.Position.z = comp.Position.z
	zoneSleep2.InheritPos = 0 
	zoneSleep2.Name = r2:genInstanceName(i18n.get("uiR2EDNameSleepRegion")):toUtf8()			
	table.insert(comp.Components, zoneSleep2)				
	
	--Herbivore sleep zone
	local zoneFood = r2.newComponent("Region")		
	r2.Utils.createRegion(zoneFood, 0, 0, FaunaRegionRadius, FaunaRegionNumCorners)
	zoneFood.Deletable = 1
	zoneFood.Position.x = comp.Position.x + FaunaRegionOffsets[3].x
	zoneFood.Position.y = comp.Position.y + FaunaRegionOffsets[3].y
	zoneFood.Position.z = comp.Position.z
	zoneFood.InheritPos = 0
	zoneFood.Name = r2:genInstanceName(i18n.get("uiR2EDNameFoodRegion")):toUtf8()			
	table.insert(comp.Components, zoneFood)	
	
	-- Herd of herbivores
	local herbivores = feature.Components.Creature.createComponent(zoneSleep1.Position.x, zoneSleep1.Position.y, herbCount, herbivoreBase,
		herbivoresName, zoneSleep1.InstanceId, zoneFood.InstanceId)
	herbivores.Name = i18n.get("uiR2EdHerbivores"):toUtf8()
	--herbivores.Position.x = zoneSleep1.Position.x--comp.Position.x + 10
	--herbivores.Position.y = zoneSleep1.Position.y--comp.Position.y + 10
	herbivores.InheritPos = 0
	--herbivores.Active = comp.Active
	herbivores.ManagerId = comp.InstanceId
	table.insert(comp.Components, herbivores)
	comp._HerbId = herbivores.InstanceId

	-- Pack of carnivores
	local carnivores = feature.Components.Creature.createComponent(zoneSleep2.Position.x, zoneSleep2.Position.y, carnCount, carnivoreBase,
		carnivoresName, zoneSleep2.InstanceId, zoneSleep1.InstanceId)
	carnivores.Name = i18n.get("uiR2EdCarnivores"):toUtf8()
	carnivores.InheritPos = 0
	carnivores.ManagerId = comp.InstanceId
	table.insert(comp.Components, carnivores)
	comp._CarnId = carnivores.InstanceId

	return comp
end

component.getLogicAction = function(entity, context, action)
	assert( action.Class == "ActionStep") 
	local component = r2:getInstanceFromId(action.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)
	
	local herbi = r2:getInstanceFromId(component._HerbId)
	assert(herbi)
	local herbivores = r2:getInstanceFromId(herbi.User._Herd)
	assert(herbivores)
	local rtHerbiGrp = r2.Translator.getRtGroup(context, herbivores.InstanceId)
	assert(rtHerbiGrp)
	
	local carni = r2:getInstanceFromId(component._CarnId)
	assert(carni)
	local carnivores = r2:getInstanceFromId(carni.User._Herd)
	assert(carnivores)
	local rtCarniGrp = r2.Translator.getRtGroup(context, carnivores.InstanceId)
	assert(rtCarniGrp)


	if action.Action.Type == "deactivate" then
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 0)
		local action2 = r2.Translator.getNpcLogicActionDeactivate(herbivores, context, action, rtHerbiGrp)
		local action3 = r2.Translator.getNpcLogicActionDeactivate(carnivores, context, action, rtCarniGrp)
		local action4 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 5)
		local multiaction = r2.Translator.createAction("multi_actions", {action1, action2, action3, action4})
		return multiaction, multiaction
	elseif (action.Action.Type == "activate") then	
		local action1 = r2.Translator.createAction("set_value", rtNpcGrp.Id, "Active", 1)
		local action2 = r2.Translator.getNpcLogicActionActivate(herbivores, context, action, rtHerbiGrp)
		local action3 = r2.Translator.getNpcLogicActionActivate(carnivores, context, action, rtCarniGrp)
		local action4 = r2.Translator.createAction("user_event_trigger", rtNpcGrp.Id, 4)
		local multiaction = r2.Translator.createAction("multi_actions", {action1, action2, action3, action4})
		return multiaction, multiaction
	end

	return r2.Translator.getFeatureActivationLogicAction(rtNpcGrp, action)
end

component.getLogicEvent = function(this, context, event)
	assert( event.Class == "LogicEntityAction") 
	local component = this 
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	return r2.Translator.getFeatureActivationLogicEvent(rtNpcGrp, event)
end

component.getLogicCondition = function(this, context, condition)
	assert( condition.Class == "ConditionStep") 
	local component = r2:getInstanceFromId(condition.Entity)
	assert(component)
	local rtNpcGrp = r2.Translator.getRtGroup(context, component.InstanceId)
	assert(rtNpcGrp)

	return r2.Translator.getFeatureActivationCondition(condition, rtNpcGrp)
end

function component:getLogicTranslations()
	local logicTranslations = {}
	r2.Translator.addActivationToTranslations(logicTranslations)
	return logicTranslations
end

function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdFaunaFeature")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "FaunaFeature")
end

r2.Features["FaunaFeature"] =  feature

