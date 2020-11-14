-- réed selection bar


r2.SelectBar = 
{
   Content={}, -- array of instance for the current select bar that is displayed
   ObserverHandles = {}, -- for each observed instance, gives its observer handle
   Touched = false,
   LastMenuPopper = nil,       -- last instance in the scenario that popped the mnu
   LastMenuHideTrigger = nil, -- last element that triggered the disparition of the select bar menu   
   InstancesType = "",
   InstancesTypes = {
						{["type"]=i18n.get("uiR2EDScene"):toUtf8(),				["icon"]="r2_palette_entities.tga" },
						{["type"]=i18n.get("uiR2EDMacroComponents"):toUtf8(),	["icon"]="r2_palette_components.tga" },
						{["type"]=i18n.get("uiR2EDbotObjects"):toUtf8(),		["icon"]="r2_palette_objets.tga" },
						{["type"]=i18n.get("uiR2EDPlotItems"):toUtf8(),			["icon"]="ICO_mission_purse.tga" }
					},
}


-- select bar observer : allows to know when a targeted object has been modified, or
-- if its name has changed
local selectBarObserver = {}

function selectBarObserver:onInstanceCreated(instance)
	-- no-op
end

function selectBarObserver:onInstanceErased(instance)
   -- debugInfo(tostring(instance))   
   -- nb : we don't use the 'instance' as a key because it is a pointer
   -- to the C++ "weak pointer object", thus 2 references to the same instance may have a different
   -- value when tested with rawequal ( == operator gives correct results however ...)   
   r2:removeInstanceObserver(r2.SelectBar.ObserverHandles[instance.InstanceId])  
    r2.SelectBar.ObserverHandles[instance.InstanceId] = nil
   -- object is not erased yet (notification is sent before the true erase), so need to clean the table before the update
   local maxElem = r2.SelectBar:getMaxNumEntries()
   for k, v in pairs(r2.SelectBar.Content) do
   if v == instance then
      for index = k, maxElem do
          r2.SelectBar.Content[index] = nil
      end
      break
   end
   end
   if instance ~= r2:getSelectedInstance() then
      -- nb : if instance is the selection, no need to update the bar right now, because setSelecteInstance(nil) is 
		-- called just after 'erase' notification messages have been sent
		r2.SelectBar:touch()
   end
   r2.SelectBar:touch()
end

function selectBarObserver:onPostHrcMove(instance)
	r2.SelectBar:touch()
end

function selectBarObserver:onAttrModified(instance, attrName, attrIndex)
	if attrName == "Name" or attrName == "Title" then
		r2.SelectBar:touch()
	end
end

--------------------------------------------------------
-- retrieve a reference to the select bar
function r2.SelectBar:getBar()
	return getUI("ui:interface:r2ed_select_bar:buttons")
end

--------------------------------------------------------
-- get a button in the bar
function r2.SelectBar:getButton(index)
	return self:getBar():find(string.format("b%d", index))
end

--------------------------------------------------------
-- return the max number of button in the select bar
function r2.SelectBar:getMaxNumEntries()
	return tonumber(getDefine("r2ed_max_num_select_bar_button"))
end

--------------------------------------------------------
-- init the select bar
function r2.SelectBar:init()

	assert(next(self.ObserverHandles) == nil) -- observer table should be empty
	local bar = self:getBar()
	bar:clear()	
	for k = 1, self:getMaxNumEntries() do
		local butt = createGroupInstance("r2ed_select_bar_button", bar.id, 
		                             { 
										id = string.format("b%d", k),
										onclick_l="lua",
										onclick_r="lua",
										params_l=string.format("r2.SelectBar:onButtonPushed(%d)", k),
										params_r=string.format("r2.SelectBar:onRightButtonPushed(%d)", k),
									 })
		butt.active = false
		bar:addChild(butt)
	end	
	bar.active = false	
end

--------------------------------------------------------
-- mark the select bar as modified for update
function r2.SelectBar:touch()	
	self.Touched = true	
	local selection = r2:getSelectedInstance()	
	if selection and selection:isKindOf("Act") then
		self.InstancesType = ""
	end
end
 
