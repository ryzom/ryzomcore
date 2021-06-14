-- Update of the anim bar found in animation mode


r2.ui.AnimBar = {} -- the anim bar class

--------------
--  PUBLIC  --
--------------


----------------------------------------------------------------------------------------------------------
-- Update the anim bar content, except for the contextual actions (those that apply on the current selected entity)
function r2.ui.AnimBar:update()
	local testModeAvailable = false
	local isDm = false
	if r2.Mode == "DM" or r2.Mode == "AnimationModeDm" then isDm = true end
	if r2.Mode == "DM" or r2.Mode == "AnimationModeDm" then -- Not in animation
		testModeAvailable = (r2.AccessMode == "Editor" or r2.AccessMode == "OutlandOwner")
	end
	self:getButton("r2ed_anim_test_mode").active = testModeAvailable
	self:getButton("r2ed_anim_dm_mode").active = (r2.Mode == "Test")		
   local actReceived = (isDm and (r2.AnimGlobals.Acts ~= nil or config.Local == 1))
   debugInfo(tostring(actReceived))
	self:getButton("r2ed_anim_acts").active = actReceived
   local triggerReceived = (isDm and (r2.AnimGlobals.UserTriggers ~= nil or config.Local == 1))
   debugInfo(tostring(triggerReceived))
	self:getButton("r2ed_anim_triggers").active = triggerReceived
	self:getButton("r2ed_anim_weather").active = (isDm)
	self:getButton("r2ed_anim_dm_gift").active = r2:isCurrentSelectionPlayer()
   self:updateButtonsPos()
end


function r2.ui.AnimBar:activateTable(tbl)	
	if (type(tbl) ~= "table") 
	then 
			local button = self:getButton(tbl)
			if button then button.active = true end	
			return	
	end

	local i, j = next(tbl, nil)
	while i do
		if i ~= 'n' then
			self:activateTable(j)
			-- r2.ui.AnimBar:activateTable(j)
		end
		i, j = next(tbl, i)
	end	
end


----------------------------------------------------------------------------------------------------------
-- Update the anim bar from a list of accessibles options (passed as a list of strings)
function r2.ui.AnimBar:updateActions(args)
    self:clearActions()
    getUI("ui:interface:r2ed_testbar:r2ed_anim_dm_gift").active = r2:isCurrentSelectionPlayer()
	self:activateTable(args)
   self:update()
   self:updateButtonsPos()
end


----------------------------------------------------------------------------------------------------------
-- clear all accessible actions in the action bar
function r2.ui.AnimBar:clearActions()
	self:getButton("r2ed_anim_speak_as").active = false
	self:getButton("r2ed_anim_stop_speak").active = false
	self:getButton("r2ed_anim_control").active = false
	self:getButton("r2ed_anim_stop_control").active = false
	self:getButton("r2ed_anim_add_hp").active = false
	self:getButton("r2ed_anim_grp_heal").active = false
	self:getButton("r2ed_anim_kill").active = false
	self:getButton("r2ed_anim_grp_kill").active = false
	self:getButton("r2ed_anim_despawn").active = false
	self:getButton("r2ed_anim_dm_gift").active = false
end

----------------------------------------------------------------------------------------------------------
-- update the window of dm controlled entities
function r2.ui.AnimBar:updateDMControlledEntitiesWindow()
	local wnd = getUI("ui:interface:dm_controlled_entities")	
	local index = 0
	local maxNumPossessedEntities = tonumber(getDefine("r2_max_num_dm_controlled_entities"))
	--
	local talkingList =  r2.getTalkingAsList()
	local numTalker = table.getn(talkingList)
	local incarnatingList =  r2.getIncarnatingList()
	local numIncarnated = table.getn(incarnatingList)
	if numTalker == 0 and numIncarnated == 0 then
		wnd.active = false
		return
	end
	--
	wnd.active = r2.Mode ~= "Edit"
	wnd = wnd.inside
	for i = 0, (numTalker / 2) - 1 do
		local button = wnd["b" .. index]
		button.active = true
		button.b.texture="r2_icon_speak_as_small.tga"
		button.b.texture_pushed="r2_icon_speak_as_small_pushed.tga"
		button.b.texture_over="r2_icon_speak_as_small_over.tga"
		button.b.params_l="r2.ui.AnimBar:stopTalk(" .. talkingList[i * 2 + 1] .. ")"
		button.b.tooltip = i18n.get("uiR2EDStopDMSpeakAs")
		setDbProp("UI:VARIABLES:R2_DM_CONTROLLED:" .. index .. ":TEXT_ID", talkingList[i * 2 + 1])
		index = index + 1
	end	
	for i = 0, (numIncarnated / 2) - 1 do
		local button = wnd["b" .. index]
		button.active = true
		button.b.texture="r2_icon_possess_small.tga"
		button.b.texture_pushed="r2_icon_possess_small_pushed.tga"
		button.b.texture_over="r2_icon_possess_small_over.tga"		
		button.b.tooltip = i18n.get("uiR2EDStopDMControl")
		button.b.params_l="r2.ui.AnimBar:stopIncarnate(" .. incarnatingList[i * 2 + 1] .. ")"
		setDbProp("UI:VARIABLES:R2_DM_CONTROLLED:" .. index .. ":TEXT_ID", incarnatingList[i * 2 + 1])
		index = index + 1
	end
	while index < maxNumPossessedEntities do
		wnd["b" .. index].active = false
		index = index + 1
	end
end

----------------------------------------------------------------------------------------------------------
-- stop talk on an entity
function r2.ui.AnimBar:stopTalk(entityId)
	r2:dssTarget('STOP_TALK ' .. entityId)	
end

----------------------------------------------------------------------------------------------------------
-- stop incarnate on an entity
function r2.ui.AnimBar:stopIncarnate(entityId)
	r2:dssTarget('STOP_CONTROL ' .. entityId)	
end

---------------
--  PRIVATE  --
---------------

----------------------------------------------------------------------------------------------------------
-- get reference on a button in the select bar by its name
function r2.ui.AnimBar:getButton(name)	
	assert(name)
	return getUI("ui:interface:r2ed_testbar:" .. name)
end

------------------------------------------------------------------------------------
-- Update pos of all buttons, so that x of first visible button will be 0
function r2.ui.AnimBar:updateButtonsPos()
   local currX = 0
   local firstButton = true
   local function updateX(name, gap)	  
	  local but = self:getButton(name)
	  if but.active then		
		if not firstButton then
			currX = currX + gap
		else
			firstButton = false
		end
		but.x = currX
		currX = currX + 44
	  else
		but.x = 0
	  end	  
   end
   
   updateX("r2ed_anim_test_mode", 0)
   updateX("r2ed_anim_dm_mode", 4)     
   updateX("r2ed_anim_acts", 4)
   updateX("r2ed_anim_triggers", 4)
   updateX("r2ed_anim_speak_as", 12)
   updateX("r2ed_anim_stop_speak", 4)
   updateX("r2ed_anim_control", 4)
   updateX("r2ed_anim_stop_control", 4)
   updateX("r2ed_anim_add_hp", 4)
   updateX("r2ed_anim_grp_heal", 4)
   updateX("r2ed_anim_kill", 4)
   updateX("r2ed_anim_grp_kill", 4)
   updateX("r2ed_anim_despawn", 4)
   updateX("r2ed_anim_weather", 12)
   updateX("r2ed_anim_dm_gift", 12)
end


