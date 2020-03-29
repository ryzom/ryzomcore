
-- In Translation file
-- Category : uiR2EdEmptyFeature --
-- CreationFrom : uiR2EdEmptyParameters


r2.Features.EmptyFeature = {}

local feature = r2.Features.EmptyFeature

feature.Name="EmptyFeature"

feature.Description="An empty feature"

feature.Components = {}


--  #########################
--  #   FEATURE FUNCTIONS   #
--  ######################### 

--
-- Registers the feature creation form used as a creation menu in the editor
--
function feature.registerForms()
	r2.Forms.EmptyFeatureForm =
	{
		Caption = "uiR2EdEmptyParameters",
		Prop =
		{
			-- The creation form's fields are defined here
			{Name="Property1", Type="String", Category="UI Subsection"}
		}
	}

end



feature.Components.EmptyFeature =
	{
		BaseClass="LogicEntity",			
		Name="EmptyFeature",
		Menu="ui:interface:r2ed_feature_menu",
		
		DisplayerUI = "R2::CDisplayerLua",
		DisplayerUIParams = "defaultUIDisplayer",
		DisplayerVisual = "R2::CDisplayerVisualEntity",

		Parameters = {},
		
		--
		-- The different actions that can be performed by the feature ("activate", "wander", "spawn" etc.)
		--
		ApplicableActions = {},
		
		--
		-- Events are the feature's states that may trigger actions on the feature. (ex: "on activation",
		-- "on arrive at camp" etc.)
		--
		Events = {},
		
		--
		-- Conditions can be tested on the feature, giving information about its state (ex: "is wandering", "is active" etc.)
		--
		Conditions = {},
		
		--
		-- TextContexts is what the feature might say upon events (2 different kinds: spontaneous & interrogation texts)
		--
		TextContexts =		{},
		
		--
		-- Not quite clear..
		--
		TextParameters =	{},
		
		--
		-- Feature's parameters which can be modified by the GM at runtime.
		--
		LiveParameters =	{},
		
		--
		-- Properties define the attributes of the feature. 
		-- They're all displayed in the properties window of the feature.
		--
		-- -Name: define the name of the property
		--
		-- -Type: define the type of the property and can be "String", "Number", "Table", "RefId" etc.
		--
		-- -Category: Define a subsection of the property window displayed within the editor, so that different properties
		-- refering to the same feature's aspect can be grouped, and not added randomly in the window
		-- 
		-- WidgetStyle: define the type of UI widget that will be used to display the property. If not specified,
		-- the property is displayed in a textBox.
		--
		Prop =
		{
			{Name="InstanceId", Type="String", Category="uiR2EDRollout_UISubsection", WidgetStyle="StaticText"}
		},

		--	
		-- from base class
		--
		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,

		--
		-- from base class
		--			
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
			r2.Classes.LogicEntity.appendInstancesByType(this, destTable, kind)
			for k, component in specPairs(this.Components) do
				component:appendInstancesByType(destTable, kind)
			end
		end,

		--
		-- from base class
		--
		getSelectBarSons = function(this)
			return Components
		end,

		--
		-- from base class		
		--
		canHaveSelectBarSons = function(this)
			return false;
		end,
		
		--
		-- Called when running EditMode to create the feature locally without sending anything into the act (speed purpose).
		--
		onPostCreate = function(this)
		end,

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
		end,

		translate = function(this, context)
			r2.Translator.translateAiGroup(this, context)
		end,

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,
	}

--  ###########################
--  #   COMPONENT FUNCTIONS   #
--  ########################### 

local component = feature.Components.EmptyFeature

--
-- Called when running test mode to create the feature and insert it into the current act. 
--
component.createGhostComponents = function(act, comp)
end

--
-- Creates an instance of the feature with attributes retrieved from the creation form
component.createComponent = function(x, y)
	
	local comp = r2.newComponent("EmptyFeature")
	assert(comp)

	comp.Base = "palette.entities.botobjects.milestone"
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdEmptyFeature")):toUtf8()	
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	

	return comp
end

--
-- Displays the creation form of the feature and calls CreateComponent with the user input values
--
component.create = function()	
	
	if not r2:checkAiQuota() then return end

	local function paramsOk(resultTable)
		r2.requestNewAction(i18n.get("uiR2EDNewEmptyFeatureAction"))
		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )

		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.Components.EmptyFeature.createComponent( x, y)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'EmptyFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'EmptyFeature' at pos (%d, %d, %d)", x, y, z))
		r2:doForm("EmptyFeatureForm", {X=x, Y=y}, paramsOk, paramsCancel)
	end
	local function posCancel()
		debugInfo("Cancel choice 'EmptyFeature' position")
	end	
	r2:choosePos("object_milestone.creature", posOk, posCancel, "createFeatureBanditCamp")
end


--
function feature.registerForms()
	r2.Forms.EmptyFeatureForm =
	{
		Caption = "uiR2EdEmptyParameters",
		Prop =
		{
			-- The creation form's fields are defined here
			{Name="Property1", Type="String", Category="uiR2EDRollout_UISubsection"}
		}
	}
end

--
-- The following functions are specific to the Feature
--
local component = feature.Components.EmptyFeature  

component.getLogicAction = function(entity, context, action)	
	local firstAction, lastAction = nil,nil	
	return firstAction, lastAction
end

component.getLogicCondition = function(this, context, condition)
	return nil,nil
end

component.getLogicEvent = function(this, context, event)		
	local eventHandler, firsCondition, lastCondition = nil, nil, nil
	return eventHandler, firsCondition, lastCondition

end

function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdEmptyFeature")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "EmptyFeature")
end

function component:getLogicTranslations()

	local logicTranslations = {
		["ApplicableActions"] = {},
		["Events"] = {},
		}
	return logicTranslations
end

r2.Features["EmptyFeature"] =  feature