---------------------------------------------------------------
-- update bar in special case of act
function r2.SelectBar:openInstancesOfType(instancesType)
	r2.SelectBar:touch()
	self.InstancesType = instancesType
end
--------------------------------------------------------
-- update select bar content if necessary
function r2.SelectBar:update()   

	if not self.Touched then return end	
	if r2.Scenario == nil then
		return -- no scenario created for now
	end
	self.Touched = false

	-- clear the observer table
	for k, v in pairs(self.ObserverHandles) do
		r2:removeInstanceObserver(v)
	end
	table.clear(self.ObserverHandles)
	local selection = r2:getSelectedInstance()	
	if selection then
		self:getBar().active = (selection.Ghost ~= true)
	else
		self:getBar().active = true
	end
	

	
	local function setupButtonWithIconAndText(butt, text, icon, buttIndex)
		butt.active = true      
		butt.b.uc_hardtext = text
		local icon = icon
		if icon ~= "" and icon ~= nil then
			butt.icon.texture = icon
			butt.icon.active = true
         -- butt.icon.color_rgba = CRGBA(255, 255, 255, 255)
			butt.b.text_x = 28
			butt.b.wmargin = 10			      
		else
         -- butt.icon.color_rgba = CRGBA(127, 127, 127, 255)
			butt.icon.active = false
			butt.b.text_x = 12
			butt.b.wmargin = 12
		end
		--butt.b.pushed = (instance == selection)
		butt:invalidateCoords()
	end

	-- setup a button from an instance
	local function setupButton(butt, instance, buttIndex)
		setupButtonWithIconAndText(butt, instance:getDisplayName(), instance:getSelectBarIcon(), buttIndex)
		butt.Env.Types = false
	end

	-- setup a button from a type
	local function setupButtonType(butt, type, buttIndex)
		local icon
		for k, v in pairs(self.InstancesTypes) do
			if v.type == type then
				icon = v.icon
				break
			end
		end
		setupButtonWithIconAndText(butt, ucstring(type), icon, buttIndex)
		butt.Env.Types = true
	end
   
	-- count number of elligible elements for display in the select bar   
	local buttCount = 0
	local currElem = selection
	if selection then
		while currElem do
			if currElem:displayInSelectBar() then
				buttCount = buttCount + 1  
			
				if currElem:isKindOf("Act") and self.InstancesType~="" then 
					buttCount = buttCount + 1         
				end       
			end
			currElem = currElem.ParentInstance
		end   
	end   

	local target = selection
	if buttCount < 1 then
		target = r2:getCurrentAct() -- ensure that at least an act is visible
		buttCount = 1
		if self.InstancesType~="" then 
			buttCount = buttCount + 1         
		end 
	end

	local buttIndex = buttCount 
	-- special case for scenario wide objects (only plot items for now)
	if selection and selection:isGlobalObject() then

		-- current act
		currElem = r2:getCurrentAct()
		setupButton(self:getButton(1), currElem, 1)
		self.Content[1] = currElem
		self.ObserverHandles[currElem.InstanceId] = r2:addInstanceObserver(currElem.InstanceId, selectBarObserver)
				
		-- special button for type
		if self.InstancesType~="" then
			setupButtonType(self:getButton(2), self.InstancesType, 2)
			self.Content[2] = currElem
		end

		-- plot item
		currElem = selection
		setupButton(self:getButton(3), currElem, 3)
		self.Content[3] = currElem
		self.ObserverHandles[currElem.InstanceId] = r2:addInstanceObserver(currElem.InstanceId, selectBarObserver)
		buttIndex = 4

		-- special case if current selection is the scenario (display a single button)
	elseif selection and selection:isSameObjectThan(r2.Scenario) then
		setupButton(self:getButton(1), selection, buttIndex)
		self.Content[1] = selection
		self.ObserverHandles[selection.InstanceId] = r2:addInstanceObserver(selection.InstanceId, selectBarObserver)
		buttIndex = 2
		target = r2:getCurrentAct() -- continue with sons of current act
	else
		-- special case for plot items      
		-- add the buttons for real
		-- parents	
		local currElem = target
		while currElem do
			if currElem:displayInSelectBar() then		
				if buttIndex <= self:getMaxNumEntries() then
					if currElem:isKindOf("Act") then
						if currElem:isBaseAct() then
							currElem = r2:getCurrentAct()
						end

						if self.InstancesType~="" then
							-- special button for different types
							local ucname = ucstring()
							ucname:fromUtf8(self.InstancesType)
							setupButtonType(self:getButton(buttIndex), ucname, buttIndex)
							self.Content[buttIndex] = currElem
							buttIndex = buttIndex - 1
						end     
					end
					setupButton(self:getButton(buttIndex), currElem, buttIndex)
					self.Content[buttIndex] = currElem
					self.ObserverHandles[currElem.InstanceId] = r2:addInstanceObserver(currElem.InstanceId, selectBarObserver)
				end
				buttIndex = buttIndex - 1           
			end
			currElem = currElem.ParentInstance
		end
		buttIndex = buttCount + 1
		-- sons   
		-- preserve previous sons if they where in previous selection,
		-- possibly renaming them
		-- (nb : update may be triggered by modification of sons from a third party, so
		--  the simpler approach 'if curr selection was in previous then leave unchanged'
		--  doesn't work here ...)   
		-- get next son that can be displayed in the select bar
		--local currParent = target
		--while self.Content[buttIndex] ~= nil and not self.Content[buttIndex].isNil do
		--	  currElem = self.Content[buttIndex]      
		--   if currElem:getFirstSelectBarParent() == currParent then
		--      -- there's a match so keep this entry   
		--		 setupButton(self:getButton(buttIndex), currElem)         
		--      currParent = self.Content[buttIndex]         
		--      buttIndex = buttIndex + 1
		--		 self.ObserverHandles[currElem.InstanceId] = r2:addInstanceObserver(currElem.InstanceId, selectBarObserver)
		--   else
		--      -- no match -> exit
		--      break
		--   end
		--end
	end

	-- old version
	-- walk down sons until we reach the max number of buttons
	-- local currSon = target:getFirstSelectBarSon()   
	-- while currSon and buttIndex <= self:getMaxNumEntries() do
	-- setupButton(self:getButton(buttIndex), currSon)
	-- self.Content[buttIndex] = currSon
	-- self.ObserverHandles[currSon.InstanceId] = r2:addInstanceObserver(currSon.InstanceId, selectBarObserver)
	-- currSon = currSon:getFirstSelectBarSon()
	-- buttIndex = buttIndex +1
	-- end


	-- if there's room, add a selection button to traverse down the hierarchy
	if buttIndex <= self:getMaxNumEntries() then
		local butt = self:getButton(buttIndex)
		butt.active = true      
		butt.b.uc_hardtext = i18n.get("uiR2EDSelectSubObject")
		butt.icon.active = false
		butt.b.text_x = 12
		butt.b.wmargin = 12						
		butt:invalidateCoords()
		--butt.b.pushed = false
		self.Content[buttIndex] = nil
		buttIndex = buttIndex + 1
	end

	-- hide remaining buttons
	for k = buttIndex, self:getMaxNumEntries() do
		self:getButton(k).active = false
		self.Content[k] = nil
	end

	-- show/hide sequence browser if necessary
	local sequenceMenuButton = self:getSequenceButton()
	if not selection then
		sequenceMenuButton.active = false
	else
		sequenceMenuButton.active = selection:isSequencable() and (selection.Ghost ~= true)
	end
	-- touch the contextual toolbar, because it y depends on us
	r2.ContextualCommands:getToolbar():invalidateCoords()
end

--------------------------------------------------------
-- called by the ui when one of the select bar button has been pushed
-- function r2.SelectBar:onButtonPushed(index)   
--    local instanceId = self.Content[index].InstanceId
--    local selectedInstance = r2:getSelectedInstance()
--    if selectedInstance and instanceId == selectedInstance.InstanceId then
-- 		-- on second click the contextual menu is displayed
-- 		self:popMenu(index)
-- 	end
--    r2:setSelectedInstanceId(instanceId)
--    self:getButton(index).b.pushed = true
-- end
-- 
-- --------------------------------------------------------
-- -- called by the ui when one of the select bar button has been pushed with the right button
-- function r2.SelectBar:onRightButtonPushed(index)      
--    r2:setSelectedInstanceId(self.Content[index].InstanceId)
--    self:update()
--    self:onButtonPushed(index)
-- end


--------------------------------------------------------
-- mark the select bar as modified for update
function r2.SelectBar:onMenuPostClickOut()   
   local uiCaller = getUICaller()
   if uiCaller and uiCaller.parent and uiCaller.parent.parent then
      local parentGroup =  uiCaller.parent.parent 
      if parentGroup == getUI("ui:interface:r2ed_select_bar:buttons") then
         self.LastMenuHideTrigger = getUICaller()
         return
      end
   end
   self.LastMenuHideTrigger = nil
   self.LastMenuPopper = nil
end


--------------------------------------------------------
-- called by the ui when one of the select bar button has been pushed
function r2.SelectBar:onButtonPushed(index)   
   local butt = self:getButton(index)
   local selectedInstance = self.Content[index]
   if selectedInstance == nil then
		-- pop menu for sons of parents
		self:popMenu(index)
      return
   end
   if butt.b == self.LastMenuHideTrigger and selectedInstance == self.LastMenuPopper then
      self.LastMenuHideTrigger = nil
      return
   end
   self.LastMenuPopper = selectedInstance
   self.LastMenuHideTrigger = nil
   r2:setSelectedInstanceId(selectedInstance.InstanceId)   
   self:touch()   
   -- on second click the contextual menu is displayed
   self:popMenu(index)
end

--------------------------------------------------------
-- called by the ui when one of the select bar button has been pushed with the right button
function r2.SelectBar:onRightButtonPushed(index)         
   self:onButtonPushed(index)
end



--------------------------------------------------------
-- get root menu for the select bar
function r2.SelectBar:getRootMenu()
	local menu = getUI("ui:interface:r2ed_select_bar_menu")
	return menu:getRootMenu()
end

--------------------------------------------------------
-- get menu for the select bar
function r2.SelectBar:getMenu()
	return getUI("ui:interface:r2ed_select_bar_menu")	
end

--------------------------------------------------------
-- get the 'sequences' button
function r2.SelectBar:getSequenceButton()
	return getUI("ui:interface:r2ed_select_bar:sequences")	
end



--------------------------------------------------------
-- display sub-instance menu for the given button
function r2.SelectBar:getMenu() 
   return getUI("ui:interface:r2ed_select_bar_menu")
end

local allSons = {}

--------------------------------------------------------
-- display sub-instance menu for the given button
function r2.SelectBar:popMenu(index)  

	local target
	if self.Content[index] == nil then
		-- the "select" button was pressed
		target = r2:getSelectedInstance()
		if target == nil then
			target = r2:getCurrentAct()
		end
	else
		target = r2:getInstanceFromId(self.Content[index].InstanceId)
		if not target:isSameObjectThan(r2.Scenario) then    
			-- to get objects of same type, just enumerate from the parent (except for scenario)
			target = target:getFirstSelectBarParent()
		end
	end

	--r2:setSelectedInstanceId(target.InstanceId)
	local menu = self:getMenu()
	local rm = self:getRootMenu()
	r2:clearMenu(rm)
	local st = os.clock()
	-- retrieve all sons (optimize this if needed ...)
	table.clear(allSons)

	if target:isKindOf("Scenario") then
		-- special case for scenario : 'appendInstancesByType' would only add the active act, and we want them all
		for k, v in specPairs(target.Acts) do 
			if not v:isBaseAct() then
				table.insert(allSons, v)
			end
		end
	elseif target:isKindOf("Act") and self:getButton(index).Env.Types~=true then

		for k, v in specPairs(r2.Scenario.PlotItems) do
			table.insert(allSons, v)
		end
		r2.Scenario:getBaseAct():appendInstancesByType(allSons, "BaseClass")
		r2.Scenario:getCurrentAct():appendInstancesByType(allSons, "BaseClass")
		local allSonsTemp = {}
		for k, v in pairs(allSons) do
			if v:getSelectBarType()==self.InstancesType then
				table.insert(allSonsTemp, v)
			end
		end
		allSons = allSonsTemp

	elseif target ~= nil then
		target:appendInstancesByType(allSons, "BaseClass")
    end
	
	local et = os.clock()
	--debugInfo("#1 " .. tostring(et - st))
	st = os.clock()
	local sons = {}
	-- retrieve direct selectable sons
	for k,v in pairs(allSons) do
		if v:displayInSelectBar() and (v:getFirstSelectBarParent()==target or (target:isKindOf("Act") and v:getFirstSelectBarParent()==r2.Scenario:getBaseAct())) then
			table.insert(sons, v)
		end
	end   

	et = os.clock()
	--debugInfo("#2 " .. tostring(et - st))
	st = os.clock()	
	-- sort by category, then by icon
	local function sorter(lhs, rhs)
   		if lhs:getClassName() ~= rhs:getClassName() then return lhs:getClassName() < rhs:getClassName() end
   		if lhs:getSelectBarIcon() ~= rhs:getSelectBarIcon() then return lhs:getSelectBarIcon() < rhs:getSelectBarIcon() end
   		return lhs:getDisplayName() < rhs:getDisplayName()
	end
	table.sort(sons, sorter)
	et = os.clock()
	--debugInfo("#3 " .. tostring(et - st))
	st = os.clock()	
	-- fill menu

	-- special case for act
	if index==2 then
		for k, v in pairs(self.InstancesTypes) do
			local ucname = ucstring()
			ucname:fromUtf8(v.type)
			r2:addMenuLine(rm, ucname, "lua", "r2.SelectBar:openInstancesOfType('".. v.type .."','" .. v.icon .."')", v.type, v.icon, 14)
		end
	else
		for k, v in pairs(sons) do
			r2:addMenuLine(rm, v:getDisplayName(), "lua", "r2:setSelectedInstanceId('" .. v.InstanceId .."')", tostring(k), v:getSelectBarIcon(), 14)
		end
	end

	target:completeSelectBarMenu(rm)
	if rm:getNumLine() == 0 then
		rm:addLine(i18n.get("uiR2EDEmpty"), "", "", "empty")
	end
	if self.Content[index] and self.Content[index].InstanceId == r2:getSelectedInstance().InstanceId then
		if r2:getSelectedInstance().BuildPropertySheet then
			rm:addSeparator()
			r2:addMenuLine(rm, concatUCString(i18n.get("uiRE2DPropertiesOf"), r2:getSelectedInstance():getDisplayName()), "lua", "r2:showProperties(r2:getSelectedInstance())", "prop", "r2_icon_properties.tga", 14)
		end
	end
	et = os.clock()
	--debugInfo("#4 " .. tostring(et - st))
	et = os.clock()

	rm:setMaxVisibleLine(15)
	launchContextMenuInGame(menu.id)	
	local butt = self:getButton(index)
	menu.x = butt.x_real
	menu.y = butt.y_real + butt.h_real
	menu:updateCoords()	
	et = os.clock()
	--debugInfo("#5 " .. tostring(et - st))
end

--------------------------------------------------------
-- TMP placeholder: called when the "sequence" menu is clicked
--------------------------------------------------------
-- called when the "sequence" menu is clicked
function r2.SelectBar:browseSequences()

	--r2:updateActivitiesAndChatsUI(r2:getSelectedInstance())
	r2.activities:initEditorAfterFirstCall()

	local logicEntity = r2:getSelectedInstance()
	local activitySequences = logicEntity:getBehavior().Activities

	local menu = self:getMenu()
	local rm = menu:getRootMenu()
	r2:clearMenu(rm)
	for s = 0, activitySequences.Size - 1 do
		local sequence = activitySequences[s]
		local uc_sequ = ucstring()
		uc_sequ:fromUtf8(sequence:getName())
		rm:addLine(uc_sequ, "lua", "r2.activities:triggerSelectSequence('".. sequence.InstanceId .. "')", sequence.InstanceId)
	end
	rm:addSeparator()
	r2:addMenuLine(rm, i18n.get("uiR2EDNewSequence"), "lua", "r2.activities:newSequenceInst()", "new_sequence", "r2_icon_create.tga", 14)	

	local sequenceMenuButton = self:getSequenceButton()
	sequenceMenuButton:updateCoords()
	launchContextMenuInGame(menu.id)	
	menu.x = sequenceMenuButton.x_real
	menu.y = sequenceMenuButton.y_real + sequenceMenuButton.h_real
	menu:updateCoords()
end
