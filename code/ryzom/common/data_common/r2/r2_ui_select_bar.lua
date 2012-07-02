-- réed selection bar


r2.SelectBar = 
{
   Content={}, -- array of instance for the current select bar that is displayed
   ObserverHandles = {}, -- for each observed instance, gives its observer handle
   Touched = false
}


-- select bar observer : allows to know when a targeted object has been modified, or
-- if its name has changed
local selectBarObserver = {}

function selectBarObserver:onInstanceCreated(instance)
	-- no-op
end

function selectBarObserver:onInstanceErased(instance)
   debugInfo(tostring(instance))   
   -- nb : we don't use the 'instance' as a key because it is a pointer
   -- to the C++ "weak pointer object", thus 2 references to the same instance may have a different
   -- value when tested with rawequal ( == operator gives correct results however ...)   
	r2:removeInstanceObserver(r2.SelectBar.ObserverHandles[instance.InstanceId])  
	r2.SelectBar.ObserverHandles[instance.InstanceId] = nil
   -- object is not erased yet, so need to clean the table before the update
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
		debugInfo(tostring(k))
		local butt = createGroupInstance("r2ed_select_bar_button", bar.id, 
		                             { 
										id = string.format("b%d", k),
										onclick_l="lua",
										params_l=string.format("r2.SelectBar:onButtonPushed(%d)", k),
                              onclick_r="lua",
										params_r=string.format("r2.SelectBar:onRightButtonPushed(%d)", k)
									 })
		butt.active = false
		bar:addChild(butt)
	end	
end

--------------------------------------------------------
-- mark the select bar as modified for update
function r2.SelectBar:touch()
	self.Touched = true
end
 
