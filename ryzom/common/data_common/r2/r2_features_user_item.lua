-- In Translation file
-- Category : uiR2EdUserItemFeature --
-- CreationFrom : uiR2EdUserItemParameters


r2.Features.UserItemFeature = {}

local feature = r2.Features.UserItemFeature

feature.Name="UserItemFeature"

feature.Description="A UserItem feature"



--  #########################
--  #   FEATURE FUNCTIONS   #
--  ######################### 

function feature.registerForms()
	r2.Forms.UserItemFeatureForm =
	{
		Caption = "uiR2EdUserItemParameters",
		Prop =
		{
			-- The creation form's fields are defined here
			{Name="Property1", Type="String", Category="uiR2EDRollout_UISubsection"}
		}
	}

end




feature.Components = {}

feature.Components.UserItemFeature =
	{
		BaseClass="LogicEntity",			
		Name="UserItemFeature",
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
		
		Prop =
		{
			{Name="InstanceId", Type="String", Category="uiR2EDRollout_UISubsection", WidgetStyle="StaticText"}
		},

		getAvailableCommands = function(this, dest)	
			r2.Classes.LogicEntity.getAvailableCommands(this, dest) -- fill by ancestor
			this:getAvailableDisplayModeCommands(dest)
		end,

		getParentTreeNode = function(this)
			return this:getFeatureParentTreeNode()
		end,
					
		appendInstancesByType = function(this, destTable, kind)
			assert(type(kind) == "string")
			--this:delegate():appendInstancesByType(destTable, kind)
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
			r2.Translator.translateAiGroup(this, context)
		end,

		pretranslate = function(this, context)
			r2.Translator.createAiGroup(this, context)
		end
	}




--  ###########################
--  #   COMPONENT FUNCTIONS   #
--  ########################### 
--
-- The following functions are specific to the Feature
--
local component = feature.Components.UserItemFeature 

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

component.createGhostComponents = function(act, comp)

end

component.createComponent = function(x, y)
	
	local comp = r2.newComponent("UserItemFeature")
	assert(comp)

	comp.Base = "palette.entities.botobjects.milestone"
	comp.Name = r2:genInstanceName(i18n.get("uiR2EdUserItemFeature")):toUtf8()			
	
	comp.Position.x = x
	comp.Position.y = y
	comp.Position.z = r2:snapZToGround(x, y)
	
	--comp._Seed = os.time() 

	return comp
end

component.create = function()	

	if not r2:checkAiQuota() then return end


	local function paramsOk(resultTable)

		r2.requestNewAction(i18n.get("uiR2EDNewUserItemFeatureAction"))

		local x = tonumber(  resultTable["X"] )
		local y = tonumber( resultTable["Y"] )

		if not x or not y 
		then
			debugInfo("Can't create Component")
			return
		end
		local component = feature.createComponent( x, y)
		r2.requestInsertNode(r2:getCurrentAct().InstanceId, "Features", -1, "", component)
	end
	
	local function paramsCancel()
		debugInfo("Cancel form for 'UserItemFeature' creation")
	end
	local function posOk(x, y, z)
		debugInfo(string.format("Validate creation of 'UserItemFeature' at pos (%d, %d, %d)", x, y, z))
		r2:doForm("UserItemFeatureForm", {X=x, Y=y}, paramsOk, paramsCancel)
	end
	local function posCancel()
		debugInfo("Cancel choice 'UserItemFeature' position")
	end	
	r2:choosePos("object_milestone.creature", posOk, posCancel, "createFeatureBanditCamp")
end

-----------------------------------------
--- register the current Feature to menu


function component:registerMenu(logicEntityMenu)
	local name = i18n.get("uiR2EdUserItemFeature")
	logicEntityMenu:addLine(ucstring(name), "lua", "", "UserItemFeature")
end

function component:getLogicEntityAttributes()
	-- register trad
	local localLogicEntityAttributes = {
		["ApplicableActions"] = {},
		["Events"] = {},
		["Conditions"] = {}
	}
	return localLogicEntityAttributes
end

r2.Features["UserItemFeature"] =  feature

