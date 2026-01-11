r2.logicUI = 
{
	MenuName = "ui:interface:r2ed_triggers_menu",
	Menu = nil,
}

function r2.logicUI:openLogicMenu(caller) 
	
	launchContextMenuInGame(self.MenuName)
	if self.Menu == nil or self.Menu.isNil then
		self.Menu = getUI(self.MenuName)
	end

	local menu = self.Menu

	menu:setParentPos(caller)
	menu:setPosRef("BL TL")

	local root = menu:getRootMenu()
	if (caller.y_real - root.h_real < 0) then
		menu.y = root.h_real + caller.h_real
	else
		menu.y = 0
	end
	menu:setMinW(85)
	menu:updateCoords()
end

function r2.logicUI:getEltUIInstId(eltUI)
	return eltUI.Env.InstanceId
end

function r2.logicUI:setEltUIInstId(eltUI, id)
	eltUI.Env.InstanceId = id
end

function r2.logicUI:setSequUIInstId(sequUI, id)
	sequUI.Env.InstanceId = id
end

function r2.logicUI:getSequUIInstId(sequUI)
	return sequUI.Env.InstanceId 
end

------- NEW ELEMENT UI ---------------------------------------------
function r2.logicUI:newElementUI(classUI, newInst, withOrder)

	local templateParams =	classUI.eltTemplateParams

	--not necessary current sequenceUI
	local sequenceUI = classUI:updatedSequUI()
	assert(sequenceUI)

	local listElements = sequenceUI:find("elements_list")
	assert(listElements)

	for i=0, listElements.childrenNb-1 do
		local elt = listElements:getChild(i)
		local instId = self:getEltUIInstId(elt)
		if instId==newInst.InstanceId then return end
	end

	classUI.elementsIdCounter = classUI.elementsIdCounter+1

	local elementId = "elt"..classUI.elementsIdCounter

	-- create new element
	local newElement = createGroupInstance("element_template", listElements.id, 
		{id=elementId, posref="TL TL", x="0", y="0", sizeref="w", hardtext="",
		 select_elt=templateParams.selectElt, open_elt_editor=templateParams.openEltEditor,
		 max_min_elt=templateParams.maxMinElt, remove_elt=templateParams.removeElt, open_chat=templateParams.openChat,
		 col_over=templateParams.colOver, col_pushed=templateParams.colPushed, multi_max_line=templateParams.multiMaxLine})
	assert(newElement)

	-- add element to list
	local eltIndex = -1
	if withOrder then
		local comps = newInst.Parent
		for i=0, comps.Size-1 do
			local comp = comps[i]			
			if comp.InstanceId == newInst.InstanceId then
				eltIndex = i
				break
			end
		end		
		local eltEditor = listElements:find("edit_element")
		assert(eltEditor)
		local indexEltEditor = listElements:getElementIndex(eltEditor)
		if indexEltEditor<= eltIndex then eltIndex=eltIndex+1 end
	else
		eltIndex = listElements.childrenNb
	end

	listElements:addChildAtIndex(newElement, eltIndex)
	listElements.parent:updateCoords()
	
	-- active global minimize / maximize button
	if classUI.maxAndMin then
		
		classUI:maximizeMinimizeElement(newElement)

		if listElements.childrenNb == 2 then
			local maxElts = sequenceUI:find("maximize_elements")
			assert(maxElts)
			maxElts.active = true
		end
	end

	-- scroll goes down to new element
	local scrollBar = sequenceUI:find("scroll_objects")
	assert(scrollBar)
	scrollBar.trackPos = 0

	-- target sroll text
	local scroll = newElement:find("scroll_bar_text")
	assert(scroll)

	local scrollText = newElement:find("scroll_text_gr")
	assert(scrollText)
	scroll:setTarget(scrollText.id)

	self:setEltUIInstId(newElement, newInst.InstanceId)

	-- update next elements title
	if eltIndex<listElements.childrenNb-1 then
		for i=eltIndex+1, listElements.childrenNb-1 do
			local elt = listElements:getChild(i)
			if r2.logicUI:getEltUIInstId(elt) then
				classUI:updateElementTitle(elt)
			end
		end
	end

	if classUI.ownCreatedInstances[newInst.InstanceId] == true 
		or r2.logicComponents.undoRedoInstances[newInst.InstanceId]==true then

		local select = newElement:find("select")
		assert(select)
		select.pushed = true 
		classUI:selectElement(select, self:getSequUIInstId(sequenceUI))
		classUI.ownCreatedInstances[newInst.InstanceId] = nil
		r2.logicComponents.undoRedoInstances[newInst.InstanceId] = nil
	end

	classUI:updateElementUI(newElement)
end