--------------------------------------------------------
-- update select bar content if necessary
function r2.SelectBar:update()   
	if not self.Touched then return end
	self.Touched = false
	-- clear the observer table
	for k, v in pairs(self.ObserverHandles) do
		r2:removeInstanceObserver(v)
	end
	table.clear(self.ObserverHandles)
	local selection = r2:getSelectedInstance()	
	self:getBar().active = true		
	-- count number of elligible elements for display in the select bar   
	local buttCount = 0
	local currElem = selection

	-- setup a button from an instance
	local function setupButton(butt, instance)
		butt.active = true
		butt.b.pushed = (instance == selection)		
		butt.b.uc_hardtext = instance:getDisplayName()
		local icon = instance:getSelectBarIcon()
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
      butt:invalidateCoords()
	end


   if selection then
      while currElem do
         if currElem:displayInSelectBar() then
            buttCount = buttCount + 1         
         end
         currElem = currElem.ParentInstance
      end   
   end
   buttCount =math.min(buttCount, self:getMaxNumEntries())
   local target = selection
   if buttCount < 2 then
      target = r2:getCurrentAct() -- ensure that at least 'scenario' and 'act' are visible
      buttCount = 2
   end
	-- add the buttons for real
    -- parents
	local buttIndex = buttCount 
	local currElem = target
	while currElem do
		if currElem:displayInSelectBar() then
			local butt = 
			setupButton(self:getButton(buttIndex), currElem)
			self.Content[buttIndex] = currElem
			self.ObserverHandles[currElem.InstanceId] = r2:addInstanceObserver(currElem.InstanceId, selectBarObserver)
			buttIndex = buttIndex - 1           
		end
		currElem = currElem.ParentInstance
	end
   -- sons   
   -- preserve previous sons if they where in previous selection,
   -- possibly renaming them
   -- (nb : update may be triggered by modification of sons from a third party, so
   --  the simpler approach 'if curr selection was in previous then leave unchanged'
   --  doesn't work here ...)
   buttIndex = buttCount + 1
   -- get next son that can be displayed in the select bar
   local currParent = target
   while self.Content[buttIndex] ~= nil and not self.Content[buttIndex].isNil do
	  currElem = self.Content[buttIndex]      
      if currElem:getFirstSelectBarParent() == currParent then
         -- there's a match so keep this entry   
		 setupButton(self:getButton(buttIndex), currElem)         
         currParent = self.Content[buttIndex]         
         buttIndex = buttIndex + 1
		 self.ObserverHandles[currElem.InstanceId] = r2:addInstanceObserver(currElem.InstanceId, selectBarObserver)
      else
         -- no match -> exit
         break
      end
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
end

--------------------------------------------------------
-- called by the ui when one of the select bar button has been pushed
function r2.SelectBar:onButtonPushed(index)   
   local instanceId = self.Content[index].InstanceId
   local selectedInstance = r2:getSelectedInstance()
   if selectedInstance and instanceId == selectedInstance.InstanceId then
		-- on second click the contextual menu is displayed
		self:popMenu(index)
	end
   r2:setSelectedInstanceId(instanceId)
   self:getButton(index).b.pushed = true
end

--------------------------------------------------------
-- called by the ui when one of the select bar button has been pushed with the right button
function r2.SelectBar:onRightButtonPushed(index)      
   r2:setSelectedInstanceId(self.Content[index].InstanceId)
   self:update()
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
function r2.SelectBar:popMenu(index)   
	local target = r2:getInstanceFromId(self.Content[index].InstanceId)
	--r2:setSelectedInstanceId(target.InstanceId)
	local menu = getUI("ui:interface:r2ed_select_bar_menu")
	local rm = self:getRootMenu()
	r2:clearMenu(rm)
	local st = os.clock()
	-- retrieve all sons (to optimize if needed ...)
	local allSons = {}
	if target:isKindOf("Scenario") then
	  -- special case for scenario : 'appendInstancesByType' would only add the active act, and we want them all
	   for k, v in specPairs(target.Acts) do 
	     table.insert(allSons, v)
	  end
	else
      target:appendInstancesByType(allSons, "BaseClass")
   end
   local et = os.clock()
   debugInfo("#1 " .. tostring(et - st))
   st = os.clock()
	local sons = {}
	-- retrieve direct selectable sons
	for k,v in pairs(allSons) do
		if v:displayInSelectBar() and v:getFirstSelectBarParent() == target then
			table.insert(sons, v)
		end
	end
   et = os.clock()
   debugInfo("#2 " .. tostring(et - st))
   st = os.clock()	
   -- sort by category, then by icon
   local function sorter(lhs, rhs)
   		if lhs:getClassName() ~= rhs:getClassName() then return lhs:getClassName() < rhs:getClassName() end
   		if lhs:getSelectBarIcon() ~= rhs:getSelectBarIcon() then return lhs:getSelectBarIcon() < rhs:getSelectBarIcon() end
   		return lhs:getDisplayName() < rhs:getDisplayName()
   end
   table.sort(sons, sorter)
   et = os.clock()
   debugInfo("#3 " .. tostring(et - st))
   st = os.clock()	
	-- fill menu
	local currentLine = 0
	for k, v in pairs(sons) do
		r2:addMenuLine(rm, v:getDisplayName(), "lua", "r2:setSelectedInstanceId('" .. v.InstanceId .."')", tostring(k), v:getSelectBarIcon(), 14)
		--rm:addLine(v:getDisplayName(), "lua", "r2:setSelectedInstanceId('" .. v.InstanceId .."')", tostring(k))
		--if v:getSelectBarIcon() ~= "" then
		--	local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = v:getSelectBarIcon(), size="14" })
		--	if menuButton then
		--		rm:setUserGroupLeft(currentLine, menuButton)
		--	end
		-- end
		currentLine = currentLine + 1
	end
	target:completeSelectBarMenu(rm)
   if rm:getNumLine() == 0 then
      rm:addLine(i18n.get("uiR2EDEmpty"), "", "", "empty")
   end
   et = os.clock()
   debugInfo("#4 " .. tostring(et - st))
   et = os.clock()
	launchContextMenuInGame(menu.id)	
	local butt = self:getButton(index)
	menu.x = butt.x_real
	menu.y = butt.y_real + butt.h_real
	menu:updateCoords()	
   et = os.clock()
   debugInfo("#5 " .. tostring(et - st))
end

--------------------------------------------------------
-- TMP placeholder: called when the "sequence" menu is clicked
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
		rm:addLine(ucstring(r2:getSequenceName(sequence)), "lua", "r2:selectActivitiesSequence(".. tostring(s) .. ")", "s")
	end
	rm:addSeparator()
	r2:addMenuLine(rm, i18n.get("uiR2EDNewSequence"), "lua", "r2:newMiniActivitySequence()", "new_sequence", "r2_icon_create.tga", 14)

	local sequenceMenuButton = self:getSequenceButton()
	sequenceMenuButton:updateCoords()
	launchContextMenuInGame(menu.id)	
	menu.x = sequenceMenuButton.x_real
	menu.y = sequenceMenuButton.y_real + sequenceMenuButton.h_real
	menu:updateCoords()
end