------ REMOVE ELEMENT UI -------------------------------------------
function r2.logicUI:removeElementUI(classUI, removedEltUI)

	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)

	local listElements = sequenceUI:find("elements_list")
	assert(listElements)

	-- update follow elements number
	local removedIndex = listElements:getElementIndex(removedEltUI)
	for i = removedIndex+1, (listElements.childrenNb-1) do
		local element = listElements:getChild(i)
		assert(element)

		-- if this element is not the element editor, update its title
		if self:getEltUIInstId(element) then
			classUI:updateElementTitle(element)
		end
	end
	
	-- delete element and update coordinates of elements list
	if removedEltUI == classUI:currentEltUI() then

		classUI:setCurrentEltUIId(nil)

		-- inactive element editor
		local eltEditor = listElements:find("edit_element")
		assert(eltEditor)
		eltEditor.active = false

		-- disactive up and down element buttons
		local orderGr = sequenceUI:find("order_group")
		assert(orderGr)
		orderGr.active = false
	end

	listElements:delChild(removedEltUI)
	listElements.parent:invalidateCoords()

	-- if any elements in list, disactive global minimize / maximize button
	if listElements.childrenNb == 1 then
		local minElts = sequenceUI:find("minimize_elements")
		assert(minElts)

		local maxElts = sequenceUI:find("maximize_elements")
		assert(maxElts)

		minElts.active = false
		maxElts.active = false
	end
end

------ UPDATE ELEMENT UI -------------------------------------------
function r2.logicUI:updateElementUI(classUI, elementUI)

	local instance = r2:getInstanceFromId(self:getEltUIInstId(elementUI))    
	assert(instance)

	classUI:updateElementTitle(elementUI)

	if instance.InstanceId == classUI:currentEltInstId() then
		classUI:updateElementEditor()
	end
end


------ DOWN/UP ELEMENT UI -------------------------------------------
function r2.logicUI:downUpElementUI(classUI, elementUI, instance)

	local listElements = elementUI.parent
	assert(listElements)

	local eltEditor = listElements:find("edit_element")
	assert(eltEditor)

	local index = listElements:getElementIndex(elementUI)
	local indexEditor = listElements:getElementIndex(eltEditor)

	local indexInstance = -1
	for i=0, instance.Parent.Size-1 do
		if instance.Parent[i]==instance then
			indexInstance = i
			break
		end
	end

	if index>=0 and indexEditor>=0 and indexInstance>=0 then

		local finalIndex = indexInstance

		if indexEditor-1 <= indexInstance then finalIndex=finalIndex+1 end

		local selectedEltUI = classUI:currentEltUI()
		local isSelected = (selectedEltUI==elementUI)

		local loop = math.abs(finalIndex-index)
		if loop>0 then

			-- replace element UI
			for i=1, loop do
				if finalIndex < index then
					listElements:upChild(elementUI)
				else
					listElements:downChild(elementUI)
				end
			end

			-- replace element editor
			if isSelected then 
				for i=1, loop do
					if finalIndex < index then
						listElements:upChild(eltEditor)
					else
						listElements:downChild(eltEditor)
					end
				end
			end

			local firstIndex = math.min(index, finalIndex)
			local lastIndex = math.min(listElements.childrenNb-1, math.max(index, finalIndex)+1)
			for i=firstIndex, lastIndex do
				local eltUI = listElements:getChild(i)
				if eltUI~=eltEditor then
					classUI:updateElementTitle(eltUI)
				end
			end

			if selectedEltUI~=nil then
				selectedEltUI.active=false
				classUI:updateElementEditor()
			end
		end
	end
end

------ MAX/MIN ELEMENT UI -------------------------------------------
function r2.logicUI:maximizeMinimizeElement(element, allMinimize)

	if element == nil then
		element = getUICaller().parent.parent.parent.parent.parent.parent
	end
	assert(element)

	local eltText = element:find("element_text")
	assert(eltText)

	local allMin, allMax = true, true
	if allMinimize ~= nil then
		allMin = allMinimize 
		allMax = not allMin
	end

	local maxButton = element:find("maximize_element")
	assert(maxButton)
	local minButton = element:find("minimize_element")
	assert(minButton)

	-- maximize
	if allMax and eltText.active==false then
		eltText.active = true
		maxButton.active = false
		minButton.active = true

	-- minimize
	elseif allMin and eltText.active==true then
		eltText.active = false
		maxButton.active = true
		minButton.active = false
	end
end

------ MAX/MIN ELEMENTS UI -------------------------------------------
function r2.logicUI:maximizeMinimizeElements(classUI)

	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)

	local elements = sequenceUI:find("elements_list")
	assert(elements)

	elements.Env.Minimize = not elements.Env.Minimize

	for i = 0, elements.childrenNb-1 do
		local element = elements:getChild(i)
		assert(element)

		-- if element is not the element editor
		if self:getEltUIInstId(element) and element~=classUI:currentEltUI() then
			classUI:maximizeMinimizeElement(element, elements.Env.Minimize)
		end
	end

	local minElts = sequenceUI:find("minimize_elements")
	assert(minElts)
	local maxElts = sequenceUI:find("maximize_elements")
	assert(maxElts)

	if elements.Env.Minimize == true then
		minElts.active = false
		maxElts.active = true
	else
		minElts.active = true
		maxElts.active = false
	end
end


------ CLOSE EDITOR ------------------------------------------
function r2.logicUI:closeEditor(classUI)

	-- reset current dialog and current chat
	classUI:setCurrentEltUIId(nil)
end


------- UTILS ------------------------------------------------

function r2.logicUI:findElementUIFromInstance(classUI, instance)

	local sequenceUI = classUI:currentSequUI()
	assert(sequenceUI)
	
	local eltsList = sequenceUI:find("elements_list")
	assert(eltsList)

	for i=0,eltsList.childrenNb-1 do
		local element = eltsList:getChild(i)
		if r2.logicUI:getEltUIInstId(element) == instance.InstanceId then
			return element
		end
	end

	return nil
end






